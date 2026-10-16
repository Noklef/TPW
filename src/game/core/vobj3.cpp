#include "vobj3.h"

#include <cstdint>

#include "../gfx/animtex.h"
#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void Delete(u8 *object) { vcall<u32>(object, 0, 1u); }

void ReleaseCom(u8 *object, u32 field) {
    if (u8 *member = at<u8 *>(object, field)) {
        comcall<u32>(member, 8);
        at<u32>(object, field) = 0;
    }
}

}  // namespace

extern "C" {

u32 __fastcall Obj_StoreComSlot80(u8 *object) {
    global<u32>(0x00879cd4) = comcall<u32>(at<u8 *>(object, 0x14), 0x80, 0u);
    return 1;
}

u8 *__fastcall Counted_Release(u8 *counted) {
    const u32 count = at<u32>(counted, 4);  // (the STL lock around it is left out)
    if (count != 0 && count != 0xffffffffu)
        at<u32>(counted, 4) = count - 1;
    return at<u32>(counted, 4) != 0 ? nullptr : counted;
}

void __fastcall CountedRef_Release(u8 **ref) {
    if (!*ref)
        return;
    if (u8 *dead = Counted_Release(*ref))
        Delete(dead);
}

u32 __fastcall Obj_ReleaseSevenComMembers(u8 *object) {
    if (at<u8>(object, 0x780) & 1) {
        if (u8 *member = at<u8 *>(object, 8))
            comcall<u32>(member, 0x4c);
    }
    ReleaseCom(object, 0x14);
    ReleaseCom(object, 0x10);
    ReleaseCom(object, 0x18);
    ReleaseCom(object, 0xc);
    ReleaseCom(object, 0);
    ReleaseCom(object, 4);
    ReleaseCom(object, 8);
    return 1;
}

u32 __fastcall Object_FreeSixPointerArrays(u8 *object) {
    AnimTextures_Clear();
    u8 *array = object;
    for (u32 group = 0; group < 6; ++group, array += 0x400) {
        const u32 countAt = 0x1800 + 4 * group;
        for (s32 i = 0; i < at<s32>(object, countAt); ++i) {
            if (u8 *record = at<u8 *>(array, 4 * i)) {
                if (u8 *held = at<u8 *>(record, 0))
                    vcall<u32>(held, 0x24, 1u);
                GameHeap_Free(at<u8 *>(array, 4 * i));
            }
        }
        at<u32>(object, countAt) = 0;
    }
    return 1;
}

u32 __fastcall ListRef_Remove(u8 **ref, void *, u8 *object) {
    u8 *list = *ref;
    if (!list)
        return 0;
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u8 *>(node, 0) != object)
        node = at<u8 *>(node, 4);
    if (!node)
        return 0;
    if (at<u8 *>(ref, 4) == object)
        at<u32>(ref, 4) = 0;
    if (node == at<u8 *>(list, 4))
        at<u8 *>(list, 4) = at<u8 *>(node, 4);
    else
        at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
    if (node == at<u8 *>(list, 8))
        at<u8 *>(list, 8) = at<u8 *>(node, 8);
    else
        at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
    const u32 left = at<u32>(list, 0xc) - 1;
    u8 *held = at<u8 *>(node, 0);
    at<u32>(list, 0xc) = left;
    GameHeap_Free(node);
    if (held)
        Delete(held);
    return 1;
}

u32 __fastcall Pool_AddSorted(u8 *pool, void *, u8 **head, u8 *object) {
    u8 *node = at<u8 *>(pool, 4);
    if (node)
        at<u8 *>(pool, 4) = at<u8 *>(node, 4);
    at<u32>(pool, 0xc) += 1;
    if (!node)
        return 0;
    at<u8 *>(node, 0) = object;
    for (u8 *p = *head; p; p = at<u8 *>(p, 4)) {
        if (at<u8 *>(p, 0) == object)
            return 1;
    }
    if (vcall<u32>(object, 8) != 1) {
        u8 *after = nullptr;
        u8 *next = *head;
        while (next && vcall<u32>(at<u8 *>(next, 0), 8) == 1) {
            after = next;
            next = at<u8 *>(next, 4);
        }
        if (after) {
            at<u8 *>(after, 4) = node;
            at<u8 *>(node, 4) = next;
            global<u32>(0x008bd508) |= 0x80000000u;
            return 1;
        }
    }
    at<u8 *>(node, 4) = *head;
    *head = node;
    global<u32>(0x008bd508) |= 0x80000000u;
    return 1;
}

}  // extern "C"
