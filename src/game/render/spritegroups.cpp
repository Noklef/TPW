#include "spritegroups.h"

#include <cstdint>

#include "batchquad.h"
#include "quad.h"
#include "../gfx/texture.h"
#include "../math/mat4.h"
#include "../math/x87.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const SpriteGroupsEnv kPortEnv = {&Batch_DrawQuad};
const SpriteGroupsEnv *g_env = &kPortEnv;

// The constants, as the game's memory holds them: 1/1024, 1/2, 1/4096 (a place), 1/65536 (a turn).
const float &kSize = gSpritegroups_Size_701ac8;
const float &kHalf = gSpritegroups_701b28;
const float &kPlace = gSpritegroups_701b24;
const float &kTurn = gSpritegroups_Turn_701ad0;

const u32 kGroups = 0x78;

// One sprite as a quad: the spec's batch and UVs by its holder, its points about its place.
void Prepare(const u8 *sprite, u8 *spec, u8 *points) {
    const float size = (float)(X87Reg((double)at<u16>(sprite, 0x14)) * kSize * kHalf);
    float halfWidth = size;
    const float halfHeight = size;
    u8 *const holder = at<u8 *>(sprite, 0x10);
    if (holder == reinterpret_cast<u8 *>(-1)) {
        at<u32>(spec, 0x10) = 0;
        at<u32>(spec, 0x24) = 0;
        at<u32>(spec, 0x20) = 0;
        at<u32>(spec, 0x1c) = 0;
        at<u32>(spec, 0x18) = 0;
        const u32 alpha = at<u32>(sprite, 0xc) & 0xff000000;
        at<u32>(spec, 0xc) = alpha == 0 || alpha >= 0xf0000000 ? 0x1256 : 0x556;
        halfWidth = (float)(X87Reg(halfWidth) * global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x4));
    } else {
        const u8 *const texture = at<const u8 *>(holder, 0);
        if (texture) {
            at<u32>(spec, 0x10) = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
            at<u32>(spec, 0xc) = Texture_FormatBits(texture);
            at<float>(spec, 0x18) = (float)X87Reg(at<float>(texture, 0x28));
            at<float>(spec, 0x1c) = (float)X87Reg(at<float>(texture, 0x2c));
            at<float>(spec, 0x20) = (float)(X87Reg(at<float>(texture, 0x20)) + at<float>(spec, 0x18));
            at<float>(spec, 0x24) = (float)(X87Reg(at<float>(texture, 0x24)) + at<float>(spec, 0x1c));
            if (at<u32>(texture, 8) & 0x100000)
                at<u32>(spec, 0) |= 4;
        } else {
            at<u32>(spec, 0x10) = 0;
            at<u32>(spec, 0xc) = 0x1256;
            at<u32>(spec, 0x24) = 0;
            at<u32>(spec, 0x20) = 0;
            at<u32>(spec, 0x1c) = 0;
            at<u32>(spec, 0x18) = 0;
        }
        halfWidth = (float)(X87Reg(at<float>(holder, 8)) * halfWidth);
    }

    const u32 flags = at<u32>(sprite, 0x18);
    if (flags & 4)
        at<u32>(spec, 0xc) = 0x200522 + (flags & 0x2000 ? 0x30 : 0);
    if (flags & 0x20000)
        at<u32>(spec, 0xc) |= 0x20000;
    if (flags & 0x4000)
        at<u32>(spec, 0xc) |= 0x800;

    float from[3], to[4];
    for (u32 k = 0; k < 3; ++k)
        from[k] = (float)(X87Reg((double)at<s32>(sprite, k * 4)) * kPlace);
    if (flags & 0x40000) {
        to[0] = from[0];
        to[1] = from[1];
        to[2] = from[2];
        at<u32>(to, 0xc) = 0x3f800000;
    } else {
        Mat4_Transform(global<const float *>(0x0087b088), nullptr, to, from, 0);
    }

    at<float>(points, 0) = (float)(X87Reg(to[0]) - halfWidth);
    at<float>(points, 4) = (float)(X87Reg(to[1]) - halfHeight);
    at<float>(points, 0x10) = (float)(X87Reg(halfWidth) + to[0]);
    at<float>(points, 0x14) = at<float>(points, 4);
    at<float>(points, 0x20) = at<float>(points, 0x10);
    at<float>(points, 0x24) = (float)(X87Reg(halfHeight) + to[1]);
    at<float>(points, 0x30) = at<float>(points, 0);
    at<float>(points, 0x34) = at<float>(points, 0x24);
    for (u32 k = 0; k < 4; ++k) {
        at<float>(points, k * 0x10 + 8) = (float)X87Reg(to[2]);
        at<float>(points, k * 0x10 + 0xc) = (float)X87Reg(to[3]);
    }

    const u16 turn = at<u16>(sprite, 0x16);
    if (turn)
        Quad_TurnXY(reinterpret_cast<float *>(points), (float)(X87Reg((double)turn) * kTurn), nullptr);
}

}  // namespace

void SpriteGroups_SetEnv(const SpriteGroupsEnv *env) { g_env = env ? env : &kPortEnv; }

u32 __fastcall SpriteGroups_Draw(u8 *object, void *) {
    typedef s32(__cdecl * CountFn)(u32);
    typedef u8 *(__cdecl * ListFn)(u32);
    typedef void(__cdecl * ReleaseFn)(u8 *);
    const u32 frame = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x78), part = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x7c);
    if (at<u32>(object, 4) == frame && at<u32>(object, 8) == part)
        return 1;
    at<u32>(object, 4) = frame;
    at<u32>(object, 8) = part;

    // The spec, its points 0x38 on as in the original's frame.
    u8 spec[0x78] = {};
    u8 *const points = spec + 0x38;
    at<u8 *>(spec, 4) = points;
    for (u32 group = 0; group < kGroups; ++group) {
        const s32 count = at<CountFn>(object, 0x9c)(group);
        if (count <= 0 || !(at<u8>(object, 0xc + group) & 0x40))
            continue;
        u8 *const list = at<ListFn>(object, 0x94)(group);
        if (!list)
            continue;
        const u8 *sprite = list;
        for (s32 k = 0; k < count; ++k, sprite += 0x1c) {
            Prepare(sprite, spec, points);
            at<u32>(spec, 8) = at<u32>(sprite, 0xc);
            at<u32>(spec, 0x28) = at<u32>(sprite, 0x18) >> 15 & 2;
            if (!g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, spec, 0xffffffff))
                return 1;
        }
        at<ReleaseFn>(object, 0x98)(list);
    }
    return 1;
}
