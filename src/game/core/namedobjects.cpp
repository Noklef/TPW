#include "namedobjects.h"

#include <cstdint>

#include "gameclock.h"
#include "gameheap.h"
#include "stdlist.h"
#include "initializers.h"
#include "language.h"
#include "methods.h"
#include "string.h"
#include "vcall.h"
#include "vobj2.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr u32 kNames[] = {0x00786b68, 0x00785e78, 0x00785f98, 0x007862c8, 0x00785f68, 0x007862f8, 0x00786d10,
                          0x007866e8, 0x007862e0, 0x00785ed8, 0x00785f80, 0x00786aa8, 0x00786040, 0x007867d8};

u8 **registry() { return *reinterpret_cast<u8 ***>((uintptr_t)0x00786b9c); }

// A list node {object, next, prev} for the object, unlinked (none when the allocation fails).
u8 *node(String *object) {
    u8 *const n = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (!n) return nullptr;
    at<String *>(n, 0) = object;
    at<u8 *>(n, 4) = nullptr;
    at<u8 *>(n, 8) = nullptr;
    return n;
}

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

void __fastcall NamedObjects_AddName(u8 *self, void *, const String *name) {
    void *const block = GameHeap_Alloc(0x14);
    String *const s = block ? String_Ctor(static_cast<String *>(block)) : nullptr;
    vcall<u32>(s, 0x18, name);
    u8 *const head = at<u8 *>(self, 0x28);
    if (at<u32>(self, 0x30) == 0) {
        u8 *const n = node(s);
        at<u8 *>(n, 4) = head;
        if (head)
            at<u8 *>(head, 8) = n;
        else
            at<u8 *>(self, 0x2c) = n;
        at<u8 *>(self, 0x28) = n;
        ++at<u32>(self, 0x30);
        return;
    }
    for (u8 *e = head; e; e = at<u8 *>(e, 4))
        if (at<String *>(e, 0) == s) return;
    u8 *before = head;
    const u8 *object = *reinterpret_cast<const u8 *const volatile *>(before);  // (sic: before the head is tested; kept)
    if (before) {
        while (Names_FirstAfter(reinterpret_cast<const u8 *>(s), object) > 0) {
            before = at<u8 *>(before, 4);
            if (!before) break;
            object = at<const u8 *>(before, 0);
        }
    }
    if (!before) {
        u8 *const n = node(s);
        u8 *const tail = at<u8 *>(self, 0x2c);
        at<u8 *>(n, 8) = tail;
        if (tail)
            at<u8 *>(tail, 4) = n;
        else
            at<u8 *>(self, 0x28) = n;
        at<u8 *>(self, 0x2c) = n;
        ++at<u32>(self, 0x30);
    } else if (before == at<u8 *>(self, 0x28)) {
        List_PushHead(self + 0x24, nullptr, (u32)(uintptr_t)s);
    } else {
        u8 *const n = node(s);
        at<u8 *>(n, 4) = before;
        at<u8 *>(n, 8) = at<u8 *>(before, 8);
        at<u8 *>(at<u8 *>(before, 8), 4) = n;
        at<u8 *>(before, 8) = n;
        ++at<u32>(self, 0x30);
    }
}

void __cdecl NamedObjects_StaticDtor() { NamedObjects_Dtor(reinterpret_cast<u8 *>(0x00786b68)); }

u32 __fastcall Flags7a1a14_Update(const u8 *object) {
    u32 &flags = *reinterpret_cast<u32 *>((uintptr_t)0x007a1a14);
    const u32 bit = (flags >> 3) & 1;
    Obj_AdvanceUnless30(reinterpret_cast<u8 *>(0x00785970));
    *reinterpret_cast<u32 *>((uintptr_t)0x007477e4) = bit;
    if (!at<u32>(object, 0x18))
        return 0;
    flags |= 2;
    return flags;
}

void __fastcall NamedObjects_SetThemeLabel(u8 *, void *, const WString *theme, u8 *label) {
    String unused;   // (constructed and destroyed, never used)
    String_Ctor(&unused);
    static constexpr u32 kThemes[][2] = {{0x00786478, 2}, {0x00786808, 1}, {0x00786970, 0}, {0x007869e8, 3}};
    for (const auto &t : kThemes) {
        if (WNames_Equal(reinterpret_cast<const u8 *>(theme), reinterpret_cast<const u8 *>((uintptr_t)t[0]))) {
            const u8 *const table = *reinterpret_cast<u8 *const *>((uintptr_t)0x0078bad4);
            vcall<void>(label, 0x14, Language_Widen(Records_DataIfTerminated(table, nullptr, t[1])));
            break;
        }
    }
    String_Dtor(&unused);
}

void __fastcall NamedObjects_SetThemeLabelAnsi(u8 *self, void *, const String *theme, u8 *label) {
    WString wide;
    WString_Ctor(&wide);
    if (WString_AssignFromAnsi(&wide, theme))
        NamedObjects_SetThemeLabel(self, nullptr, &wide, label);
    WString_Dtor(&wide);
}
}
