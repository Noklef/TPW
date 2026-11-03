#include "camera.h"

#include <cmath>
#include <cstdint>

#include "../core/gameclock.h"
#include "../math/mat4.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/highlight.h"
#include "../coaster/registry.h"
#include "../model/holder.h"
#include "../model/parts.h"
#include "../model/partstate.h"
#include "../render/state.h"
#include "../render/viewquad.h"
#include "../sprite/loading.h"
#include "../terrain/procedural.h"
#include "mapcell.h"
#include "mousemode.h"
#include "person.h"
#include "things.h"

namespace {

template <typename T = u32> T &global(u32 address) { return *reinterpret_cast<T *>(address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void __fastcall SetChat(void *, void *, u32 on) { ChatButtons_SetChat(on); }

const CameraEnv kGameEnv = {&Highlight_Suspend,     &SetChat,           &Terrain_ProceduralLeave, &Terrain_ProceduralEnter,
                            &Holder_AddToGrid,      &Holder_Wake,       &Model_FindPartByKey4,    &PartStates_Update,
                            &Sprites_UseGroundFrames, &Sprites_UseParkFrames, &Camera_FollowThing, &Camera_SnapToRide, &Camera_SetMode};
const CameraEnv *g_env = &kGameEnv;

// The model parts of a ride's instance (+8 -> +4) marked by their flags: face bit 0x8000 set by set, else cleared by clear; with check
// and none of 0x16 in gCameraFlags' byte (read for each part) every face cleared.
void mark_parts(const u8 *inst, u32 set, u32 clear, bool check) {
    u8 *const model = at<u8 *>(at<u8 *>(inst, 8), 4);
    const u32 *part = at<u32 *>(model, 0x7c);
    for (u32 j = 0; j < at<u16>(model, 0x48); ++j, part += 5) {
        const u32 k = j + at<u16>(model, 0x46);
        const u32 n = at<u16>(model, 0x44);
        u32 *face = k < n ? reinterpret_cast<u32 *>(at<u8 *>(model, 0x70) + k * 0xa0) : reinterpret_cast<u32 *>(at<u8 *>(model, 0x74) + (k - n) * 0x58);
        if (check && !(global<u8>(0x00790ab0) & 0x16)) {
            *face &= ~0x8000u;
        } else if (*part & set) {
            *face |= 0x8000;
        } else if (*part & clear) {
            *face &= ~0x8000u;
        }
    }
}

}  // namespace

extern "C" {

void __cdecl Camera_InitUnusedOne() { global<float>(0x007909e8) = 1.0f; }

void __cdecl Camera_InitUnusedEpsilon() { global<float>(0x00790a94) = 1.0f / 8388608.0f; }

void __cdecl Camera_Init7909ec() { global<float>(0x007909ec) = 110.0f; }

void __cdecl Camera_InitYaw() { global<float>(0x00790a38) = 0.0f; }

void __cdecl Camera_Init7909e0() { global<float>(0x007909e0) = 0.0f; }

void __cdecl Camera_InitPoint() {
    global<float>(0x007908f0) = 475.0f;
    global<float>(0x007908f4) = 0.0f;
    global<float>(0x007908f8) = 175.0f;
}

u32 __cdecl Camera_IsFollowing(const void *thing) {
    const void *const followed = global<const void *>(0x00790a44);
    return (global<u8>(0x00790ab0) & 4) && global(0x00790988) && followed && followed == thing;
}

u32 __cdecl Camera_MatricesClose(const float *a, const float *b) {
    const auto diff = [a, b](u32 i) { return X87Reg(std::fabs((X87Reg(a[i]) - b[i]).v)); };
    // (|d1| + |d0|), then for each pair (j, k): (|dj| + stored) + |dk|.
    X87Reg sum = diff(1) + diff(0);
    const u32 pairs[5][2] = {{2, 4}, {5, 6}, {8, 9}, {10, 12}, {13, 14}};
    for (const auto &pair : pairs) sum = diff(pair[0]) + (float)sum + diff(pair[1]);
    return !(sum.v >= global<float>(0x0074c980));  // fcomp: NaN counts as less
}

const void *__cdecl Camera_Followed() {
    return (global<u8>(0x00790ab0) & 4) && global(0x00790988) ? global<const void *>(0x00790a44) : nullptr;
}

u8 *__cdecl Camera_PartFace(u8 *const *instance, const u32 *part) {
    const u8 *const model = *reinterpret_cast<u8 *const *>(*instance + 4);
    const u32 index = *reinterpret_cast<const u16 *>(model + 0x46) + *part;
    const u32 first = *reinterpret_cast<const u16 *>(model + 0x44);
    return index < first ? *reinterpret_cast<u8 *const *>(model + 0x70) + index * 0xa0
                         : *reinterpret_cast<u8 *const *>(model + 0x74) + (index - first) * 0x58;
}

void __cdecl Camera_SetView(const u32 *x, const u32 *z, const u32 *zoom, const u32 *yaw, const u32 *other) {
    global(0x00790a88) = 0;
    global(0x00790a8c) = 0;
    global(0x00790a90) = 0;
    if (x) {
        global(0x007908f0) = *x;
        global(0x007908f8) = *z;  // sic: tests x again, so z is read whenever x is given
    }
    if (zoom) global(0x007909ec) = *zoom;
    if (yaw) global(0x00790a38) = *yaw;
    if (other) global(0x0074c9bc) = *other;
}

void __cdecl Camera_CentreOnCell(const u16 *cell) {
    const u32 v = static_cast<u32>(*cell) - 1;
    const float x = static_cast<float>(static_cast<s32>((v & 0x7f) * 10));
    const float z = static_cast<float>(static_cast<s32>((v >> 7) * 10));
    Camera_SetView(reinterpret_cast<const u32 *>(&x), reinterpret_cast<const u32 *>(&z), nullptr, nullptr, nullptr);
}

void __cdecl Camera_CentreOnThing(u32 handle) {
    const u16 h = static_cast<u16>(handle);
    if (!h) return;
    const u8 *const thing = *reinterpret_cast<const u8 *const *>(0x007cfb90 + 20 * static_cast<u32>(h));
    if (!thing) return;
    const u16 cell = static_cast<u16>(thing[7] * 128 + thing[5] + 1);
    Camera_CentreOnCell(&cell);
}

void __cdecl Camera_CentreOnSprite(const u8 *sprite) {
    if (!sprite) return;
    const s32 elapsed = *reinterpret_cast<const s32 *>(0x008786bc) - *reinterpret_cast<const s32 *>(0x00878a1c);
    const float t = (float)(X87Reg((double)elapsed) * X87Reg((double)*reinterpret_cast<const float *>(0x006ff16c)));
    float x, z;
    u32 height, frame;
    Sprite_InterpolatedPos(sprite, nullptr, t, 1, &x, &height, &z, &frame);
    Camera_SetView(reinterpret_cast<const u32 *>(&x), reinterpret_cast<const u32 *>(&z), nullptr, nullptr, nullptr);
}

void __cdecl Camera_SetFlag80(u32 on) {
    if (on)
        global(0x00790ab0) |= 0x80;
    else
        global(0x00790ab0) &= ~0x80u;
}

u32 __cdecl Camera_SetControl100() {
    global(0x00790aac) |= 0x100;
    return 1;
}

u32 __cdecl Camera_ToggleControl200() {
    global(0x00790aac) ^= 0x200;
    return 1;
}

void __cdecl Camera_ClampCell(s32 *x, s32 *z) {
    if (*x < 0) *x = 0;
    if (*z < 0) *z = 0;
    if (*x >= global<s32>(0x007a0314)) *x = global<s32>(0x007a0314) - 1;
    if (*z >= global<s32>(0x007a0374)) *z = global<s32>(0x007a0374) - 1;
}

void __cdecl Camera_CellOf(const float *position, s32 *x, s32 *z) {
    if (x) *x = (s32)(long long)(X87Reg(position[0]) * 0.100000001f).v;  // 0x6fdd58, __ftol
    if (z) *z = (s32)(long long)(X87Reg(position[2]) * 0.100000001f).v;
}

void __cdecl Camera_PackCellFractions(u32 x, u32 z, u8 *outX, u8 *outZ) {
    const auto pack = [](u32 bits, u8 *out) {
        const X87Reg tenths = X87Reg(reinterpret_cast<const float &>(bits)) * 0.100000001f;  // 0x700184
        const s32 whole = (s32)(long long)tenths.v;  // __ftol
        out[1] = (u8)whole;
        out[0] = (u8)(s32)(long long)((tenths - X87Reg((double)(whole & 0xff))) * 256.0f).v;  // 0x700188
    };
    pack(x, outX);
    pack(z, outZ);
}

void __cdecl Camera_CentreOnGlobalThing() {
    const u8 *const thing = global<const u8 *>(0x007c2658);
    u16 handle;
    Thing_GetHandle(thing, nullptr, &handle);
    // (the high word what ECX held: the thing pointer's)
    Camera_CentreOnThing((static_cast<u32>(reinterpret_cast<uintptr_t>(thing)) & 0xffff0000u) | handle);
}

void __cdecl Camera_ToggleWalkMode() {
    if (!(global<u8>(0x00790ab0) & 0x16)) {
        s32 cx = (s32)(long long)(X87Reg(global<float>(0x007a1c70)) * 0.100000001f).v;  // 0x6fdd58, __ftol
        s32 cz = (s32)(long long)(X87Reg(global<float>(0x007a1c78)) * 0.100000001f).v;
        if (cx < 0) cx = 0;
        if (cz < 0) cz = 0;
        if (cx >= global<s32>(0x007a0314)) cx = global<s32>(0x007a0314) - 1;
        if (cz >= global<s32>(0x007a0374)) cz = global<s32>(0x007a0374) - 1;
        const u32 id = (((u32)cz << 7) + cx + 1) & 0xffff;
        const u8 *const cell = *global<u8 **>(0x007cf6ec) + id * 0x44 + 0x294;
        if (!MapCell_IsType1e(cell) && !MapCell_IsType1(cell) && !MapCell_IsType3Or9(cell) && !MapCell_IsType0(cell)) return;
    }
    g_env->setMode(2, 1, 0, 0, nullptr, 0);
    u32 *const point = &global<u32>(0x007908f0);
    u32 *const saved = &global<u32>(0x00790ad0);
    if (global<u8>(0x00790ab0) & 0x16) {
        saved[0] = point[0];
        saved[1] = point[1];
        saved[2] = point[2];
        global<u32>(0x00790a9c) = global<u32>(0x00790a38);
        point[0] = global<u32>(0x007a1c70);
        point[1] = 0;
        point[2] = global<u32>(0x007a1c78);
        g_env->useGround();
    } else {
        point[0] = saved[0];
        point[1] = saved[1];
        point[2] = saved[2];
        global<u32>(0x00790a38) = global<u32>(0x00790a9c);
        g_env->usePark();
    }
}

void __cdecl Camera_SnapToRide() {
    float &x = global<float>(0x007908f0);
    float &z = global<float>(0x007908f8);
    s32 cx = (s32)(long long)(X87Reg(x) * 0.100000001f).v;  // 0x6fdd58, __ftol
    s32 cz = (s32)(long long)(X87Reg(z) * 0.100000001f).v;
    if (cx < 0) cx = 0;
    if (cz < 0) cz = 0;
    if (cx >= global<s32>(0x007a0314)) cx = global<s32>(0x007a0314) - 1;
    if (cz >= global<s32>(0x007a0374)) cz = global<s32>(0x007a0374) - 1;
    const u8 *const thing = Cell_FindRideableObject(cx, cz);
    if (!thing) return;
    const u32 id = *reinterpret_cast<const u16 *>(thing + 0x38);
    const u32 v = id - 1;
    x = (float)X87_SubR(X87_MulInt(v & 0x7f, -10.0f), 5.0f);  // 0x6fdd5c, 0x6fdd60
    z = (float)X87_SubR(X87_MulInt(v >> 7, -10.0f), 5.0f);
    const u8 *const world = **reinterpret_cast<u8 *const *const *>(0x007cf6ec);
    u32 &yaw = global(0x00790a38);
    switch (MapCell_Direction(world + 0x294 + id * 0x44) & 0x55) {
    case 1: yaw = 0x40490fdb; break;   // pi
    case 4: yaw = 0x3fc90fdb; break;   // pi / 2
    case 0x10: yaw = 0; break;
    case 0x40: yaw = 0x4096cbe4; break;   // 3 pi / 2
    }
}

void __cdecl Camera_Reset() {
    global(0x007909ec) = global(0x006fdd40);
    global(0x00790a38) = global(0x006fdd44);
    global(0x00790ab0) = 0;
    global(0x007908f0) = global(0x006fdd48);
    global(0x007908f4) = global(0x006fdd4c);
    global(0x007908f8) = global(0x006fdd50);
    Matrix_Identity(&global<float>(0x00790990));
    Matrix_Identity(&global<float>(0x00790a48));
    Matrix_Identity(&global<float>(0x00790948));
    global(0x007909c4) = X87_FloatThrough(global(0x007908f4));
    global(0x007909c0) = global(0x007908f0);
    global(0x007909c8) = X87_FloatThrough(global(0x007908f8));
    Mat4_Assign(global<u32 *>(0x0087afe8), nullptr, &global(0x00790990));
    const float fov = (float)(X87Reg(global<float>(0x0074c9b8)) * X87Reg(global<float>(0x006fdd80)));
    Mat4_Perspective(reinterpret_cast<float *>(0x0087b110), nullptr, 0.1f, 1000.0f, fov);
    ViewQuad_SetFov(global<float *>(0x00879d54), nullptr, fov);
    const u32 flags = (global(0x00790aac) & 0x200) | 0x80;
    global(0x00790aac) = flags;
    global(0x007909d0) = flags;
    global(0x007909e4) = Clocks_RealNow(reinterpret_cast<u8 *>(0x00785970));
    global(0x00790a88) = 0;
    global(0x007908e8) = 1;
    global(0x00790a8c) = 0;
    global(0x00790a90) = 0;
    global(0x00790ab0) = 0;
}

void __cdecl Camera_SnapYaw() {
    constexpr float kHalfPi = 1.57079637f, kQuarterPi = 0.785398185f;   // (0x6fdd68 / 0x6fdd98; 0x6fdd88 is the half as a double)
    float &yaw = global<float>(0x00790a38);
    const X87Reg r = std::fmod((double)yaw, (double)kHalfPi);   // _CIfmod (0x67b24a), exact
    if (!(r.v >= 0.0)) {   // (below 0 or unordered)
        if (!(r.v >= -kQuarterPi))
            yaw = (float)X87_SubR(r - X87Reg(-kHalfPi), X87Reg(yaw));
        else
            yaw = (float)(X87Reg(yaw) - r);
    } else if (r.v > kQuarterPi) {
        yaw = (float)((X87Reg(kHalfPi) - r) + X87Reg(yaw));
    } else {
        yaw = (float)(X87Reg(yaw) - r);
    }
}

void __cdecl Camera_Input(s32 message, u32 param) {
    const s32 height = global<s16>(0x00faa5c4), width = global<s16>(0x00faa5c0);
    const s32 y = (s32)(param >> 16) * height / 2048;       // (idiv by shifts and a magic multiply: truncated)
    const s32 x = (s32)(param & 0xffff) * width / 0x600;
    u32 &flags = global(0x00790aac);
    const auto set_point = [x, y] {
        global<float>(0x00790ac8) = (float)y;
        global<float>(0x00790acc) = (float)x;
    };
    const auto by_button = [param, &flags](bool on) {   // (buttons 0, 1, 2: bits 1, 4, 2)
        const u32 bit = param == 0 ? 1 : param == 1 ? 4 : param == 2 ? 2 : 0;
        flags = on ? flags | bit : flags & ~bit;
    };
    switch (message) {
    case 0x1e:
        if (flags & 2) {
            float &yaw = global<float>(0x00790a38);
            yaw = (float)((((X87Reg(global<float>(0x00790ac8)) * 2048.0f) / X87Reg((double)height)) - 1024.0f) * global<float>(0x0074c9c0) + yaw);
        }
        break;
    case 0x20:
        flags &= 0xffffff87;
        break;
    case 0x22:
        flags &= 0xffffff80;
        global(0x00790ab0) |= 0x100;
        break;
    case 0x10001:
        global(0x00790ab0) &= ~0x100u;
        set_point();
        break;
    case 0x10002:
        global(0x00790ab0) |= 0x100;
        break;
    case 0x10003:
        set_point();
        break;
    case 0x10004:
        by_button(false);
        break;
    case 0x10005:
        by_button(true);
        break;
    case 0x1000d: {   // (the wheel: 0x7909ec less -10 for a negative param, less 10 otherwise)
        float &zoom = global<float>(0x007909ec);
        zoom = (float)(X87Reg(zoom) - ((s32)param < 0 ? -10.0f : 10.0f));
        break;
    }
    default:
        break;
    }
}

void Camera_SetEnv(const CameraEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Camera_StopFollowing() {
    if ((*reinterpret_cast<const u8 *>((uintptr_t)0x00790ab0) & 4) == 0) return;
    u32 out;
    g_env->follow(*reinterpret_cast<const u32 *>((uintptr_t)0x00790a44), 0, 0, &out, 1, 0);
    if (out != 0) g_env->snap();
}

u32 __cdecl Camera_SetMode(u32 mode, u32 toggle, u32 ride, u32 force, u32 *out, u32 fresh) {
    u32 &flags = global(0x00790ab0);
    flags &= ~0x800u;
    const u32 was = flags & 0x16;
    u32 enter = 0, leave = 0, changed = 0;
    if (out) *out = 0;
    if (mode == 0x40) {
        flags = (flags & ~0xffu) | (flags & 0x41);
        if (was) leave = 1;
        changed = 1;
    } else if (mode == 4) {
        u32 f = flags;
        if (!(f & 0x92) || force) {
            f = (f & ~0xffu) | (f & 0x45);
            if (toggle && (f & 4))
                f &= ~4u;
            else
                f |= 4;
            flags = f;
            if (f & 4) {
                if (was) {
                    flags = f | 0x400;
                } else {
                    enter = 1;
                    if (fresh) {
                        global(0x00790ae0) = 0;
                        flags = f | 0x800;
                        global(0x00790adc) = 0;
                    }
                }
                changed = 1;
            } else if (was) {
                if (f & 0x400) {
                    flags = (f & ~0x400u) | 2;
                    if (out) *out = 1;
                } else {
                    leave = 1;
                }
            }
        }
    } else if (mode == 2) {
        u32 f = flags;
        if (!(f & 0x84) || force) {
            f = (f & ~0xffu) | (f & 0x43);
            if (toggle && (f & 2))
                f &= ~2u;
            else
                f |= 2;
            flags = f;
            if (f & 2) {
                if (!was) enter = 1;
                changed = 1;
            } else if (was) {
                leave = 1;
            }
        }
    }
    if (enter) {
        g_env->suspendHighlight(1);
        g_env->setChat(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, 1);
        if (Terrain_ProceduralOn()) {
            flags |= 0x200;
            g_env->leaveProcedural();
        }
        if (u8 *const inst = global<u8 *>(0x00790a34)) {
            if (at<u32>(inst, 4) & 0x4000)
                g_env->addToGrid(at<u8 *>(inst, 8), at<u32>(inst, 0xc0), at<u32>(inst, 0xc4));
            else
                g_env->wake(at<u8 *>(inst, 8));
        }
    } else if (leave) {
        g_env->suspendHighlight(0);
        g_env->setChat(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, 0);
        if (flags & 0x200) {
            flags &= ~0x200u;
            g_env->enterProcedural();
        }
    }
    u8 *const *const rides = reinterpret_cast<u8 *const *>(0x007a4610);
    for (u32 i = 1; i <= global(0x007a3ebc) + global(0x007a4084); ++i) {
        if (rides[i]) mark_parts(rides[i], 0x4000000, 0x8000000, true);
    }
    if (ride && (global<u8>(0x00790ab0) & 4) && rides[ride]) mark_parts(rides[ride], 0x10000000, 0x20000000, false);
    flags &= ~0x1000u;
    return changed;
}

void __cdecl Camera_FollowThing(u32 ride, u32 ground, u32 force, u32 *out, u32 key, u32 fresh) {
    if (out) *out = 0;
    if (u32 *const part = global<u32 *>(0x00790940)) *part &= ~4u;
    global(0x00790ab4) = 0;
    global(0x00790a30) = 0;
    global(0x00790940) = 0;
    if (global<u8>(0x00790ab0) & 4) {
        if (u8 *const followed = global<u8 *>(0x00790988)) at<u32>(followed, 4) &= ~0x2000u;
    }
    global(0x00790988) = 0;
    if (Ride_Bit18(ride)) ride = CoasterLink_IdValue(ride);
    u8 *const *const rides = reinterpret_cast<u8 *const *>(0x007a4610);
    if (ride) {
        bool follow = true;
        if (!(global<u8>(0x00790ab0) & 4)) {
            u8 *const inst = rides[ride];
            if (!inst) {
                follow = false;
            } else {
                u32 part = g_env->findPart(inst, 0x1000, key);
                if (key != 1 && part == 0xffffffff) part = g_env->findPart(inst, 0x1000, 1);
                if (part == 0xffffffff) {
                    follow = false;
                } else {
                    global(0x00790a44) = ride;
                    u8 *const *const holder = reinterpret_cast<u8 *const *>(inst + 8);
                    global<u8 *>(0x00790ab4) = Camera_PartFace(holder, &part);
                    u32 *const state = reinterpret_cast<u32 *>(at<u8 *>(at<u8 *>(*holder, 0x28), 4) + part * 0x14);
                    global<u32 *>(0x00790940) = state;
                    global(0x00790a30) = at<u32>(at<u8 *>(at<u8 *>(*holder, 0x28), 4), part * 0x14 + 4);
                    global<u8 *>(0x00790988) = inst;
                    *state |= 4;
                }
            }
        } else {
            if (u32 *const part = global<u32 *>(0x00790940)) *part &= ~4u;
            global(0x00790a44) = 0;
            global(0x00790ab4) = 0;
            global(0x00790940) = 0;
            global(0x00790a30) = 0;
        }
        if (follow && Camera_SetMode(4, 1, ride, force, out, fresh)) {
            u8 *const object = at<u8 *>(rides[ride], 8);
            if (at<u32>(object, 0x28)) g_env->partStates(object);
            at<u32>(rides[ride], 4) |= 0x2000;
        }
    }
    if (ground == 1 || (global<u8>(0x00790ab0) & 2))
        g_env->useGround();
    else
        g_env->usePark();
}

namespace {
// x / length of (x, y, z) for each, the length through a float (fst m32; fstp m32; fdiv m32).
void Normalize(const float *in, float *out) {
    const float length = (float)X87_Sqrt(X87Reg(in[0]) * in[0] + X87Reg(in[1]) * in[1] + X87Reg(in[2]) * in[2]);
    for (int i = 0; i < 3; i++) out[i] = (float)(X87Reg(in[i]) / length);
}
// fcom's C3: equal or unordered.
bool SameOrNan(X87Reg x, float y) { return !(x.v < y) && !(x.v > y); }
}  // namespace

void __fastcall Camera_SetLookAt(u8 *self) {
    float *const view = reinterpret_cast<float *>((uintptr_t)0x007909f0);
    const float *const position = reinterpret_cast<const float *>(self + 8), *const target = reinterpret_cast<const float *>(self + 0x14);
    Camera_SetFlag80(1);
    Matrix_Identity(view);

    const X87Reg d0 = X87Reg(target[0]) - position[0], d1 = X87Reg(target[1]) - position[1], d2 = X87Reg(target[2]) - position[2];
    const float d[3] = {(float)d0, (float)d1, (float)d2};
    const float distance = (float)X87_Sqrt(d0 * d0 + d1 * d[1] + d2 * d[2]);
    const float forward0 = (float)(X87Reg(d[0]) / distance), forward1 = (float)(X87Reg(d[1]) / distance);
    const X87Reg forward2 = X87Reg(d[2]) / distance;

    const float kZero = *reinterpret_cast<const float *>((uintptr_t)0x00702a38);
    const X87Reg sideX = forward2 - 0.0f;
    float side[3] = {(float)sideX, kZero, (float)(X87Reg(0.0f) - forward0)};
    if (SameOrNan(sideX, kZero) && SameOrNan(X87Reg(side[1]), kZero) && SameOrNan(X87Reg(side[2]), kZero)) {
        side[0] = 1.0f;
        side[1] = 0.0f;
        side[2] = 0.0f;
    }
    Normalize(side, side);
    const X87Reg up0r = X87Reg(side[2]) * forward1 - X87Reg(side[1]) * forward2;
    const X87Reg up1r = X87Reg(side[0]) * forward2 - X87Reg(side[2]) * forward0;
    const X87Reg up2r = X87Reg(side[1]) * forward0 - X87Reg(side[0]) * forward1;
    float up[3] = {(float)up0r, (float)up1r, (float)up2r};
    Normalize(up, up);

    view[0] = side[0];
    view[1] = side[1];
    view[2] = side[2];
    view[4] = up[0];
    view[5] = up[1];
    view[6] = up[2];
    view[8] = forward0;
    view[9] = forward1;
    view[10] = (float)forward2;
    view[12] = X87_Through(position[0]);
    view[13] = X87_Through(position[1]);
    view[14] = X87_Through(position[2]);

    u8 *const light = *reinterpret_cast<u8 **>(self + 0x20);
    if (!light || *reinterpret_cast<const u32 *>(self + 0x24) == 0) return;
    const float k = *reinterpret_cast<const float *>(self + 0x28), m = *reinterpret_cast<const float *>(self + 0x2c);
    const bool flat = *reinterpret_cast<const u32 *>(self + 0x30) != 0;
    const float along0 = (float)(X87Reg(k) * up[0] + position[0]), along1 = (float)(X87Reg(k) * up[1] + position[1]), along2 = (float)(X87Reg(k) * up[2] + position[2]);
    const float place0 = (float)(X87Reg(m) * side[0] + along0), place1 = (float)(X87Reg(m) * side[1] + along1);
    float to[3];
    to[0] = (float)(X87Reg(target[0]) - place0);
    if (flat) *reinterpret_cast<u32 *>(&to[1]) = 0;
    else to[1] = (float)(X87Reg(target[1]) - place1);
    to[2] = (float)(X87Reg(target[2]) - (X87Reg(m) * side[2] + along2));
    Normalize(to, to);
    if (flat) {
        to[1] = (float)(X87Reg(to[1]) - *reinterpret_cast<const float *>((uintptr_t)0x00702a48));
        Normalize(to, to);
    }
    SceneFlags_Touch(reinterpret_cast<u32 *>((uintptr_t)0x008bd508));
    memcpy(light + 0x2c, to, sizeof to);
}
}
