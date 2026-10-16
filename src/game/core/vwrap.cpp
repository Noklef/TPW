#include "vwrap.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_CallSlot18(u8 *object, void *, u32 a) { return vcall<u32>(object, 0x18, a); }

u32 __fastcall Obj_PassToSlot10(u8 *object, void *, u8 *target) { return vcall<u32>(target, 0x10, object); }

u32 __fastcall Obj_RaiseIfSlot24Zero(u8 *object) {
    const u32 result = vcall<u32>(object, 0x24);
    if (result == 0)
        at<u32>(object, 4) = 1;
    return result;
}

u32 __fastcall Obj_ClearIfSlot24Set(u8 *object) {
    const u32 result = vcall<u32>(object, 0x24);
    if (result != 0)
        at<u32>(object, 4) = 0;
    return result;
}

u32 __stdcall Global7b05d8_CallSlot14(u32 a, u32 b) { return vcall<u32>(global<u8 *>(0x007b05d8), 0x14, a, b); }

void __cdecl Global7ca1e0_Release() {
    if (u8 *object = global<u8 *>(0x007ca1e0))
        vcall<u32>(object, 8, 1u);
}

void __cdecl Global7ca1c8_Release() {
    if (u8 *object = global<u8 *>(0x007ca1c8))
        vcall<u32>(object, 8, 1u);
}

void __cdecl Global87b1d8_Delete() {
    if (u8 *object = global<u8 *>(0x0087b1d8)) {
        vcall<u32>(object, 0, 1u);
        global<u32>(0x0087b1d8) = 0;
    }
}

u32 __fastcall Obj_CallSub14Slot18(u8 *object, void *, u32 a) { return vcall<u32>(object + 0x14, 0x18, a); }

u32 __fastcall Obj_CallSub28Slot18(u8 *object, void *, u32 a) { return vcall<u32>(object + 0x28, 0x18, a); }

}  // extern "C"
