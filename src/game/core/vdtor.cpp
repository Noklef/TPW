#include "vdtor.h"

#include <cstdint>

#include "gameheap.h"
#include "small20.h"
#include "vcall.h"

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

// The destructor the 0x7028xx classes share, for the vtable pointer they set.
void Class7028_Dtor(u8 *object, u32 vtable) {
    u8 *shared = at<u8 *>(object, 0x30);
    at<u32>(object, 0) = vtable;
    if (shared) {
        const s32 uses = at<s32>(shared, 0xc) - 1;
        at<s32>(shared, 0xc) = uses;
        if (uses == 0) {
            u8 *array = at<u8 *>(shared, 4);
            if (array) {
                CRT_VectorDtorIterator(array, 0x30, at<s32>(array, (u32)-4), global<void(__fastcall *)(void *)>(0x006fd500));
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

}  // extern "C"
