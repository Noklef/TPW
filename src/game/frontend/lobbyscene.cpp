#include "lobbyscene.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/soundplay.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/userslots.h"
#include "../math/mat4.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../render/state.h"
#include "../render/viewquad.h"
#include "../render/weatherfx.h"
#include "../world/camera.h"
#include "../world/cameraupdate.h"
#include "../world/rides.h"
#include "lobbyflight.h"
#include "lobbymodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const LobbySceneEnv kPorts = {&Ride_Destroy,
                              &String_Dtor,
                              &LobbyModel_Play,
                              &LobbyModel_TimeLeft,
                              &Sound_PlayAt,
                              reinterpret_cast<int(__cdecl *)()>((uintptr_t)0x0067b5c0),
                              reinterpret_cast<decltype(LobbySceneEnv::weatherBurst)>((uintptr_t)0x00580320),
                              &LobbyFlight_Ctor,
                              &RideUser_Acquire,
                              &LobbyFlyer_Step,
                              reinterpret_cast<decltype(LobbySceneEnv::perspective)>(&Mat4_Perspective),
                              reinterpret_cast<decltype(LobbySceneEnv::setFov)>(&ViewQuad_SetFov),
                              &Camera_SetLookAt,
                              &Camera_Update,
                              &Render_FadeFog};
const LobbySceneEnv *g_env = &kPorts;

// FCOM against a float: C0 (below, or unordered), C3 (equal, or unordered), C0 or C3 (not above).
bool Below(X87Reg a, float b) { return !(a.v >= b); }
bool EqualOr(X87Reg a, float b) { return !(a.v < b || a.v > b); }
bool NotAbove(X87Reg a, float b) { return !(a.v > b); }

float FrameScale() { return at<float>(global<u8 *>(0x00f82884), 8); }

// The scene's generator ([0xf82884] + 0x34).
u32 NextRandom() {
    u8 *const scene = global<u8 *>(0x00f82884);
    const u32 next = at<u32>(scene, 0x34) * 0x19660du + 0x3c6ef35fu;
    const u32 value = next >> 13 | next << 19;
    at<u32>(scene, 0x34) = value;
    return value;
}

// A draw as 0 to 1 (fild; fmul 2^-30).
X87Reg RandomUnit() { return X87_MulInt(NextRandom() & 0x3fffffff, global<float>(0x00702c58)); }

void Copy3(void *to, const void *from) { std::memcpy(to, from, 12); }

// v (three floats) over its length sqrt(x * x + y * y + z * z), in place.
void Normalise(float *v) {
    const X87Reg x = v[0], y = v[1], z = v[2];
    const X87Reg length = X87_Sqrt(x * x + y * y + z * z);
    v[0] = (float)(X87Reg(v[0]) / length);
    v[1] = (float)(X87Reg(v[1]) / length);
    v[2] = (float)(X87Reg(v[2]) / length);
}

// The heading `dir` eased a tenth of the way (by the frame's scale) to the way from `pos` to `target` (stored in `want`; 1, 0, 0 for no way).
void Steer(const float *target, const float *pos, float *want, float *dir) {
    for (u32 k = 0; k < 3; ++k)
        want[k] = (float)(X87Reg(target[k]) - pos[k]);
    if (EqualOr(X87Reg(want[0]), 0.0f) && EqualOr(X87Reg(want[1]), 0.0f) && EqualOr(X87Reg(want[2]), 0.0f)) {
        const u32 one = 0x3f800000, zero = 0;
        std::memcpy(&want[0], &one, 4);
        std::memcpy(&want[1], &zero, 4);
        std::memcpy(&want[2], &zero, 4);
    }
    Normalise(want);
    const X87Reg ease = X87Reg(FrameScale()) * global<float>(0x00702c7c);  // 0.1
    for (u32 k = 0; k < 3; ++k)
        dir[k] = (float)((X87Reg(want[k]) - dir[k]) * ease + dir[k]);
    Normalise(dir);
}

// pos moved along dir by step; the squared distance left to target.
X87Reg Advance(float *pos, const float *dir, X87Reg step, const float *target) {
    pos[0] = (float)(step * dir[0] + pos[0]);
    pos[1] = (float)(X87Reg(dir[1]) * step + pos[1]);
    pos[2] = (float)(X87Reg(dir[2]) * step + pos[2]);
    const X87Reg dx = X87Reg(target[0]) - pos[0], dy = X87Reg(target[1]) - pos[1], dz = X87Reg(target[2]) - pos[2];
    return dx * dx + dy * dy + dz * dz;
}

// a brought under the full turn, then up from below 0 (both endless for a NaN, as the original's).
X87Reg Wrap(X87Reg a, float full) {
    if (!Below(a, full)) {
        do
            a = a - full;
        while (!Below(a, full));
    }
    if (Below(a, 0.0f)) {
        do
            a = a + full;
        while (Below(a, 0.0f));
    }
    return a;
}

// The camera on the model at +0xc, by the state.
void Follow(u8 *object) {
    u8 *const camera = global<u8 *>(0x00f83778);
    const u8 *const model = at<const u8 *>(object, 0xc);
    const u8 *const place = at<const u8 *>(global<u8 *>(0x00f82884), 0x10);
    const float pi = global<float>(0x00702c18);
    const float eight = global<float>(0x00702c68);
    float &angle = at<float>(object, 0x18);
    const u32 state = at<u32>(object, 0x14);
    if (state == 2) {
        angle = (float)(X87Reg(at<float>(model, 0x14)) + pi);
        at<float>(object, 0x20) = (float)(at<float>(object, 0x20) - X87Reg(FrameScale()) * at<float>(object, 0x20) * global<float>(0x00702c60));   // 0.07
        at<float>(object, 0x24) = (float)(at<float>(object, 0x24) - X87Reg(FrameScale()) * at<float>(object, 0x24) * global<float>(0x00702c64));   // 0.6
        if (Below(X87Reg(at<float>(object, 0x20)), eight)) {
            at<u32>(camera, 0x60) = 0x10000;
            vcall<u32>(object, 0x48);
            return;
        }
        X87Reg range = X87Reg(at<float>(place, 0xc)) - eight;
        if (EqualOr(range, 0.0f))
            range = 1.0;
        X87Reg left = (global<float>(0x00702c4c) - (X87Reg(at<float>(object, 0x20)) - eight) / range) * global<float>(0x00702c6c);   // (1 - that) * 65536
        if (Below(left, 0.0f)) {
            at<u32>(camera, 0x60) = 0;
            return;
        }
        if (!NotAbove(left, global<float>(0x00702c70)))   // 65535
            left = X87Reg(global<float>(0x00702c70));
        at<s32>(camera, 0x60) = (s32)(long long)left.v;
    } else if (state == 1) {
        at<u32>(camera, 0x60) = 0;
        Copy3(object + 0x1c, place + 8);
        const float full = (float)(X87Reg(pi) + X87Reg(pi));
        const X87Reg heading = Wrap(X87Reg(at<float>(model, 0x14)) + pi, full);
        const X87Reg off = Wrap(X87Reg(angle) - heading, full);
        const X87Reg rate = X87Reg(FrameScale()) * global<float>(0x00702c78);   // 0.05
        const float step = (float)rate;
        const X87Reg half = rate * global<float>(0x00702c5c);   // 0.5
        if (!(off.v >= half.v) || off.v > (full - half).v) {
            g_env->play(at<const u8 *>(model, 8), nullptr, 0, 0);
            at<u32>(object, 0x14) = 2;
        } else if (Below(off, pi)) {
            angle = (float)(heading + (off - step));
        } else {
            angle = (float)(heading + (off + step));
        }
    } else if (state == 0) {
        at<u32>(camera, 0x60) = 0;
        Copy3(object + 0x1c, place + 8);
        angle = (float)(X87Reg(FrameScale()) * at<float>(object, 0x1c) + angle);
        const X87Reg turn = X87Reg(pi) + X87Reg(pi);
        const float full = (float)turn;
        if (!Below(X87Reg(angle), full)) {
            X87Reg a = 0.0;
            do {
                a = X87Reg(angle) - full;
                angle = (float)a;
            } while (!Below(a, full));
        }
        if (Below(X87Reg(angle), 0.0f)) {
            X87Reg a = 0.0;
            do {
                a = turn + angle;
                angle = (float)a;
            } while (Below(a, 0.0f));
        }
    }
}

// The camera on its own: the position (+0x78) and look-at (+0xb4) each steered to their targets; the list's nearest model taken.
void Roam(u8 *object, s32 count) {
    u8 *const camera = global<u8 *>(0x00f83778);
    at<u32>(camera, 0x60) = 0;
    float *const pos = &at<float>(object, 0x78), *const target = &at<float>(object, 0x84), *const dir = &at<float>(object, 0x90);
    Steer(target, pos, &at<float>(object, 0x9c), dir);
    const X87Reg left = Advance(pos, dir, X87Reg(at<float>(object, 0x70)) * FrameScale(), target);
    if (Below(left, at<float>(object, 0x74))) {
        for (u32 k = 0; k < 3; ++k) {
            const float extent = at<float>(object, 0x64 + 4 * k);
            const X87Reg spread = RandomUnit() * extent;
            target[k] = (float)((spread - X87Reg(extent) * global<float>(0x00702c5c)) + at<float>(object, 0x58 + 4 * k));
        }
    }
    Copy3(camera + 8, pos);
    at<u32>(object, 0xc) = 0;
    float *const look = &at<float>(object, 0xb4), *const lookTarget = &at<float>(object, 0xc0), *const lookDir = &at<float>(object, 0xcc);
    if (count <= 0) {
        Copy3(lookTarget, object + 0x58);
        at<u32>(object, 0xc4) = 0x41200000;  // 10
    } else {
        float best = global<float>(0x00702c80);   // 9999999
        for (const u8 *node = at<const u8 *>(at<const u8 *>(object, 8), 4); node; node = at<const u8 *>(node, 4)) {
            u8 *const model = at<u8 *>(node, 0);
            const X87Reg dx = X87Reg(at<float>(model, 0x3c)) - at<float>(object, 0x28);
            const X87Reg dy = X87Reg(at<float>(model, 0x40)) - at<float>(object, 0x2c);
            const X87Reg dz = X87Reg(at<float>(model, 0x44)) - at<float>(object, 0x30);
            const X87Reg distance = dx * dx + dy * dy + dz * dz;
            if (Below(distance, best)) {
                best = (float)distance;
                Copy3(lookTarget, model + 0x3c);
                at<u8 *>(object, 0xc) = model;
            }
        }
    }
    Steer(lookTarget, look, &at<float>(object, 0xd8), lookDir);
    const X87Reg lookLeft = Advance(look, lookDir, X87Reg(FrameScale()) * at<float>(object, 0xac), lookTarget);
    float &speed = at<float>(object, 0xac);
    const float top = at<float>(object, 0xa8);
    if (Below(lookLeft, at<float>(object, 0xb0))) {
        if (!NotAbove(X87Reg(speed), 0.0f))
            speed = (float)(speed - X87Reg(top) * global<float>(0x00702c78));   // 0.05
        else
            at<u32>(object, 0xac) = 0;
    } else if (Below(X87Reg(speed), top)) {
        speed = (float)(speed - X87Reg(top) * global<float>(0x00702c84));   // -0.05
    } else {
        at<u32>(object, 0xac) = at<u32>(object, 0xa8);
    }
    Copy3(camera + 0x14, look);
    Copy3(object + 0x28, pos);
    Copy3(object + 0x40, look);
}

// The weather's drops, the model's burst and sounds, and the list's models played again.
void Ambience(u8 *object) {
    u8 *const weather = global<u8 *>(0x008bcbac);
    u8 *const camera = global<u8 *>(0x00f83778);
    Obj_SetFirst(weather, nullptr, 1);
    X87Reg sum = 0.0;
    s32 flagged = 0;
    for (const u8 *node = at<const u8 *>(at<const u8 *>(object, 8), 4); node; node = at<const u8 *>(node, 4)) {
        const u8 *const model = at<const u8 *>(node, 0);
        if (at<u32>(model, 0x60) == 0)
            continue;
        ++flagged;
        const X87Reg dx = X87Reg(at<float>(model, 0x3c)) - at<float>(camera, 8);
        const X87Reg dz = X87Reg(at<float>(model, 0x44)) - at<float>(camera, 0x10);
        sum = sum + (dx * dx + dz * dz);
    }
    float drops = 0.0f;
    if (flagged > 0) {
        const X87Reg mean = sum / X87Reg((double)flagged);
        const X87Reg square = mean * mean;
        drops = (float)(global<float>(0x00702c88) / (square * square));   // 1e17
        if (Below(X87Reg(drops), global<float>(0x00702c8c)))   // 20
            drops = 0.0f;
        else if (!NotAbove(X87Reg(drops), global<float>(0x00702c90)))   // 400
            drops = 400.0f;
    }
    WeatherFx_SetDropCount(weather, nullptr, (u32)(s32)(long long)(double)drops);

    const u8 *const model = at<const u8 *>(object, 0xc);
    if (!model)
        return;
    if (at<s32>(model, 0x5c) > 0 && (at<u32>(model, 0x5c) & NextRandom()) == 1) {
        const float hundred = global<float>(0x00702c94), fifty = global<float>(0x00702c98), twenty = global<float>(0x00702c8c), ten = global<float>(0x00702ca0);
        const float x = at<float>(model, 0x3c), z = at<float>(model, 0x44);
        const float x1 = (float)(((RandomUnit() * hundred) + x) - fifty);
        const float z1 = (float)(((RandomUnit() * hundred) + z) - fifty);
        const float x2 = (float)(((RandomUnit() * twenty) + x1) - ten);
        const float z2 = (float)(((RandomUnit() * twenty) + z1) - ten);
        g_env->weatherBurst(weather, nullptr, x2, 0.0f, z2, x1, 500.0f, z1, 0x1f4, 1);
        WeatherFx_RequestReload(weather);
        g_env->playAt(0, global<u32>(0x00803a48), 1, 0, 0, 0);
    }
    if ((NextRandom() & 0xf) == 1) {
        const u32 sound = 3 + NextRandom() % 3;
        g_env->playAt(0, at<u32>(global<const u8 *>(0x00803a4c), at<u32>(model, 0x18) * 8), sound, 0, 0, 0);
    }
    Copy3(camera + 0x50, model + 0x64);
    for (const u8 *node = at<const u8 *>(at<const u8 *>(object, 8), 4); node; node = at<const u8 *>(node, 4)) {
        const u8 *const item = at<const u8 *>(node, 0);
        if ((s32)g_env->timeLeft(at<const u8 *>(item, 4)) <= 0) {
            const u32 entry = (u32)g_env->rand() & 1;
            g_env->play(at<const u8 *>(item, 4), nullptr, entry, 0);
        }
    }
}

}  // namespace

void LobbyScene_SetEnv(const LobbySceneEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

void __fastcall Class702a50_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702a50;
    vcall<u32>(*reinterpret_cast<u8 **>((uintptr_t)0x00f83774), 0x20, object);
    g_env->destroy(at<u32>(object, 4));
    g_env->stringDtor(reinterpret_cast<String *>(object + 0x20));
    g_env->stringDtor(reinterpret_cast<String *>(object + 0xc));
}

u8 *__fastcall Class702a50_DeletingDtor(u8 *object, void *, u32 flags) {
    Class702a50_Dtor(object);
    if (flags & 1) GameHeap_Free(object);
    return object;
}

void __fastcall LobbyCamera_Frame(u8 *camera) {
    const u32 fov = at<u32>(camera, 4);
    g_env->perspective(reinterpret_cast<void *>((uintptr_t)0x0087b110), nullptr, 0x3dcccccd, 0x447a0000, fov);
    g_env->setFov(global<void *>(0x00879d54), nullptr, fov);
    g_env->lookAt(camera);
    g_env->cameraUpdate();
    g_env->fadeFog(camera);
    const u32 fade = at<u32>(camera, 0x5c);
    const u32 gap = at<u32>(camera, 0x60) - fade;
    if (gap) at<u32>(camera, 0x5c) = fade + static_cast<u32>(static_cast<s32>(gap) >> 3);
    if (at<s32>(camera, 0x5c) > 0) {
        global<u32>(0x008bcbc8) |= 0x80000;
        const s32 alpha = at<s32>(camera, 0x5c) >> 8;
        global<u32>(0x008bd4e4) = static_cast<u32>(alpha > 0xff ? 0xff : alpha) << 24;
    } else {
        global<u32>(0x008bcbc8) &= 0xfff7ffff;
    }
}

void __fastcall Class702af0_Step(u8 *object) {
    if (at<s32>(object, 0xc) < at<s32>(object, 8)) {
        u32 bits;
        do {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x60));
            u8 *const flyer = block ? g_env->flightCtor(block, nullptr, object, at<u32>(object, 0x18), at<u32>(object, 0x2c), reinterpret_cast<const u32 *>(object + 0x38), reinterpret_cast<const u32 *>(object + 0x44),
                                                        at<u32>(object, 0x50), at<u32>(object, 0x54))
                                    : nullptr;
            if (at<u32>(object, 4) && at<u32>(flyer, 0xc) == 0) {
                g_env->acquire(at<u8 *>(flyer, 8));
                at<u32>(flyer, 0xc) = 1;
            }
            if (++at<s32>(object, 0xc) >= at<s32>(object, 8)) break;
            u8 *const frame = global<u8 *>(0x00f82884);
            const u32 next = at<u32>(frame, 0x34) * 0x19660d + 0x3c6ef35f;
            bits = next >> 13 | next << 19;
            at<u32>(frame, 0x34) = bits;
        } while ((bits & 7) != 1);
    }
    for (u8 *node = at<u8 *>(at<u8 *>(object, 0x58), 4); node;) {
        u8 *const flyer = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        g_env->flyerStep(flyer);
    }
}

void __fastcall Class702af0_StepAll(u8 *all) {
    for (u8 *node = at<u8 *>(at<u8 *>(all, 4), 4); node;) {
        u8 *const scene = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        Class702af0_Step(scene);
    }
}

void __fastcall Class702af0_AcquireModels(u8 *object) {
    if (at<u32>(object, 4) != 0)
        return;
    for (u8 *node = at<u8 *>(at<u8 *>(object, 0x58), 4); node;) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (at<u32>(item, 0xc) == 0) {
            RideUser_Acquire(at<u8 *>(item, 8));
            at<u32>(item, 0xc) = 1;
        }
    }
    at<u32>(object, 4) = 1;
}

void __fastcall Class702ca8_AcquireModels(u8 *object) {
    if (at<u32>(object, 0x10) != 0)
        return;
    RideUser_Acquire(at<u8 *>(object, 4));
    RideUser_Acquire(at<u8 *>(object, 8));
    at<u32>(object, 0x10) = 1;
}

u32 __fastcall Class702cb0_StartPendingPlay(u8 *object) {
    const u32 state = at<u32>(object, 0x14);
    if (state == 0)
        return 0;
    if (state == 2)
        LobbyModel_Play(at<u8 *>(at<u8 *>(object, 0xc), 8), nullptr, 1, 0);
    at<u32>(object, 0x14) = 0;
    return 1;
}

void __fastcall Class702cb0_Step(u8 *object) {
    const s32 count = at<s32>(at<const u8 *>(object, 8), 0xc);
    if (count != 0 && at<s32>(UserSlots_Instance(), 0x60) != -1 && at<u32>(object, 0xc) != 0) {
        Follow(object);
        Class702cb0_StepCamera(object);
        Copy3(object + 0x78, object + 0x28);
        Copy3(object + 0xb4, object + 0x40);
    } else {
        Roam(object, count);
    }
    if (count > 0)
        Ambience(object);
}

void __fastcall Class702cb0_StepCamera(u8 *object) {
    const float angle = at<float>(object, 0x18);
    const X87Reg s = X87_Sin(angle);
    const u8 *const model = at<const u8 *>(object, 0xc);
    for (u32 i = 0; i < 0xc; i += 4)  // the model's +0x3c to the target (+0x4c), then to the eye (+0x34)
        at<u32>(object, 0x4c + i) = at<u32>(model, 0x3c + i);
    for (u32 i = 0; i < 0xc; i += 4)
        at<u32>(object, 0x34 + i) = at<u32>(model, 0x3c + i);
    const X87Reg c = X87_Cos(angle);  // (also stored to a local nothing reads)
    const float radius = at<float>(object, 0x20);
    const float ex = (float)(s * radius + at<float>(object, 0x34));
    const float ey = (float)(X87Reg(at<float>(object, 0x24)) + at<float>(object, 0x38));
    const float ez = (float)(c * radius + at<float>(object, 0x3c));
    at<float>(object, 0x34) = ex;
    at<float>(object, 0x38) = ey;
    at<float>(object, 0x3c) = ez;

    // the position (+0x28) eased a tenth of the way to the eye, the look-at (+0x40) a fifth to the target, by the frame's scale
    const X87Reg near1 = X87Reg(at<float>(at<const u8 *>(reinterpret_cast<const void *>(0x00f82884), 0), 8)) * 0.100000001f;  // 0x702c7c
    float *const pos = &at<float>(object, 0x28);
    const float px = (float)((X87Reg(pos[3]) - pos[0]) * near1 + pos[0]);
    const float py = (float)((X87Reg(pos[4]) - pos[1]) * near1 + pos[1]);
    const float pz = (float)((X87Reg(pos[5]) - pos[2]) * near1 + pos[2]);
    pos[0] = px;
    pos[1] = py;
    pos[2] = pz;
    const X87Reg near2 = X87Reg(at<float>(at<const u8 *>(reinterpret_cast<const void *>(0x00f82884), 0), 8)) * 0.200000003f;  // 0x702ca4
    float *const look = &at<float>(object, 0x40);
    for (int i = 0; i < 3; i++)
        look[i] = (float)((X87Reg(look[i + 3]) - look[i]) * near2 + look[i]);

    u8 *camera = at<u8 *>(reinterpret_cast<const void *>(0x00f83778), 0);
    for (u32 i = 0; i < 0xc; i += 4)
        at<u32>(camera, 8 + i) = at<u32>(pos, i);
    camera = at<u8 *>(reinterpret_cast<const void *>(0x00f83778), 0);
    for (u32 i = 0; i < 0xc; i += 4)
        at<u32>(camera, 0x14 + i) = at<u32>(look, i);
}

u8 *__fastcall Class702af0_Ctor(u8 *object, void *, s32 count, const char *name, const char *name2, const u32 *first, const u32 *second, u32 a,
                                u32 b) {
    String_Ctor(reinterpret_cast<String *>(object + 0x10));
    String_Ctor(reinterpret_cast<String *>(object + 0x24));
    at<u32>(object, 0) = 0x00702af0;
    u32 *list = static_cast<u32 *>(GameHeap_Alloc(0x10));
    if (list) {
        list[1] = 0;
        list[2] = 0;
        list[3] = 0;
        list[0] = 0x00702aec;
    }
    at<u32>(object, 4) = 0;
    at<u32 *>(object, 0x58) = list;
    s32 share = (s32)((u32)count * *reinterpret_cast<u32 *>((uintptr_t)0x007858dc)) / 100;
    if (share < 1)
        share = 1;
    else if (share > count)
        share = count;
    at<s32>(object, 8) = share;
    at<u32>(object, 0xc) = 0;
    vcall<void>(object + 0x10, 0x14, name);
    vcall<void>(object + 0x24, 0x14, name2);
    at<u32>(object, 0x38) = first[0];
    at<u32>(object, 0x3c) = first[1];
    at<u32>(object, 0x40) = first[2];
    at<u32>(object, 0x44) = second[0];
    at<u32>(object, 0x48) = second[1];
    const u32 last = second[2];
    at<u32>(object, 0x50) = a;
    at<u32>(object, 0x4c) = last;
    at<u32>(object, 0x54) = b;
    vcall<void>(*reinterpret_cast<u8 **>((uintptr_t)0x00f83784), 0x18, object);
    return object;
}
}
