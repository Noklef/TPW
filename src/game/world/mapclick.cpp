// The game mode's map click: see mapclick.h.
#include "mapclick.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/globals.h"
#include "../core/soundplay.h"
#include "../core/thunks.h"
#include "../core/trivial.h"
#include "../math/x87.h"
#include "../ui/cursor.h"
#include "advisor.h"
#include "cellrun.h"
#include "dirstep.h"
#include "focuscell.h"
#include "hoardinggrid.h"
#include "mapcell.h"
#include "mapline.h"
#include "money.h"
#include "objectmsg.h"
#include "staff.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *base, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)base + offset); }
template <typename F> F raw(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const MapClickEnv kGameEnv = {
    raw<void(__cdecl *)(s32, s32, u32)>(0x005234d0),
    raw<void(__cdecl *)()>(0x00446050),
    raw<void(__cdecl *)(u32)>(0x00446300),
    raw<void(__cdecl *)(u32)>(0x00435570),
    raw<void(__cdecl *)(s32, s32, u32, u8 *)>(0x004419b0),
    raw<u32(__cdecl *)(s32, s32)>(0x00445a70),
    raw<void(__cdecl *)(s32, s32, u32)>(0x00527ee0),
    raw<u32(__cdecl *)(s32, s32, u32, u32, u32, u32)>(0x00528a70),
    raw<u32(__cdecl *)(u32)>(0x0052a970),
    raw<u32(__cdecl *)(u8 *, u8 *, u32)>(0x0052b400),
    raw<void(__cdecl *)(s32, s32, s32, s32)>(0x0052bdd0),
    raw<u8(__cdecl *)(u8 *, u32, u32, u32)>(0x0052c6f0),
    raw<void(__cdecl *)(u8 *, u32, u32, u32)>(0x0052ccb0),
    raw<void(__cdecl *)(u32, u32)>(0x0052f200),
    raw<void(__cdecl *)(u32, u32)>(0x0052f580),
    raw<void(__cdecl *)(u32)>(0x0052fc80),
    raw<void(__cdecl *)(u8 *)>(0x00530120),
    raw<void(__cdecl *)(s32, s32, s32, s32)>(0x005327e0),
    raw<u32(__cdecl *)(u32, s32, s32, s32 *, s32 *)>(0x00536100),
    raw<void(__cdecl *)(u32)>(0x00545450),
    &Sound_PlayAt,
    &Map_ApplyLine,
    &MapCell_PathLines,
    &MapCell_PathNudge,
    &FocusCell_Update,
    &Cell_ExtendPath,
    &Map_ApplyEntranceLines,
    &Position_StepA,
    &Position_StepB,
    &TrackRun_AnimateA,
    &TrackRun_AnimateB,
    &TrackRun_TraceA,
    &CellRun_Corners,
    &HoardingGrid_CheckArea,
    &HoardingGrid_Fill,
    &GameCursor_Set,
    &Global81d7a4_SetRecorded,
    &Advisor_PostId,
    raw<void(__fastcall *)(u8 *)>(0x004de1f0),
    raw<void(__fastcall *)(u8 *)>(0x004df390),
    &Object_CanOpen,
    &MapHandle_Cell44,
    &Cell_FirstObject,
    &Staff_SetPatrolRegion,
    &Map_StepCell28InDirection,
    &MapCell_AddNeighbours,
    &MapCell_SetDirection,
    &MapCell_IsTypeBDOr10Or12Or19,
    &MapCell_IsTypeCOr11,
    &MapCell_IsTypeDOr12ViaParent,
    &MapCell_Word24,
    raw<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>(0x00539760),
    raw<u32(__fastcall *)(u8 *, void *, u32, u32)>(0x00535670),
    &ActionRec_WriteBB,
    &ActionRec_WriteBBW,
    &GameTable7cfb90_Get1da726_5195d0,
    &Money_Debit,
};
const MapClickEnv *g_env = &kGameEnv;

u8 *World() { return global<u8 *>(0x007cf83c); }
u8 *CurThing() { return global<u8 *>(0x008186ac); }
s32 &StartX() { return global<s32>(0x0081ede4); }
s32 &StartY() { return global<s32>(0x0081ede8); }

// The 0x28-byte map cell of a 16-bit cell id: [0x7cf83c] + (id + 0x6cde) * 0x28.
u8 *CellOf(u32 id) { return World() + ((id & 0xffff) + 0x6cde) * 0x28; }
u8 *CellAt(s32 x, s32 y) { return CellOf(((u32)y << 7) + (u32)x + 1); }
// A cell's parent (its u16 +0x10).
u8 *Parent(const u8 *cell) {
    const u32 id = at<u16>(cell, 0x10);
    return World() + (id + 0x6cde) * 0x28;
}
// The 0x44-byte record of a cell id: its dword +0x29c.
u32 Record29c(u32 id) { return at<u32>(World(), (id & 0xffff) * 0x44 + 0x29c); }

// |v| as cdq; xor; sub has it (the most negative int is itself).
s32 Abs32(s32 v) {
    const u32 s = (u32)(v >> 31);
    return (s32)(((u32)v ^ s) - s);
}

// fild v; fmul [0x700c9c]; __ftol (the FPU at 24 bits).
s32 Scaled(s32 v) {
    const float k = global<float>(0x00700c9c);
    const double r = v < 0 ? -X87_MulInt(0u - (u32)v, k) : X87_MulInt((u32)v, k);
    return (s32)(long long)r;
}

void PlaySound(const MapClickEnv *E, u32 sound, s32 x, s32 y) {
    const s32 sy = Scaled(y);
    const s32 sx = Scaled(x);
    E->soundPlayAt(0, global<u32>(0x00803a2c), sound, sx, 0, sy);
}

// The focus cell from the thing [0x8186ac]: [0x818698] its cell id (row +7, column +5); 0x7639a4 / 0x7639a8 its column and row, and
// 0x7639e0 / 0x7639e4 and 0x7639e8 / 0x7639ec as well, 0x7639ac its facing (+0x10).
void FocusFromCurThing() {
    const u8 *const thing = CurThing();
    global<u16>(0x00818698) = (u16)(((u32)at<u8>(thing, 7) << 7) + at<u8>(thing, 5) + 1);
    const u32 f = (global<u32>(0x00818698) & 0xffff) - 1;
    const u32 fx = f & 0x7f, fy = f >> 7;
    global<u32>(0x007639a4) = fx;
    global<u32>(0x007639e0) = fx;
    global<u32>(0x007639a8) = fy;
    global<u32>(0x007639e4) = fy;
    const u32 facing = at<u32>(thing, 0x10);
    global<u32>(0x007639e8) = fx;
    global<u32>(0x007639ac) = facing;
    global<u32>(0x007639ec) = fy;
}

void StepFocus(const MapClickEnv *E, bool keep) {
    const u32 a = E->stepA(&global<s32>(0x007639e0), &global<s32>(0x007639e4), global<s32>(0x007639ac));
    if (keep) global<u8>(0x00818c50) = (u8)a;
    E->stepB(&global<s32>(0x007639e8), &global<s32>(0x007639ec), global<s32>(0x007639ac));
}

// The run's start set from the thing (its +0x2c = 0x52a970 (a)), animated from start (b) when set, else from 0 both ways; with a run, an
// object that can open is told (0x4df390). The run returned.
u32 SetRun(const MapClickEnv *E, u32 a, u32 b) {
    const u32 run = E->f52a970(a);
    at<u32>(CurThing(), 0x2c) = run;
    if (run != 0) {
        E->animateA(CurThing(), b);
    } else {
        E->animateA(CurThing(), 0);
        E->animateB(CurThing(), 0);
    }
    return run;
}

void OpenIfRun(const MapClickEnv *E, u32 run) {
    if (run != 0 && E->canOpen(CurThing()) != 0) E->f4df390(CurThing());
}

// A line from (x, y) to itself, each end as the line leaves it.
void Line(const MapClickEnv *E, u32 code, s32 &x, s32 &y) { E->applyLine(code, x, y, &x, &y); }
u32 Line6(const MapClickEnv *E, u32 code, s32 &x, s32 &y) { return E->f536100(code, x, y, &x, &y); }
// A line from the drag's start to (x, y).
u32 FromStart(const MapClickEnv *E, u32 code, s32 &x, s32 &y) { return E->applyLine(code, StartX(), StartY(), &x, &y); }
u32 FromStart6(const MapClickEnv *E, u32 code, s32 &x, s32 &y) { return E->f536100(code, StartX(), StartY(), &x, &y); }

// The direction bit from a facing (0: 4, 0x5a: 1, 0xb4: 0x40, 0x10e: 0x10, else 0).
u8 FacingBit(s32 facing) {
    if (facing > 0x5a) return facing == 0xb4 ? 0x40 : facing == 0x10e ? 0x10 : 0;
    if (facing == 0x5a) return 1;
    return facing == 0 ? 4 : 0;
}

// The direction bit from the focus's two steps: 0x40 / 4 as the first column is less / more than the second, else 1 / 0x10 by the rows.
u8 StepBit() {
    const s32 dx = (s32)(global<u32>(0x007639e0) - global<u32>(0x007639e8));
    if (dx < 0) return 0x40;
    if (dx > 0) return 4;
    const s32 dy = (s32)(global<u32>(0x007639e4) - global<u32>(0x007639ec));
    if (dy < 0) return 1;
    if (dy > 0) return 0x10;
    return 0;
}

// The end pulled onto the start's row or column, the nearer one ((sx, sy) the start).
void Snap(s32 sx, s32 sy, s32 &x, s32 &y) {
    const s32 d = Abs32((s32)((u32)x - (u32)sx));
    if (d >= Abs32((s32)((u32)y - (u32)sy))) y = sy;
    if (d <= Abs32((s32)((u32)y - (u32)sy))) x = sx;
}

// v - origin rounded toward zero to even, back from origin.
s32 Even(s32 v, s32 origin) {
    s32 d = (s32)((u32)v - (u32)origin);
    if (d < 0 && d % 2 != 0) --d;
    return (s32)((u32)origin + (u32)(d / 2 * 2));
}

// Modes 4 and 0x3b: a path piece (4) or a queue piece (0x3b) laid from the thing's entrance (0x528a70's point) to the click.
void EntrancePiece(const MapClickEnv *E, s32 x, s32 y, bool queue, u32 &endFlag) {
    const u32 point = E->f528a70(x, y, 4, Global81d7a4_Get_52f1f0(), 1, 0);
    endFlag = 0;
    if (Global81b0cc_IfNoFlag() == 0) {
        endFlag = 2;   // (0x52f580 (0, 0) unless the flags' low word is 0xc)
        return;
    }
    if (Global81ecac_IfNoFlag() != 0) {
        global<u8 *>(0x008186c0) = E->extendPath(reinterpret_cast<const s32 *>((uintptr_t)point), &global<s32>(0x007639b8), &global<s32>(0x007639bc));
        if (Global81ecac_IfNoFlag() != 3) {
            const u32 kind = Global81ecac_IfNoFlag();
            u8 *const entrance = E->entranceLines(reinterpret_cast<const u8 *>((uintptr_t)point), &StartX(), &StartY(), kind);
            global<u8 *>(0x008186c4) = entrance;
            const u32 id = (u32)at<u16>(entrance, 0) - 1;
            s32 lx = (s32)(id & 0x7f), ly = (s32)(id >> 7);
            Fn_52fbd0();
            switch (Global81ecac_IfNoFlag()) {
            case 1: E->f52f580(0xb, 0); break;
            case 2: E->f52f580(0x10, 0); break;
            }
            Line(E, 0x87, lx, ly);
            Line(E, Global81ae2c_Get_52f840(), lx, ly);
            Line(E, 0x80, lx, ly);
            if (queue) {
                Line(E, 0x82, lx, ly);
            } else {
                u8 *const cell = CellAt(lx, ly);
                E->setDirection(cell, nullptr, FacingBit(at<s32>(CurThing(), 0x10)));
            }
            Line(E, 0x83, lx, ly);
            Line(E, 0x85, lx, ly);
            Line(E, 0x86, lx, ly);
            Line(E, 0x81, lx, ly);
            PairTable_Add((u32)lx, (u32)ly);
            const u32 f = (global<u32>(0x00818698) & 0xffff) - 1;
            global<u32>(0x007639e0) = f & 0x7f;
            global<u32>(0x007639e8) = f & 0x7f;
            global<u32>(0x007639e4) = f >> 7;
            global<u32>(0x007639ec) = f >> 7;
            StepFocus(E, false);
            lx = global<s32>(0x007639e8);
            ly = global<s32>(0x007639ec);
            const u8 bit = StepBit();
            Line(E, 0x87, lx, ly);
            Line(E, Global81ae2c_Get_52f840(), lx, ly);
            Line(E, 0x80, lx, ly);
            u8 *const cell = CellAt(lx, ly);
            E->addNeighbours(cell, nullptr, bit);
            E->setDirection(cell, nullptr, bit);
            u8 *const next = E->stepCell(reinterpret_cast<const u16 *>(cell), nullptr, bit, 2);
            if (next != nullptr) E->addNeighbours(next, nullptr, Nibble_Scale(bit));
            if (queue) Line(E, 0x82, lx, ly);
            Line(E, 0x83, lx, ly);
            Line(E, 0x85, lx, ly);
            Line(E, 0x86, lx, ly);
            Line(E, 0x81, lx, ly);
        }
        if (CurThing() != nullptr && global<u32>(0x008187f8) == 0) {
            at<u32>(CurThing(), 0x2c) = 0;
            FocusFromCurThing();
            if (Global81ecac_IfNoFlag() != 3) StepFocus(E, true);
        }
    }
    u8 *const end = E->extendPath(reinterpret_cast<const s32 *>((uintptr_t)point), &StartX(), &StartY());
    global<u8 *>(0x008186c0) = end;
    const u32 id = at<u16>(end, 0);
    s32 lx = (s32)((id - 1) & 0x7f), ly = (s32)((id - 1) >> 7);
    Fn_52fbd0();
    E->f52f580(3, 0);
    if (StartX() == x && StartY() == y && Global820a8c_Get_52fbe0() != 1) return;
    PairTable_Add((u32)lx, (u32)ly);
    if (!queue && global<u32>(0x008186c0) != 0) global<u32>(0x008186c0) = 0;
    Line6(E, 0x85, lx, ly);
    Line6(E, 0x86, lx, ly);
    Line6(E, 0x83, lx, ly);
    Line6(E, 0x81, lx, ly);
    E->f4de1f0(CurThing());
    global<u32>(0x00763ac0) = 0x80;
    global<u32>(0x00763ac4) = 0x80;
    if (Record29c(id) == 1) {
        E->f52f200(0, 0);
        endFlag = 1;   // (the flags' test only)
    }
}

}  // namespace

void MapClick_SetEnv(const MapClickEnv *e) { g_env = e ? e : &kGameEnv; }

void __cdecl GameMode_MapClick(s32 x, s32 y, u32 flags, u32 useStored) {
    const MapClickEnv *const E = g_env;
    ActionRec *const record = reinterpret_cast<ActionRec *>((uintptr_t)0x00785a08);
    // (x and y both 0: Debug_Printf "HUH?", left out)
    if (global<u32>(0x00785a2c) != 0 && global<u8>(0x0081b4cc) == 0) {
        global<u32>(0x00816d50) = 0;
        if (global<u32>(0x008187f8) == 0) E->click5234d0(x, y, flags);
    }
    global<u8>(0x008186d4) = 0;
    if (useStored != 0) {
        x = global<s32>(0x008186b8);
        y = global<s32>(0x008186b4);
    }
    if ((global<u32>(0x00816d48) == 0 && global<u8>(0x0081b4cc) == 0) || Global81ae2c_Equals(0x16) != 0) {
        if ((u16)flags == 0)
            E->writeBB(record, nullptr, 0x14, (u32)x, (u32)y);
        else
            E->writeBBW(record, nullptr, 0x2d, (u32)x, (u32)y, flags);
    }
    u32 lineEnded = 0;   // (ESI: the last line answered 0)
    if (Global81ae2c_Equals(0) != 0) global<u32>(0x008186b0) = Wrap_Global7c2650_Get_481590();
    if (Global81ae2c_Equals(0x38) == 0 && global<u32>(0x00816d48) != 0) {
        if (global<u32>(0x00785a2c) == 0 && global<u8>(0x008186d8) != 0) PlaySound(E, 0xaf, x, y);
        switch (Global81ae2c_Get_52f840()) {
        case 1: case 3: case 0xb: case 0xe: case 0x10: case 0x13: case 0x36:
            goto clear_end;
        case 0x16:
            E->f446050();
            global<u32>(0x00816d48) = 0;
            global<u8>(0x00818c54) = 0;
            global<u8>(0x008186d8) = 0;
            return;
        default:
            goto end;
        }
    }
    E->focusUpdate(0);
    if (Global81ae2c_Equals(0) != 0) goto end;
    if (Global81ae2c_Equals(1) != 0 || Global81ae2c_Equals(3) != 0) goto drag;

    if (Global81ae2c_Equals(4) != 0) {
        global<u8>(0x008186d4) = 1;
        global<u32>(0x008186c0) = 0;
        global<u32>(0x008186c4) = 0;
        global<u32>(0x008186cc) = 0;
        if (global<u32>(0x008187f8) != 0) {
            E->focusUpdate(0);
            u8 *cell = CellAt(x, y);
            if (E->isTypeBD(cell) != 0 || E->isTypeC(cell) != 0) {
                if (E->isTypeC(cell) != 0) cell = Parent(cell);
                u8 *const handle = E->cell44(reinterpret_cast<const u16 *>(Parent(cell)));
                u16 out;
                const u32 index = *E->firstObject(handle, nullptr, &out);
                // (Debug_Printf when the thing table 0x7cfb90 is empty, left out)
                global<u8 *>(0x008186ac) = global<u8 *>(0x007cfb90 + index * 0x14);
                E->f545450(at<u32>(CurThing(), 0x28));
                E->f528a70(x, y, 4, Global81d7a4_Get_52f1f0(), 1, 0);
                E->pathLines(x, y);
                FocusFromCurThing();
                StepFocus(E, true);
                OpenIfRun(E, SetRun(E, 0, 1));
            }
            // (else Debug_Printf, left out)
            if ((u16)flags != 0xc) {
                E->f52f580(0, 0);
                global<u32>(0x008187f8) = 0;
            }
            goto recorded_end;
        }
        u32 endFlag;
        EntrancePiece(E, x, y, false, endFlag);
        if (endFlag == 2) {
            if ((u16)flags == 0xc) goto piece_end;
            E->f52f580(0, 0);
        }
        goto flags_end;
    }
    if (Global81ae2c_Equals(0x3b) != 0) {
        global<u8>(0x008186d4) = 1;
        global<u32>(0x008186c0) = 0;
        global<u32>(0x008186c4) = 0;
        global<u32>(0x008186cc) = 0;
        u32 endFlag;
        EntrancePiece(E, x, y, true, endFlag);
        if (endFlag == 2) {
            if ((u16)flags == 0xc) goto piece_end;
            E->f52f580(0, 0);
        }
        goto flags_end;
    }
    if (Global81ae2c_Equals(0xb) != 0 || Global81ae2c_Equals(0x10) != 0) goto track;
    if (Global81ae2c_Equals(0x19) != 0) {
        const s32 sx = StartX(), sy = StartY();
        if (sx == -1 && sy == -1) {
            StartX() = x;
            StartY() = y;
            Fn_52fbd0();
            PairTable_Add((u32)x, (u32)y);
            goto end;
        }
        Snap(sx, sy, x, y);
        s32 ex = sx, ey = sy;
        if (sx != x || sy != y || (global<s32>(0x007639f0) == x && global<s32>(0x007639f4) == y)) {
            E->applyLine(Global81ae2c_Get_52f840(), sx, sy, &x, &y);
            FromStart(E, 0x80, x, y);
            FromStart(E, 0x82, x, y);
            FromStart(E, 0x85, x, y);
            FromStart(E, 0x86, x, y);
            FromStart(E, 0x81, x, y);
            PairTable_Add((u32)x, (u32)y);
            ex = StartX();
            ey = StartY();
        }
        if (ex == x && ey == y) {
            StartX() = -1;
            StartY() = -1;
            global<u32>(0x00763ac0) = 0x80;
            global<u32>(0x00763b00) = 0x80;
            global<u32>(0x00763b04) = 0x80;
            E->f52f200(0, 0);
        } else {
            StartX() = x;
            StartY() = y;
        }
        goto end;
    }
    if (Global81ae2c_Equals(0x39) != 0) {
        s32 cx = x > 1 ? x : 1;
        s32 cy = y > 1 ? y : 1;
        const s32 w = (s32)(global<u32>(0x0081d740) - 2);
        if (cx >= w) cx = w;
        const s32 h = (s32)(global<u32>(0x0081b1b0) - 3);
        if (cy >= h) cy = h;
        x = Even(cx, global<s32>(0x007639f0));
        y = Even(cy, global<s32>(0x007639f4));
        const s32 sx = StartX(), sy = StartY();
        if (sx == -1 && sy == -1) {
            StartX() = x;
            StartY() = y;
            E->fill();
            goto end;
        }
        if (E->checkArea(sx, sy, x, y, 1) != 0) {
            E->f5327e0(StartX(), StartY(), x, y);
            const u32 cost = Global81af4c_Get_52fbc0();
            u8 *const money = reinterpret_cast<u8 *>((uintptr_t)E->table5195d0(World()));
            E->moneyDebit(money, nullptr, (s32)cost);
        }
        StartX() = -1;
        StartY() = -1;
        E->f52f200(0, 0);
        E->cursorSet(0);
        goto end;
    }
    if (Global81ae2c_Equals(0x3a) != 0) {
        if (StartX() == -1 && StartY() == -1) {
            StartX() = x;
            StartY() = y;
            goto end;
        }
        if (Const1_424ec0() != 0) E->f52bdd0(StartX(), StartY(), x, y);
        StartX() = -1;
        StartY() = -1;
        E->f52f200(0, 0);
        goto end;
    }
    if (Global81ae2c_Equals(0x33) != 0) {
        global<u32>(0x008186c0) = 0;
        global<u32>(0x008186c4) = 0;
        global<u32>(0x008186cc) = 0;
        E->setRecorded(0, 0);
        E->f527ee0(x, y, 0);
        goto end;
    }
    if (Global81ae2c_Equals(0x32) != 0) {
        if (Record29c(((u32)y << 7) + (u32)x + 1) == 4) goto end;
        Line6(E, 0x32, x, y);
        Line6(E, 0x86, x, y);
        goto end;
    }
    if (Global81ae2c_Equals(0x34) != 0) {
        if (StartX() == -1 && StartY() == -1) {
            StartX() = x;
            StartY() = y;
            goto end;
        }
        FromStart6(E, 0x32, x, y);
        FromStart6(E, 0x86, x, y);
        StartX() = x;
        StartY() = y;
        goto end;
    }
    if (Global81ae2c_Equals(0x35) != 0) {
        if (StartX() != -1 || StartY() != -1) FromStart(E, 0x32, x, y);
        StartX() = x;
        StartY() = y;
        goto end;
    }
    if (Global81ae2c_Equals(0x36) != 0) {
        const s32 sx = StartX(), sy = StartY();
        if (sx == -1 && sy == -1) {
            StartX() = x;
            StartY() = y;
            global<u32>(0x008186b0) = Wrap_Global7c2650_Get_481590();
            goto end;
        }
        u8 *const staff = global<u8 *>(0x008186b0);
        if (staff != nullptr) {
            const s32 minX = sx < x ? sx : x, minY = sy < y ? sy : y;
            const s32 maxX = sx > x ? sx : x, maxY = sy > y ? sy : y;
            // (each id written over the low word of a stack argument: flags' and useStored's high words come along)
            const u32 tr = (flags & 0xffff0000) | (u16)(((u32)maxY << 7) + (u32)maxX + 1);
            const u32 bl = (useStored & 0xffff0000) | (u16)(((u32)minY << 7) + (u32)minX + 1);
            E->setPatrol(staff, nullptr, bl, tr);
        }
        StartX() = -1;
        StartY() = -1;
        E->f52f200(0, 0);
        goto end;
    }
    if (Global81ae2c_Equals(0x14) != 0) {
        E->f530120(CurThing());
        E->f4de1f0(CurThing());
        E->f52f580(3, 0);
        goto end;
    }
    if (Global81ae2c_Equals(0xe) != 0 || Global81ae2c_Equals(0x13) != 0) goto ride_entrance;
    if (Global81ae2c_Equals(0x1a) != 0) {
        x = Even(x, global<s32>(0x007639a4));
        y = Even(y, global<s32>(0x007639a8));
        global<s32>(0x008186b8) = x;
        global<s32>(0x008186b4) = y;
        if (global<u16>(0x007639b0) == 0xffff) {
            global<u16>(0x00818694) = (u16)(((u32)y << 7) + (u32)x + 1);
            global<u16>(0x007639b0) = E->word24(CellOf(global<u32>(0x00818694)));
            if (E->f539760(CellOf(global<u32>(0x00818694)), nullptr, 0x1a, 0, 0) != 1) goto end;
            global<u16>(0x00818694) = 0;
            global<u16>(0x007639b0) = 0xffff;
            goto end;
        }
        global<u16>(0x00818684) = (u16)(((u32)y << 7) + (u32)x + 1);
        global<u16>(0x007639b4) = E->word24(CellOf(global<u32>(0x00818684)));
        u8 *cell = CellOf(global<u32>(0x00818684));
        if (E->isTypeC(cell) != 0) cell = Parent(cell);
        if (E->f539760(cell, nullptr, 0x1a, 0, 0) != 9) {
            global<u16>(0x00818684) = global<u16>(0x00818694);
            global<u16>(0x007639b4) = global<u16>(0x007639b0);
            goto end;
        }
        u32 joined;
        if ((s16)global<u16>(0x007639b0) > (s16)global<u16>(0x007639b4))
            joined = E->f52b400(CellOf(global<u32>(0x00818684)), CellOf(global<u32>(0x00818694)), 1);
        else
            joined = E->f52b400(CellOf(global<u32>(0x00818694)), CellOf(global<u32>(0x00818684)), 1);
        global<u16>(0x00818694) = 0;
        global<u16>(0x007639b0) = 0xffff;
        global<u16>(0x00818684) = 0;
        global<u16>(0x007639b4) = 0xffff;
        StartX() = (s32)(((u32)at<u16>(cell, 0) - 1) & 0x7f);
        global<u32>(0x007639c0) = 0;
        StartY() = (s32)(((u32)at<u16>(cell, 0) - 1) >> 7);
        if (joined == 0xffffffff) goto end;
        E->corners();
        E->f52f580(0x19, 0);
        goto end;
    }
    if (Global81ae2c_Equals(0x15) != 0) {
        if (global<u32>(0x00785a2c) == 0) PlaySound(E, 0x65, x, y);
        E->f446300(0);
        E->advise(0xcd);
        PairTable_Add((u32)x, (u32)y);
        Line6(E, 0x87, x, y);
        Line6(E, Global81ae2c_Get_52f840(), x, y);
        Line6(E, 0x83, x, y);
        Line6(E, 0x84, x, y);
        if (Line6(E, 0x81, x, y) != 0) {
            E->f4419b0(x, y, 0, CellAt(x, y) + 4);
            goto end;
        }
        E->f435570(0);
        if (global<u32>(0x008186c0) == 0) goto clear_end;
        E->f52f580(3, 0);
        StartX() = global<s32>(0x007639b8);
        StartY() = global<s32>(0x007639bc);
        if (StartX() == x && StartY() == y && Global820a8c_Get_52fbe0() != 1) goto end;
        s32 lx = StartX(), ly = StartY();
        const u32 id = (u16)(((u32)ly << 7) + (u32)lx + 1);
        PairTable_Add((u32)lx, (u32)ly);
        Line6(E, 0x85, lx, ly);
        Line6(E, 0x86, lx, ly);
        Line6(E, 0x83, lx, ly);
        Line6(E, 0x81, lx, ly);
        global<u32>(0x00763ac0) = 0x80;
        global<u32>(0x00763ac4) = 0x80;
        global<u32>(0x008186c0) = 0;
        if (Record29c(id) != 1) goto end;
        goto clear_end;
    }
    if (Global81ae2c_Equals(0x16) == 0) goto end;
    if (global<u8>(0x00818c54) == 0) {
        E->f446300(0);
        goto end;
    }
    PairTable_Add((u32)x, (u32)y);
    Line6(E, 0x87, x, y);
    Line6(E, 0x15, x, y);
    Line6(E, 0x83, x, y);
    Line6(E, 0x81, x, y);
    {
        u8 *const cell = CellAt(x, y);
        at<u32>(cell, 4) = E->f445a70(x, y);
    }
    global<u32>(0x007639a0) = 0xffffffff;
    global<u8>(0x00818c54) = 0;
    goto end;

ride_entrance: {   // modes 0xe and 0x13
    bool tail = true;   // (BL)
    if (at<u32>(CurThing(), 0x2c) == 0) {
        E->traceA(CurThing());
        goto entrance_modes;
    }
    if (global<u16>(0x007639b0) == 0xffff) {
        u8 *cell = CellAt(x, y);
        if (E->isTypeC(cell) != 0) cell = Parent(cell);
        if (cell == nullptr || E->isTypeD(cell) != 0) goto entrance_tail;
        s32 nx = x, ny = y;
        E->pathNudge(&nx, &ny);
        const u16 there = E->word24(CellAt(nx, ny));
        if ((s16)there < (s16)E->word24(cell)) {
            global<s32>(0x008186b8) = nx;
            global<s32>(0x008186b4) = ny;
        }
        global<u16>(0x00818694) = (u16)(((u32)y << 7) + (u32)x + 1);
        global<u16>(0x007639b0) = E->word24(cell);
        if (E->f539760(cell, nullptr, 0xe, 0, 0) == 1) {
            global<u16>(0x00818694) = 0;
            global<u16>(0x007639b0) = 0xffff;
        }
        const u32 run = E->f52a970(1);
        at<u32>(CurThing(), 0x2c) = run;
        OpenIfRun(E, run);
        tail = false;
        if (run != 0) goto entrance_tail;
        goto entrance_trace;
    } else {
        u8 *cell = CellAt(x, y);
        if (E->isTypeC(cell) != 0) cell = Parent(cell);
        if (cell == nullptr || E->isTypeD(cell) != 0) goto entrance_tail;
        s32 nx = x, ny = y;
        E->pathNudge(&nx, &ny);
        const u16 there = E->word24(CellAt(nx, ny));
        if ((s16)there > (s16)E->word24(cell)) {
            global<s32>(0x008186b8) = nx;
            global<s32>(0x008186b4) = ny;
        }
        global<u16>(0x00818684) = (u16)(((u32)y << 7) + (u32)x + 1);
        global<u16>(0x007639b4) = E->word24(CellOf(global<u32>(0x00818684)));
        u8 *exit = CellOf(global<u32>(0x00818684));
        if (E->isTypeC(exit) != 0) exit = Parent(exit);
        if (E->f539760(exit, nullptr, 0xe, 0, 0) != 9) {
            E->f52f200(0, 0);
            goto entrance_tail;
        }
        if ((s16)global<u16>(0x007639b0) > (s16)global<u16>(0x007639b4))
            E->f52b400(CellOf(global<u32>(0x00818684)), CellOf(global<u32>(0x00818694)), 1);
        else
            E->f52b400(CellOf(global<u32>(0x00818694)), CellOf(global<u32>(0x00818684)), 1);
        E->f52fc80(0);
        if (E->f52c6f0(CurThing(), 1, 0, 0) == 0) E->f52ccb0(CurThing(), 0, 0, 0);
        global<u16>(0x00818694) = 0;
        global<u16>(0x007639b0) = 0xffff;
        global<u16>(0x00818684) = 0;
        global<u16>(0x007639b4) = 0xffff;
        StartX() = (s32)(((u32)at<u16>(exit, 0) - 1) & 0x7f);
        StartY() = (s32)(((u32)at<u16>(exit, 0) - 1) >> 7);
        const u32 run = SetRun(E, 0, 1);
        if (run == 0) goto entrance_trace;
        OpenIfRun(E, run);
        E->f52f580(0, 0);
        goto entrance_tail;
    }
entrance_trace:
    E->traceA(CurThing());
entrance_modes:
    if (Global81ae2c_Equals(0xe) != 0) E->f52f580(0xb, 0);
    if (Global81ae2c_Equals(0x13) != 0) E->f52f580(0x10, 0);
entrance_tail:
    if (!tail) goto end;
    E->f52c6f0(CurThing(), 0, 0, 0);
    E->f52ccb0(CurThing(), 1, 0, 0);
    goto end;
}

track: {   // modes 0xb and 0x10
    if (global<u32>(0x00785a2c) == 0) PlaySound(E, 0xce, x, y);
    s32 sx = StartX(), sy = StartY();
    if (sx == -1 && sy == -1) {
        StartX() = x;
        StartY() = y;
        Fn_52fbd0();
        PairTable_Add((u32)x, (u32)y);
        E->advise(0xcc);
        goto end;
    }
    Snap(sx, sy, x, y);
    if (sx != x || sy != y || (global<s32>(0x007639e0) == x && global<s32>(0x007639e4) == y)) {
        E->applyLine(0x87, sx, sy, &x, &y);
        FromStart(E, Global81ae2c_Get_52f840(), x, y);
        FromStart(E, 0x80, x, y);
        FromStart(E, 0x82, x, y);
        FromStart(E, 0x83, x, y);
        FromStart(E, 0x85, x, y);
        FromStart(E, 0x86, x, y);
        lineEnded = FromStart(E, 0x81, x, y) == 0;
        PairTable_Add((u32)x, (u32)y);
        sx = StartX();
        sy = StartY();
    }
    if ((sx != x || sy != y) && lineEnded == 0) {
        StartY() = y;
        StartX() = x;
        if (CurThing() == nullptr) goto end;
        E->f52c6f0(CurThing(), 0, 0, 0);
        E->f52ccb0(CurThing(), 1, 0, 0);
        goto end;
    }
    {
        const u32 run = E->f52a970(0);
        at<u32>(CurThing(), 0x2c) = run;
        OpenIfRun(E, run);
        E->f52c6f0(CurThing(), 1, 0, 0);
        E->f52ccb0(CurThing(), 0, 0, 0);
        if (run != 0) {
            E->animateA(CurThing(), 0);
        } else {
            E->animateA(CurThing(), 0);
            E->animateB(CurThing(), 0);
        }
    }
    StartX() = -1;
    StartY() = -1;
    global<u32>(0x00763ac0) = 0x80;
    global<u32>(0x00763b00) = 0x80;
    global<u32>(0x00763b04) = 0x80;
    if (global<u32>(0x008186c0) == 0) {
        if ((u16)flags == 0xc) goto end;
        goto clear_end;
    }
    StartX() = global<s32>(0x007639b8);
    StartY() = global<s32>(0x007639bc);
    global<u16>(0x00818694) = 0;
    global<u16>(0x007639b0) = 0xffff;
    global<u16>(0x00818684) = 0;
    global<u16>(0x007639b4) = 0xffff;
    global<u32>(0x007639a0) = 0xffffffff;
    global<u32>(0x008187f8) = 0;
    global<u32>(0x0081b604) = 0;
    Fn_52fbd0();
    E->f52f580(3, 0);
    if (StartX() == x && StartY() == y && Global820a8c_Get_52fbe0() != 1) goto end;
    PairTable_Add(global<u32>(0x007639b8), global<u32>(0x007639bc));
    if (global<u32>(0x008186c0) != 0) global<u32>(0x008186c0) = 0;
    s32 &ex = global<s32>(0x007639b8);
    s32 &ey = global<s32>(0x007639bc);
    Line6(E, 0x85, ex, ey);
    Line6(E, 0x86, ex, ey);
    Line6(E, 0x83, ex, ey);
    Line6(E, 0x81, ex, ey);
    global<u32>(0x00763ac0) = 0x80;
    global<u32>(0x00763ac4) = 0x80;
    if (Record29c(((u32)ey << 7) + (u32)ex + 1) != 1) goto end;
    goto clear_end;
}

drag: {   // modes 1 and 3
    if (global<u32>(0x00785a2c) == 0) PlaySound(E, 0x65, x, y);
    if (Global81ae2c_Equals(1) != 0) global<u32>(0x00818c10) = 2;
    if (Global81ae2c_Equals(3) != 0) global<u32>(0x00818c10) = 3;
    s32 sy = StartY();
    if (StartX() == -1 && sy == -1) {
        u8 *const tile = World() + ((((u32)y << 7) + (u32)x + 1) & 0xffff) * 0x44 + 0x294;
        if (Global81ae2c_Equals(1) == 0 || E->f535670(tile, nullptr, 1, 0) == 1) {
            if (Global81ae2c_Equals(3) == 0) goto end;
            if (E->f535670(tile, nullptr, 3, 0) == 1) goto end;
        }
        StartX() = x;
        StartY() = y;
        Fn_52fbd0();
        PairTable_Add((u32)x, (u32)y);
        if (Global81ae2c_Equals(1) != 0) E->advise(0xb8);
        global<u32>(0x008186dc) = 0;
        goto end;
    }
    if (++global<u32>(0x008186dc) == 3) {
        E->advise(0xd6);
        sy = StartY();
    }
    {
        const s32 d = Abs32((s32)((u32)x - (u32)StartX()));
        if (d >= Abs32((s32)((u32)y - (u32)sy))) y = sy;
        if (d <= Abs32((s32)((u32)y - (u32)sy))) x = StartX();
    }
    if (StartX() != x || sy != y || Global820a8c_Get_52fbe0() == 1) {
        PairTable_Add((u32)x, (u32)y);
        FromStart6(E, 0x87, x, y);
        FromStart6(E, Global81ae2c_Get_52f840(), x, y);
        FromStart6(E, 0x80, x, y);
        FromStart6(E, 0x82, x, y);
        if (global<u32>(0x008186c0) != 0) global<u32>(0x008186c0) = 0;
        FromStart6(E, 0x85, x, y);
        FromStart6(E, 0x86, x, y);
        FromStart6(E, 0x83, x, y);
        lineEnded = FromStart6(E, 0x81, x, y) == 0;
    }
    if (Global81ae2c_Equals(3) != 0) {
        E->f4de1f0(CurThing());
        if (global<u32>(0x00785a2c) == 0 && global<u32>(0x00816d60) != 0) PlaySound(E, 0x8b, x, y);
    }
    if ((StartX() != x || StartY() != y) && lineEnded == 0) {
        StartX() = x;
        StartY() = y;
        goto end;
    }
    global<u32>(0x008186d0) = 0;
    StartX() = -1;
    StartY() = -1;
    global<u32>(0x00763ac0) = 0x80;
    global<u32>(0x00763b00) = 0x80;
    global<u32>(0x00763b04) = 0x80;
    const bool three = Global81ae2c_Equals(3) != 0;
    E->f52f200(0, 0);
    if (!three || global<u32>(0x00785a2c) != 0 || global<u32>(0x00816d60) != 0) goto end;
    E->advise(0x151);
    goto end;
}

piece_end:   // 0x525a06
    global<u8>(0x008186d4) = 0;
    global<u8>(0x008186d8) = 0;
    return;
flags_end:   // 0x5259f4
    if ((u16)flags == 0xc) goto piece_end;
recorded_end:   // 0x5259fc
    E->setRecorded(0, 0);
    goto piece_end;
clear_end:   // 0x5266a3
    E->f52f200(0, 0);
end:   // 0x52765b
    global<u8>(0x008186d8) = 0;
}
