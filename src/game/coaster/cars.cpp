#include "cars.h"

#include <cmath>
#include <cstdint>

#include "../core/actionrec.h"
#include "../core/soundplay.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/partstate.h"
#include "contacts.h"
#include "nodes.h"
#include "queue.h"
#include "registry.h"
#include "train.h"
#include "carsound.h"
#include "../model/holder.h"
#include "../motion/animator.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
const float *position(const u8 *car) { return &at<const float>(car, 0x4c); }

const float kPi = 3.14159274f;  // 0x6fe028, and its negation at 0x6fe02c

}  // namespace

extern "C" {

double __cdecl CoasterCar_SwingRate(const u8 *coaster, const u8 *car, float x) {
    const float angle = at<float>(car, 0x94);
    if (!(angle < 0.0f || angle > 0.0f)) return 0.0;  // fcomp 0.0; test ah,0x40
    if (at<u32>(car, 8) & 0x100) return 0.0;
    const float behind = (float)Vec3_Distance(position(at<const u8 *>(car, 0x68)), position(car));
    const X87Reg both = X87Reg(Vec3_Distance(position(at<const u8 *>(car, 0x64)), position(car))) + behind;
    const X87Reg swing =
        (X87Reg(angle < 0.0f ? -kPi : kPi) - angle) * x / at<float>(at<const u8 *>(coaster, 4), 0x10);
    return X87_DivR(both, swing).v;
}

void __cdecl CoasterCar_Wake(u8 *coaster, u8 *car) {
    if (!(coaster[0x3c] & 4)) return;
    u8 *const *const strips = reinterpret_cast<u8 *const *>(0x007a4610);
    at<u32>(coaster, 0) |= 0x800;
    Motion_Time(0);   // (its answer unused)
    at<u32>(car, 0) |= 2;
    u8 *const segments = at<u8 *>(car, 0x28);
    for (u32 i = 0; i < at<u32>(car, 0xc); ++i) at<u32>(segments + 0x60 * i, 0x1c) = 0;
    u8 *segment = at<u8 *>(car, 0x28) + at<u32>(car, 0xc) * 0x60 - 0x60;   // the last first
    for (u32 n = 0; n < at<u32>(car, 0xc); ++n, segment -= 0x60) {
        at<u32>(segment, 0) |= 1;
        if (const u32 index = at<u32>(segment, 0xc)) {
            u8 *const strip = strips[index];
            if (at<u32>(strip, 4) & 0x4000)
                Holder_AddToGrid(at<u8 *>(strip, 8), at<u32>(strip, 0xc0), at<u32>(strip, 0xc4));
            else
                Holder_Wake(at<u8 *>(strip, 8));
        }
        at<u32>(segment, 0x28) = at<u32>(segment, 0x20);
        at<u32>(segment, 0x2c) = at<u32>(segment, 0x24);
        at<u32>(segment, 0x30) = 0;
        at<u32>(segment, 0x34) = 0;
        const u8 type = *at<const u8 *>(segment, 4);   // (read before the index is tested)
        if (!(type & 1) && at<u32>(segment, 0xc)) Animator_Play(strips[at<u32>(segment, 0xc)], 5, 0, 1, 1.0f, 0);
    }
    CoasterCar_StartSounds(coaster, car, 0);
}

void __cdecl CoasterCar_Sleep(u8 *coaster, u8 *car) {
    u8 *const *const strips = reinterpret_cast<u8 *const *>(0x007a4610);
    u8 *segment = at<u8 *>(car, 0x28) + at<u32>(car, 8) * 0x60;
    for (u32 n = at<u32>(car, 0xc); n != 0; --n, segment += 0x60) {
        at<u32>(segment, 0) &= ~1u;
        if (const u32 index = at<u32>(segment, 0xc)) {
            u8 *const strip = strips[index];
            if (at<u32>(strip, 4) & 0x4000)
                Obj_RemoveFromRect(at<u8 *>(strip, 8), at<u32>(strip, 0xc0), at<u32>(strip, 0xc4));
            else
                Holder_Sleep(at<u8 *>(strip, 8));
        }
    }
    CoasterCar_StopSounds(coaster, car);
    at<u32>(car, 0) &= ~2u;
}

void __cdecl Coaster_SetState(u8 *coaster, u32 value) {
    const u32 state = value & 0xf;
    if (state) {
        const u32 old = at<u32>(coaster, 8);
        const u32 from = old & 0xf;
        if (from == 1 && state == 2) {
            if (!(old & 0x40)) at<u32>(coaster, 0x2c) = 0x3f800000;   // 1.0f
            u8 *car = at<u8 *>(coaster, 0x128);
            for (u32 i = 0; i < at<u32>(coaster, 0xf4); ++i, car += 0x80) CoasterCar_Wake(coaster, car);
            Coaster_FlashCar(coaster, 0);
        } else if (from == 2 && state == 1) {
            for (u32 i = 0; i < at<u32>(coaster, 0xf4); ++i) CoasterCar_Sleep(coaster, at<u8 *>(coaster, 0x128) + i * 0x80);
            at<u32>(coaster, 0) &= ~0x800u;
            Coaster_FlashCar(coaster, 2);
        } else if ((from == 4 || from == 8) && state == 2) {
            u8 *car = at<u8 *>(coaster, 0x128);
            for (u32 i = 0; i < at<u32>(coaster, 0xf4); ++i, car += 0x80) {
                CoasterTrain_Unload(coaster, car, 0);
                CoasterTrain_Unload(coaster, car, 1);
            }
            CoasterQueue_Advance(coaster);
        } else if ((from == 1 || from == 2 || from == 4 || from == 8) && state == 8) {
            CoasterQueue_Advance(coaster);
        }
        at<u32>(coaster, 8) = (at<u32>(coaster, 8) & ~0xfu) | state;
    }
    const u32 mode = value & 0x70;
    if (!mode) return;
    const u32 was = at<u32>(coaster, 8) & 0x70;
    if ((was == 0x10 || was == 0x20) && mode == 0x40) {
        at<u32>(coaster, 0x2c) = 0;
    } else if (was == 0x40 && (mode == 0x10 || mode == 0x20)) {
        at<u32>(coaster, 0x2c) = 0x3f800000;   // 1.0f
        u8 *const cars = at<u8 *>(coaster, 0x128);
        for (u32 i = 0; i < at<u32>(coaster, 0xf4); ++i) at<u32>(cars + i * 0x80, 0) = (at<u32>(cars + i * 0x80, 0) & ~0x40u) | 0x80;
    }
    at<u32>(coaster, 8) = (at<u32>(coaster, 8) & ~0x70u) | mode;
}

void __cdecl Coaster_ResumeRunning(u8 *coaster) {
    at<u32>(at<u8 *>(coaster, 0x60), 8) &= ~0x50000u;
    const u32 first = at<u32>(at<u8 *>(coaster, 0x60), 8);
    at<u32>(at<u8 *>(coaster, 0x64), 8) &= (first & 0x400) ? ~0x20000u : ~0xa0000u;
    const u32 flags = at<u32>(coaster, 0x3c);
    at<u32>(coaster, 0x60) = at<u32>(coaster, 0x58);
    at<u32>(coaster, 0x3c) = flags & ~2u;
    Coaster_SetState(coaster, 2);
    Flags_SetChanged(reinterpret_cast<u32 *>(coaster), 1);
}

void __cdecl Coaster_SpaceCars(u8 *coaster) {
    const float gap = (float)(X87Reg((double)at<u32>(at<const u8 *>(coaster, 4), 0x2ec)) * 0.001f);
    const float scale = at<float>(coaster, 0xac);
    const u32 count = at<u32>(coaster, 0xf4);
    u8 *const cars = at<u8 *>(coaster, 0x128);
    for (u32 i = 0; i < count; ++i) {
        u8 *car = cars + i * 0x80;
        const u32 k = at<u32>(car, 8) + at<u32>(car, 0xc);
        float &p = at<float>(car, 0x30);
        p = X87_Through(at<float>(at<u8 *>(car, 0x28), k * 0x60 - 0x20));
        if (!(p >= 0.0f)) p = (float)(X87Reg(at<float>(coaster, 0xa8)) + p);
    }
    u32 next = 1 % count;  // no cars: divide by zero, as the original
    const u8 *before = cars;
    at<u8 *>(coaster, 0x124) = cars;
    for (u32 left = count - 1; left; --left) {
        u8 *car = cars + next * 0x80;
        const u8 *nodes = at<const u8 *>(coaster, 0x9c);
        double whole;
        const double frac = std::modf((double)at<float>(before, 0x30), &whole);
        u32 node = (u32)(long long)whole;
        X87Reg last = 0.0;
        X87Reg sum = (X87Reg(1.0f) - frac) * scale / at<float>(nodes, node * 0x38 + 0x1c);
        while (!(sum.v >= gap)) {  // sic: forever on a NaN
            node = (node + 1) % at<u32>(coaster, 0xa4);
            last = sum;
            sum = sum + X87Reg(scale) / at<float>(nodes, node * 0x38 + 0x1c);
        }
        const X87Reg w = (sum - gap) / (sum - last);
        const float x = (float)(X87Reg((double)(int)(node - 1)) * (X87Reg(1.0f) - w) + X87Reg((double)(int)node) * w);
        const X87Reg length = at<float>(coaster, 0xa8);
        at<float>(car, 0x30) = (float)std::fmod((length + x).v, length.v);
        next = (next + 1) % count;
        before = car;
    }
}

void __cdecl CoasterCar_SetSegmentOffsets(const u8 *coaster, u8 *car) {
    const u8 *type = at<const u8 *>(car, 4);
    const auto segment = [car](s32 i) { return at<u8 *>(car, 0x28) + i * 0x60; };
    const auto length = [car, type](const u8 *seg) {
        const u32 i = at<u32>(seg, 8);
        return at<float>(type, i == 0 ? 0x1c : i == at<u32>(car, 0xc) - 1 ? 0x24 : 0x20);
    };
    const auto place = [coaster](u8 *seg, X87Reg offset) {
        at<float>(seg, 0x38) = (float)offset;
        at<float>(seg, 0x3c) = (float)(offset / at<float>(coaster, 0xac));
    };
    const s32 mid = at<s32>(car, 0x2c);
    at<u32>(segment(mid), 0x38) = 0;
    at<u32>(segment(mid), 0x3c) = 0;
    X87Reg sum = 0.0f;
    for (s32 i = (s32)((u32)mid - 1); i >= 0; --i) {
        u8 *seg = segment(i);
        sum = sum + length(seg);
        place(seg, sum);
    }
    sum = 0.0f;
    for (s32 i = (s32)((u32)mid + 1); i < at<s32>(car, 0xc); ++i) {
        u8 *seg = segment(i);
        sum = sum - length(seg);
        place(seg, sum);
    }
}

void __cdecl Coaster_SetSegmentWindow(u8 *coaster) {
    const auto value = [](const u8 *car) { return at<float>(at<const u8 *>(at<const u8 *>(car, 0x90), 4), 8); };
    const u8 *car = at<const u8 *>(coaster, 0x54);
    const float a = value(car), b = value(at<const u8 *>(at<const u8 *>(car, 0x64), 0x64));
    const float k = at<float>(at<const u8 *>(coaster, 4), 0x31c);
    const X87Reg mid = (X87Reg(b) + a) * 0.5f, near = X87Reg(k) * mid;
    at<float>(coaster, 0xf8) = (float)(near + (X87Reg(1.0f) - k) * a);
    at<float>(coaster, 0xfc) = (float)(near + (X87Reg(1.0f) - k) * b);
}

void __cdecl CoasterCar_FindSegmentsInWindow(const u8 *coaster, const u8 *car, s32 *first, s32 *last) {
    const float scale = at<float>(coaster, 0xac);
    const u8 *seg = at<const u8 *>(car, 0x28) + at<u32>(car, 8) * 0x60;
    const u8 *found = nullptr, *final = nullptr;
    for (u32 n = at<u32>(car, 0xc); n != 0; --n, seg += 0x60) {
        const u8 *type = at<const u8 *>(seg, 4);
        const X87Reg ahead = X87Reg(at<float>(type, 0x24)) / scale, behind = X87Reg(at<float>(type, 0x28)) / scale;
        const float at40 = at<float>(seg, 0x40);
        if (!((ahead + at40).v >= at<float>(coaster, 0xf8))) continue;
        if (X87_SubR(behind, at40).v > at<float>(coaster, 0xfc)) continue;
        if (!found) found = seg;
        final = seg;
    }
    const uintptr_t base = (uintptr_t)at<const u8 *>(car, 0x28);
    *first = (s32)((uintptr_t)found - base) / 0x60;
    *last = (s32)((uintptr_t)final - base) / 0x60;
}

void __cdecl CoasterCar_SetExtent(const u8 *coaster, u8 *car) {
    const float scale = at<float>(coaster, 0xac);
    const u8 *front = at<const u8 *>(car, 0x28) + at<u32>(car, 8) * 0x60;
    const u8 *back = front + at<u32>(car, 0xc) * 0x60 - 0x60;
    const auto offset = [scale, car](const u8 *seg) { return X87Reg(at<float>(seg, 0x38)) / scale + at<float>(car, 0x74); };
    const auto length = [scale](const u8 *seg, u32 at_) { return X87Reg(at<float>(at<const u8 *>(seg, 4), at_)) / scale; };
    at<float>(car, 0x20) = (float)(offset(front) + length(front, 0x24));
    at<float>(car, 0x24) = (float)(offset(back) - length(back, 0x28));
}

double __cdecl Coaster_SpeedAfterDrop(const u8 *params, float speed, float minimum, float fromY, float toY) {
    const X87Reg drop = (X87Reg(fromY) - toY) * at<float>(params, 0x24);
    const X87Reg next = drop * at<float>(params, toY > fromY ? 0x28 : 0x2c) + speed;
    return next.v > minimum ? next.v : minimum;
}

void __cdecl Coaster_SetupCars(u8 *coaster, u32 riders, u32 segments, u32 flags) {
    u8 *car = at<u8 *>(coaster, 0x128);
    for (u32 i = 0; i < at<u32>(at<const u8 *>(coaster, 4), 0x2e8); ++i, car += 0x80) {
        if ((at<u8>(car, 0) & 4) || (segments != 0 && segments != at<u32>(car, 0xc))) {
            if (segments) at<u32>(car, 0xc) = segments;
            const u32 count = at<u32>(car, 0xc);
            at<u32>(car, 8) = 0;
            at<u32>(car, 0x2c) = (count - 1) >> 1;
            if (count) {
                CoasterCar_SetSegmentOffsets(coaster, car);
                CoasterCar_SetExtent(coaster, car);
            }
        }
        u8 *seg = at<u8 *>(car, 0x28);
        if (!(flags & 1)) continue;
        if (!riders) riders = at<u32>(at<const u8 *>(car, 4), 8);
        at<u32>(coaster, 0xf0) = riders;
        const u8 *type = at<const u8 *>(car, 4);
        u32 seats = at<u32>(car, 0xc);
        if (at<u32>(type, 0xc) != at<u32>(type, 0x10)) --seats;
        if (at<u32>(type, 0x14) != at<u32>(type, 0x10)) --seats;
        u32 left = riders, seated = 0;
        for (u32 j = 0; j < at<u32>(car, 0xc); ++j, seg += 0x60) {
            if (at<u32>(seg, 4) != at<u32>(at<const u8 *>(car, 4), 0x10)) {
                at<u32>(seg, 0x44) = 0;
                continue;
            }
            const u32 share = left / (seats - seated++);
            left -= share;
            at<u32>(seg, 0x44) = share;
        }
    }
    at<u32>(coaster, 0xb8) = at<u32>(coaster, 0xf0);
}

void __cdecl Coaster_SetRiders(u32 index, s32 n) {
    u8 *coaster = reinterpret_cast<u8 **>(0x00790bd0)[index];
    if (n < 0) n = 0;
    const s32 most = at<s32>(at<const u8 *>(coaster, 4), 0x318);
    if (most < n) n = most;
    const s32 seats = at<s32>(at<const u8 *>(at<const u8 *>(coaster, 0x128), 4), 8);
    if (seats < n) n = seats;
    if ((u32)n != at<u32>(coaster, 0xf0)) Coaster_SetupCars(coaster, (u32)n, 0, 1);
}

void __cdecl Coaster_SetMix(u8 *coaster, u32 value, u32 quiet) {
    if (!quiet) ActionRec_RecordCoaster45(coaster, value);
    const u8 *type = at<const u8 *>(coaster, 4);
    const X87Reg p = X87_MulInt(value, 0.01f);  // (0x6fe034)
    const X87Reg mix = (X87Reg(1.0f) - p) * X87Reg(at<const float>(type, 0x4c)) + X87Reg(at<const float>(type, 0x50)) * p;
    at<u32>(coaster, 0x30) = value;
    at<float>(coaster, 0x34) = (float)mix;
    Flags_SetChanged(reinterpret_cast<u32 *>(coaster), 1);
    if (at<const u8>(at<const u8 *>(coaster, 4), 0) & 8) Contacts_ClearAll(coaster);
}

void __cdecl Coaster_SetMixAt(u32 index, u32 value) {
    u8 *coaster = reinterpret_cast<u8 **>(0x00790bd0)[index];
    if (!coaster || value == at<u32>(coaster, 0x30)) return;
    if (!*reinterpret_cast<const u32 *>(0x00785a2c) && (at<u32>(coaster, 0) & 0x8000)) return;
    Coaster_SetMix(coaster, value, 0);
}

void __cdecl Coaster_SetMixAtPinned(u32 index, u32 value) {
    Coaster_SetMixAt(index, value);
    if (u32 *const pinned = reinterpret_cast<u32 **>(0x00790bd0)[index]) *pinned |= 0x8000;
}

void __cdecl Coaster_SetModeAt(u32 index, u32 which) {
    u8 *const coaster = reinterpret_cast<u8 **>(0x00790bd0)[index];
    switch (which) {
    case 0: Coaster_SetState(coaster, 0x10); break;
    case 1: Coaster_SetState(coaster, 0x40); break;
    case 2: Coaster_SetState(coaster, 0x20); break;
    default: break;
    }
}

void __cdecl Coaster_StepStateAt(u32 index, u32 on) {
    u8 *const coaster = reinterpret_cast<u8 **>(0x00790bd0)[index];
    const u8 low = at<u8>(coaster, 8);
    if (on) {
        if (low & 4) Coaster_SetState(coaster, 8);
    } else if (low & 0xa) {
        Coaster_SetState(coaster, 4);
    }
}

void __cdecl Coaster_BoardRiders(u8 *queue, u8 *coaster, u32 first, u32 count) {
    while (at<u32>(coaster, 0x14) && count) {
        if (!at<u32>(queue, 0xbc)) return;
        const uintptr_t read = at<uintptr_t>(queue, 0xc4), slots = at<uintptr_t>(queue, 0xb4);
        const u32 rider = *reinterpret_cast<const u32 *>(read);
        at<uintptr_t>(queue, 0xc4) = slots + 4 * ((u32)((s32)(read - slots + 4) >> 2) % at<u32>(queue, 0xb0));
        --at<u32>(queue, 0xbc);
        if (!rider) return;
        u32 &seed = *reinterpret_cast<u32 *>((uintptr_t)0x007b0958);
        seed = seed * 214013 + 2531011;
        const uintptr_t cars = at<uintptr_t>(coaster, 0x28);
        uintptr_t car = cars + (u32)(((seed >> 16) % count + first) * 0x60);
        for (u32 k = 0; k < count; ++k) {
            if (at<u32>((void *)car, 0x34) < at<u32>((void *)car, 0x44)) {
                u8 *const c = reinterpret_cast<u8 *>(car);
                const u8 *const kind = *reinterpret_cast<u8 *const *>((uintptr_t)(0x007a4610 + 4 * at<u32>(c, 0xc)));
                PartStates_Activate(at<u8 *>(kind, 8), at<const u32 *>(at<const u8 *>(c, 4), 0x2c)[at<u32>(c, 0x34)], rider);
                ++at<u32>(coaster, 0x10);
                at<u32 *>(c, 0x2c)[at<u32>(c, 0x34)] = rider;
                ++at<u32>(c, 0x34);
                break;
            }
            car += 0x60;
            if (car >= cars + (u32)((first + count) * 0x60)) car = cars + (u32)(first * 0x60);
        }
        --at<u32>(coaster, 0x14);
    }
}

void __cdecl CoasterTrain_AtStation(u8 *coaster, u8 *train) {
    const auto seats = [train](u32 first, u32 count) {   // the seats free in the cars from first
        at<u32>(train, 0x14) = 0;
        const u8 *car = at<u8 *>(train, 0x28) + first * 0x60;
        for (u32 i = 0; i < count; ++i, car += 0x60) at<u32>(train, 0x14) += at<u32>(car, 0x44) - at<u32>(car, 0x34);
    };
    const bool there = at<float>(train, 0x20) >= at<float>(coaster, 0xf8) && !(at<float>(train, 0x24) > at<float>(coaster, 0xfc));
    if (!there) {
        if (!(at<u8>(train, 0) & 0x20)) return;
        if (at<u8>(coaster, 8) & 4) {
            const u32 first = at<u32>(train, 0x18), count = at<u32>(train, 0xc) - first;
            seats(first, count);
            Coaster_BoardRiders(coaster, train, first, count);
        }
        at<u32>(train, 0x18) = 0;
        at<u32>(train, 0x1c) = 0;
        u8 *car = at<u8 *>(train, 0x28) + at<u32>(train, 8) * 0x60;
        for (u32 i = 0; i < at<u32>(train, 0xc); ++i, car += 0x60) {
            const u32 off = at<u32>(car, 0x28);
            at<u32>(car, 0x28) = at<u32>(car, 0x2c);
            at<u32>(car, 0x2c) = off;
            at<u32>(car, 0x30) = at<u32>(car, 0x34);
            at<u32>(car, 0x34) = 0;
        }
        at<u32>(train, 0) &= ~0x20u;
        return;
    }
    if (!(at<u8>(train, 0) & 0x20)) CoasterTrain_Unload(coaster, train, 1);
    if (at<u8>(coaster, 8) & 4) {
        s32 first, last;
        CoasterCar_FindSegmentsInWindow(coaster, train, &first, &last);
        const u32 from = (u32)first < at<u32>(train, 0x18) ? (u32)first : at<u32>(train, 0x18);
        const u32 to = (u32)last > at<u32>(train, 0x1c) ? (u32)last : at<u32>(train, 0x1c);
        seats(from, to - from + 1);
        Coaster_BoardRiders(coaster, train, from, to - from + 1);
        at<s32>(train, 0x18) = first;
        at<s32>(train, 0x1c) = last;
    }
    at<u32>(train, 0) |= 0x20;
}

void __cdecl CoasterCars_InitUnusedOne() { *reinterpret_cast<float *>(0x007923a0) = 1.0f; }

void __cdecl CoasterCars_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007923a4) = 1.0f / 8388608.0f; }

void __cdecl CoasterCar_PlayClank(const u8 *coaster, const u8 *car) {
    // fld car+0x20; fld coaster+0xa8; _CIfmod; modf(that, &whole); fstp float (the argument's slot); __ftol(whole)
    double whole;
    const double place = std::fmod((double)at<float>(car, 0x20), (double)at<float>(coaster, 0xa8));
    const float t = (float)std::modf(place, &whole);
    float out[3];
    CoasterNode_Lerp(coaster, (u32)(s32)(long long)whole, t, out);
    if (*reinterpret_cast<const u32 *>(0x00785a2c) != 0)
        return;
    const auto ftol = [](float f) { return (s32)(long long)f; };
    Sound_PlayAt(0, *reinterpret_cast<const u32 *>(0x00803a28), 0xc, ftol(out[0]), ftol(out[1]), ftol(out[2]));
}

void __cdecl CoasterCar_Bump(const u8 *coaster, u8 *car) {
    const u32 count = at<u32>(coaster, 0xf4);
    if (count <= 1)
        return;
    const u32 base = at<u32>(coaster, 0x128);
    const u32 index = (u32)(((s32)((u32)(uintptr_t)car - base) >> 7) + 1) % count;
    u8 *const next = reinterpret_cast<u8 *>((uintptr_t)(index * 0x80 + base));
    // (Re-read through volatile where the original reads memory again: an off-grid car's +0x78 can be the next car's flags.)
    const auto mem = [](u8 *p, u32 o) { return *reinterpret_cast<volatile float *>(p + o); };
    X87Reg reach = X87Reg(at<float>(next, 0x24));
    if (!(at<float>(next, 0x74) >= at<float>(car, 0x74)))   // (below or unordered)
        reach = reach + at<float>(coaster, 0xa8);
    if (reach.v >= (double)at<float>(car, 0x20) || at<float>(next, 0x78) >= at<float>(car, 0x78))
        return;
    const float kMinus2 = -2.0f, kThird = 0.333333343f;   // (0x6fe05c, 0x6fe060)
    const X87Reg speed = X87Reg(mem(car, 0x78));
    const X87Reg pushed = X87Reg(mem(next, 0x78)) * kMinus2;
    const X87Reg kept = speed * kMinus2;
    *reinterpret_cast<volatile float *>(car + 0x78) = (float)((speed - pushed) * kThird);
    *reinterpret_cast<volatile u32 *>(next) = *reinterpret_cast<volatile u32 *>(next) | 0x40;
    const X87Reg ahead = (mem(next, 0x78) - kept) * kThird;
    *reinterpret_cast<volatile float *>(next + 0x78) = (float)ahead;
    const X87Reg gap = X87Reg(mem(car, 0x78)) - ahead;
    if (std::fabs(gap.v) > 1.0)   // (0x6fe020; not when below, equal or unordered)
        CoasterCar_PlayClank(coaster, car);
}
}
