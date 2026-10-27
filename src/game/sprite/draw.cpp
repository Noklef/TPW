// The sprite draw lists: reimplementation of 0x540ab0, 0x540b30, 0x542010 and 0x5408e0/0x5408f0.
#include "draw.h"

#include <cmath>
#include <cstring>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../ui/guimanager.h"
#include "../world/mapcell.h"
#include "script.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

s32 *counts() { return reinterpret_cast<s32 *>(0x00876490); }  // per cell
u32 **lists() { return reinterpret_cast<u32 **>(0x00875528); }  // per cell
s32 &overlayCount() { return global<s32>(0x00875864); }
u32 *&overlayList() { return global<u32 *>(0x0087692c); }
float &heading() { return global<float>(0x00876b24); }
u8 *sprite(u32 index) { return reinterpret_cast<u8 **>(0x00876b48)[index]; }  // gSprites
u32 folderFirst(u32 folder) { return global<u32>(0x00763f8c + 12u * folder); }

constexpr double kTwoPi = 6.2831854820251465;  // 0x700e68, double
constexpr float kNegTwoPi = -6.2831854820251465f, kHeadingBias = -0.39269909262657166f,
                kInvTwoPi = 0.15915493667125702f, kEighths = 8.0f;  // 0x700e70-0x700e7c

u32 ftol(double x) { return (u32)(long long)x; }  // __ftol: truncates, the low half
float as_float(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}
u32 bits_of(float f) {
    u32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

// The cell a position (float bits) falls in along one axis, 0-3.
s32 cell(u32 position, s32 size) {
    s32 c = (s32)ftol(as_float(position)) / 10 / size;
    if (c >= 3) c = 3;
    return c <= 0 ? 0 : c;
}

void __cdecl game_set_picked(u32 thing) {
    reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(0x00423b50)(reinterpret_cast<void *>(0x007b05a8),
                                                                          nullptr, thing);
}
const SpriteDrawEnv kGameEnv = {
    reinterpret_cast<double(__cdecl *)(u32, u32)>(0x004527f0),
    reinterpret_cast<u32(__cdecl *)()>(0x00532bd0),
    &game_set_picked,
};
const SpriteDrawEnv *g_env = &kGameEnv;
const SpriteDrawEnv &env() { return *g_env; }

}  // namespace

void SpriteDraw_SetEnv(const SpriteDrawEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" void __cdecl SpriteDraw_Begin() {
    for (u32 i = 0; i < 16; ++i) counts()[i] = 0;
    overlayCount() = 0;
    global<u32>(0x00877348) = 1;
    if (global<u8>(0x00790ab0) & 0x16) {
        heading() = (float)Matrix_Heading(global<const float *>(0x00790a98));
        return;
    }
    heading() = (float)std::fmod((double)global<float>(0x00790a38), kTwoPi);  // _CIfmod, exact
    if (!(heading() >= 0.0f)) heading() = (float)X87_Round((double)heading() - (double)kNegTwoPi);
}

extern "C" void __cdecl SpriteDraw_End() {
    if (global<u32>(0x00877348)) {
        for (u32 i = 0; i < 16; ++i) global<s32>(0x008751f4 + 0x24 * i) = counts()[i];
        global<s32>(0x00876424) = overlayCount();
    }
    const u32 thing = env().pick();
    env().setPicked(global<u32>(0x00877350));
    global<u32>(0x00877350) = thing;
}

extern "C" void __cdecl Sprite_DrawScript(u8 *s) {
    if (!at<u32>(s, 0x114)) return;
    const u32 frame = at<u32>(s, 0xb8);  // r13
    if (frame == 0xffffffffu || !global<u8>(0x00763f84)) return;
    const u32 folder = at<u32>(s, 0xac);  // r10
    u32 index = folderFirst(folder) + at<u32>(s, 0xb0);
    const s32 turn = at<s32>(s, 0xc0);  // r15
    const s32 anim = at<s32>(s, 0xb4);  // r12
    const u32 flags = at<u32>(s, 0xc4);  // r16
    const u32 alpha = at<u8>(s, 0xa0), r8 = at<u32>(s, 0xa4), r9 = at<u32>(s, 0xa8);
    const u32 x = at<u32>(s, 0x88), y = at<u32>(s, 0x8c), z = at<u32>(s, 0x90);

    u32 height;
    if (flags & 0x1000000) {
        height = X87_FloatThrough(y);
        at<u32>(s, 0xe8) = height;
        at<u32>(s, 0xe4) = x;
        at<u32>(s, 0xec) = z;
    } else {
        height = bits_of((float)X87_Add(env().height(x, z), (double)as_float(y)));
    }

    const u32 bucket = (u32)cell(z, global<s32>(0x0087614c)) + 4u * (u32)cell(x, global<s32>(0x008762b4));
    if (counts()[bucket] >= 0x800) return;

    index += (u32)(anim / 16);
    const s32 slot = anim % 16;  // the animation's entry; negative for a negative animation
    const u32 first = *reinterpret_cast<const u16 *>(sprite(index) + 0x220 + 4 * slot);
    const bool overlay = (flags & 0x40000) != 0;
    u32 offset = 0;
    if (!overlay) {
        const double eighths = X87_Round(
            X87_Round(X87_Round((double)heading() - (double)kHeadingBias) * kInvTwoPi) * kEighths);
        offset = 8 - (ftol(eighths) & 7);
    }
    global<u32>(0x0087734c) = offset;

    const u8 dirs = sprite(index)[0x223 + 4 * slot];
    u32 dir = 0;
    bool mirrored = false;
    if (dirs && !(flags & 0x2000000)) {
        dir = (offset + (u32)turn) & 7;
        if ((s32)dir > (s32)dirs - 1) {
            mirrored = true;
            dir = 8 - dir;
        }
    }
    const u8 perDir = sprite(index)[0x222 + 4 * slot];
    const u32 shown = perDir * dir + frame + first;

    at<u32>(s, 0xe0) = (flags & 0xffffff) | (folder == 5 ? 0x100000u : 0) | (mirrored ? 0x10u : 0);
    at<u32>(s, 0xe8) = height;
    at<u32>(s, 0xe4) = x;
    at<u32>(s, 0xec) = z;
    at<u32>(s, 0xfc) = r8;
    at<u32>(s, 0x100) = r9;
    at<u32>(s, 0xf4) = (alpha << 24) + 0xffffff;
    const u32 frames = *reinterpret_cast<const u32 *>(sprite(index));
    at<u32>(s, 0xf8) = *reinterpret_cast<const u32 *>(frames + 4) + shown * 24;
    if (flags & 0x200) {
        const s32 d = turn > (s32)dirs - 1 ? 8 - turn : turn;
        at<u8>(s, 0x10a) = (u8)((dirs << 4) + perDir);
        at<u32>(s, 0x104) = *reinterpret_cast<const u32 *>(sprite(index));
        at<u8>(s, 0x10b) = (u8)((d << 4) + (u8)frame);
        at<u16>(s, 0x108) = (u16)first;
    }

    u32 *const record = reinterpret_cast<u32 *>(s + 0xd0);
    if (overlay) {
        overlayList()[overlayCount()] = (u32)(uintptr_t)record;
        ++overlayCount();
    } else {
        lists()[bucket][counts()[bucket]] = (u32)(uintptr_t)record;
        ++counts()[bucket];
    }
}

extern "C" u32 __cdecl SpriteDraw_Pick() {
    const u32 tile = (u32)global<u16>(0x007b05cc) - 1;
    const u32 row = tile >> 7, column = tile & 0x7f;
    u16 x, y;
    Gui_GetMousePos(&x, &y);
    global<float>(0x008bd4f0) = (float)(X87Reg((double)(s16)x) * global<float>(0x00700d18) - global<float>(0x00700d10));
    global<float>(0x008bd4f4) = (float)(X87Reg((double)(s16)y) * global<float>(0x00700d1c) - global<float>(0x00700d10));
    u8 *const picked = global<u8 *>(0x008bd4fc);
    if (!picked)
        return 0;
    const s32 slot = SpriteScript_FindShown(picked);
    if (!slot)
        return 0;
    for (u32 pair = 0x0081ed44; pair < 0x0081ed8c; pair += 8) {
        const s32 cx = global<s32>(pair - 4) + (s32)column;
        const s32 cy = global<s32>(pair) + (s32)row;
        if (!Map_InBounds(cx, cy))
            continue;
        const u32 cell = ((u32)cy << 7) + (u32)cx + 1;
        for (u16 thing = at<u16>(global<u8 *>(0x007cf83c), (cell & 0xffff) * 0x44 + 0x2b8); thing;) {
            u8 *const record = global<u8 *>(0x007cfb90 + 0x14 * (u32)thing);
            const u8 kind = record[2];
            if ((kind == 1 || kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7 || kind == 0x12) && at<s32>(record, 0xc) == slot)
                return (u32)(uintptr_t)record;
            thing = at<u16>(record, 0xa);
        }
    }
    return 0;
}

extern "C" void __cdecl SpriteDraw_Enable() { global<u8>(0x00763f84) = 1; }
extern "C" void __cdecl SpriteDraw_Disable() { global<u8>(0x00763f84) = 0; }
