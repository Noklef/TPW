#include "scriptkill.h"

#include <cstdint>

#include "../coaster/ridebounds.h"
#include "../core/soundset.h"
#include "../files/file.h"
#include "../math/x87.h"
#include "../model/parts.h"
#include "../model/partstate.h"
#include "../render/particles.h"
#include "../world/flyingrides.h"
#include "../world/objectdefs.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

u8 *ride(u8 *c) { return global<u8 *>(0x007a4610 + 4 * at<u32>(c, 0xc8)); }

// The ride's part of the key (mask 0x80) released, when found (the ride read again for each call).
void release(u8 *c, u32 key) {
    const u32 part = Model_FindPartByKey4(ride(c), 0x80, key);
    if (part != 0xffffffffu) PartStates_Release(at<u8 *>(ride(c), 8), part);
}

// The record of the id (null for none, or without the system: that prints, left out).
RsseContext *find(s32 id) {
    if (!global<u32>(0x008791a0)) return nullptr;
    RsseContext *v = global<RsseContext *>(0x008791b0);
    while (v && v->id != id) v = v->next;
    return v;
}

}  // namespace

extern "C" {

RsseContext *__cdecl RSSE_FreeContext(RsseContext *ctx, u32 flags) {
    if (!ctx) return nullptr;
    u8 *const c = reinterpret_cast<u8 *>(ctx);
    if (at<u8>(c, 0xb9)) Sound_Set803ad0(0);
    if (at<u32>(c, 0xc8)) FlyingRide_Free(at<s32>(c, 0x9c));
    if (const u32 system = at<u32>(c, 0xcc)) {
        ParticleSystem_SetAge(system, 0xfffffffeu);
        at<u32>(c, 0xcc) = 0;
    }
    if (const u32 sound = at<u32>(c, 0xd0)) Sound_Stop(sound);
    if (at<u32 *>(c, 0x30)) {
        for (s32 i = 0; i < at<s32>(c, 0x4c); ++i) {
            if (!at<u32 *>(c, 0x30)[i]) continue;
            if (flags & 4) release(c, (u32)i + 1);
            at<u32 *>(c, 0x30)[i] = 0;
        }
    }
    if (at<u8 *>(c, 0x2c)) {
        for (s32 i = 0; i < at<s32>(c, 0x7c); ++i) {
            u8 *const w = at<u8 *>(c, 0x2c) + i * 0x20;
            if (at<u16>(w, 0x16) != 4 || at<u16>(w, 0x18) != 2) continue;
            if (flags & 4) release(c, (u32)(s32)at<s16>(w, 2));
            at<u16>(w, 0x18) = 0;
        }
    }
    --global<s32>(0x008791ac);
    if (ctx->prev) ctx->prev->next = ctx->next;
    if (ctx->next) ctx->next->prev = ctx->prev;
    for (u32 off = 0x18; off <= 0x38; off += 4)
        if (at<void *>(c, off)) Mem_GlobalFree(at<void *>(c, off));
    for (u8 *node = at<u8 *>(c, 0xb0); node;) {
        switch (at<u32>(node, 8)) {
        case 1:
        case 2:
            ParticleSystem_SetAge(at<u32>(node, 0xc), 0xfffffffeu);
            break;
        case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
            Sound_Forward6(at<u32>(node, 0xc));
            break;
        default:   // ("Unknown object type": Debug_Printf, left out)
            break;
        }
        if (at<u8 *>(node, 4)) at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
        if (at<u8 *>(node, 0)) at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
        u8 *const head = at<u8 *>(c, 0xb0);
        u8 *const next = at<u8 *>(node, 0);
        if (node == head) at<u8 *>(c, 0xb0) = next;
        Mem_GlobalFree(node);
        --global<s32>(0x008791b4);
        node = next;
    }
    RsseContext *const first = global<RsseContext *>(0x008791b0);
    RsseContext *const next = ctx->next;
    if (ctx == first) global<RsseContext *>(0x008791b0) = next;
    Mem_GlobalFree(ctx);
    return next;
}

void __cdecl RSSE_KillScript(s32 id, u32 flags) {
    if (!global<u32>(0x008791a0)) return;   // ("Cant kill a script - system is not initialised": Debug_Printf, left out)
    RsseContext *ctx = global<RsseContext *>(0x008791b0);
    while (ctx && ctx->id != id) ctx = ctx->next;
    if (!ctx) return;
    u8 *const c = reinterpret_cast<u8 *>(ctx);
    if (const u16 slot = at<u16>(c, 0xac)) {
        // (with no thing table [0x7cfb90] it prints first: Debug_Printf, left out)
        u8 *const thing = global<u8 *>(0x007cfb90 + slot * 0x14u);
        const s32 effect = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x64);
        if (effect && (flags & 2)) {
            float box[6], x = 0.0f, y = 0.0f;
            X87Reg z = 0.0;   // (0x700fd8)
            if (at<u32>(c, 0xc8) && Ride_Bounds(at<s32>(c, 0xc8), box)) {
                const float half = global<float>(0x00700fd0);
                x = (float)((X87Reg(box[0]) + box[3]) * half);
                z = (X87Reg(box[2]) + box[5]) * half;
                y = X87_Through(at<float>(at<u8 *>(at<u8 *>(at<u8 *>(ride(c), 8), 4), 0x78), 0x44));
            }
            const float scale = global<float>(0x00700fdc);
            const s32 iz = Ftol(z * scale);
            const s32 iy = Ftol(X87Reg(y) * scale);
            const s32 ix = Ftol(X87Reg(x) * scale);
            const s32 system = Particle_Spawn(effect, ix, iy, iz);
            if (at<u32>(c, 0xc8) && Ride_Bounds(at<s32>(c, 0xc8), box)) {
                const X87Reg dz = X87Reg(box[5]) - box[2];
                const X87Reg dy = X87Reg(box[4]) - box[1];
                const float dx = (float)(X87Reg(box[3]) - box[0]);
                const s32 jz = Ftol(dz * global<float>(0x00701050));
                const s32 jy = Ftol(dy * global<float>(0x00701054));
                const s32 jx = Ftol(X87Reg(dx) * global<float>(0x00701050));
                ParticleSystem_SetVectorB((u32)system, jx, jy, jz);
            }
        }
        if (const u32 occluder = at<u32>(c, 0xd4)) {
            Sound_RemoveOccluder(occluder);
            at<u32>(c, 0xd4) = 0;
        }
    }
    if (at<s32>(c, 0x14)) RSSE_FreeContext(find(at<s32>(c, 0x14)), flags);
    if (ctx->child_id) RSSE_FreeContext(find(ctx->child_id), flags);
    if (at<s32>(c, 0x10)) {
        RsseContext *const parent = find(at<s32>(c, 0x10));
        *reinterpret_cast<volatile s32 *>((uintptr_t)parent + 0xc) = 0;   // (sic: a parent not in the list faults, the write going to 0xc)
    }
    RSSE_FreeContext(ctx, flags);
}
}
