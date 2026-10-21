#include "soundslot.h"

#include <cstdint>

#include "releaselist.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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

}  // extern "C"
