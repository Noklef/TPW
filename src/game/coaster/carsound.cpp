#include "carsound.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../rsse/scream.h"
#include "../world/ridelist.h"

namespace {

template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

bool Silenced() { return *reinterpret_cast<const u32 *>(0x00785a2c) != 0; }

}  // namespace

extern "C" {

void __cdecl CoasterCar_StopSounds(u8 *, u8 *car) {
    if (at<u32>(car, 0x64) && !Silenced()) Sound_Forward6(at<u32>(car, 0x64));
    if (at<u32>(car, 0x68) && !Silenced()) Sound_Forward6(at<u32>(car, 0x68));
    if (at<u32>(car, 0x70) && !Silenced()) RSSE_StopScream(at<u32>(car, 0x70));
    for (u32 offset = 0x54; offset <= 0x70; offset += 4) at<u32>(car, offset) = 0;
}

void __cdecl CoasterCar_StartSounds(u8 *coaster, u8 *car, u32 quiet) {
    CoasterCar_StopSounds(coaster, car);
    if (!Silenced()) {
        for (s32 k = 0; k < 4; ++k) at<u32>(car, 0x54 + 4 * k) = Rides_GetLinkedElement(at<s32>(coaster, 0x24), 5 + k);
        const auto ftol = [](float f) { return (s32)(long long)f; };   // __ftol
        for (s32 k = 0; k < 2; ++k) {
            const s32 z = ftol(at<float>(car, 0x50)), y = ftol(at<float>(car, 0x4c)), x = ftol(at<float>(car, 0x48));
            at<u32>(car, 0x64 + 4 * k) = Sound_PlayRideElement(at<s32>(coaster, 0x24), 0, *reinterpret_cast<const u32 *>(0x00803a3c), k, x, y, z);
        }
    }
    if (!quiet) CoasterCar_SetSoundMode(car, 2, 100);
}

void __cdecl CoasterCar_SetSoundMode(u8 *car, s32 mode, u32 value) {
    if (mode != at<s32>(car, 0x38)) {
        u32 level = 0;   // (a jump table on mode - 1, 0..15)
        switch (mode) {
        case 1: level = 0xf; break;
        case 2: level = 0x1e; break;
        case 4: level = 0x46; break;
        case 8: level = 0x5a; break;
        case 16: level = 0x32; break;
        default: break;
        }
        if (level && !Silenced())
            at<u32>(car, 0x68) = Sound_SetParam(at<u32>(car, 0x68), at<u32>(car, 0x5c), level);
        at<s32>(car, 0x38) = mode;
    }
    if (!Silenced())
        at<u32>(car, 0x68) = Sound_SetParam(at<u32>(car, 0x68), at<u32>(car, 0x60), value);
}

void __cdecl CoasterCar_UpdateScream(u8 *link, u8 *car, s32 x, s32 y, s32 z) {
    const s32 index = _mm_cvtss_si32(_mm_set_ss(at<float>(car, 0x74)));   // (fistp)
    u8 *const entry = at<u8 *>(link, 0x9c) + index * 0x38;
    const u8 bits = entry[0];
    const u32 flags = at<u32>(car, 0);   // (as read at the start)
    if (flags & 0x100) {
        const u32 sound = at<u32>(car, 0x6c);
        if (bits & 2) {
            if (sound) {
                if (!Silenced()) at<u32>(car, 0x6c) = RSSE_MoveScream(sound, x, y, z);
            } else if (!(flags & 0x10) && !(link[8] & 0x40)) {
                at<u32>(car, 0x6c) = KidSound_PlayGraded(1, 100, 100, x, y, z);   // (0x785a2c not looked at)
            }
        } else if (sound) {
            at<u32>(car, 0x6c) = RSSE_MoveScream(sound, x, y, z);   // (0x785a2c not looked at)
        }
        if ((entry[0] & 4) && !(car[0] & 0x10) && !(link[8] & 0x40)) {
            if (Silenced()) return;
            RSSE_MoveScream(at<u32>(car, 0x70), x, y, z);
            RSSE_SetScreamVolume(at<u32>(car, 0x70), 0x4b, 0x4b);
            return;
        }
        if (!Silenced()) RSSE_StopScream(at<u32>(car, 0x70));
        at<u32>(car, 0x70) = 0;
        at<u32>(car, 0) &= ~0x100u;
        return;
    }
    if ((bits & 2) && !(flags & 0x10) && !(link[8] & 0x40) && !Silenced())
        at<u32>(car, 0x6c) = KidSound_PlayGraded(1, 100, 100, x, y, z);
    if ((entry[0] & 4) && !(car[0] & 0x10) && !(link[8] & 0x40)) {
        if (!at<u32>(car, 0x70) && !Silenced())
            at<u32>(car, 0x70) = RSSE_StartScream(0, (at<u32>(car, 0x10) >> 1) + 1, 0x4b, 0x4b, x, y, z);
        at<u32>(car, 0) |= 0x100;
    }
}
}
