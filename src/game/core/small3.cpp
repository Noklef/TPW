#include "small3.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Global81ae2c_Equals(u32 value) { return global<u32>(0x0081ae2c) == value; }

u32 __cdecl Global81b0cc_IfNoFlag() { return global<u32>(0x008187f8) ? 0 : global<u32>(0x0081b0cc); }

u32 __cdecl Global81ecac_IfNoFlag() { return global<u32>(0x008187f8) ? 0 : global<u32>(0x0081ecac); }

u32 __cdecl Global81af4c_Add(u32 amount) {
    global<u32>(0x0081af4c) += amount;
    return amount;
}

u32 __cdecl Obj_Read30_34_38(const u8 *source, u32 *a, u32 *b, u32 *c) {
    *a = at<u32>(source, 0x30);
    *b = at<u32>(source, 0x34);
    return *c = at<u32>(source, 0x38);
}

u32 __cdecl Global879a24_Get4c4() { return at<u32>(reinterpret_cast<const u8 *>(global<u32>(0x00879a24)), 0x4c4); }

u32 __fastcall VirtualBase_SetVtbl7099dc(u8 *object) {
    const u32 vtable = at<u32>(object, (u32)-8);
    at<u32>(object, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 8) = 0x007099dc;
    return vtable;
}

u32 __fastcall VirtualBase_SetVtbl70995c(u8 *object) {
    const u32 vtable = at<u32>(object, (u32)-4);
    at<u32>(object, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 4) = 0x0070995c;
    return vtable;
}

u32 *__fastcall ListCursor_Next(u32 *cursor, void *, u32 *out, u32) {
    const u32 node = *cursor;
    const u32 next = *reinterpret_cast<const u32 *>(node);
    *cursor = next;
    *out = node;
    return out;
}

}  // extern "C"
