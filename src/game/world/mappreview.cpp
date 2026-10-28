// The game mode's map preview: see mappreview.h.
#include "mappreview.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/globals.h"
#include "../core/trivial.h"
#include "../files/wrapsources.h"
#include "mapcell.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *base, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)base + offset); }
template <typename F> F raw(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const MapPreviewEnv kGameEnv = {
    raw<u32(__cdecl *)(u32, s32, s32, s32 *, s32 *)>(0x00536100),
    raw<u32(__cdecl *)(u32, s32, s32, s32 *, s32 *)>(0x0053aac0),
    raw<s32 *(__cdecl *)(s32, s32, u32, s32, u32, u32)>(0x00528a70),
    raw<void(__cdecl *)(u32, u32)>(0x0052f1b0),
    raw<void(__cdecl *)(s32, s32, u32)>(0x00527960),
    raw<void(__cdecl *)(s32, s32, u32, u32)>(0x00524960),
    raw<void(__cdecl *)(u32, u32)>(0x0052f580),
    raw<void(__cdecl *)()>(0x00530e80),
    raw<s32(__cdecl *)(u8 *, u8 *, u32)>(0x0052b400),
    raw<s32(__cdecl *)(s32, s32, s32, s32, u8)>(0x00531a50),
    raw<u32(__cdecl *)(s32, s32, s32, s32)>(0x00532a50),
    raw<s32(__cdecl *)(u32, u32, u8 *, u32, u32, u32, u32)>(0x00434740),
    raw<void(__cdecl *)()>(0x0052f950),
    raw<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>(0x00539760),
    raw<u32(__fastcall *)(u8 *, void *, u32)>(0x004d4b70),
    raw<u16 *(__fastcall *)(const u8 *, void *, u16 *)>(0x004d99e0),
    raw<u8 *(__fastcall *)(u8 *, void *, u16)>(0x00412e90),
    raw<u32(__fastcall *)(u8 *)>(0x005195d0),
};
const MapPreviewEnv *g_env = &kGameEnv;

using Line = u32(__cdecl *)(u32, s32, s32, s32 *, s32 *);

u8 *World() { return global<u8 *>(0x007cf83c); }
u32 IdAt(s32 x, s32 y) { return (((u32)y << 7) + (u32)x + 1) & 0xffff; }
// The 0x28-byte map cell of a cell id: [0x7cf83c] + (id + 0x6cde) * 0x28; the 0x44-byte one: [0x7cf83c] + 0x294 + id * 0x44.
u8 *Cell28(u32 id) { return World() + ((id & 0xffff) + 0x6cde) * 0x28; }
u8 *Cell44(u32 id) { return World() + 0x294 + (id & 0xffff) * 0x44; }
s32 &StartX() { return global<s32>(0x0081ede4); }
s32 &StartY() { return global<s32>(0x0081ede8); }
bool HasStart() { return StartX() != -1 && StartY() != -1; }
s32 &PieceX() { return global<s32>(0x008186a4); }
s32 &PieceY() { return global<s32>(0x008186a8); }

// The definition of the piece [0x8186e0] (ObjectDefs_Find on 0x7890a0).
u8 *Def(const MapPreviewEnv *E) { return E->findDef(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, global<u16>(0x008186e0)); }

u32 Mode() { return Global81ae2c_Get_52f840() | 0x100; }

// |v| as cdq; xor; sub has it (the most negative int is itself).
s32 Abs32(s32 v) {
    const u32 s = (u32)(v >> 31);
    return (s32)(((u32)v ^ s) - s);
}

s32 Sub(s32 a, s32 b) { return (s32)((u32)a - (u32)b); }

// v to base's parity: the difference less 1 when negative and odd, halved toward zero and doubled, base added back.
s32 Even(s32 v, s32 base) {
    s32 d = Sub(v, base);
    if (d < 0 && d % 2 != 0) d--;
    return (s32)((u32)base + (u32)(d / 2 * 2));
}

// The end pulled onto the start's row (|dx| >= |dy|) and then its column (|dx| <= the new |dy|), kept at 0x8186b8 / 0x8186b4.
void Pull(s32 &x, s32 &y) {
    const s32 dx = Abs32(Sub(x, StartX()));
    if (dx >= Abs32(Sub(y, StartY()))) y = StartY();
    if (dx <= Abs32(Sub(y, StartY()))) x = StartX();
    global<s32>(0x008186b8) = x;
    global<s32>(0x008186b4) = y;
}

// A drag's line: from the start (pulled to) when there is one, else from (x, y) to itself.
void DragLine(Line line, s32 &x, s32 &y) {
    if (HasStart()) {
        Pull(x, y);
        line(Mode(), StartX(), StartY(), &x, &y);
    } else {
        line(Mode(), x, y, &x, &y);
    }
}

// The rectangle of the start and (x, y): the top from the left + inset to the right - inset (the corners kept out when the sides differ), the
// right from the top down, the bottom likewise, the left from the bottom up. code 0: the mode | 0x100 at each call.
void Rect(Line line, u32 code, s32 x, s32 y, s32 inset) {
    const s32 sx = StartX(), sy = StartY();
    const s32 x0 = sx < x ? sx : x, y0 = sy < y ? sy : y, x1 = sx > x ? sx : x, y1 = sy > y ? sy : y;
    const auto mode = [code] { return code != 0 ? code : Mode(); };
    const s32 in0 = (s32)((u32)x0 + (u32)inset), in1 = Sub(x1, inset);
    s32 ex = x0 != x1 ? in1 : x1, ey = y0;
    line(mode(), x0 != x1 ? in0 : x0, y0, &ex, &ey);
    ex = x1;
    ey = y1;
    line(mode(), x1, y0, &ex, &ey);
    ex = x0 != x1 ? in1 : x1;
    ey = y1;
    line(mode(), x0 != x1 ? in0 : x0, y1, &ex, &ey);
    ex = x0;
    ey = y0;
    line(mode(), x0, y1, &ex, &ey);
}

// The coaster's rectangle (Coasters_GetState: columns from a below c, rows from b below d, unsigned): each cell CoasterPiece_CanPlaceAt
// allows that is free gets Map_SendLine 0x11f to itself, the column and row read back.
void CoasterArea(const MapPreviewEnv *E) {
    u32 a, b, c, d;
    Coasters_GetState(&a, &b, &c, &d);
    for (s32 row = (s32)b; (u32)row < d; row++) {
        for (s32 col = (s32)a; (u32)col < c; col++) {
            const u8 *cell = Cell44(IdAt(col, row));
            const u16 *handle = reinterpret_cast<const u16 *>(cell);
            if (E->canPlaceAt((u32)col, (u32)row, nullptr, 0, 0, 0, 0) == 0) continue;
            if (MapCell_HasFlags(cell, nullptr, 0x40) || MapHandle_ParentIsType19(handle) || MapHandle_IsTypeBDOr10Or12Or19(handle)) continue;
            if (MapCell_IsType9(cell) || MapCell_IsTypeA(cell) || MapCell_IsType3Or9(cell) || MapCell_IsType4(cell) || MapCell_IsType7Or1e(cell)) continue;
            if (MapCell_IsType1(cell) && MapCell_HasFlags(cell, nullptr, 0x20)) continue;
            E->sendLine(0x11f, col, row, &col, &row);
        }
    }
}

// The footprint at 0x8186a4 / 0x8186a8: tested (0 sets [0x81b604]) then laid; [0x874fc4] = 0.
void Footprint(const MapPreviewEnv *E) {
    if (E->footprint(PieceX(), PieceY(), 4, (s32)Global81d7a4_Get_52f1f0(), 0, 1) == nullptr) global<u32>(0x0081b604) = 1;
    E->footprint(PieceX(), PieceY(), 4, (s32)Global81d7a4_Get_52f1f0(), 0, 0);
    global<u32>(0x00874fc4) = 0;
}

// Mode 4: a path or queue piece.
void Piece(const MapPreviewEnv *E, s32 &x, s32 &y) {
    global<u8>(0x008186d4) = 1;
    if (at<s32>(Def(E), 0xc4) > 0 && (u8)E->hasKey(World() + 0x2c0, nullptr, global<u16>(0x008186e0)) == 0) {
        Global81af4c_Set_52fb90(0);
    } else {
        Global81af4c_Set_52fb90(at<u32>(Def(E), 0x1b8));
        if (global<u32>(0x00785a2c) == 0) {
            const s32 money = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)E->table5195d0(World())));
            if ((s32)Global81af4c_Get_52fbc0() > money) {
                global<u32>(0x00816d50) = 1;
                global<u32>(0x00816d5c) = 1;
            }
        }
    }
    PieceX() = x;
    PieceY() = y;
    global<u32>(0x0081b604) = 0;
    if (global<u16>(0x008186e0) != 0) {
        global<u32>(0x00818c10) = at<u32>(Def(E), 0x54);
        if (global<u32>(0x008187f8) != 0 && Map_InBounds(x, y)) {
            const u8 *def = Def(E);
            const u32 angle = Global81d7a4_Get_52f1f0();
            x = Even(x, global<s32>(0x007639a4));
            y = Even(y, global<s32>(0x007639a8));
            PieceX() = x;
            PieceY() = y;
            if (Map_InBounds(x, y)) {
                const u8 *cell = Cell28(IdAt(PieceX(), PieceY()));
                if (MapCell_IsTypeCOr11(cell)) cell = Cell28(at<u16>(cell, 0x10));
                if (MapCell_IsTypeBDOr10Or12Or19(cell)) {
                    u32 a = angle;
                    switch (MapCell_Direction(cell)) {
                    case 1: a = 0; break;
                    case 4: a = 0x10e; break;
                    case 0x10: a = 0xb4; break;
                    case 0x40: a = 0x5a; break;
                    }
                    E->setRecorded(a, 0);
                }
                u32 dx = 0, dy = 0;
                switch ((s32)Global81d7a4_Get_52f1f0()) {
                case 0: dx = 0xa4, dy = 0xa8; break;
                case 0x5a: dx = 0xbc, dy = 0xc0; break;
                case 0xb4: dx = 0xb4, dy = 0xb8; break;
                case 0x10e: dx = 0xac, dy = 0xb0; break;
                }
                if (dx != 0) {
                    PieceX() = (s32)((u32)PieceX() + at<u32>(def, dx));
                    PieceY() = (s32)((u32)PieceY() + at<u32>(def, dy));
                }
                switch (at<s32>(global<const u8 *>(0x008186ac), 0x10)) {
                case 0x5a: PieceX()--; break;
                case 0xb4: PieceX()--, PieceY()++; break;
                case 0x10e: PieceY()++; break;
                }
            }
        }
        global<s32>(0x008186b8) = PieceX();
        global<s32>(0x008186b4) = PieceY();
        Footprint(E);
    }
    global<u8>(0x008186d4) = 0;
}

// Mode 0x1a: a ride's entrance or exit dragged.
void EntranceDrag(const MapPreviewEnv *E, s32 &x, s32 &y, u32 flags) {
    if (global<u16>(0x00818698) == 0) return;
    x = Even(x, global<s32>(0x007639a4));
    y = Even(y, global<s32>(0x007639a8));
    global<s32>(0x008186b8) = x;
    global<s32>(0x008186b4) = y;
    if (global<u16>(0x007639b0) == 0xffff) E->applyLine(Mode(), x, y, &x, &y);
    if (global<u32>(0x007639c0) == 0) {
        E->corners();
        E->mapClick(StartX(), StartY(), flags, 0);
        if (Global81ae2c_Equals(0x1a)) E->setOnly(0x19, 1);
        return;
    }
    if (global<u16>(0x007639b0) == 0xffff) return;
    u16 &here = global<u16>(0x00818684);
    here = (u16)IdAt(x, y);
    global<u16>(0x007639b4) = MapCell_Word24(Cell28(here));
    const u32 kind = E->canBuildTrack(Cell28(here), nullptr, Global81ae2c_Get_52f840(), 0, 0);
    const u16 end = global<u16>(0x007639b0), other = global<u16>(0x00818694);
    if (kind != 9) {
        here = other;
        global<u16>(0x007639b4) = end;
    }
    if ((s16)end > (s16)global<u16>(0x007639b4)) E->connect(Cell28(here), Cell28(other), 0);
    else E->connect(Cell28(other), Cell28(here), 0);
}

}  // namespace

void MapPreview_SetEnv(const MapPreviewEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" void __cdecl GameMode_MapPreview(s32 x, s32 y, u32 flags) {
    const MapPreviewEnv *const E = g_env;
    global<s32>(0x008186b8) = x;
    global<s32>(0x008186b4) = y;
    static const u32 kCleared[] = {0x00816d48, 0x00816d50, 0x00816d58, 0x00816d54, 0x00816d60, 0x00816d5c, 0x00816d4c};
    for (const u32 a : kCleared) global<u32>(a) = 0;
    Global871ee0_Clear();
    global<u32>(0x00818c10) = 0;
    global<u32>(0x00818c14) = 0;
    const u32 mode = global<u32>(0x0081ae2c);
    if (mode != 0) {
        switch (mode) {
        case 1:
        case 3:
            global<u32>(0x00818c10) = mode == 1 ? 2 : 3;
            DragLine(E->sendLine, x, y);
            break;
        case 0x38:
            if (HasStart()) E->sendLine(Global81af64_Get_52f850() | 0x100, StartX(), StartY(), &x, &y);
            else E->sendLine(Global81af64_Get_52f850() | 0x100, x, y, &x, &y);
            break;
        case 4:
            Piece(E, x, y);
            break;
        case 0x3b:
            global<u8>(0x008186d4) = 1;
            global<u32>(0x0081b604) = 0;
            PieceX() = x;
            PieceY() = y;
            Global81af4c_Set_52fb90(at<u32>(Def(E), 0x1b8));
            global<u32>(0x00818c10) = at<u32>(Def(E), 0x54);
            Footprint(E);
            global<u8>(0x008186d4) = 0;
            break;
        case 0xb:
        case 0x10:
        case 0x19:
            global<u32>(0x00818c10) = 3;
            DragLine(E->applyLine, x, y);
            break;
        case 0x39: {
            s32 cx = x > 1 ? x : 1, cy = y > 1 ? y : 1;
            const s32 w = Sub(global<s32>(0x0081d740), 2), h = Sub(global<s32>(0x0081b1b0), 3);
            if (!(cx < w)) cx = w;
            if (!(cy < h)) cy = h;
            x = Even(cx, global<s32>(0x007639f0));
            y = Even(cy, global<s32>(0x007639f4));
            if (HasStart()) {
                global<u32>(0x00816d50) = E->checkArea(StartX(), StartY(), x, y, 1) == 0;
                Rect(E->applyLine, 0, x, y, 2);
            } else {
                E->applyLine(Mode(), x, y, &x, &y);
            }
            break;
        }
        case 0x3a:
            Global81af4c_Set_52fb90(0xffffd8f1);
            if (HasStart()) {
                global<u32>(0x00816d50) = Const1_424ec0() == 0;
                Rect(E->sendLine, 0x13a, x, y, 1);
            } else {
                E->sendLine(0x13a, x, y, &x, &y);
            }
            break;
        case 0x36:
            Global81af4c_Set_52fb90(0xffffd8f1);
            if (HasStart()) {
                const s32 sx = StartX(), sy = StartY();
                global<u32>(0x00816d50) = E->areaBlocked(sx < x ? sx : x, sy < y ? sy : y, sx > x ? sx : x, sy > y ? sy : y) == 0;
                Rect(E->sendLine, 0, x, y, 1);
            } else {
                E->sendLine(Mode(), x, y, &x, &y);
            }
            break;
        case 0xe:
        case 0x13:
            if (x != 0 || y != 0) E->trackDrag(x, y, flags);
            break;
        case 0x1a:
            EntranceDrag(E, x, y, flags);
            break;
        case 0x15:
            global<u32>(0x00818c10) = 3;
            global<u32>(0x00816d50) = E->canPlaceAt((u32)x, (u32)y, nullptr, 0, 0, 0, 0) == 0;
            E->sendLine(0x115, x, y, &x, &y);
            CoasterArea(E);
            break;
        case 0x16:
            global<u32>(0x00818c10) = 3;
            if (global<u8>(0x00818c54) != 0) global<u32>(0x00816d50) = E->canPlaceAt((u32)x, (u32)y, nullptr, 0, 0, 0, 0) == 0;
            if (global<s32>(0x007639a0) == -1) global<s32>(0x007639a0) = (s16)MapCell_Word40(Cell44(IdAt(x, y)));
            E->sendLine(0x116, x, y, &x, &y);
            if (global<u8>(0x00818c54) != 0) CoasterArea(E);
            break;
        }
        // A type 4 cell at the end whose parent's first object is a thing of kind 3: [0x818c14] = its definition's +0x54.
        if (Map_InBounds(x, y)) {
            const u8 *cell = Cell44(IdAt(x, y));
            if (MapCell_IsType4(cell)) {
                u16 out = 0;
                const u16 handle = *E->firstObject(Cell44(at<u16>(cell, 0x10)), nullptr, &out);
                const u8 *thing = global<const u8 *>(0x007cfb90 + (u32)handle * 20);
                if (at<u8>(thing, 2) == 3)
                    global<u32>(0x00818c14) = at<u32>(E->findDef(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x54);
            }
        }
    }
    E->cursorUpdate();
    global<u32>(0x00818688) = StartX() == x && StartY() == y;
}
