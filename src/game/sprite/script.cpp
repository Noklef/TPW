// The sprite script VM: reimplementation of 0x475010-0x475f40, 0x476020-0x476a10,
// 0x540b90-0x540c70 and 0x541f60.
#include "script.h"

#include <cstring>

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

namespace {
const SpriteScriptEnv &env();  // (below, with the list)
}  // namespace

extern "C" u32 __cdecl SpriteScript_Run(u8 *script) {
    if (env().op) return SpriteScript_RunWith(env().op, script);
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
        const auto ra = [a] { return (s32)reg(a); };  // read in each test: a bad code reads nothing
        switch (test) {
        case 1: holds = (s32)reg(c) == ra(); break;
        case 2: holds = (s32)reg(c) != ra(); break;
        case 3: holds = ra() > (s32)reg(c); break;
        case 4: holds = ra() < (s32)reg(c); break;
        case 5: holds = (s32)c == ra(); break;
        case 6: holds = (s32)c != ra(); break;
        case 7: holds = ra() > (s32)c; break;
        case 8: holds = ra() < (s32)c; break;
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
        const auto ra = [a] { return (s32)reg(a); };  // read in each test: a bad code reads nothing
        switch (test) {
        case 1: again = (s32)reg(c) != ra(); break;
        case 2: again = (s32)reg(c) == ra(); break;
        case 3: again = ra() <= (s32)reg(c); break;
        case 4: again = ra() >= (s32)reg(c); break;
        case 5: again = (s32)c != ra(); break;
        case 6: again = (s32)c == ra(); break;
        case 7: again = ra() <= (s32)c; break;
        case 8: again = ra() >= (s32)c; break;
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

// ---- the sprite lookups -------------------------------------------------------------

extern "C" u32 __fastcall Sprite_GetAnimFrameCount(const u8 *sprite, void *, u32 anim) {
    return sprite[0x222 + 4u * anim];
}

extern "C" void __fastcall Sprite_GetAction(const u8 *sprite, void *, u32 action, u32 *index, u32 *anim, u32 *r17,
                                            u32 *r18) {
    if (action > 3) return;
    const u8 *entry = sprite + 0x20e + 4u * action;
    *anim = entry[1] - 1u;
    *index = entry[0] - 1u;
    *r17 = entry[2];
    *r18 = entry[3];
}

extern "C" s32 __cdecl SpriteFolder_Count(u32 folder) {
    return *reinterpret_cast<const s32 *>(0x00763f90 + 12u * folder);
}

// ---- the script list ----------------------------------------------------------------

namespace {

s32 __cdecl game_now() {
    using Clock = s32(__fastcall *)(void *self, void *unused);  // thiscall without arguments
    return reinterpret_cast<Clock>(0x00402d70)(reinterpret_cast<void *>(0x00785970), nullptr);
}

const SpriteScriptEnv kGameEnv = {
    reinterpret_cast<void *(__cdecl *)(u32)>(0x005f5660),
    reinterpret_cast<void(__cdecl *)(void *)>(0x005f5680),
    &game_now,
    reinterpret_cast<u32(__cdecl *)(u8 *)>(0x00475010),
    reinterpret_cast<void(__cdecl *)()>(0x00540ab0),
    reinterpret_cast<void(__cdecl *)(u8 *)>(0x00542010),
    reinterpret_cast<void(__cdecl *)()>(0x00540b30),
    reinterpret_cast<void(__cdecl *)(u32 *, void *, const void *, u32, u32)>(0x005f5e50),
    reinterpret_cast<void(__cdecl *)(u32 *, void *, void *, u32, u32)>(0x005f5e20),
    nullptr,
};
const SpriteScriptEnv *g_env = &kGameEnv;
const SpriteScriptEnv &env() { return *g_env; }

constexpr u32 kCodeBase = 0x0074dab8, kMagic = 0x53435054;  // "TPCS"
constexpr u32 kScriptSize = 0x118;

// The script in a slot; null for slot 0 or past the table (empty slots hold null too).
u8 *script_at(s32 slot) { return slot < 1 || slot >= g().slots ? nullptr : g().scripts[slot]; }

// Kill's body, which the other loops inline: only while the list is ready.
void kill(s32 slot) {
    if (!g().ready || slot <= 0 || slot >= g().slots) return;
    u8 *script = g().scripts[slot];
    if (!script) return;
    g().scripts[slot] = nullptr;
    env().free(script);
}

void new_table(s32 slots) {
    g().slots = slots;
    g().scripts = static_cast<u8 **>(env().alloc(4u * (u32)slots));
    std::memset(g().scripts, 0, 4u * (u32)g().slots);
}

// The script into the first free slot, the table growing by 50 until there is one.
s32 insert(u8 *script) {
    for (;;) {
        for (s32 slot = 1; slot < g().slots; ++slot) {
            if (!g().scripts[slot]) {
                g().scripts[slot] = script;
                at<s32>(script, 4) = slot;
                return slot;
            }
        }
        g().slots += 50;
        u8 **grown = static_cast<u8 **>(env().alloc(4u * (u32)g().slots));
        std::memset(grown, 0, 4u * (u32)g().slots);
        std::memcpy(grown, g().scripts, 4u * (u32)g().slots - 200);
        env().free(g().scripts);
        g().scripts = grown;
    }
}

// The sprite a script draws: gSprites (0x876b48)[the folder's first index + sprite].
const u8 *sprite_of(u32 folder, u32 sprite) {
    const u32 first = *reinterpret_cast<const u32 *>(0x00763f8c + 12u * folder);
    return *reinterpret_cast<const u8 *const *>(0x00876b48 + 4u * (first + sprite));
}
u32 action_code(u32 index) { return *reinterpret_cast<const u32 *>(0x0074f7c8 + 4u * index); }
bool is_action(u32 code) { return (s32)code >= 0 && (s32)code <= 3; }

}  // namespace

void SpriteScripts_SetEnv(const SpriteScriptEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" void __cdecl SpriteScripts_Init() {
    if (g().ready) return;
    g().ready = 1;
    g().stepLimit = 50;
    new_table(100);
}

extern "C" void __cdecl SpriteScripts_Shutdown() {
    if (!g().ready) return;
    g().ready = 0;
    for (s32 slot = 0; slot < g().slots; ++slot) kill(slot);  // not ready: frees nothing
    env().free(g().scripts);
    g().scripts = nullptr;
    g().slots = 0;
}

extern "C" u32 __cdecl SpriteScripts_Update() {
    g().steps = 0;
    if (g().ready) {
        g().now = (u32)env().now();
        for (s32 slot = 1; slot < g().slots; ++slot) {
            u8 *script = g().scripts[slot];
            if (!script || (s32)g().now <= at<s32>(script, 0x7c)) continue;
            if (at<s32>(script, 0x18) == 4) {
                kill(slot);
            } else {
                g().steps += env().run(script);
                at<u32>(script, 0x7c) = at<u32>(script, 0x80) + g().now;
            }
        }
    }
    return g().steps;
}

extern "C" void __cdecl SpriteScripts_Draw() {
    if (g().ready) {
        env().drawBegin();
        for (s32 slot = 1; slot < g().slots; ++slot)
            if (u8 *script = g().scripts[slot]) env().draw(script);
    }
    env().drawEnd();
}

extern "C" void __cdecl SpriteScript_SetPosition(s32 slot, u32 x, u32 y, u32 z, u32 r15) {
    u8 *script = script_at(slot);
    if (!g().ready || !script) return;
    at<u32>(script, 0x88) = x;
    at<u32>(script, 0x8c) = y;
    at<u32>(script, 0x90) = z;
    at<u32>(script, 0xc0) = r15;
}

extern "C" void __cdecl SpriteScript_GetPosition(s32 slot, u32 *x, u32 *y, u32 *z, u32 *r15) {
    u8 *script = script_at(slot);
    if (!g().ready || !script) return;
    if (x) *x = at<u32>(script, 0x88);
    if (y) *y = at<u32>(script, 0x8c);
    if (z) *z = at<u32>(script, 0x90);
    if (r15) *r15 = at<u32>(script, 0xc0);
}

extern "C" void __cdecl SpriteScript_Kill(s32 slot) { kill(slot); }

extern "C" void __cdecl SpriteScript_SetReg(s32 slot, u32 index, u32 value) {
    u8 *script = script_at(slot);
    if (g().ready && script && index < 19) at<u32>(script, 0x84 + 4 * index) = value;
}

extern "C" void __cdecl SpriteScript_SetRegF(s32 slot, u32 index, u32 bits) {
    u8 *script = script_at(slot);
    if (g().ready && script && index < 19) at<u32>(script, 0x84 + 4 * index) = X87_FloatThrough(bits);  // fld; fstp
}

extern "C" void __cdecl SpriteScript_SetPeriod(s32 slot, u32 ms) {
    u8 *script = script_at(slot);
    if (g().ready && script) at<u32>(script, 0x80) = ms;
}

extern "C" void __cdecl SpriteScripts_Save(void *stream) {
    if (!g().ready) return;
    u32 result, value = kMagic;
    env().write(&result, stream, &value, 4, 0);
    value = kScriptSize;
    env().write(&result, stream, &value, 4, 0);
    env().write(&result, stream, &g().slots, 4, 0);
    env().write(&result, stream, g().scripts, 4u * (u32)g().slots, 0);
    for (s32 slot = 1; slot < g().slots; ++slot)
        if (u8 *script = g().scripts[slot]) env().write(&result, stream, script, kScriptSize, 0);
}

extern "C" u32 __cdecl SpriteScripts_Load(void *stream) {
    if (!g().ready) return 1;
    for (s32 slot = 0; slot < g().slots; ++slot) kill(slot);
    env().free(g().scripts);
    u32 result, magic, size;
    env().read(&result, stream, &magic, 4, 0);
    env().read(&result, stream, &size, 4, 0);
    if (magic != kMagic || size != kScriptSize) {
        new_table(100);
        return 1;
    }
    env().read(&result, stream, &g().slots, 4, 0);
    g().scripts = static_cast<u8 **>(env().alloc(4u * (u32)g().slots));
    env().read(&result, stream, g().scripts, 4u * (u32)g().slots, 0);
    g().scripts[0] = nullptr;
    for (s32 slot = 1; slot < g().slots; ++slot) {
        if (!g().scripts[slot]) continue;  // the saved pointers: nonzero where a script follows
        g().scripts[slot] = static_cast<u8 *>(env().alloc(kScriptSize));
        env().read(&result, stream, g().scripts[slot], kScriptSize, 0);
        at<u32>(g().scripts[slot], 0x14) = kCodeBase;
    }
    return 0;
}

extern "C" void __cdecl SpriteScript_Init(u8 *script, u32 code, u32 folder, u32 sprite, u32 anim, u32 x, u32 y,
                                          u32 z, u32 r17, u32 r18) {
    const s32 pc = (s32)(code - kCodeBase) >> 2;
    at<s32>(script, 8) = pc;
    at<s32>(script, 0xc) = pc;
    at<u32>(script, 0x14) = kCodeBase;
    at<s32>(script, 0x1c) = 20;
    at<s32>(script, 0x18) = 1;
    at<s32>(script, 0x70) = at<s32>(script, 0x74) = at<s32>(script, 0x78) = 0;
    u32 *r = &at<u32>(script, 0x84);
    r[1] = x, r[2] = y, r[3] = z;
    r[4] = r[5] = r[6] = 0;
    r[7] = 0xff;
    r[8] = r[9] = 0x3f800000;  // 1.0f
    r[10] = folder, r[11] = sprite, r[12] = anim;
    r[15] = r[16] = 0;
    r[17] = r17, r[18] = r18;
    at<u32>(script, 0xf8) = 0;
    at<u32>(script, 0x114) = 0;
    at<s32>(script, 0x80) = 62;
    at<s32>(script, 0x7c) = env().now() + at<s32>(script, 0x80);
    r[14] = Sprite_GetAnimFrameCount(sprite_of(folder, sprite), nullptr, anim);
}

extern "C" s32 __cdecl SpriteScript_Start(u32 code, u32 folder, u32 sprite, u32 anim, u32 x, u32 y, u32 z) {
    if (!g().ready) return 0;
    u32 r17 = 0, r18 = 0;
    u8 *script = static_cast<u8 *>(env().alloc(kScriptSize));
    if (script) {
        if (is_action(code)) {  // (and Debug_Printf, empty, of the folder's name)
            u32 index;
            Sprite_GetAction(sprite_of(folder, sprite), nullptr, code, &index, &anim, &r17, &r18);
            code = action_code(index);
        }
        SpriteScript_Init(script, code, folder, sprite, anim, x, y, z, r17, r18);
    }
    return insert(script);
}

extern "C" void __cdecl SpriteScript_Jump(s32 slot, u32 code) {
    u8 *script = script_at(slot);
    if (!g().ready || !script) return;
    if (is_action(code)) {
        const u8 *sprite = sprite_of(at<u32>(script, 0xac), at<u32>(script, 0xb0));
        u32 index, r17, r18;
        Sprite_GetAction(sprite, nullptr, code, &index, &at<u32>(script, 0xb4), &r17, &r18);
        code = action_code(index);
        at<u32>(script, 0xbc) = Sprite_GetAnimFrameCount(sprite, nullptr, at<u32>(script, 0xb4));
    }
    at<s32>(script, 0x1c) = 20;
    at<s32>(script, 0x70) = 0;
    const s32 pc = (s32)(code - at<u32>(script, 0x14)) >> 2;
    at<s32>(script, 8) = pc;
    at<s32>(script, 0x74) = at<s32>(script, 0x78) = 0;
    at<s32>(script, 0xc) = pc;
}

extern "C" u32 __cdecl SpriteScript_IsAt(s32 slot, u32 code) {
    if (!g().ready) return 0;
    u8 *script = script_at(slot);
    if (!script) return 0;
    if (is_action(code)) {
        u32 index, r17, r18;
        Sprite_GetAction(sprite_of(at<u32>(script, 0xac), at<u32>(script, 0xb0)), nullptr, code, &index,
                         &at<u32>(script, 0xb4), &r17, &r18);
        code = action_code(index);
    }
    return kCodeBase + 4u * at<u32>(script, 0xc) == code ? 1 : 0;
}

extern "C" s32 __cdecl SpriteScript_Clone(s32 slot, u32 x, u32 y, u32 z, u32 r8) {
    if (!g().ready) return 0;
    u8 *script = static_cast<u8 *>(env().alloc(kScriptSize));
    if (script) {
        std::memcpy(script, script_at(slot), kScriptSize);
        at<u32>(script, 0x88) = x;
        at<u32>(script, 0x8c) = y;
        at<u32>(script, 0x90) = z;
        at<u32>(script, 0xa4) = r8;
        at<u32>(script, 0xc4) = (at<u32>(script, 0xc4) & ~0x200u) | 0x50000;
    }
    return insert(script);
}

extern "C" void __cdecl SpriteScript_CopyFrom(s32 slot, s32 from) {
    static const u32 kCopied[] = {0x14, 0x8, 0xac, 0xb0, 0xb4, 0xc8, 0xcc, 0xa0, 0xa8, 0xc0, 0x114, 0xb8, 0xbc};
    u8 *to = script_at(slot), *src = script_at(from);
    for (u32 offset : kCopied) at<u32>(to, offset) = at<u32>(src, offset);
    at<u32>(to, 0xc4) = (at<u32>(to, 0xc4) & ~0x200u) | 0x50000;
}

extern "C" s32 __cdecl SpriteScript_FindShown(const u8 *drawn) {
    if (!g().ready) return 0;
    for (s32 slot = 1; slot < g().slots; ++slot) {
        u8 *script = g().scripts[slot];
        if (script && at<u32>(script, 0x114) && script + 0xd0 == drawn) return slot;
    }
    return 0;
}

extern "C" void __cdecl SpriteScripts_WrapSprites() {
    if (!g().ready) return;
    for (s32 slot = 1; slot < g().slots; ++slot) {
        if (u8 *script = g().scripts[slot]) {
            const s32 count = SpriteFolder_Count(at<u32>(script, 0xac));
            at<s32>(script, 0xb0) %= count;
        }
    }
}

extern "C" void __cdecl SpriteScripts_InitUnusedOne() { *reinterpret_cast<float *>(0x007b4a1c) = 1.0f; }

extern "C" void __cdecl SpriteScripts_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007b4a20) = 1.0f / 8388608.0f; }
