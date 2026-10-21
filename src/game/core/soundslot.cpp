#include "soundslot.h"

#include <cfloat>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "releaselist.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool isNan(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return _isnan(f) != 0;   // (fld; fstp double; _isnan 0x67b410)
}

}  // namespace

extern "C" {

u8 *__fastcall SoundSlot_Init(u8 *slot) {
    static const u8 kCleared[] = {0x0, 0x4, 0x8, 0xc, 0x14, 0x18, 0x1c, 0x24};
    for (const u8 offset : kCleared) at<u32>(slot, offset) = 0;
    return slot;
}

u32 __fastcall SoundSlot_Call7(u8 *slot, void *, u32 value) {
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) return 0;
    return comcall<u32>(object, 0x1c, value);
}

u32 *__fastcall SoundSlot_Call5(u8 *slot, void *, u32 *out, u32 value) {
    u8 *const object = at<u8 *>(slot, 0);
    *out = object ? comcall<u32>(object, 0x14, value) : 0xffffffff;
    return out;
}

u8 *__fastcall SoundSlot_Set(u8 *slot, void *, u8 *object) {
    if (u8 *const old = at<u8 *>(slot, 0)) {
        comcall<u32>(old, 0x38);
        ReleaseList_Push(ReleaseList_Get(), nullptr, at<u32>(slot, 0));   // (+0 read again)
        at<u8 *>(slot, 0) = nullptr;
    }
    at<u8 *>(slot, 0) = object;
    return object;
}

u32 *__fastcall SoundSlot_Call59(u8 *slot, void *, u32 *out, u32 a, u32 b) {
    u8 *const object = at<u8 *>(slot, 0);
    *out = object ? comcall<u32>(object, 0xec, a, b) : 0xffffffff;
    return out;
}

u32 __fastcall SoundSlot_IsSet(u8 *slot) { return at<u8 *>(slot, 0) != nullptr; }

u32 *__fastcall SoundSlot_Restart(u8 *slot, void *, u32 *out) {
    if (!at<u8 *>(slot, 0)) {
        *out = 0xffffffff;
        return out;
    }
    u32 answer;
    if (at<u8>(slot, 0x24) & 4) SoundSlot_SetDistances(slot, nullptr, &answer, &at<u32>(slot, 0x14));
    if (at<u8>(slot, 0x24) & 2) SoundSlot_SetPosition(slot, nullptr, &answer, &at<u32>(slot, 4));
    if (at<u8>(slot, 0x24) & 1) SoundSlot_SetPitch(slot, nullptr, &answer, at<s32>(slot, 0x10));
    if (at<u8>(slot, 0x24) & 8) SoundSlot_SetGain(slot, nullptr, &answer, at<u32>(slot, 0x20));
    if (at<u8>(slot, 0x24) & 0x20) SoundSlot_Set48(slot, nullptr, &answer, at<u32>(slot, 0x28));
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) {   // (read again)
        *out = 0xffffffff;
        return out;
    }
    *out = comcall<u32>(object, 0x34, 1u);
    return out;
}

void __fastcall SoundSlotGuard_Dtor(u8 *guard) {
    if (!at<u8>(guard, 4)) return;
    u8 *const system = global<u8 *>(0x00fb2724);
    u32 answer;
    vcall<u32>(system, 0x28, &answer, at<u32>(guard, 8));
}

u32 *__fastcall SoundSlot_Free(u8 *slot, void *, u32 *out) {
    if (u8 *const object = at<u8 *>(slot, 0)) {
        comcall<u32>(object, 0x38);
        ReleaseList_Push(ReleaseList_Get(), nullptr, at<u32>(slot, 0));   // (+0 read again)
        at<u8 *>(slot, 0) = nullptr;
    }
    *out = 0;
    return out;
}

u32 *__fastcall SoundSlot_SetPosition(u8 *slot, void *, u32 *out, const u32 *vector) {
    at<u32>(slot, 0x24) |= 2;
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) {
        at<u32>(slot, 4) = vector[0];
        at<u32>(slot, 8) = vector[1];
        at<u32>(slot, 0xc) = vector[2];
        *out = 0xffffffff;
        return out;
    }
    const u32 x = vector[0], y = vector[1], z = vector[2];
    if (isNan(x) || isNan(y) || isNan(z)) {
        *out = 0xffffffff;
        return out;
    }
    *out = comcall<u32>(object, 0x50, x, 0u, z);   // sic: y not passed
    return out;
}

u32 *__fastcall SoundSlot_SetPitch(u8 *slot, void *, u32 *out, s32 steps) {
    at<u32>(slot, 0x24) |= 1;
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) {
        at<s32>(slot, 0x10) = steps;
        *out = 0xffffffff;
        return out;
    }
    u32 ratio;
    if (steps == 0) ratio = 0;
    else if (steps < 0) ratio = global<u32>(0x00784610 - (u32)steps * 4);
    else ratio = X87_FloatThrough(global<u32>(0x00784010 + (u32)steps * 4));
    *out = comcall<u32>(object, 0xa8, ratio);
    return out;
}

u32 *__fastcall SoundSlot_SetDistances(u8 *slot, void *, u32 *out, const u32 *vector) {
    at<u32>(slot, 0x24) |= 4;
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) {
        at<u32>(slot, 0x14) = vector[0];
        at<u32>(slot, 0x18) = vector[1];
        at<u32>(slot, 0x1c) = vector[2];
        *out = 0xffffffff;
        return out;
    }
    comcall<u32>(object, 0x98, vector[0], vector[1], 1u);
    comcall<u32>(at<u8 *>(slot, 0), 0xb8, vector[2]);   // (+0 read again)
    *out = 0;
    return out;
}

u32 *__fastcall SoundSlot_SetGain(u8 *slot, void *, u32 *out, u32 gain) {
    at<u32>(slot, 0x24) |= 8;
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) {
        at<u32>(slot, 0x20) = gain;
        *out = 0xffffffff;
        return out;
    }
    const float level = (float)X87_MulInt(gain, global<float>(0x00711c18));
    u32 bits;
    std::memcpy(&bits, &level, 4);
    *out = comcall<u32>(object, 0xa0, bits);
    return out;
}

u32 *__fastcall SoundSlot_Set48(u8 *slot, void *, u32 *out, u32 value) {
    at<u32>(slot, 0x24) |= 0x20;
    u8 *const object = at<u8 *>(slot, 0);
    if (!object) {
        at<u32>(slot, 0x28) = value;
        *out = 0xffffffff;
        return out;
    }
    *out = comcall<u32>(object, 0xc0, value);
    return out;
}

}  // extern "C"
