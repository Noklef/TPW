#include "small22.h"

#include <cstdint>

#include "gameheap.h"
#include "small21.h"
#include "vcall.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 **Share(u8 *shared, u8 **out) {
    *out = shared;
    Counted_AddRef(shared);
    return out;
}

}  // namespace

extern "C" {

void __fastcall Counted_AddRef(u8 *counted) {  // (the STL lock around it is left out)
    const u32 count = at<u32>(counted, 4);
    if (count != 0xffffffffu)
        at<u32>(counted, 4) = count + 1;
}

u8 **__fastcall Obj_ShareMember20(u8 *object, void *, u8 **out) { return Share(at<u8 *>(object, 0x20), out); }

u8 **__fastcall Obj_ShareMember34(u8 *object, void *, u8 **out) { return Share(at<u8 *>(object, 0x34), out); }

u32 __fastcall Id_Get(u32 *id) {
    if (*id == 0) {
        const u32 next = global<u32>(0x00fb1dd4) + 1;
        global<u32>(0x00fb1dd4) = next;
        *id = next;
    }
    return *id;
}

u8 *__fastcall Obj_Init4f1520(u8 *object, void *, u32 a) {
    at<u32>(object, 0) = a;
    at<u32>(object, 8) = global<u32>(0x007099f0);
    at<u32>(object, 0xc) = global<u32>(0x007099f4);
    at<u32>(object, 0x10) = global<u32>(0x00fb34b0);
    return object;
}

u8 *__fastcall Counted_CopyCtor(u8 *object, void *, const u8 *from) {
    u8 *shared = at<u8 *>(from, 0);
    at<u8 *>(object, 0) = shared;
    Counted_AddRef(shared);
    return object;
}

u8 *__stdcall Node14_New(u32 a, u32 b) {
    u8 *node = (u8 *)GameHeap_Alloc(0x14);
    at<u32>(node, 4) = a;
    at<u32>(node, 0x10) = b;
    return node;
}

u8 *__fastcall Sentinel_Ctor(u8 *object, void *, const u8 *from) {
    at<u8>(object, 0) = at<u8>(from, 0);
    u8 *node = (u8 *)GameHeap_Alloc(0xc);
    at<u8 *>(node, 0) = node;
    at<u8 *>(node, 4) = node;
    at<u8 *>(object, 4) = node;
    at<u32>(object, 8) = 0;
    return object;
}

u8 *__fastcall Obj_NameToWString(u8 *object, void *, u8 *out) {
    WString_CtorStr((WString *)out, nullptr, at<const u16 *>(object, 0x54));
    return out;
}

void __cdecl Global7ced44_Release() {
    u8 *shared = global<u8 *>(0x007ced44);
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count != 0xffffffffu)
        at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0)
        vcall<u32>(shared, 0, 1u);
    global<u32>(0x007ced44) = 0;
}

void __cdecl Globalf827dc_Release() {
    u8 *shared = global<u8 *>(0x00f827dc);
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count != 0xffffffffu)
        at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0)
        vcall<u32>(shared, 0, 1u);
    global<u32>(0x00f827dc) = 0;
}

}  // extern "C"
