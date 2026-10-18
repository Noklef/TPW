#include "litter.h"

#include <cstdint>

#include "../render/particles.h"
#include "../sprite/script.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Litter_Remove(u8 *litter) {
    const u32 kind = at<u32>(litter, 0x2c);
    if (kind == 7) {
        Map_SpreadRemove(global<u8 *>(0x008023a0), nullptr, 2, at<u16>(litter, 0));
    } else if (kind == 8) {
        Map_SpreadRemove(global<u8 *>(0x008023a0), nullptr, 5, at<u16>(litter, 0));
        if (at<u32>(litter, 0x38) != 0)
            ParticleSystem_End(at<u32>(litter, 0x38));
    }
    SpriteScript_Kill((s32)at<u32>(litter, 0x28));
    at<u32>(litter, 0x28) = 0;
    at<u16>(litter, 0x30) = 0;
    at<u32>(litter, 0x2c) = 0;
}

u32 __fastcall Obj_StampAt30(u8 *object, void *, u32 word) {
    at<u16>(object, 0x30) = (u16)word;
    const u32 stamp = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(object, 0x34) = stamp;
    return stamp;
}
}  // extern "C"
