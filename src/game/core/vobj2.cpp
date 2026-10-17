#include "vobj2.h"

#include <cstdint>

#include "crt.h"
#include "gameheap.h"
#include "vcall.h"
#include "small20.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void Delete(u8 *object) { vcall<u32>(object, 0, 1u); }

void CountedDtor(u8 *object, u32 vtable) {
    at<u32>(object, 0) = vtable;
    u8 *shared = at<u8 *>(object, 0x34);
    if (!shared)
        return;
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count < 0xffffffffu)
        at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0)  // (the STL lock around it is left out)
        Delete(shared);
}

}  // namespace

extern "C" {

void __fastcall Obj_ReleaseTwoComMembers(u8 *object) {
    u8 *a = at<u8 *>(object, 0xac);
    at<u32>(object, 0xa8) = 0;
    at<u32>(object, 0xa4) = 0;
    if (a)
        comcall<u32>(a, 8);
    u8 *b = at<u8 *>(object, 0xb0);
    if (b)
        comcall<u32>(b, 8);
    at<u32>(object, 0xac) = 0;
    at<u32>(object, 0xb0) = 0;
}

void __fastcall Obj_DeleteFiveMembers(u8 *object) {
    for (u8 *node = at<u8 *>(at<u8 *>(object, 0x18), 4); node;) {
        u8 *next = at<u8 *>(node, 4);
        vcall<u32>(at<u8 *>(node, 0), 0x10);
        node = next;
    }
    u8 *member = at<u8 *>(object, 0x14);
    if (member)
        Delete(member);
    member = at<u8 *>(object, 0x10);
    at<u32>(object, 0x14) = 0;
    if (member)
        Delete(member);
    member = at<u8 *>(object, 0xc);
    at<u32>(object, 0x10) = 0;
    if (member)
        Delete(member);
    member = at<u8 *>(object, 8);
    at<u32>(object, 0xc) = 0;
    if (member)
        Delete(member);
    member = at<u8 *>(object, 4);
    at<u32>(object, 8) = 0;
    if (member)
        Delete(member);
    at<u32>(object, 4) = 0;
}

void __fastcall ListRef_DrainAndDelete(u8 **ref) {
    u8 *list = *ref;
    if (!list)
        return;
    while (at<u32>(list, 0xc) > 0) {
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
            Delete(held);
        list = *ref;
    }
    if (*ref)
        Delete(*ref);
    *ref = nullptr;
}

u32 __fastcall ListRef_AnyHandles(u8 **ref, void *, u32 a, u32 b) {
    u8 *list = *ref;
    if (!list)
        return 0;
    for (u8 *node = at<u8 *>(list, 4); node;) {
        u8 *next = at<u8 *>(node, 4);
        if (vcall<u32>(at<u8 *>(node, 0), 8, a, b, 0x80000001u) == 1)
            return 1;
        node = next;
    }
    return 0;
}

void __fastcall Obj_SetNameAndWords(u8 *object, void *, u32 flag, u8 *text, const u16 *table) {
    at<u32>(object, 0x108) = flag;
    if (!flag)
        return;
    vcall<u32>(object + 0x10c, 0x14, text ? text : (u8 *)(uintptr_t)0x00786de4);
    for (u32 i = 0; i < 0x100; ++i)
        at<u16>(object, 0x120 + 2 * i) = table[i];
}

void __fastcall Class709914_Dtor(u8 *object) { CountedDtor(object, 0x00709914); }
void __fastcall Class7099a4_Dtor(u8 *object) { CountedDtor(object, 0x007099a4); }

u32 __fastcall Pointers_MarkAndCallSlot4(u8 *object) {
    for (s32 i = 0; i < at<s32>(object, 0x400); ++i) {
        u8 *thing = at<u8 *>(object, 4 * i);
        at<u32>(thing, 4) |= 0x10;
        thing = at<u8 *>(object, 4 * i);
        u8 *base = thing + at<u32>(at<u8 *>(thing, 0), 4);
        vcall<u32>(base, 4);
    }
    return 1;
}

u8 *__fastcall Class702198_DeletingDtor(u8 *field, void *, u32 flags) {
    u8 *object = field - 0x3c;
    u32 offset = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object + offset, 0) = 0x007021f0;
    offset = at<u32>(at<u8 *>(object, 0), 4);  // (read again: the vtable pointer was just replaced when the virtual base is the object itself)
    at<u32>(object, offset - 4) = offset - 0x3c;
    offset = at<u32>(at<u8 *>(object, 0), 4);
    vcall<u32>(object + offset, 4);
    at<u32>(object, 0x3c) = 0x00702198;
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

void __fastcall Obj_PickByName(u8 *object, void *, u32 flag) {
    at<u32>(object, 0xc) = at<u32>(at<u8 *>(at<u8 *>(object, 8), 4), 0);
    if (flag) {
        vcall<u32>((u8 *)(uintptr_t)0x00f85480, 0x14, (u8 *)(uintptr_t)0x00786de4);
        return;
    }
    for (u8 *node = at<u8 *>(at<u8 *>(object, 8), 4); node;) {
        u8 *next = at<u8 *>(node, 4);
        u8 *entry = at<u8 *>(node, 0);
        if (Crt().stricmp(at<const char *>(entry, 0x30), global<const char *>(0x00f85488)) == 0)
            at<u8 *>(object, 0xc) = entry;
        node = next;
    }
}

void __fastcall Class702da0_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702da0;
    u8 *node = at<u8 *>(object, 0xc);
    at<u32>(object, 8) = 0x00702f10;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0xc) = 0;
    while (node) {
        u8 *next = at<u8 *>(node, 4);
        GameHeap_Free(node);
        node = next;
    }
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0) = 0x00702d48;
    vcall<u32>(global<u8 *>(0x00f85614), 0x1c, object);
}

u32 __fastcall Object_ReleaseSavedPairs(u8 *object) {
    if ((at<u32>(object, 4) & 0x20000000) != 0 && at<s32>(object, 0x12c) > 0) {
        u8 *pairs = at<u8 *>(object, 0x124);
        for (s32 i = at<s32>(object, 0x12c) - 1; i >= 0; i--)
            *at<u32 *>(pairs, 4 + i * 8) = at<u32>(pairs, i * 8);
        if (at<u8 *>(object, 0x124)) {
            GameHeap_Free(at<u8 *>(object, 0x124));
            at<u32>(object, 0x124) = 0;
        }
        if (u8 *elements = at<u8 *>(object, 0x128)) {
            CRT_VectorDtorIterator(elements, 0x10, at<s32>(elements - 4, 0), reinterpret_cast<void(__fastcall *)(void *)>(0x00575bf0));
            GameHeap_Free(elements - 4);
            at<u32>(object, 0x128) = 0;
        }
        at<u32>(object, 4) = at<u32>(object, 4) & 0xdfffffff;
    }
    at<u32>(object, 0x12c) = 0;
    return 1;
}

}  // extern "C"
