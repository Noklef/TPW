#include "vdtor.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "crt.h"
#include "gameheap.h"
#include "initializers.h"
#include "misc.h"
#include "string.h"
#include "vcall.h"
#include "../model/holder.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// A 0x10-byte helper object {vtable, three zero dwords} (0 when the allocation fails).
u8 *Helper(u32 vtable) {
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (!block)
        return nullptr;
    at<u32>(block, 4) = 0;
    at<u32>(block, 8) = 0;
    at<u32>(block, 0xc) = 0;
    at<u32>(block, 0) = vtable;
    return block;
}

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

// The destructor the 0x7028xx classes share, for the vtable pointer they set: their shared arrays hold the WEACITYR records of the online lists,
// each class's own kind (its element size and its destructor's import slot).
void Class7028_Dtor(u8 *object, u32 vtable) {
    u32 size = 0x30, slot = 0x006fd500;  // ccCityInfo
    if (vtable == 0x00702890) size = 0x66, slot = 0x006fd504;  // ccParkInfo
    if (vtable == 0x007028d0) size = 0x10, slot = 0x006fd50c;  // ccThemeInfo
    if (vtable == 0x007028f8) size = 0x26, slot = 0x006fd508;  // ccServerInfo
    if (vtable == 0x00702920) size = 0x34, slot = 0x006fd510;  // the rides'
    u8 *shared = at<u8 *>(object, 0x30);
    at<u32>(object, 0) = vtable;
    if (shared) {
        const s32 uses = at<s32>(shared, 0xc) - 1;
        at<s32>(shared, 0xc) = uses;
        if (uses == 0) {
            u8 *array = at<u8 *>(shared, 4);
            if (array) {
                CRT_VectorDtorIterator(array, size, at<s32>(array, (u32)-4), global<void(__fastcall *)(void *)>(slot));
                GameHeap_Free(array - 4);
            }
            at<u32>(shared, 4) = 0;
            GameHeap_Free(shared);
        }
        at<u32>(object, 0x30) = 0;
    }
    for (u8 *list = at<u8 *>(object, 0x2c); at<u32>(list, 0xc) != 0; list = at<u8 *>(object, 0x2c)) {
        u8 *node = at<u8 *>(list, 4);
        u8 *popped = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;
            const u32 left = at<u32>(list, 0xc) - 1;
            popped = at<u8 *>(node, 0);
            at<u32>(list, 0xc) = left;
            GameHeap_Free(node);
        }
        if (popped)
            vcall<u32>(popped, 0, 1u);
    }
    u8 *list = at<u8 *>(object, 0x2c);
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0;
    if (list)
        vcall<u32>(list, 0, 1u);
}

u8 *Class7028_Ctor(u8 *object, u32 vtable, u32 memberVtable, u32 finalVtable) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0;
    at<u32>(object, 0x30) = 0;
    at<u32>(object, 0) = vtable;
    at<u32>(object, 0x18) = 0;
    u8 *member = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (member) {
        at<u32>(member, 4) = 0;
        at<u32>(member, 8) = 0;
        at<u32>(member, 0xc) = 0;
        at<u32>(member, 0) = memberVtable;
        at<u8 *>(object, 0x2c) = member;
        at<u32>(object, 0) = finalVtable;
        return object;
    }
    at<u32>(object, 0) = finalVtable;
    at<u32>(object, 0x2c) = 0;
    return object;
}

void TakeOffParentList(u8 *object) {
    u8 *list = at<u8 *>(at<u8 *>(object, 4), 0x2c);
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u8 *>(node, 0) != object)
        node = at<u8 *>(node, 4);
    if (!node)
        return;
    if (node == at<u8 *>(list, 4))
        at<u8 *>(list, 4) = at<u8 *>(node, 4);
    else
        at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
    if (node == at<u8 *>(list, 8))
        at<u8 *>(list, 8) = at<u8 *>(node, 8);
    else
        at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
    at<u32>(list, 0xc) -= 1;
    GameHeap_Free(node);
}

void Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00703780;
    if (u8 *member = at<u8 *>(object, 8))
        vcall<u32>(member, 0);
    at<u32>(object, 0) = 0x00703770;
}

u8 *NewList(u32 vtable) {
    u8 *list = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (!list)
        return nullptr;
    at<u32>(list, 4) = 0;
    at<u32>(list, 8) = 0;
    at<u32>(list, 0xc) = 0;
    at<u32>(list, 0) = vtable;
    return list;
}

void DrainList(u8 *object, u32 field) {
    u8 *list = at<u8 *>(object, field);
    while (at<u32>(list, 0xc) != 0) {
        u8 *node = at<u8 *>(list, 4);
        u8 *held = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;
            const u32 left = at<u32>(list, 0xc) - 1;
            held = at<u8 *>(node, 0);
            at<u32>(list, 0xc) = left;
            GameHeap_Free(node);
        }
        if (held)
            vcall<u32>(held, 0, 1u);
        list = at<u8 *>(object, field);
    }
    if (at<u8 *>(object, field))
        vcall<u32>(at<u8 *>(object, field), 0, 1u);
    at<u32>(object, field) = 0;
}

u8 *Class7028_Ctor(u8 *object, u32 vtable, u32 memberVtable) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0;
    at<u32>(object, 0x30) = 0;
    at<u32>(object, 0) = vtable;
    at<u32>(object, 0x18) = 0;
    u8 *member = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (member) {
        at<u32>(member, 4) = 0;
        at<u32>(member, 8) = 0;
        at<u32>(member, 0xc) = 0;
        at<u32>(member, 0) = memberVtable;
        at<u8 *>(object, 0x2c) = member;
        return object;
    }
    at<u32>(object, 0x2c) = 0;
    return object;
}

}  // namespace

extern "C" {

u8 *__fastcall Class702d48_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00702d48;
    at<u32>(object, 4) = 0;
    vcall<u32>(global<u8 *>(0x00f85614), 0x18, object);
    return object;
}

u32 __fastcall Class702d48_Release(u8 *object) {
    at<u32>(object, 0) = 0x00702d48;
    return vcall<u32>(global<u8 *>(0x00f85614), 0x1c, object);
}

u8 *__fastcall Class702d48_DeletingDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x00702d48;
    vcall<u32>(global<u8 *>(0x00f85614), 0x1c, object);
    return Finish(object, flags);
}

u8 *__fastcall Class7029d0_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x007029d0;
    vcall<u32>(global<u8 *>(0x00f82884), 0, object);
    return object;
}

u32 __fastcall Class7029d0_Release(u8 *object) {
    at<u32>(object, 0) = 0x007029d0;
    return vcall<u32>(global<u8 *>(0x00f82884), 4, object);
}

u8 *__fastcall Class7029d0_DeletingDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x007029d0;
    vcall<u32>(global<u8 *>(0x00f82884), 4, object);
    return Finish(object, flags);
}

u8 *__fastcall Class702ae8_DeletingDtor(u8 *object, void *, u32 flags) {
    u8 *member = at<u8 *>(object, 4);
    at<u32>(object, 0) = 0x00702ae8;
    vcall<u32>(member, 8, object);
    return Finish(object, flags);
}

u8 *__fastcall Class702bb4_DeletingDtor(u8 *object, void *, u32 flags) {
    u8 *member = at<u8 *>(object, 8);
    at<u32>(object, 0) = 0x00702bb4;
    vcall<u32>(member, 0x3c, object);
    return Finish(object, flags);
}

u8 *__fastcall Class702bb8_DeletingDtor(u8 *object, void *, u32 flags) {
    u8 *member = at<u8 *>(object, 8);
    at<u32>(object, 0) = 0x00702bb8;
    vcall<u32>(member, 0x38, object);
    return Finish(object, flags);
}

u32 __fastcall Obj_CallSlot20ThenSub1c(u8 *object) {
    vcall<u32>(object, 0x20);
    return vcall<u32>(at<u8 *>(global<u8 *>(0x00f85614), 4), 0x1c);
}

u32 __fastcall Obj_CallSlot20ThenSub1cAnd48(u8 *object) {
    vcall<u32>(object, 0x20);
    vcall<u32>(at<u8 *>(global<u8 *>(0x00f85614), 4), 0x1c);
    return vcall<u32>(at<u8 *>(global<u8 *>(0x00f85614), 4), 0x48);
}

u32 __fastcall VBase_CallSlot8(u8 *object, void *, u32 a, u32) {
    u8 *base = object - 0x18;
    u8 *target = base + at<u32>(at<u8 *>(base, 0), 4);
    return vcall<u32>(target, 8, a);
}

void __fastcall Class702880_Dtor(u8 *object) { Class7028_Dtor(object, 0x00702880); }
void __fastcall Class702890_Dtor(u8 *object) { Class7028_Dtor(object, 0x00702890); }
void __fastcall Class7028d0_Dtor(u8 *object) { Class7028_Dtor(object, 0x007028d0); }
void __fastcall Class7028f8_Dtor(u8 *object) { Class7028_Dtor(object, 0x007028f8); }
void __fastcall Class702920_Dtor(u8 *object) { Class7028_Dtor(object, 0x00702920); }

void __fastcall Class702828_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702828;
    WString_Dtor((WString *)(object + 0x34));
    Class7028_Dtor(object, 0x00702890);
}

u8 *__fastcall Class702880_DeletingDtor(u8 *object, void *, u32 flags) {
    Class7028_Dtor(object, 0x00702880);
    return Finish(object, flags);
}

u8 *__fastcall Class702890_DeletingDtor(u8 *object, void *, u32 flags) {
    Class7028_Dtor(object, 0x00702890);
    return Finish(object, flags);
}

u8 *__fastcall Class7028d0_DeletingDtor(u8 *object, void *, u32 flags) {
    Class7028_Dtor(object, 0x007028d0);
    return Finish(object, flags);
}

u8 *__fastcall Class7028f8_DeletingDtor(u8 *object, void *, u32 flags) {
    Class7028_Dtor(object, 0x007028f8);
    return Finish(object, flags);
}

u8 *__fastcall Class702920_DeletingDtor(u8 *object, void *, u32 flags) {
    Class7028_Dtor(object, 0x00702920);
    return Finish(object, flags);
}

u8 *__fastcall Class702bc8_Ctor(u8 *object) {
    Class702d48_Ctor(object);
    at<u32>(object, 0) = 0x00702bc8;
    at<u8 *>(object, 8) = Helper(0x00702bc0);
    at<u8 *>(object, 0x6c) = Helper(0x00702bbc);
    at<u32>(object, 0x78) = 0;
    at<u32>(object, 0x74) = 0;
    at<u32>(object, 0x1c) = 0x3ecccccd;
    at<u32>(object, 0x20) = 0x3e19999a;
    at<u32>(object, 0x24) = 0x3f000000;
    at<u32>(object, 0x38) = 0x43960000;
    at<u32>(object, 0x28) = 0x3e4ccccd;
    at<u32>(object, 0x2c) = 0x3d99999a;
    at<u32>(object, 0x30) = 0x3e800000;
    at<u32>(object, 0x3c) = 0x43160000;
    at<u32>(object, 0x40) = 0x3fc00000;
    return object;
}

u8 *__fastcall Class7028b8_Ctor(u8 *object) { return Class7028_Ctor(object, 0x007028d0, 0x007028c8, 0x007028b8); }

u8 *__fastcall Class7028e0_Ctor(u8 *object) { return Class7028_Ctor(object, 0x007028f8, 0x007028f0, 0x007028e0); }

u8 *__fastcall Class702908_Ctor(u8 *object) { return Class7028_Ctor(object, 0x00702920, 0x00702918, 0x00702908); }

void __fastcall Class702938_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702938;
    TakeOffParentList(object);
    WString_Dtor((WString *)(object + 0x44));
    WString_Dtor((WString *)(object + 0x20));
    WString_Dtor((WString *)(object + 0xc));
}

void __fastcall Class702940_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702940;
    TakeOffParentList(object);
    String_Dtor((String *)(object + 0x34));
    String_Dtor((String *)(object + 0x20));
    String_Dtor((String *)(object + 0xc));
}

void __fastcall Class702948_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702948;
    TakeOffParentList(object);
    WString_Dtor((WString *)(object + 0x4c));
    WString_Dtor((WString *)(object + 0x20));
    WString_Dtor((WString *)(object + 0xc));
}

void __fastcall Class70293c_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x0070293c;
    if (at<u8 *>(object, 4))
        TakeOffParentList(object);
    WString_Dtor((WString *)(object + 0x94));
    String_Dtor((String *)(object + 0x7c));
    WString_Dtor((WString *)(object + 0x34));
    WString_Dtor((WString *)(object + 0x20));
    WString_Dtor((WString *)(object + 0xc));
}

void __fastcall Class702944_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702944;
    TakeOffParentList(object);
    WString_Dtor((WString *)(object + 0x6c));
    WString_Dtor((WString *)(object + 0x54));
    WString_Dtor((WString *)(object + 0x40));
    WString_Dtor((WString *)(object + 0x24));
    WString_Dtor((WString *)(object + 0x10));
}

void __fastcall Class703780_DtorA(u8 *object) { Dtor(object); }

void __fastcall Class703780_DtorB(u8 *object) { Dtor(object); }

void __fastcall Class703780_DtorC(u8 *object) { Dtor(object); }

u8 *__fastcall Class702b08_Ctor(u8 *object) {
    Class7029d0_Ctor(object);
    at<u32>(object, 0) = 0x00702b08;
    at<u8 *>(object, 4) = NewList(0x00702b00);
    return object;
}

u8 *__fastcall Class702d78_Ctor(u8 *object) {
    Class7029d0_Ctor(object);
    at<u32>(object, 0) = 0x00702d78;
    at<u8 *>(object, 0x18) = NewList(0x00702d70);
    return object;
}

void __fastcall Class702b08_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702b08;
    DrainList(object, 4);
    Class7029d0_Release(object);
}

void __fastcall Class702cb0_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702cb0;
    DrainList(object, 8);
    Class702d48_Release(object);
}

void __fastcall Class702d78_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702d78;
    DrainList(object, 0x18);
    Class7029d0_Release(object);
}

void __fastcall Class702ec0_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x00702ec0;
    Class702cb0_Dtor(object);
}

void __fastcall Class702dd0_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x00702dd0;
    Class702cb0_Dtor(object);
}

void __fastcall Class702e70_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x00702e70;
    Class702bc8_Dtor(object);
}

void __fastcall Class702e20_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x00702e20;
    Class702bc8_Dtor(object);
}

void __fastcall Class7037b8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x007037b8;
    u8 *const head = at<u8 *>(object, 8);
    for (u8 *node = at<u8 *>(head, 0); node != at<u8 *>(object, 8); node = at<u8 *>(node, 0))
        GameHeap_Free(at<void *>(node, 8));
    u8 *const list = at<u8 *>(object, 8);
    u8 *node = at<u8 *>(list, 0);
    while (node != list) {
        u8 *const next = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
        GameHeap_Free(node);
        at<u32>(object, 0xc) = at<u32>(object, 0xc) - 1;
        node = next;
    }
    GameHeap_Free(at<void *>(object, 8));
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

u8 *__fastcall Class702958_Ctor(u8 *object) {
    Class70295c_Ctor(object);
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x420) = 0;
    at<u32>(object, 0) = 0x00702958;
    for (u32 i = 0; i < 0x100; ++i)
        at<u32>(object, 0x20 + 4 * i) = 0;
    return object;
}

u8 *__fastcall Class702880_Ctor(u8 *object) { return Class7028_Ctor(object, 0x00702880, 0x007028a8); }

u8 *__fastcall Class702890_Ctor(u8 *object) { return Class7028_Ctor(object, 0x00702890, 0x007028ac); }

void __fastcall ListDtor6fd708_407c30(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fd708; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor6fd9e4_41c9a0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fd9e4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor6fdb08_41eb50(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fdb08; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor6fdb10_421640(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fdb10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor700aa0_5193d0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00700aa0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7027fc_5ac550(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007027fc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7027fc_5ac590(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007027fc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7028a0_5b58c0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028a0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7028b0_5b9a80(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028b0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7028b4_5b9ac0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028b4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor70294c_5beae0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x0070294c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702950_5beb20(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702950; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702954_5beb60(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702954; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7029f8_5d5800(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007029f8; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702a88_5d8610(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702a88; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702a8c_5d8650(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702a8c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702afc_5d9f10(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702afc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702b28_5da2b0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702b28; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702c0c_5dc6c0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702c0c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702c10_5dc700(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702c10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702d00_5e0370(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702d00; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702d98_5e3cd0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702d98; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702f10_5e40a0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702f10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor703084_5f3900(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00703084; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}
}  // extern "C"

namespace {

// The member list at offset drained (see vdtor.h); a list whose count is set but whose head is none loops for ever, as the original does.
void DrainMemberList(u8 *object, u32 offset) {
    for (u8 *list = at<u8 *>(object, offset); at<u32>(list, 0xc) != 0; list = at<u8 *>(object, offset)) {
        u8 *node = at<u8 *>(list, 4);
        u8 *popped = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;  // (the tail)
            const u32 left = at<u32>(list, 0xc) - 1;
            popped = at<u8 *>(node, 0);
            at<u32>(list, 0xc) = left;
            GameHeap_Free(node);
        }
        if (popped)
            vcall<u32>(popped, 0, 1u);
    }
    u8 *list = at<u8 *>(object, offset);
    if (list)
        vcall<u32>(list, 0, 1u);
    at<u32>(object, offset) = 0;
}

}  // namespace

extern "C" {

void __fastcall Class702af0_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702af0;
    vcall<u32>(global<u8 *>(0x00f83784), 0x1c, object);
    DrainMemberList(object, 0x58);
    String_Dtor(reinterpret_cast<String *>(object + 0x24));
    String_Dtor(reinterpret_cast<String *>(object + 0x10));
}

u8 *__fastcall Class702af0_DeletingDtor(u8 *object, void *, u32 flags) {
    Class702af0_Dtor(object);
    return Finish(object, flags);
}

void __fastcall Class702bc8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702bc8;
    DrainMemberList(object, 8);
    DrainMemberList(object, 0x6c);
    Class702d48_Release(object);
}

u8 *__fastcall Class702bc8_DeletingDtor(u8 *object, void *, u32 flags) {
    Class702bc8_Dtor(object);
    return Finish(object, flags);
}

void __fastcall Class702ca8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702ca8;
    vcall<u32>(at<u8 *>(object, 0xc), 0x34, object);
    WString_Dtor(reinterpret_cast<WString *>(object + 0x48));
    String_Dtor(reinterpret_cast<String *>(object + 0x28));
}

u8 *__fastcall Class702ca8_DeletingDtor(u8 *object, void *, u32 flags) {
    Class702ca8_Dtor(object);
    return Finish(object, flags);
}

void __fastcall Class702a60_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702a60;
    DrainMemberList(object, 0xc);
    for (u8 *list = at<u8 *>(object, 8); at<u32>(list, 0xc) != 0; list = at<u8 *>(object, 8)) {
        u8 *node = at<u8 *>(list, 4);
        u8 *item = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;  // (the tail)
            const u32 left = at<u32>(list, 0xc) - 1;
            item = at<u8 *>(node, 0);
            at<u32>(list, 0xc) = left;
            GameHeap_Free(node);
        }
        if (item) {
            vcall<u32>(global<u8 *>(0x00f83774), 0x24, item);
            String_Dtor(reinterpret_cast<String *>(item + 0x18));
            String_Dtor(reinterpret_cast<String *>(item + 4));
            GameHeap_Free(item);
        }
    }
    if (u8 *list = at<u8 *>(object, 8))
        vcall<u32>(list, 0, 1u);
    at<u32>(object, 8) = 0;
    InstanceTypes_FreeAll();
    Terrain_FreeLoaded();
    Class7029d0_Release(object);
}

u8 *__fastcall Class702a60_DeletingDtor(u8 *object, void *, u32 flags) {
    Class702a60_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class70295c_Ctor(u8 *object) {
    Init_5d36c0(object);
    at<u32>(object, 0) = 0x0070295c;
    for (u32 offset : {0x8u, 0xcu, 0x10u, 0x14u})
        at<u32>(object, offset) = 0;
    return object;
}

}  // extern "C"
