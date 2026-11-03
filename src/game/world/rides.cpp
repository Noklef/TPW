#include "rides.h"

#include <cstdint>
#include <cstring>

#include "../coaster/helpers.h"
#include "../coaster/registry.h"
#include "../math/x87.h"
#include "../model/bounds.h"
#include "../model/gridfit.h"
#include "../model/highlight.h"
#include "../model/holder.h"
#include "../model/objmodel.h"
#include "../model/partstate.h"
#include "../model/stripanim.h"
#include "../model/worldverts.h"
#include "../motion/animator.h"
#include "../motion/normals.h"
#include "../motion/placement.h"
#include "../terrain/land.h"
#include "camera.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 **instances() { return reinterpret_cast<u8 **>(0x007a4610); }

const RidesEnv kGameEnv = {&Camera_StopFollowing, &CoasterLink_Destroy, &Instance_Clone, &Animator_Reset,
                           &Animator_HasMotion, &Animator_Play, &Terrain_FootprintHeight, &Animator_Place, &Model_FitToGrid,
                           &ObjModel_BlendTerrainTiles, &Model_TransformBounds, &StripAnim_FitGround, &Holder_AddToGrid, &Holder_Wake,
                           &PartStates_Update, &CoasterLink_Create, &Animator_SetPlaying, &Highlight_Flash, &Model_UpdateNormals};
const RidesEnv *g_env = &kGameEnv;

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

// The prototype's footprint bits 0x38 (an edge) as the turn sees them (0x463970 / 0x4639e0 / 0x4639a8: 0, 8, 0x10, 0x20 -> ...; 0x18 and
// past 0x20 nothing).
u32 turned_edge(s32 angle, u32 edge) {
    switch (angle) {
    case 0: return edge;
    case 90: return edge == 0 ? 8 : edge == 8 ? 0x10 : edge == 0x10 ? 0x20 : 0;
    case 180: return edge == 0 ? 0x10 : edge == 8 ? 0x20 : edge == 0x20 ? 8 : 0;
    case 270: return edge == 0 ? 0x20 : edge == 0x10 ? 8 : edge == 0x20 ? 0x10 : 0;
    default: return 0;
    }
}

}  // namespace

extern "C" {

void Rides_SetEnv(const RidesEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Ride_Destroy(u32 ride) {
    if (ride == 0) return;
    u8 *const instance = instances()[ride];
    if (!instance) return;
    if (Highlight_Current() == instance) Highlight_Clear();
    if (instance == global<u8 *>(0x007afda0)) global<u8 *>(0x007afda0) = nullptr;
    if (Camera_IsFollowing(reinterpret_cast<const void *>((uintptr_t)ride))) g_env->stopFollowing();
    if (at<u32>(instance, 4) & 0x4000000u) Highlight_Unflash(instance);
    g_env->forgetCoaster(ride);
    Instance_Dispose(instance);
    instances()[ride] = nullptr;
    const u32 slot = ride - 1;
    u32 holes = global<u32>(0x007a3ebc);
    u32 count = global<u32>(0x007a4084);
    if (holes == 0) {
        --count;
        if (slot == count) {
            global<u32>(0x007a4084) = count;
            --global<u32>(0x007aedf4);
        } else {
            global<u32>(0x007a3ebc) = 1;
            global<u32>(0x007aedf4) = slot;
            global<u32>(0x007a4084) = count;
        }
        return;
    }
    if (slot == holes + count - 1) {
        global<u32>(0x007a4084) = --count;
        for (u8 **entry = &instances()[count + holes]; *entry == nullptr; --entry) {
            if (--holes == 0) break;
        }
        global<u32>(0x007a3ebc) = holes;
        return;
    }
    global<u32>(0x007a3ebc) = holes + 1;
    global<u32>(0x007a4084) = count - 1;
    if (slot < global<u32>(0x007aedf4)) global<u32>(0x007aedf4) = slot;
}

u32 __cdecl Ride_Create(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32, u32, s32 ride, u32 loaded) {
    if (type == 0) return 0;
    u8 *proto = reinterpret_cast<u8 *const *>(0x007ae6a8)[type];
    if (!proto) return 0;
    const u32 cloneFlags = (flags & 0x800) ? 0x231f : 0x211f;
    if ((flags & 0x400) && at<u8 *>(proto, 0xd0)) proto = at<u8 *>(proto, 0xd0);
    u8 *inst = nullptr;
    if (g_env->clone(proto, 0, cloneFlags, &inst, mask) != 1) return 0;
    at<u32>(inst, 4) &= ~1u;
    at<u32>(inst, 0xe4) = flags;
    at<u32>(inst, 0xd4) = d4;
    u32 slot;
    if (ride == -1) {
        const u32 hole = global<u32>(0x007aedf4), count = global<u32>(0x007a4084);
        slot = hole;
        instances()[hole + 1] = inst;
        if (hole == count) {
            global<u32>(0x007aedf4) = hole + 1;
            global<u32>(0x007a4084) = count + 1;
        } else {
            const u32 end = global<u32>(0x007a3ebc) + count;
            for (u32 i = hole + 1; i <= end; ++i) {
                if (!instances()[i + 1]) {
                    global<u32>(0x007aedf4) = i;
                    break;
                }
            }
            global<u32>(0x007a3ebc) -= 1;
            global<u32>(0x007a4084) = count + 1;
        }
    } else {
        slot = (u32)ride - 1;
        instances()[ride] = inst;
    }
    if (flags & 1) at<u32>(inst, 4) |= 4;
    const u32 two = flags & 2;
    if (two) at<u32>(inst, 4) |= 0x4000;
    if (flags & 0x80) at<u32>(inst, 4) |= 0x8000;
    g_env->reset(inst, 1, 0, 0);
    if (g_env->hasMotion(&inst, 0, 0)) {
        g_env->play(inst, 0, 0, 0, 1.0f, 0);
        g_env->play(inst, 0xd, 0, 0, 1.0f, 0);
    }
    s32 x = 0, y = 0, cx = 0, cy = 0;   // (cx, cy and the height: uninitialised in the original without flags 0x26)
    u32 height = 0;
    if (flags & 0x26) {
        x = (s32)desc[0];
        y = (s32)desc[1];
        const s32 angle = (s32)desc[2];
        const u32 size = desc[3];
        at<u32>(inst, 0xe8) = desc[0];
        at<u32>(inst, 0xec) = desc[1];
        const u8 *world = global<u8 *>(0x007a0854);
        if (world && at<u8 *>(world, 0x6c))
            Terrain_RotateFootprint(&x, &y, angle, &cx, &cy, at<s32>(inst, 0xf0), at<s32>(inst, 0xf4));
        if (size && (flags & 8)) {
            height = bits((float)g_env->footprintHeight(cx, cy, reinterpret_cast<const u8 *>((uintptr_t)size), at<s32>(inst, 0xf0),
                                                        at<s32>(inst, 0xf4), flags, angle));
        } else {
            const u8 *map = at<u8 *>(global<u8 *>(0x007a0854), 0x6c);
            const u32 i = (at<u32>(map, 0x18) + 1) * (u32)y + (u32)x;
            height = X87_FloatThrough(at<u32>(at<u8 *>(map, 0x28), i * 4));
        }
        g_env->place(inst, cx, cy, x, y, angle, height, flags, at<s32>(inst, 0xf0), at<s32>(inst, 0xf4));
    }
    if (flags & 8) {
        g_env->fitToGrid(at<u8 *>(inst, 8), global<u8 *>(0x007a0854), height, 0);
        if (global<u32>(0x007a1a8c) & 0x4000) {
            u8 *obj = at<u8 *>(inst, 8);
            if (at<u8>(obj, 0) & 0x40) {
                const u8 *map = at<u8 *>(global<u8 *>(0x007a0854), 0x6c);
                const u8 *foot = at<u8 *>(at<u8 *>(inst, 0), 0xcc);
                const s32 angle = at<s32>(obj, 0x10);
                const u16 *cell = at<u16 *>(obj, 0x18);
                const u8 *root = at<u8 *>(at<u8 *>(obj, 4), 0x78) + 0x10;
                u32 bx = (u32)(long long)(X87Reg(at<float>(root, 0x30)) / at<float>(map, 0x10)).v;   // __ftol
                u32 by = (u32)(long long)(X87Reg(at<float>(root, 0x38)) / at<float>(map, 0x14)).v;
                if (angle == 90) {
                    --by;
                } else if (angle == 180) {
                    --bx;
                    --by;
                } else if (angle == 270) {
                    --bx;
                }
                u32 cellX = two, cellY = two;   // sic: any other angle
                for (u32 row = 0; row < at<u16>(foot, 0xa); ++row) {
                    for (u32 col = 0; col < at<u16>(foot, 8); ++col, cell += 2) {
                        switch (angle) {
                        case 0: cellX = bx + col, cellY = by + row; break;
                        case 90: cellX = bx + row, cellY = by - col; break;
                        case 180: cellX = bx - col, cellY = by - row; break;
                        case 270: cellX = bx - row, cellY = by + col; break;
                        default: break;
                        }
                        const u16 v = cell[0];
                        if (!(v & 0x80)) continue;
                        u8 *p = at<u8 *>(map, 0x2c) + (at<u32>(map, 0x18) * cellY + cellX) * 4u;
                        const u32 edge = turned_edge(angle, v & 0x38u);
                        at<u16>(p, 0) = (u16)((p[0] & 6) | (v & 0x40) | edge | 0x281);
                        at<u16>(p, 2) = cell[1];
                    }
                }
                const u32 from = at<u32>(inst, 0xc0), to = at<u32>(inst, 0xc4);
                g_env->blendTiles(at<u8 *>(at<u8 *>(inst, 8), 0x1c), nullptr, (s32)(from >> 16), (s32)(from & 0xffff), (s32)(to >> 16),
                                  (s32)(to & 0xffff));   // (0: the original's Debug_Printf, left out)
            }
        }
    }
    if (two) g_env->transformBounds(at<u8 *>(at<u8 *>(inst, 8), 4), at<u8 *>(at<u8 *>(inst, 8), 4));
    if (flags & 4) g_env->fitGround(inst);
    u8 *obj = at<u8 *>(inst, 8);
    u8 *frame = at<u8 *>(obj, 0x2c);
    const s32 turn = at<s32>(obj, 0x10);
    if (frame) {
        u8 *out = global<u8 *>(0x007aecd4);
        u32 found = 0;
        const u8 *group = at<u8 *>(frame, 0x6c);   // (0x10 bytes each: +4 the first triangle, +6 the count)
        for (u32 groups = at<u16>(frame, 0x5a); groups; --groups, group += 0x10) {
            u8 *tri = at<u8 *>(frame, 0x70) + at<u16>(group, 4) * 8u;
            for (u32 n = at<u16>(group, 6); n; --n, tri += 8) {
                float v0[3], v1[3], v2[3], a[3], b[3], normal[3];
                WorldVerts_Fetch(at<u8 *>(frame, 0x60), at<u16>(at<u8 *>(frame, 0x94), at<u16>(tri, 2) * 2u), reinterpret_cast<u32 *>(v0));
                WorldVerts_Fetch(at<u8 *>(frame, 0x60), at<u16>(at<u8 *>(frame, 0x94), at<u16>(tri, 4) * 2u), reinterpret_cast<u32 *>(v1));
                WorldVerts_Fetch(at<u8 *>(frame, 0x60), at<u16>(at<u8 *>(frame, 0x94), at<u16>(tri, 6) * 2u), reinterpret_cast<u32 *>(v2));
                for (int k = 0; k < 3; ++k) a[k] = (float)(X87Reg(v0[k]) - v1[k]);
                for (int k = 0; k < 3; ++k) b[k] = (float)(X87Reg(v0[k]) - v2[k]);
                Vec3_Cross(a, b, normal);
                Vec3_Normalize(normal);
                Vec3_TurnY(normal, turn);
                if (!(X87Reg(normal[1]).v >= (double)0.999f)) {   // 0x6fe71c (below or unordered)
                    tri[1] |= 0x80;
                    std::memcpy(out, normal, 12);
                    out += 12;
                    ++found;
                }
            }
        }
        if (found) {
            const u8 *f = at<u8 *>(obj, 0x2c);
            at<u8 *>(obj, 0x30) = at<u8 *>(f, 0x64) + at<u16>(f, 0x58) * 12u;
        }
    }
    if (flags & 0x100) {
        if (at<u32>(inst, 4) & 0x4000)
            g_env->addToGrid(at<u8 *>(inst, 8), at<u32>(inst, 0xc0), at<u32>(inst, 0xc4));
        else
            g_env->wake(at<u8 *>(inst, 8));
    }
    obj = at<u8 *>(inst, 8);
    if (at<u32>(obj, 0x28) && !(at<u32>(inst, 4) & 0x20000000)) g_env->partStates(obj);
    if ((flags & 0x200) && Table7ae6a8_Bit18(type)) g_env->createLink(type, slot + 1, (u32)cx, (u32)cy, loaded, a5);
    at<u8 *>(at<u8 *>(at<u8 *>(inst, 8), 4), 0xb4) = inst;
    if (u8 *blend = at<u8 *>(at<u8 *>(inst, 8), 0x1c)) at<u32>(blend, 0x140) = 0x00463030;
    if (at<u32>(inst, 4) & 0x1000000) g_env->setPlaying(inst);
    if (at<u32>(inst, 4) & 0x2000000) g_env->flash(inst);
    u8 *model = at<u8 *>(at<u8 *>(inst, 8), 4);
    u8 *entry = at<u8 *>(model, 0x70);
    for (u32 n = at<u16>(model, 0x44); n; --n, entry += 0xa0) at<u32>(entry, 0) |= 0x10000;
    at<u32>(model, 0x30) |= 0x40000;
    g_env->updateNormals(at<u8 *>(at<u8 *>(inst, 8), 4));
    return slot + 1;
}
}
