#include "carsound.h"

#include <cstdint>

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
}
