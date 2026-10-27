#include "carsound.h"

#include <cstdint>

#include "../core/soundset.h"

namespace {

template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

bool Silenced() { return *reinterpret_cast<const u32 *>(0x00785a2c) != 0; }

}  // namespace

extern "C" {

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
