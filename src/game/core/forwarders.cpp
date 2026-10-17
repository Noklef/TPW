#include "forwarders.h"

#include <cstdint>

#include "accessors.h"
#include "ptrlist.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Append to the list the owner keeps at offset. What the original append leaves in EAX is the new node
// when the list was empty, else the new count.
u32 Append(u8 *owner, u32 offset, u32 item) {
    PtrList *list = at<PtrList *>(owner, offset);
    const bool wasEmpty = !at<u32>(list, 8);
    PtrList_Append(list, nullptr, reinterpret_cast<void *>(item));
    return wasEmpty ? at<u32>(list, 8) : at<u32>(list, 0xc);
}

}  // namespace

extern "C" {

u32 __fastcall Owner18_Append(u8 *owner, void *, u32 item) { return Append(owner, 0x18, item); }

u32 __fastcall OwnerC_Append(u8 *owner, void *, u32 item) { return Append(owner, 0xc, item); }

u32 __fastcall Owner58_Append(u8 *owner, void *, u32 item) { return Append(owner, 0x58, item); }

u32 __fastcall Owner4_Append(u8 *owner, void *, u32 item) { return Append(owner, 4, item); }

u32 __fastcall Owner8_Append(u8 *owner, void *, u32 item) { return Append(owner, 8, item); }

u32 __fastcall Owner6c_Append(u8 *owner, void *, u32 item) { return Append(owner, 0x6c, item); }

u32 __fastcall Owner64_Copy4At34(u8 *owner, void *, const u32 *source) {
    return Obj_Copy4At34(at<u8 *>(owner, 0x64), nullptr, source);
}
}
