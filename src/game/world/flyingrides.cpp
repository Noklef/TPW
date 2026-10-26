#include "flyingrides.h"

#include <cstdint>

#include "objectdefs.h"
#include "../coaster/registry.h"
#include "../core/initializers.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/parts.h"
#include "../model/partstate.h"
#include "../motion/animator.h"
#include "../render/particles.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *const *table() { return reinterpret_cast<u8 *const *>(0x008791f8); }

// The record for index, null for one outside 1-99 or an empty entry.
u8 *record(s32 index) { return index > 0 && index < 100 ? table()[index] : nullptr; }

u8 *slot(u8 *rec, u32 i) { return rec + 0x5c + i * 0xe4; }

s32 bound(u32 address) { return *reinterpret_cast<const s32 *>((uintptr_t)address); }

// The ride part step names released: key (base + step) % count + 1 among the flag-0x80 parts.
void release(u8 *s, s32 step) {
    const s32 count = at<s32>(s, 0xdc);
    if (count == 0) return;
    const s32 key = (at<s32>(s, 0xd8) + step) % count + 1;
    if (at<u32>(s, 0xc0) == 0) return;
    u8 *const *rides = reinterpret_cast<u8 *const *>(0x007a4610);
    const u32 part = Model_FindPartByKey4(rides[at<u32>(s, 0xc0)], 0x80, (u32)key);
    if (part == 0xffffffff) return;
    PartStates_Release(at<u8 *>(rides[at<u32>(s, 0xc0)], 8), part);
}

// |x| as cdq; xor; sub has it (INT_MIN stays).
s32 magnitude(s32 x) { return x < 0 ? (s32)(0u - (u32)x) : x; }

}  // namespace

extern "C" {

u32 __cdecl FlyingRide_FirstSlotRide(s32 index) {
    u8 *const rec = record(index);
    return rec ? at<u32>(slot(rec, 0), 0xc0) : 0;
}

void __cdecl FlyingRide_PlaySlotMotion(s32 index, u32 slotIndex, u32 bank, u32 entry, u32 loop) {
    u8 *const s = slot(table()[index], slotIndex);
    const u32 ride = at<u32>(s, 0xc0);
    if (ride == 0) return;
    if (at<u32>(s, 0xcc) == bank && at<u32>(s, 0xd0) == entry && at<u32>(s, 0xd4) == loop) return;
    u8 *const *rides = reinterpret_cast<u8 *const *>(0x007a4610);
    Animator_Play(rides[ride], bank, entry, loop != 0 ? 1 : 0, 1.0f, 0);
    at<u32>(s, 0xcc) = bank;
    at<u32>(s, 0xd0) = entry;
    at<u32>(s, 0xd4) = loop;
}

// The four fly-by bounds (0x7660bc, 0x7660c4, 0x7660b8, 0x7660c0): the globals Fn_52c6c0 copies, scaled by 3000 and offset by 0x2904 (the last two negatively).
void __cdecl FlyingRide_SetBounds() {
    u32 &a = *reinterpret_cast<u32 *>(0x007660bc), &b = *reinterpret_cast<u32 *>(0x007660c4), &c = *reinterpret_cast<u32 *>(0x007660b8), &d = *reinterpret_cast<u32 *>(0x007660c0);
    Fn_52c6c0(0x007660bc, 0x007660c4, 0x007660b8, 0x007660c0);
    a = a * 3000 + 0x2904;
    b = b * 3000 + 0x2904;
    c = c * 3000 - 0x2904;
    d = d * 3000 - 0x2904;
}

void __cdecl FlyingRide_NextCameraCorner(s32 *camera) {
    camera[0x16] = camera[0x13];
    camera[0x18] = camera[0x15];
    const u8 *record = table()[camera[0]];
    camera[0x17] = camera[0x14];
    s32 height = at<s32>(record, 0x18) + 5000;
    if (height < 14000) height = 14000;
    camera[0x14] = height;
    const u32 corner = ((u32)camera[0x22] + 1) & 3;
    camera[0x22] = (s32)corner;
    static const s32 signs[4][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};
    record = table()[camera[0]];
    camera[0x13] = at<s32>(record, 0x14) + signs[corner][0] * 25000;
    camera[0x15] = at<s32>(record, 0x1c) + signs[corner][1] * 25000;
}

// The part a slot of the record at index (not checked: the table entry is used as it is) steps to.
void __cdecl FlyingRide_ReleaseSlotPart(s32 index, s32 slotIndex, s32 step) {
    release(slot(table()[index], (u32)slotIndex), step);
}

u32 __cdecl FlyingRide_StepInSlot(s32 index) {
    u8 *rec = record(index);
    if (!rec) return 0;
    u32 i = 0;
    while (at<s32>(slot(rec, i), 0x80) != 1) {
        if (++i >= 20) return 0;
    }
    if (at<s32>(slot(rec, i), 0x8c) >= 0) return 0;
    release(slot(rec, i), (s32)(0u - at<u32>(slot(rec, i), 0x8c)));
    ++at<s32>(slot(table()[index], i), 0x8c);
    --at<s32>(table()[index], 0x48);
    rec = table()[index];
    return at<u32>(slot(rec, i), 0x90 + (0u - at<u32>(slot(rec, i), 0x8c)) * 4);
}

u32 __cdecl FlyingRide_StepOutSlot(s32 index) {
    u8 *rec = record(index);
    if (!rec) return 0;
    u32 i = 0;
    while (at<s32>(slot(rec, i), 0x8c) <= 0) {
        if (++i >= 20) return 0;
    }
    release(slot(rec, i), magnitude(at<s32>(slot(rec, i), 0x8c)));
    s32 &step = at<s32>(slot(table()[index], i), 0x8c);
    step = step > 0 ? step - 1 : step + 1;
    --at<s32>(table()[index], 0x48);
    rec = table()[index];
    return at<u32>(slot(rec, i), 0x90 + (u32)magnitude(at<s32>(slot(rec, i), 0x8c)) * 4);
}

u32 __cdecl FlyingRide_AddToSlot(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (!rec) return 0;
    u32 i = 0;
    while (at<s32>(slot(rec, i), 0x80) != 1) {
        if (++i >= 20) return 0;
    }
    const s32 count = at<s32>(slot(rec, i), 0x8c);
    if (count < 0) return 0;
    at<u32>(slot(rec, i), 0x90 + (u32)count * 4) = value;
    ++at<s32>(slot(table()[index], i), 0x8c);
    u8 *const s = slot(table()[index], i);
    const s32 parts = at<s32>(s, 0xdc);
    if (parts != 0) {
        const s32 key = (s32)((u32)at<s32>(s, 0xd8) + (u32)at<s32>(s, 0x8c)) % parts + 1;
        if (at<u32>(s, 0xc0) != 0) {
            u8 *const *rides = reinterpret_cast<u8 *const *>(0x007a4610);
            const u32 part = Model_FindPartByKey4(rides[at<u32>(s, 0xc0)], 0x80, (u32)key);
            if (part != 0xffffffff) PartStates_Activate(at<u8 *>(rides[at<u32>(s, 0xc0)], 8), part, value);
        }
    }
    ++at<s32>(table()[index], 0x48);
    return 1;
}

void __cdecl FlyingRide_SetCap(s32 index, s32 value) {
    u8 *const rec = record(index);
    if (!rec) return;
    const s32 cap = value / 64;
    at<s32>(rec, 0x10) = cap;
    for (u32 i = 0; i < 20; ++i) {
        s32 &v = at<s32>(slot(table()[index], i), 0x84);
        if (v > cap) v = cap;
    }
}

void __cdecl FlyingRide_SetTenth(s32 index, s32 value) {
    u8 *const rec = record(index);
    if (rec) at<s32>(rec, 0x50) = value / 10;
}

void __cdecl FlyingRide_SetOn(s32 index, u32 on) {
    u8 *const rec = record(index);
    if (!rec) return;
    if (on) {
        const u32 value = at<u32>(rec, 4);
        at<u32>(rec, 0x38) = 1;
        at<u32>(rec, 0) = value;
        return;
    }
    if (at<u32>(rec, 0x34) == 3) return;
    at<u32>(rec, 0) = 1;
    for (u32 i = 0; i < 20; ++i) {
        u8 *const s = slot(rec, i);
        if (at<u32>(s, 0x80) != 0) at<u32>(s, 0x84) = 0;
        if (at<u32>(s, 0x80) == 1 && at<s32>(s, 0x8c) > 0) at<s32>(s, 0x8c) = (s32)(0u - at<u32>(s, 0x8c));
        at<u32>(rec, 0x38) = 0;
    }
}

void __cdecl FlyingRide_Set4(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (!rec) return;
    at<u32>(rec, 4) = value;
    u8 *const again = table()[index];
    if (at<u32>(again, 0x38) != 0) at<u32>(again, 0) = value;
}

u32 __cdecl FlyingRide_Get3c(s32 index) {
    u8 *const rec = record(index);
    return rec ? at<u32>(rec, 0x3c) : 0;
}

u32 __cdecl FlyingRide_FirstSlotReady(s32 index) {
    u8 *const rec = record(index);
    return rec && at<u32>(slot(rec, 0), 0xb8) != 0 && at<u32>(slot(rec, 0), 0x80) == 1 ? 1 : 0;
}

u32 __cdecl FlyingRide_Take3c(s32 index) {
    u8 *const rec = record(index);
    if (!rec) return 0;
    if (at<u32>(rec, 0x38) == 0 && at<u32>(rec, 0x48) == 0 && at<s32>(rec, 0xc) <= 1) return 0;
    if (at<u32>(rec, 0x3c) == 0) return 0;
    at<u32>(rec, 0x3c) = 0;
    return 1;
}

void __cdecl FlyingRide_Set44(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (rec) at<u32>(rec, 0x44) = value;
}

void __cdecl FlyingRide_Set20(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (rec) at<u32>(rec, 0x20) = value;
}

void __cdecl FlyingRide_Set24(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (rec) at<u32>(rec, 0x24) = value;
}

u32 __cdecl FlyingRide_Busy(s32 index) {
    u8 *const rec = record(index);
    if (!rec) return 0;
    return at<u32>(rec, 0x48) == 0 && at<u32>(rec, 0x3c) != 0 && at<u32>(rec, 0xc) == 1 ? 0 : 1;
}

void __cdecl FlyingRides_ReclaimRides() {
    for (s32 index = 1; index < 100; ++index) {
        u8 *rec = table()[index];
        if (!rec)
            continue;
        for (u32 i = 0; i < 20; ++i) {
            u8 *s = slot(rec, i);
            if (!at<u32>(s, 0x80))
                continue;
            const u16 thing = at<u16>(rec, 0x58);
            const u8 *object = *reinterpret_cast<u8 *const *>((uintptr_t)(0x007cfb90 + 20 * thing));
            u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, at<u16>(object, 0xe));
            at<u32>(s, 0xc0) = Ride_Claim(at<u32>(s, 0xc0), at<u32>(def, 0x4bc), 0x101, nullptr, 0, 0, 0, 0);
        }
    }
}

s32 __cdecl FlyingRides_FloorAt(s32 x, s32 y) {
    if (x >= (s32)((u32)bound(0x007660b8) + 20000) || y >= (s32)((u32)bound(0x007660c0) + 20000) || x < (s32)((u32)bound(0x007660bc) - 20000) ||
        y < (s32)((u32)bound(0x007660c4) - 20000))
        return 100000;
    for (s32 index = 1; index < 100; index++) {
        const u8 *const ride = table()[index];
        if (!ride)
            continue;
        const s32 dx = (s32)((u32)x - at<u32>(ride, 0x14)) / 32, dy = (s32)((u32)y - at<u32>(ride, 0x1c)) / 32;
        const s32 square = (s32)((u32)dx * (u32)dx + (u32)dy * (u32)dy);
        if ((s32)((u32)(long long)X87_Sqrt(X87Reg((double)square)).v << 5) < 10000)   // (a wrapped square's NaN: __ftol's 0)
            return 30000;
    }
    if (x >= bound(0x007660b8) || y >= bound(0x007660c0) || x < bound(0x007660bc) || y < bound(0x007660c4))
        return 0;
    s32 cx = x / 300 / 10, cy = y / 300 / 10;
    if (cx < 0)
        cx = 0;
    if (cy < 0)
        cy = 0;
    if (cx >= bound(0x007a0314))
        cx = bound(0x007a0314) - 1;
    if (cy >= bound(0x007a0374))
        cy = bound(0x007a0374) - 1;
    return (s32)((u32)(long long)Terrain_SurfaceHeight(cx, cy, nullptr) * 300);
}
}

namespace {

float &global_float(u32 address) { return *reinterpret_cast<float *>((uintptr_t)address); }
u32 bits_of(float f) { u32 b; std::memcpy(&b, &f, 4); return b; }

const float kPerUnit = 1.0f / 300;   // 0x7010bc
const float kPi = 3.14159265358979f;   // 0x70105c

// fild old; fild (cur - old); fmul t; faddp; fmul 1/300.
float lerp_position(s32 cur, s32 old, float t) {
    const s32 step = (s32)((u32)cur - (u32)old);
    return (float)((X87Reg(X87_MulIntDouble(step, t)) + X87Reg((double)old)) * X87Reg(kPerUnit));
}

// The 12-bit step from old to cur (0x800 and up less 0xfff, sic), times t, plus old.
float lerp_angle(s32 cur, s32 old, float t) {
    s32 step = (s32)(((u32)cur - (u32)old) & 0xfff);
    if (step >= 0x800) step -= 0xfff;
    return (float)(X87Reg(X87_MulIntDouble(step, t)) + X87Reg((double)old));
}

// (now + 3 * last) / 4: (now - last * -3) * 0.25.
float ease_position(float now, float last) {
    return (float)(X87_SubR(X87Reg(last) * X87Reg(-3.0f), X87Reg(now)) * X87Reg(0.25f));
}

// now and last __ftol'd to 12 bits, now brought within 0x800 of last, (now + (weight - 1) * last) / weight (signed), 12-bit.
float ease_angle(float now, float last, s32 weight) {
    s32 a = (s32)(long long)now & 0xfff;
    const s32 b = (s32)(long long)last & 0xfff;
    const s32 diff = a - b;
    if (diff > 0x800) a -= 0x1000;
    else if (diff < -0x800) a += 0x1000;
    return (float)(((b * (weight - 1) + a) / weight) & 0xfff);
}

u8 *ride_of(u32 index) { return reinterpret_cast<u8 *const *>(0x007a4610)[index]; }
float *ride_matrix(u32 index) { return at<float *>(at<u8 *>(at<u8 *>(ride_of(index), 8), 4), 0x78); }

}

extern "C" {

void __cdecl FlyingRides_LerpSlots(float t) {
    if (t > 2.0f) t = 2.0f;   // 0x7010b8
    float &x = global_float(0x008791e0), &y = global_float(0x008791f4), &z = global_float(0x008791d8);
    float &a = global_float(0x008791e8), &b = global_float(0x008791c4), &c = global_float(0x008791c0);
    for (u32 address = 0x008791fc; address < 0x00879388; address += 4) {
        u8 *const rec = *reinterpret_cast<u8 *const *>((uintptr_t)address);
        if (!rec) continue;
        for (u32 i = 0; i < 20; ++i) {
            u8 *const s = slot(rec, i);
            if (!at<u32>(s, 0x80)) continue;
            if (at<u32>(s, 0xc8)) {
                x = (float)X87_MulIntDouble(at<s32>(s, 4), kPerUnit);
                y = (float)X87_MulIntDouble(at<s32>(s, 8), kPerUnit);
                z = (float)X87_MulIntDouble(at<s32>(s, 0xc), kPerUnit);
                a = (float)at<s32>(s, 0x10);
                b = (float)at<s32>(s, 0x18);
                c = (float)at<s32>(s, 0x14);
                at<u32>(s, 0xc8) = 0;
            } else {
                x = lerp_position(at<s32>(s, 4), at<s32>(s, 0x1c), t);
                y = lerp_position(at<s32>(s, 8), at<s32>(s, 0x20), t);
                z = lerp_position(at<s32>(s, 0xc), at<s32>(s, 0x24), t);
                a = lerp_angle(at<s32>(s, 0x10), at<s32>(s, 0x28), t);
                b = lerp_angle(at<s32>(s, 0x18), at<s32>(s, 0x30), t);
                c = lerp_angle(at<s32>(s, 0x14), at<s32>(s, 0x2c), t);
                x = ease_position(x, at<float>(s, 0x34));
                y = ease_position(y, at<float>(s, 0x38));
                z = ease_position(z, at<float>(s, 0x3c));
                a = ease_angle(a, at<float>(s, 0x48), 4);
                b = ease_angle(b, at<float>(s, 0x40), 4);
                c = ease_angle(c, at<float>(s, 0x44), 8);
            }
            at<u32>(s, 0x34) = bits_of(x);
            at<u32>(s, 0x38) = bits_of(y);
            at<u32>(s, 0x3c) = bits_of(z);
            at<u32>(s, 0x48) = bits_of(a);
            at<u32>(s, 0x40) = bits_of(b);
            at<u32>(s, 0x44) = bits_of(c);
            float angles[3] = {0, 0, 0};
            const u32 ride = at<u32>(s, 0xc0);
            if (!ride) continue;
            Matrix_ResetRotation(ride_matrix(ride) + 4);
            u8 *const m = reinterpret_cast<u8 *>(ride_matrix(ride));
            at<u32>(m, 0x40) = bits_of(x);
            at<u32>(m, 0x44) = X87_FloatThrough(bits_of(y));
            at<u32>(m, 0x48) = X87_FloatThrough(bits_of(z));
            angles[0] = (float)(X87Reg(kPi) * X87Reg(b) * X87Reg(-1.0f / 2048));   // 0x7010cc
            angles[2] = (float)(X87_SubR((X87Reg(3072.0f) - X87Reg(a)) * X87Reg(-1.0f / 2048), X87Reg(1.0f)) * X87Reg(kPi));
            angles[1] = (float)(X87Reg(kPi) * X87Reg(c) * X87Reg(-1.0f / 512));   // 0x7010d4
            Matrix_RotateEulerFast(ride_matrix(ride) + 4, 0xe, angles);
            const u32 system = at<u32>(s, 0xc4);
            if (!system) continue;
            const u32 part = Model_FindPartByKey4(ride_of(ride), 0x100, 1);
            if (part == 0xffffffff) continue;
            const u8 *const entry = at<const u8 *>(at<u8 *>(at<u8 *>(at<u8 *>(ride_of(ride), 8), 0x28), 4) + part * 0x14, 4);
            float px = 0, py = 0, pz = 0;
            if (entry) {
                px = at<float>(entry, 0x30);
                py = at<float>(entry, 0x34);
                pz = at<float>(entry, 0x38);
            }
            const s32 vz = (s32)((u32)(s32)(long long)pz << 10);
            const s32 vy = (s32)((u32)(s32)(long long)py << 10);
            const s32 vx = (s32)((u32)(s32)(long long)px << 10);
            ParticleSystem_SetVectorA(system, vx, vy, vz);
        }
    }
}
}
