#include "namedobjects.h"

#include <cstdint>

#include "class6fd9d8.h"
#include "gameclock.h"
#include "gameheap.h"
#include "stdlist.h"
#include "initializers.h"
#include "language.h"
#include "methods.h"
#include "observers.h"
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

void __fastcall NamedObjects_AddFolders(u8 *self, void *, const WString *root) {
    using Ctor = u8 *(__fastcall *)(u8 *, void *, const WString *, const WString *);
    struct Entry {
        u32 size;
        Ctor ctor;
        u32 name;
        bool at8;   // (added by Obj_SetAt8AndAdd)
        u32 next;   // (the String the next folder gets appended; 0 for the last)
    };
    static const Entry kEntries[] = {
        {0x48, &Class6fda58_Ctor, 0x00786b68, false, 0x007869d0}, {0x40, &Class6fda48_Ctor, 0x00785e78, true, 0x007864f0},
        {0x40, &Class6fda48_Ctor, 0x00785f98, true, 0x007865f8},  {0x50, &Class6fda98_Ctor, 0x007862c8, false, 0x00786640},
        {0x50, &Class6fda98_Ctor, 0x00785f68, false, 0x00786388}, {0x50, &Class6fda98_Ctor, 0x007862f8, false, 0x00785f38},
        {0x50, &Class6fda98_Ctor, 0x00786d10, false, 0x007868e0}, {0x50, &Class6fda98_Ctor, 0x007866e8, false, 0x00786778},
        {0x50, &Class6fda88_Ctor, 0x007862e0, false, 0x00786af0}, {0x48, &Class6fdad8_Ctor, 0x00785ed8, false, 0x007863b8},
        {0x48, &Class6fda58_Ctor, 0x00785f80, false, 0x00786850}, {0x48, &Class6fda58_Ctor, 0x00786aa8, false, 0x007869a0},
        {0x48, &Class6fda58_Ctor, 0x00786040, false, 0x007869b8}, {0x48, &Class6fda58_Ctor, 0x007867d8, false, 0},
    };
    String folder;
    String_Ctor(&folder);
    vcall<void>(&folder, 0x18, root);
    for (const Entry &e : kEntries) {
        void *const block = GameHeap_Alloc(e.size);
        u8 *const object = block ? e.ctor(static_cast<u8 *>(block), nullptr, reinterpret_cast<const WString *>((uintptr_t)e.name),
                                          reinterpret_cast<const WString *>(&folder))
                                 : nullptr;
        Class6fd9d8_AddFolderAndWad(object, nullptr, reinterpret_cast<const StringView *>(&folder));
        if (e.at8)
            Obj_SetAt8AndAdd(at<u8 *>(self, 0x34), nullptr, object);
        else
            ObserverList0_Add(at<u8 *>(self, 0x34), nullptr, object);
        if (!e.next) break;
        vcall<void>(&folder, 0x18, root);
        String_EnsureTrailingBackslash(&folder);
        vcall<void>(&folder, 0x28, reinterpret_cast<const StringView *>((uintptr_t)e.next));
    }
    String_Dtor(&folder);
}

void __fastcall NamedObjects_AddListedFolders(u8 *self) {
    static constexpr u32 kParts[][2] = {{0x00786430, 0x00786a48}, {0x00785e90, 0x00786ce0}, {0x00786670, 0x007860e8}};
    const auto *const root = reinterpret_cast<const WString *>((uintptr_t)0x00785e48);
    for (u8 *node = *reinterpret_cast<u8 **>((uintptr_t)0x00786b90); node;) {
        String folder, name;
        String_Ctor(&folder);
        String_Ctor(&name);
        const auto *const listed = at<const StringView *>(node, 0);
        node = at<u8 *>(node, 4);
        for (u32 k = 0; k < 3; ++k) {
            vcall<void>(&folder, 0x18, root);
            String_EnsureTrailingBackslash(&folder);
            vcall<void>(&folder, 0x28, listed);
            String_EnsureTrailingBackslash(&folder);
            vcall<void>(&folder, 0x28, reinterpret_cast<const StringView *>((uintptr_t)kParts[k][0]));
            vcall<void>(&name, 0x18, reinterpret_cast<const WString *>((uintptr_t)kParts[k][1]));
            vcall<void>(&name, 0x28, listed);
            void *const block = GameHeap_Alloc(0x40);
            u8 *const object = block ? Class6fda68_Ctor(static_cast<u8 *>(block), nullptr, reinterpret_cast<const WString *>(&name),
                                                        reinterpret_cast<const WString *>(&folder))
                                     : nullptr;
            if (k < 2)
                Class6fd9d8_AddFolderAndWad(object, nullptr, reinterpret_cast<const StringView *>(&folder));
            else
                Class6fd9d8_AddFolderAndWadBoth(object, nullptr, reinterpret_cast<const StringView *>(&folder));
            ObserverList0_Add(at<u8 *>(self, 0x34), nullptr, object);
        }
        String_Dtor(&name);
        String_Dtor(&folder);
    }
}

void __fastcall NamedObjects_AddFixedFolders(u8 *self) {
    using Ctor = u8 *(__fastcall *)(u8 *, void *, const WString *, const WString *);
    enum : u8 { kFolderAndWad, kBoth, kFolder };
    struct Entry {
        u32 size;
        Ctor ctor;
        u32 name, folder;
        u8 add;
        u32 wad;   // (a WAD String added after the folder, kFolder only; 0 for none)
    };
    static const Entry kEntries[] = {
        {0x48, &Class6fda58_Ctor, 0x00786940, 0x007861a8, kFolderAndWad, 0}, {0x48, &Class6fda58_Ctor, 0x00786b38, 0x00786088, kFolderAndWad, 0},
        {0x40, &Class6fdaf8_Ctor, 0x00786cc8, 0x007866a0, kBoth, 0},         {0x40, &Class6fda68_Ctor, 0x00786358, 0x00785f20, kFolderAndWad, 0},
        {0x40, &Class6fda68_Ctor, 0x007867f0, 0x00786bf0, kFolderAndWad, 0}, {0x40, &Class6fdaa8_Ctor, 0x00785fc8, 0x00786a00, kFolderAndWad, 0},
        {0x50, &Class6fdac8_Ctor, 0x00786610, 0x00786bc0, kFolder, 0x00786028},
        {0x40, &Class6fdab8_Ctor, 0x00786c08, 0x00786520, kFolder, 0x007860a0}, {0x48, &Class6fda58_Ctor, 0x00786298, 0x00786598, kFolder, 0},
        {0x40, &Class6fdae8_Ctor, 0x00786d88, 0x00786dd0, kFolder, 0},        {0x48, &Class6fda58_Ctor, 0x00786310, 0x00786cb0, kFolder, 0x007860b8},
        {0x50, &Class6fda88_Ctor, 0x00786ba8, 0x00786208, kFolderAndWad, 0}, {0x48, &Class6fda58_Ctor, 0x00786d40, 0x00786988, kFolder, 0x00786d28},
    };
    for (u32 i = 0; i < sizeof kEntries / sizeof kEntries[0]; ++i) {
        const Entry &e = kEntries[i];
        if (i == 7) NamedObjects_AddListedFolders(self);
        const auto *const folder = reinterpret_cast<const StringView *>((uintptr_t)e.folder);
        void *const block = GameHeap_Alloc(e.size);
        u8 *const object = block ? e.ctor(static_cast<u8 *>(block), nullptr, reinterpret_cast<const WString *>((uintptr_t)e.name),
                                          reinterpret_cast<const WString *>(folder))
                                 : nullptr;
        if (e.add == kFolderAndWad)
            Class6fd9d8_AddFolderAndWad(object, nullptr, folder);
        else if (e.add == kBoth)
            Class6fd9d8_AddFolderAndWadBoth(object, nullptr, folder);
        else {
            Class6fd9d8_AddFolder(object, nullptr, reinterpret_cast<const String *>(folder));
            if (e.wad) Class6fd9d8_AddWad(object, nullptr, reinterpret_cast<const String *>((uintptr_t)e.wad));
        }
        ObserverList0_Add(at<u8 *>(self, 0x34), nullptr, object);
    }
}
}
