#include "items.h"

#include <cstdint>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../core/ptrlist.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

WString *Wide(u8 *item, u32 offset) { return reinterpret_cast<WString *>(item + offset); }
String *Narrow(u8 *item, u32 offset) { return reinterpret_cast<String *>(item + offset); }

// The vtable's +0x14 (AssignStr) with the text a record holds a pointer to.
template <typename S> void AssignText(S *s, const u8 *record, u32 offset) { vcall<void>(s, 0x14, at<const void *>(record, offset)); }

// The item at the tail of its owner's list (the owner's +0x2c).
void List(u8 *item) { PtrList_Append(at<PtrList *>(at<u8 *>(item, 4), 0x2c), nullptr, item); }

// fld float; fmul double [0x702930] (1.01); fstp float.
float Scaled(const u8 *record, u32 offset) {
    return static_cast<float>(X87_MulFloatDouble(at<float>(record, offset), *reinterpret_cast<const double *>((uintptr_t)0x00702930)));
}

template <u8 *(__fastcall *Ctor)(u8 *, void *, const u8 *, u8 *)> void AddAll(u8 *owner, const u32 *array, u32 size, u32 stride) {
    const u8 *record = reinterpret_cast<const u8 *>(array[1]);
    for (s32 i = 0; i < static_cast<s32>(array[0]); ++i, record += stride)
        if (u8 *const item = static_cast<u8 *>(GameHeap_Alloc(size)))
            Ctor(item, nullptr, record, owner);
}

}  // namespace

extern "C" {

u8 *__fastcall Class702938_Ctor(u8 *item, void *, const u8 *record, u8 *owner) {
    WString_Ctor(Wide(item, 0xc));
    WString_Ctor(Wide(item, 0x20));
    WString_Ctor(Wide(item, 0x44));
    at<u32>(item, 0) = 0x00702938;
    at<u8 *>(item, 4) = owner;
    at<u32>(item, 8) = at<u32>(record, 0);
    AssignText(Wide(item, 0xc), record, 4);
    AssignText(Wide(item, 0x20), record, 8);
    at<float>(item, 0x34) = Scaled(record, 0xc);
    at<float>(item, 0x38) = Scaled(record, 0x14);
    at<float>(item, 0x3c) = Scaled(record, 0x10);
    at<u32>(item, 0x40) = at<u32>(record, 0x18);
    AssignText(Wide(item, 0x44), record, 0x1c);
    for (u32 k = 0; k < 0x10; k += 4)
        at<u32>(item, 0x58 + k) = at<u32>(record, 0x20 + k);
    List(item);
    return item;
}

u8 *__fastcall Class70293c_Ctor(u8 *item) {
    const u16 *const emptyWide = reinterpret_cast<const u16 *>((uintptr_t)0x0078a374);
    WString_Ctor(Wide(item, 0xc));
    WString_Ctor(Wide(item, 0x20));
    WString_Ctor(Wide(item, 0x34));
    String_Ctor(Narrow(item, 0x7c));
    WString_Ctor(Wide(item, 0x94));
    at<u32>(item, 0) = 0x0070293c;
    at<u32>(item, 4) = 0;
    at<u32>(item, 8) = 0xffffffff;
    vcall<void>(Wide(item, 0xc), 0x14, emptyWide);
    vcall<void>(Wide(item, 0x20), 0x14, emptyWide);
    vcall<void>(Wide(item, 0x34), 0x14, emptyWide);
    for (u32 offset = 0x48; offset <= 0x54; offset += 4)
        at<u32>(item, offset) = 0xffffffff;
    for (u32 k = 0; k < 0x20; ++k)
        at<u8>(item, 0x58 + k) = 0xff;
    at<u32>(item, 0x78) = 0xffffffff;
    vcall<void>(Narrow(item, 0x7c), 0x14, reinterpret_cast<const char *>((uintptr_t)0x00786de4));
    at<u32>(item, 0x90) = 0xffffffff;
    vcall<void>(Wide(item, 0x94), 0x14, emptyWide);
    for (u32 offset = 0xa8; offset <= 0xac; offset += 2)
        at<u16>(item, offset) = 0xffff;
    for (u32 offset = 0xb0; offset <= 0xbc; offset += 4)
        at<u32>(item, offset) = 0xffffffff;
    return item;
}

u8 *__fastcall Class70293c_CtorFrom(u8 *item, void *, const u8 *record, u8 *owner) {
    WString_Ctor(Wide(item, 0xc));
    WString_Ctor(Wide(item, 0x20));
    WString_Ctor(Wide(item, 0x34));
    String_Ctor(Narrow(item, 0x7c));
    WString_Ctor(Wide(item, 0x94));
    at<u32>(item, 0) = 0x0070293c;
    at<u8 *>(item, 4) = owner;
    at<u32>(item, 8) = at<u32>(record, 0);
    AssignText(Wide(item, 0xc), record, 4);
    AssignText(Wide(item, 0x20), record, 8);
    AssignText(Wide(item, 0x34), record, 0xc);
    for (u32 k = 0; k < 0x10; k += 4)
        at<u32>(item, 0x48 + k) = at<u32>(record, 0x10 + k);
    for (u32 k = 0; k < 0x20; ++k)
        at<u8>(item, 0x58 + k) = at<u8>(record, 0x20 + k);
    at<u32>(item, 0x78) = at<u32>(record, 0x40);
    AssignText(Narrow(item, 0x7c), record, 0x44);
    at<u32>(item, 0x90) = at<u32>(record, 0x48);
    AssignText(Wide(item, 0x94), record, 0x4c);
    for (u32 k = 0; k < 6; k += 2)
        at<u16>(item, 0xa8 + k) = at<u16>(record, 0x50 + k);
    for (u32 k = 0; k < 0x10; k += 4)
        at<u32>(item, 0xb0 + k) = at<u32>(record, 0x56 + k);
    List(item);
    return item;
}

u8 *__fastcall Class70293c_Assign(u8 *item, void *, const u8 *other) {
    at<u32>(item, 8) = at<u32>(other, 8);
    for (u32 offset : {0xcu, 0x20u, 0x34u})
        vcall<void>(Wide(item, offset), 0x18, other + offset);
    for (u32 offset = 0x48; offset <= 0x54; offset += 4)
        at<u32>(item, offset) = at<u32>(other, offset);
    for (u32 k = 0; k < 0x20; ++k)
        at<u8>(item, 0x58 + k) = at<u8>(other, 0x58 + k);
    at<u32>(item, 0x78) = at<u32>(other, 0x78);
    vcall<void>(Narrow(item, 0x7c), 0x18, other + 0x7c);
    at<u32>(item, 0x90) = at<u32>(other, 0x90);
    vcall<void>(Wide(item, 0x94), 0x18, other + 0x94);
    for (u32 offset = 0xa8; offset <= 0xac; offset += 2)
        at<u16>(item, offset) = at<u16>(other, offset);
    for (u32 offset = 0xb0; offset <= 0xbc; offset += 4)
        at<u32>(item, offset) = at<u32>(other, offset);
    return item;
}

u8 *__fastcall Class702940_Ctor(u8 *item, void *, const u8 *record, u8 *owner) {
    String_Ctor(Narrow(item, 0xc));
    String_Ctor(Narrow(item, 0x20));
    String_Ctor(Narrow(item, 0x34));
    at<u32>(item, 0) = 0x00702940;
    at<u8 *>(item, 4) = owner;
    at<u32>(item, 8) = at<u32>(record, 0);
    WString text;
    WString_Ctor(&text);
    for (u32 k = 0; k < 3; ++k) {
        AssignText(&text, record, 4 + 4 * k);
        String_AssignFromWide(Narrow(item, 0xc + 0x14 * k), &text);
    }
    List(item);
    WString_Dtor(&text);
    return item;
}

u8 *__fastcall Class702944_Ctor(u8 *item, void *, const u8 *record, u8 *owner) {
    for (u32 offset : {0x10u, 0x24u, 0x40u, 0x54u, 0x6cu})
        WString_Ctor(Wide(item, offset));
    at<u32>(item, 0) = 0x00702944;
    at<u8 *>(item, 4) = owner;
    at<u32>(item, 8) = at<u32>(record, 0);
    at<u32>(item, 0xc) = at<u32>(record, 4);
    AssignText(Wide(item, 0x10), record, 8);
    AssignText(Wide(item, 0x24), record, 0xc);
    at<u32>(item, 0x38) = at<u32>(record, 0x10);
    at<u16>(item, 0x3c) = at<u16>(record, 0x14);
    AssignText(Wide(item, 0x40), record, 0x16);
    AssignText(Wide(item, 0x54), record, 0x1a);
    at<u32>(item, 0x68) = at<u32>(record, 0x1e);
    AssignText(Wide(item, 0x6c), record, 0x22);
    List(item);
    return item;
}

u8 *__fastcall Class702948_Ctor(u8 *item, void *, const u8 *record, u8 *owner) {
    WString_Ctor(Wide(item, 0xc));
    WString_Ctor(Wide(item, 0x20));
    WString_Ctor(Wide(item, 0x4c));
    at<u32>(item, 0) = 0x00702948;
    at<u8 *>(item, 4) = owner;
    at<u32>(item, 8) = at<u32>(record, 0);
    AssignText(Wide(item, 0xc), record, 4);
    AssignText(Wide(item, 0x20), record, 8);
    for (u32 k = 0; k < 0x10; k += 4)
        at<u32>(item, 0x34 + k) = at<u32>(record, 0xc + k);
    at<u32>(item, 0x44) = at<u32>(record, 0x1c);
    at<u32>(item, 0x48) = at<u32>(record, 0x20);
    AssignText(Wide(item, 0x4c), record, 0x24);
    for (u32 k = 0; k < 0xc; k += 4)
        at<u32>(item, 0x60 + k) = at<u32>(record, 0x28 + k);
    List(item);
    return item;
}

void __fastcall Owner2c_Add702938s(u8 *owner, void *, const u32 *array) { AddAll<Class702938_Ctor>(owner, array, 0x68, 0x30); }
void __fastcall Owner2c_Add70293cs(u8 *owner, void *, const u32 *array) { AddAll<Class70293c_CtorFrom>(owner, array, 0xc0, 0x66); }
void __fastcall Owner2c_Add702940s(u8 *owner, void *, const u32 *array) { AddAll<Class702940_Ctor>(owner, array, 0x48, 0x10); }
void __fastcall Owner2c_Add702944s(u8 *owner, void *, const u32 *array) { AddAll<Class702944_Ctor>(owner, array, 0x80, 0x26); }
void __fastcall Owner2c_Add702948s(u8 *owner, void *, const u32 *array) { AddAll<Class702948_Ctor>(owner, array, 0x6c, 0x34); }
}  // extern "C"
