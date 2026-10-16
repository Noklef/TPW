#include "vstream.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The original loads the wide character into CX, so the character it pushes has the high half of the object's address (ECX held `this`) in its upper bits (sic).
u32 Widened(const u8 *object, u16 character) { return ((u32)(uintptr_t)object & 0xffff0000u) | character; }

}  // namespace

extern "C" {

u32 __fastcall WBuf_Peek(u8 *object) {
    u8 *position = *at<u8 **>(object, 0x1c);
    if (position) {
        u8 *end = position + 2 * *at<u32 *>(object, 0x2c);
        if (position < end)
            return ((u32)(uintptr_t)position & 0xffff0000u) | at<u16>(position, 0);
    }
    return vcall<u32>(object, 0x10);
}

u32 __fastcall Buf_Peek(u8 *object) {
    u8 *position = *at<u8 **>(object, 0x1c);
    if (position) {
        u8 *end = position + *at<u32 *>(object, 0x2c);
        if (position < end)
            return at<u8>(position, 0);
    }
    return vcall<u32>(object, 0x10);
}

u32 __fastcall WBuf_Next(u8 *object) {
    const u32 result = vcall<u32>(object, 0x10);
    if ((u16)result == 0xffff)
        return 0xffff;
    u32 *count = at<u32 *>(object, 0x2c);
    *count = *count - 1;
    u8 **cursor = at<u8 **>(object, 0x1c);
    u8 *position = *cursor;
    *cursor = position + 2;
    return ((u32)(uintptr_t)position & 0xffff0000u) | at<u16>(position, 0);
}

u32 __fastcall Buf_Next(u8 *object) {
    const u32 result = vcall<u32>(object, 0x10);
    if (result == 0xffffffffu)
        return 0xffffffffu;
    u32 *count = at<u32 *>(object, 0x2c);
    *count = *count - 1;
    u8 **cursor = at<u8 **>(object, 0x1c);
    u8 *position = *cursor;
    *cursor = position + 1;
    return at<u8>(position, 0);
}

u32 __fastcall CType_Is(u8 *object, void *, u32 mask, u32 character) {
    const s8 index = (s8)vcall<u32>(object, 0x30, character, 0u);
    const u16 bits = at<u16>(at<u8 *>(object, 0x10), 2 * (s32)index);
    return (bits & (u16)mask) != 0 ? 1 : 0;
}

u32 __fastcall Obj_RefreshMember(u8 *object) {
    u8 *member = at<u8 *>(object, 0x20);
    void *const *memberVtable = *reinterpret_cast<void *const *const *>(member);
    const u32 result = vcall<u32>(object, 0x34);
    using Slot = u32(__fastcall *)(u8 *, void *, u32);
    reinterpret_cast<Slot>(memberVtable[0x24 / 4])(at<u8 *>(object, 0x20), nullptr, result);
    vcall<u32>(object, 0x18);
    return vcall<u32>(object, 0x14);
}

u16 *__fastcall CType_ScanIs(u8 *object, void *, u32 mask, u16 *first, u16 *last) {
    u16 *p = first;
    while (p != last) {
        if ((u8)vcall<u32>(object, 8, mask, Widened(object, *p)))
            break;
        ++p;
    }
    return p;
}

u16 *__fastcall CType_ScanNot(u8 *object, void *, u32 mask, u16 *first, u16 *last) {
    u16 *p = first;
    while (p != last) {
        if (!(u8)vcall<u32>(object, 8, mask, Widened(object, *p)))
            break;
        ++p;
    }
    return p;
}

u16 *__fastcall CType_MapWords(u8 *object, void *, u16 *first, u16 *last, u16 *dest) {
    u16 *p = first;
    while (p != last) {
        const s8 index = (s8)vcall<u32>(object, 0x30, Widened(object, *p), 0u);
        ++p;
        *dest++ = at<u16>(at<u8 *>(object, 0x10), 2 * (s32)index);
    }
    return p;
}

u32 __fastcall WBuf_PeekConverted(u8 *object) {
    u8 *position = *at<u8 **>(object, 0x1c);
    if (position) {
        u8 *end = position + 2 * *at<u32 *>(object, 0x2c);
        if (position < end)
            return ((u32)(uintptr_t)position & 0xffff0000u) | at<u16>(position, 0);
    }
    void *const *vtable = *reinterpret_cast<void *const *const *>(object);
    using Slot0 = u32(__fastcall *)(u8 *, void *);
    using Slot1 = u32(__fastcall *)(u8 *, void *, u32);
    const u32 result = reinterpret_cast<Slot0>(vtable[0x14 / 4])(object, nullptr);
    return reinterpret_cast<Slot1>(vtable[8 / 4])(object, nullptr, result);
}

u32 __fastcall Buf_PeekConverted(u8 *object) {
    u8 *position = *at<u8 **>(object, 0x1c);
    if (position) {
        u8 *end = position + *at<u32 *>(object, 0x2c);
        if (position < end)
            return at<u8>(position, 0);
    }
    void *const *vtable = *reinterpret_cast<void *const *const *>(object);
    using Slot0 = u32(__fastcall *)(u8 *, void *);
    using Slot1 = u32(__fastcall *)(u8 *, void *, u32);
    const u32 result = reinterpret_cast<Slot0>(vtable[0x14 / 4])(object, nullptr);
    return reinterpret_cast<Slot1>(vtable[8 / 4])(object, nullptr, result);
}

}  // extern "C"
