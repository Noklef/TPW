#include "namedobjects.h"

#include <cstdint>

#include "gameheap.h"
#include "initializers.h"
#include "string.h"
#include "vcall.h"
#include "vobj2.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr u32 kNames[] = {0x00786b68, 0x00785e78, 0x00785f98, 0x007862c8, 0x00785f68, 0x007862f8, 0x00786d10,
                          0x007866e8, 0x007862e0, 0x00785ed8, 0x00785f80, 0x00786aa8, 0x00786040, 0x007867d8};

u8 **registry() { return *reinterpret_cast<u8 ***>((uintptr_t)0x00786b9c); }

}  // namespace

extern "C" {

u32 __cdecl NamedObjects_RemoveGlobals() {
    u32 result = 0;
    for (const u32 name : kNames) {
        u8 *object;
        result = NamedList_Find(registry(), nullptr, reinterpret_cast<const u8 *>((uintptr_t)name), &object);
        if (result == 1) result = ListRef_Remove(registry(), nullptr, object);
    }
    return result;
}

u8 *__fastcall NamedObjects_Ctor(u8 *self) {
    String_Ctor(reinterpret_cast<String *>(self));
    at<u32>(self, 0x24) = 0x006fd704;
    at<u32>(self, 0x28) = 0;
    at<u32>(self, 0x2c) = 0;
    at<u32>(self, 0x30) = 0;
    at<u32>(self, 0x3c) = 1;
    at<u32>(self, 0x18) = 0;
    at<u32>(self, 0x1c) = 0;
    at<u32>(self, 0x14) = 0xffffffffu;
    vcall<u32>(self, 0x18, reinterpret_cast<const WString *>(0x007861c0));
    at<u8>(self, 0x20) = 0;
    u8 *const ref = static_cast<u8 *>(GameHeap_Alloc(0xc));
    at<u8 *>(self, 0x34) = ref ? Init_421510(ref) : nullptr;
    at<u32>(self, 0x38) = 0;
    return self;
}

void __fastcall NamedObjects_Dtor(u8 *self) {
    while (at<u32>(self, 0x30) > 0) {
        u8 *const node = at<u8 *>(self, 0x28);
        u8 *object = nullptr;
        if (node) {
            u8 *const next = at<u8 *>(node, 4);
            at<u8 *>(self, 0x28) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(self, 0x2c) = 0;
            object = at<u8 *>(node, 0);
            at<u32>(self, 0x30) -= 1;
            GameHeap_Free(node);
        }
        if (object) vcall<u32>(object, 0xc, 1u);
    }
    if (u8 *const ref = at<u8 *>(self, 0x34)) {
        ListRef_DrainAndDelete(reinterpret_cast<u8 **>(ref));
        GameHeap_Free(ref);
    }
    if (u8 *const block = at<u8 *>(self, 0x38)) GameHeap_Free(block);  // (its destructor, 0x5da3c0, is empty)
    u8 *node = at<u8 *>(self, 0x28);
    at<u32>(self, 0x24) = 0x006fd708;
    at<u32>(self, 0x2c) = 0;
    at<u32>(self, 0x28) = 0;
    while (node) {
        u8 *const freed = node;
        node = at<u8 *>(node, 4);
        GameHeap_Free(freed);
    }
    at<u32>(self, 0x30) = 0;
    String_Dtor(reinterpret_cast<String *>(self));
}

void __cdecl NamedObjects_StaticDtor() { NamedObjects_Dtor(reinterpret_cast<u8 *>(0x00786b68)); }
}
