#include "soundsource.h"

#include <cstdint>

#include "../math/x87.h"
#include "methods.h"
#include "sounddevice.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

u32 *__fastcall SoundSource711920_SetRate(u8 *source, void *, u32 *out, u32 low) {
    u32 status;   // (the original's is the low argument's own slot)
    if (static_cast<s32>(*Obj_SetRateAtC(source, nullptr, &status, low)) < 0) {
        *out = 0;
        return out;
    }
    const u32 rate = at<u32>(source, 0xc);
    SoundDevice_SetFormat(SoundDevice_Get(), nullptr, out, 2, 0x10, rate);
    return out;
}

u32 *__fastcall SoundSource711b08_SetOrientation(u8 *source, void *, u32 *out, const u32 *front, const u32 *top) {
    u8 *const listener = at<u8 *>(source, 0x1c);
    if (!listener) {
        *out = 0xffffffff;
        return out;
    }
    *out = comcall<u32>(listener, 0x2c, X87_FloatThrough(front[0]), 0u, X87_FloatThrough(front[2]), X87_FloatThrough(top[0]),
                        X87_FloatThrough(top[1]), X87_FloatThrough(top[2]));
    return out;
}
