// The sprite script VM: reimplementation of 0x475010-0x475260, 0x476020-0x476a10 and
// 0x540b90.
#include "script.h"

#include "../math/x87.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

// The VM's globals, 0x7b49f0-0x7b4a1b.
struct Globals {
    u8 **scripts;     // 0x7b49f0 slot table
    s32 slots;        // 0x7b49f4
    u8 *regs;         // 0x7b49f8 the current script's registers (+0x84)
    u32 now;          // 0x7b49fc
    u32 code;         // 0x7b4a00 the current script's code base
    u32 unknown04;    // 0x7b4a04
    u32 steps;        // 0x7b4a08
    u32 unknown0c;    // 0x7b4a0c
    u32 stepLimit;    // 0x7b4a10
    s32 ready;        // 0x7b4a14
    u8 *current;      // 0x7b4a18
};
Globals &g() { return *reinterpret_cast<Globals *>(0x007b49f0); }

constexpr u32 kOpIf = 0x00476050, kOpEndIf = 0x004762f0, kOpElse = 0x00476320, kOpEnd = 0x005da3c0;
constexpr s32 kCodeEnd = 0x0074f7c0;

s32 &pc() { return at<s32>(g().current, 8); }
u32 word(s32 index) { return *reinterpret_cast<const u32 *>(g().code + 4u * (u32)index); }
u32 fetch() {
    const u32 v = word(pc());
    ++pc();
    return v;
}
u32 &reg(u32 index) { return *reinterpret_cast<u32 *>(g().regs + 4u * index); }
float &freg(u32 index) { return *reinterpret_cast<float *>(g().regs + 4u * index); }
bool is_float(u32 index) { return index >= 1 && index <= 6; }

// fcom: the x87 condition codes C0 (below) and C3 (equal); both when unordered.
struct Fcom {
    bool c0, c3;
};
Fcom fcom(double st0, double src) {
    if (st0 != st0 || src != src) return {true, true};
    return {st0 < src, st0 == src};
}

// The code word at a pc (word index) of a code base, as the skippers read it: the address
// compares signed against the end of the code table.
bool in_code(u32 base, s32 index) { return (s32)(base + 4u * (u32)index) < kCodeEnd; }
u32 code_at(u32 base, s32 index) { return *reinterpret_cast<const u32 *>(base + 4u * (u32)index); }

// pc = the word index of an address in the code (sar: the difference is signed).
s32 index_of(u32 address) { return (s32)(address - g().code) >> 2; }

}  // namespace

extern "C" void __cdecl SpriteScript_Push(u32 value) {
    s32 &sp = at<s32>(g().current, 0x1c);
    if (sp > 0) {
        --sp;
        at<u32>(g().current, 0x20 + 4u * (u32)sp) = value;
    }
}

extern "C" u32 __cdecl SpriteScript_Pop() {
    s32 &sp = at<s32>(g().current, 0x1c);
    if (sp >= 0x14) return 0;
    ++sp;
    return at<u32>(g().current, 0x1c + 4u * (u32)sp);
}

extern "C" void __cdecl SpriteScript_SkipBlock() {
    u8 *script = g().current;
    const u32 base = at<u32>(script, 0x14);
    while (in_code(base, at<s32>(script, 8))) {
        const u32 op = code_at(base, at<s32>(script, 8));
        if (op == kOpEndIf) return;
        if (op == kOpIf) {
            ++at<s32>(script, 8);
            ++at<s32>(g().current, 0x70);
            SpriteScript_SkipBlock();
            --at<s32>(g().current, 0x70);
            script = g().current;
        }
        ++at<s32>(script, 8);
        script = g().current;
    }
}

extern "C" void __cdecl SpriteScript_SkipToElse() {
    u8 *script = g().current;
    const u32 base = at<u32>(script, 0x14);
    while (in_code(base, at<s32>(script, 8))) {
        const u32 op = code_at(base, at<s32>(script, 8));
        if (op == kOpElse || op == kOpEndIf) break;
        if (op == kOpIf) {  // a nested block: skipped whole (SkipBlock inlined)
            ++at<s32>(script, 8);
            ++at<s32>(g().current, 0x70);
            SpriteScript_SkipBlock();
            script = g().current;
            --at<s32>(script, 0x70);
            script = g().current;
        }
        ++at<s32>(script, 8);
        script = g().current;
    }
    const s32 index = at<s32>(script, 8);
    if (in_code(base, index) && code_at(base, index) == kOpElse) at<s32>(script, 8) = index + 1;
}

u32 SpriteScript_RunWith(void(__cdecl *op)(u32 address), u8 *script) {
    g().current = script;
    g().code = at<u32>(script, 0x14);
    g().regs = script + 0x84;
    if (at<s32>(script, 0x18) == 3) return 0;
    at<s32>(script, 0x18) = 1;
    u32 steps = 1;
    s32 index = pc();
    u32 next = word(index);
    at<s32>(g().current, 0x10) = index;
    ++pc();
    for (;;) {
        op(next);
        if (next == kOpEnd) {
            at<s32>(g().current, 0x18) = 4;
            at<u32>(g().current, 0x114) = 0;
            return steps;
        }
        u8 *current = g().current;
        if (at<s32>(current, 0x18) != 1 || steps >= g().stepLimit) return steps;
        index = at<s32>(current, 8);
        next = word(index);
        at<s32>(current, 0x10) = index;
        ++steps;
        ++pc();
    }
}

extern "C" u32 __cdecl SpriteScript_Run(u8 *script) {
    return SpriteScript_RunWith([](u32 address) { reinterpret_cast<void(__cdecl *)()>(address)(); }, script);
}

extern "C" void __cdecl SpriteOp_Goto() {
    const u32 target = fetch();
    pc() = index_of(target);
}

extern "C" void __cdecl SpriteOp_If() {
    const u32 a = fetch(), test = fetch(), c = fetch();
    ++at<s32>(g().current, 0x70);  // past 10: "Too many nested IFs" (Debug_Printf, empty)
    bool holds;
    if (is_float(a)) {
        const double ra = freg(a);
        Fcom f;
        switch (test) {
        case 1: f = fcom(freg(c), ra); holds = f.c3; break;
        case 2: f = fcom(freg(c), ra); holds = !f.c3; break;
        case 3: f = fcom(ra, freg(c)); holds = !(f.c0 || f.c3); break;
        case 4: f = fcom(ra, freg(c)); holds = f.c0; break;
        case 5: f = fcom((s32)c, ra); holds = f.c3; break;
        case 6: f = fcom((s32)c, ra); holds = !f.c3; break;
        case 7: f = fcom((s32)c, ra); holds = f.c0; break;
        case 8: f = fcom((s32)c, ra); holds = !(f.c0 || f.c3); break;
        default: return;  // "Bad test code in IF statement": runs the block
        }
    } else {
        const s32 ra = (s32)reg(a);
        switch (test) {
        case 1: holds = (s32)reg(c) == ra; break;
        case 2: holds = (s32)reg(c) != ra; break;
        case 3: holds = ra > (s32)reg(c); break;
        case 4: holds = ra < (s32)reg(c); break;
        case 5: holds = (s32)c == ra; break;
        case 6: holds = (s32)c != ra; break;
        case 7: holds = ra > (s32)c; break;
        case 8: holds = ra < (s32)c; break;
        default: return;
        }
    }
    if (!holds) SpriteScript_SkipToElse();
}

extern "C" void __cdecl SpriteOp_EndIf() { --at<s32>(g().current, 0x70); }  // below 0: "ENDIF without IF"

extern "C" void __cdecl SpriteOp_Else() { SpriteScript_SkipBlock(); }  // depth 0: "ELSE without IF"

extern "C" void __cdecl SpriteOp_Gosub() {
    const u32 target = fetch();
    SpriteScript_Push((u32)pc());
    pc() = index_of(target);
    ++at<s32>(g().current, 0x74);  // past 10: "Too many nested GOSUBS"
}

extern "C" void __cdecl SpriteOp_Repeat() {
    SpriteScript_Push((u32)pc());
    ++at<s32>(g().current, 0x78);
}

extern "C" void __cdecl SpriteOp_Until() {
    const u32 a = fetch(), test = fetch(), c = fetch();
    if (at<s32>(g().current, 0x78) < 1) return;  // "UNTIL without REPEAT"
    bool again = false;
    if (is_float(a)) {
        const double ra = freg(a);
        Fcom f;
        switch (test) {
        case 1: f = fcom(freg(c), ra); again = !f.c3; break;
        case 2: f = fcom(freg(c), ra); again = f.c3; break;
        case 3: f = fcom(ra, freg(c)); again = f.c0 || f.c3; break;
        case 4: f = fcom(ra, freg(c)); again = !f.c0; break;
        case 5: f = fcom((s32)c, ra); again = !f.c3; break;
        case 6: f = fcom((s32)c, ra); again = f.c3; break;
        case 7: f = fcom((s32)c, ra); again = !f.c0; break;
        case 8: f = fcom((s32)c, ra); again = f.c0 || f.c3; break;
        default: break;  // "Bad test code in UNTIL statement": the loop ends
        }
    } else {
        const s32 ra = (s32)reg(a);
        switch (test) {
        case 1: again = (s32)reg(c) != ra; break;
        case 2: again = (s32)reg(c) == ra; break;
        case 3: again = ra <= (s32)reg(c); break;
        case 4: again = ra >= (s32)reg(c); break;
        case 5: again = (s32)c != ra; break;
        case 6: again = (s32)c == ra; break;
        case 7: again = ra <= (s32)c; break;
        case 8: again = ra >= (s32)c; break;
        default: break;
        }
    }
    if (again) {
        const u32 start = SpriteScript_Pop();
        SpriteScript_Push(start);
        pc() = (s32)start;
    } else {
        SpriteScript_Pop();
        --at<s32>(g().current, 0x78);
    }
}

extern "C" void __cdecl SpriteOp_Yield() { at<s32>(g().current, 0x18) = 2; }

extern "C" void __cdecl SpriteOp_Set() {
    const u32 a = fetch();
    if (is_float(a)) freg(a) = (float)(s32)word(pc());  // fild; fstp
    else reg(a) = word(pc());
    ++pc();
}

extern "C" void __cdecl SpriteOp_Add() {
    const u32 a = fetch();
    if (is_float(a)) freg(a) = (float)X87_Add((double)(s32)word(pc()), freg(a));
    else reg(a) += word(pc());
    ++pc();
}

extern "C" void __cdecl SpriteOp_Sub() {
    const u32 a = fetch();
    if (is_float(a)) freg(a) = (float)X87_Add(freg(a), -(double)(s32)word(pc()));  // fild; fsubr
    else reg(a) -= word(pc());
    ++pc();
}

extern "C" void __cdecl SpriteOp_Move() {
    const u32 to = fetch(), from = fetch();
    reg(to) = reg(from);
}

extern "C" void __cdecl SpriteOp_SetAnim() { reg(12) = fetch(); }

namespace {
void play(u32 anim, bool always) {
    reg(13) = anim;
    if (always || reg(13) != 0xffffffffu) Sprite_Show(g().current);
    else at<u32>(g().current, 0x114) = 0;
    at<s32>(g().current, 0x18) = 2;
}
}  // namespace

extern "C" void __cdecl SpriteOp_PlayReg() { play(reg(fetch()), false); }
extern "C" void __cdecl SpriteOp_Play() { play(fetch(), false); }
extern "C" void __cdecl SpriteOp_PlayAlways() { play(fetch(), true); }

extern "C" void __cdecl SpriteOp_Hide() {
    at<u32>(g().current, 0xf8) = 0;
    at<u32>(g().current, 0x114) = 0;
}

extern "C" void __cdecl SpriteOp_StepR9() {
    float &r9 = *reinterpret_cast<float *>(g().regs + 0x24);
    r9 = (float)X87_Round((double)r9 - (double)-0.01f);  // the float at 0x6feda0
}

extern "C" void __cdecl Sprite_Show(u8 *script) { at<u32>(script, 0x114) = 1; }
