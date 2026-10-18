#include "class6fd9d8.h"

#include <cstdint>
#include <initializer_list>

#include "gameheap.h"
#include "language.h"
#include "string.h"
#include "vcall.h"
#include "../files/handles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

namespace {

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1) GameHeap_Free(object);
    return object;
}

void BaseFinish(u8 *object) {
    at<u32>(object, 0) = 0x006fda58;
    Handle_Dtor(reinterpret_cast<Handle *>(object + 0x40));
    Class6fd9d8_Dtor(object);
}

void BaseCtor(u8 *object, const WString *name, const WString *folder) {
    String *const first = reinterpret_cast<String *>(object + 4);
    String *const path = reinterpret_cast<String *>(object + 0x18);
    String_Ctor(first);
    String_Ctor(path);
    at<u32>(object, 0) = 0x006fd9d8;
    vcall<void>(first, 0x18, name);
    vcall<void>(path, 0x18, folder);
    String_EnsureTrailingBackslash(path);
    for (u32 offset : {0x30u, 0x34u, 0x38u, 0x3cu, 0x2cu})
        at<u32>(object, offset) = 0;
    u32 *const list = static_cast<u32 *>(GameHeap_Alloc(0x10));
    if (list != nullptr) {
        list[1] = list[2] = list[3] = 0;
        list[0] = 0x006fd9d4;
    }
    at<u32 *>(object, 0x2c) = list;
}

u8 *Plain(u8 *object, const WString *name, const WString *folder, u32 vtable) {
    BaseCtor(object, name, folder);
    at<u32>(object, 0) = vtable;
    return object;
}

u8 *WithHandle(u8 *object, const WString *name, const WString *folder, u32 vtable) {
    BaseCtor(object, name, folder);
    Handle_Ctor(reinterpret_cast<Handle *>(object + 0x40));
    at<u32>(object, 0) = vtable;
    return object;
}

u8 *WithTwoHandles(u8 *object, const WString *name, const WString *folder, u32 middle, u32 vtable) {
    WithHandle(object, name, folder, middle);
    Handle_Ctor(reinterpret_cast<Handle *>(object + 0x48));
    at<u32>(object, 0) = vtable;
    return object;
}

void Derived(u8 *object, u32 vtable) {
    at<u32>(object, 0) = vtable;
    Handle_Dtor(reinterpret_cast<Handle *>(object + 0x48));
    BaseFinish(object);
}

}  // namespace

extern "C" {

void __fastcall Class6fd9d8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fd9d8;
    if (u8 *member = at<u8 *>(object, 0x3c))
        vcall<u32>(member, 0xc, 1u);
    u8 *list = at<u8 *>(object, 0x2c);
    at<u32>(object, 0x3c) = 0;
    if (list) {
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
                held = at<u8 *>(node, 0);
                at<u32>(list, 0xc) = at<u32>(list, 0xc) - 1;
                GameHeap_Free(node);
            }
            if (held)
                vcall<u32>(held, 0x2c, 1u);
            list = at<u8 *>(object, 0x2c);
        }
        if (u8 *l = at<u8 *>(object, 0x2c))
            vcall<u32>(l, 0, 1u);
        at<u32>(object, 0x2c) = 0;
    }
    String_Dtor(reinterpret_cast<String *>(object + 0x18));
    String_Dtor(reinterpret_cast<String *>(object + 4));
}

void __fastcall Class6fda48_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fda48;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fda68_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fda68;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdaa8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdaa8;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdab8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdab8;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdae8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdae8;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdaf8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdaf8;
    Class6fd9d8_Dtor(object);
}

u8 *__fastcall Class6fda48_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda48_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fda68_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda68_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdaa8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdaa8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdab8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdab8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdae8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdae8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdaf8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdaf8_Dtor(object);
    return Finish(object, flags);
}

void __fastcall Class6fda58_DtorA(u8 *object) { BaseFinish(object); }

void __fastcall Class6fda58_DtorB(u8 *object) { BaseFinish(object); }

void __fastcall Class6fda58_DtorC(u8 *object) { BaseFinish(object); }

void __fastcall Class6fda88_FullDtor(u8 *object) { Derived(object, 0x006fda88); }

void __fastcall Class6fdac8_FullDtor(u8 *object) { Derived(object, 0x006fdac8); }

void __fastcall Class6fdaa4_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fdaa4;
    Handle_Dtor(reinterpret_cast<Handle *>(object + 0x2c));
    String_Dtor(reinterpret_cast<String *>(object + 0x18));
    String_Dtor(reinterpret_cast<String *>(object + 4));
}

u8 *__fastcall Class6fda58_ScalarDtorB(u8 *object, void *, u32 flags) {
    Class6fda58_DtorB(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fda88_FullScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda88_FullDtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdaa4_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdaa4_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdac8_FullScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdac8_FullDtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fda58_ScalarDtorC(u8 *object, void *, u32 flags) {
    Class6fda58_DtorC(object);
    return Finish(object, flags);
}
}  // extern "C"

namespace {

// A new handle (Handle_Ctor) with the vtable given, or nullptr when the allocation fails.
Handle *NewHandle(u32 vtable) {
    Handle *const h = static_cast<Handle *>(GameHeap_Alloc(8));
    if (!h)
        return nullptr;
    Handle_Ctor(h);
    at<u32>(h, 0) = vtable;
    return h;
}

// The handle appended to the list at +0x2c (nodes {object, next, prev}); a failed node allocation is written through (sic).
void Hold(u8 *object, Handle *h) {
    u8 *const list = at<u8 *>(object, 0x2c);
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (node) {
        at<Handle *>(node, 0) = h;
        at<u32>(node, 4) = 0;
        at<u32>(node, 8) = 0;
    }
    *reinterpret_cast<volatile u32 *>((uintptr_t)node + 8) = at<u32>(list, 8);
    if (u8 *const tail = at<u8 *>(list, 8))
        at<u8 *>(tail, 4) = node;
    else
        at<u8 *>(list, 4) = node;
    at<u8 *>(list, 8) = node;
    at<u32>(list, 0xc) = at<u32>(list, 0xc) + 1;
}

}  // namespace

extern "C" {

void __fastcall Class6fd9d8_AddFolder(u8 *object, void *, const String *path) {
    Handle *const h = NewHandle(0x006fd9e8);
    if (!h)
        return;
    s32 result;
    if (*Handle_Open28(h, nullptr, &result, (u32)(uintptr_t)path->data) < 0) {
        FolderHandle_DeletingDtor(h, nullptr, 1);  // (the vtable's +0x2c)
        return;
    }
    StreamBase stream;
    StreamBase_Ctor(&stream);
    Hold(object, h);
    StreamBase_Dtor(&stream);
}

void __fastcall Class6fd9d8_AddWad(u8 *object, void *, const String *path) {
    Handle *const h = NewHandle(0x006fda18);
    if (!h)
        return;
    s32 result;
    if (*Handle_OpenE0(h, nullptr, &result, (u32)(uintptr_t)path->data, 0) < 0) {
        FolderHandle_DeletingDtor(h, nullptr, 1);  // (the vtable's +0x2c)
        return;
    }
    Hold(object, h);
}

void __fastcall Class6fd9d8_AddFolderAndWad(u8 *object, void *, const StringView *path) {
    String name;
    String_CtorCopy(&name, nullptr, path);
    Class6fd9d8_AddFolder(object, nullptr, &name);
    String_Append(&name, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x0078bb08));  // (the vtable's +0x28)
    Class6fd9d8_AddWad(object, nullptr, &name);
    String_Dtor(&name);
}

void __fastcall Class6fd9d8_AddFolderAndWadIn(u8 *object, void *, const StringView *path) {
    String name;
    String_CtorCopy(&name, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f816d4));   // (Obj_Part14 of 0xf816c0)
    String_EnsureTrailingBackslash(&name);
    String_Append(&name, nullptr, path);   // (the vtable's +0x28)
    Class6fd9d8_AddFolder(object, nullptr, &name);
    String_Append(&name, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x0078bb08));
    Class6fd9d8_AddWad(object, nullptr, &name);
    String_Dtor(&name);
}

u32 __fastcall Class6fd9d8_Find(u8 *object, void *, u32 key) {
    u8 *const current = at<u8 *>(object, 0x34);
    if (!current)
        return 0;
    s32 result;
    if (*vcall<s32 *>(current, 8, &result, key) >= 0)
        return 1;
    u8 *node = at<u8 *>(at<u8 *>(object, 0x2c), 4);
    while (node && at<u8 *>(node, 0) != at<u8 *>(object, 0x34))
        node = at<u8 *>(node, 4);
    u8 *next = *reinterpret_cast<u8 *volatile *>((uintptr_t)node + 4);  // (none found: the null node read through, sic)
    while (next) {
        u8 *const held = at<u8 *>(next, 0);
        next = at<u8 *>(next, 4);
        if (*vcall<s32 *>(held, 4, &result, at<u32>(at<u8 *>(object, 0x3c), 8), key) >= 0) {
            at<u8 *>(object, 0x34) = held;
            at<u8 *>(object, 0x38) = held;
            return 1;
        }
    }
    if (u8 *const member = at<u8 *>(object, 0x3c))
        vcall<u32>(member, 0xc, 1u);
    at<u32>(object, 0x3c) = 0;
    at<u32>(object, 0x34) = 0;
    return 0;
}

void __fastcall Class6fda98_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fda98;
    if (u8 *list = at<u8 *>(object, 0x4c)) {
        while (at<u32>(list, 0xc) > 0) {
            u8 *const node = at<u8 *>(list, 4);
            u8 *held = nullptr;
            if (node) {
                u8 *const next = at<u8 *>(node, 4);
                at<u8 *>(list, 4) = next;
                if (next)
                    at<u32>(next, 8) = 0;
                else
                    at<u32>(list, 8) = 0;
                held = at<u8 *>(node, 0);
                at<u32>(list, 0xc) = at<u32>(list, 0xc) - 1;
                GameHeap_Free(node);
            }
            if (held)
                vcall<u32>(held, 0, 1u);
            list = at<u8 *>(object, 0x4c);
        }
        if (u8 *const again = at<u8 *>(object, 0x4c))
            vcall<u32>(again, 0, 1u);
        at<u32>(object, 0x4c) = 0;
        at<u32>(object, 0x48) = 0;
    }
    BaseFinish(object);
}

u8 *__fastcall Class6fda98_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda98_Dtor(object);
    return Finish(object, flags);
}
}  // extern "C"

namespace {

const StringView *GlobalString(u32 address) { return reinterpret_cast<const StringView *>((uintptr_t)address); }

// The opener's slot 3 (&result, stream, the name's text, 0x80000001, 0): whether the result it points to is not negative.
u32 OpenNamed(u8 *opener, u32 stream, const String *name) {
    s32 result;
    return *vcall<s32 *>(opener, 0xc, &result, stream, static_cast<const char *>(name->data), 0x80000001u, 0u) >= 0 ? 1 : 0;
}

// A name built as prefix + base + suffix (copies of the global strings; prefix 0: the base copied) opened through the opener.
u32 OpenAffixed(u32 prefix, const StringView *base, u32 suffix, u8 *opener, u32 stream) {
    String name;
    if (prefix) {
        String_CtorCopy(&name, nullptr, GlobalString(prefix));
        String_Append(&name, nullptr, base);
    } else {
        String_CtorCopy(&name, nullptr, base);
    }
    String_Append(&name, nullptr, GlobalString(suffix));
    const u32 opened = OpenNamed(opener, stream, &name);
    String_Dtor(&name);
    return opened;
}

// A name assigned (the vtable's +0x18, String_AssignWide) from text to an empty string, the suffix appended, opened through the handle at +0x2c.
u32 OpenAssigned(u8 *object, const void *text, u32 suffix, u32 stream) {
    String name;
    String_Ctor(&name);
    String_AssignWide(&name, nullptr, static_cast<const WString *>(text));
    String_Append(&name, nullptr, GlobalString(suffix));
    const u32 opened = OpenNamed(object + 0x2c, stream, &name);
    String_Dtor(&name);
    return opened;
}

// Prefix + the object's name (+4), then OpenAssigned with ".sam".
u32 OpenPrefixedOwn(u8 *object, u32 prefix, u32 stream) {
    String full;
    String_CtorCopy(&full, nullptr, GlobalString(prefix));
    String_Append(&full, nullptr, reinterpret_cast<const StringView *>(object + 4));
    const u32 opened = OpenAssigned(object, &full, 0x0078c780, stream);
    String_Dtor(&full);
    return opened;
}

}  // namespace

extern "C" {

u32 __stdcall Class6fdaa4_OpenSamAt(const StringView *base, u8 *opener, u32 stream) {
    return OpenAffixed(0, base, 0x0078c780, opener, stream);
}

u32 __fastcall Class6fdaa4_OpenSamWith(u8 *object, void *, u8 *opener, u32 stream) {
    return Class6fdaa4_OpenSamAt(reinterpret_cast<const StringView *>(object + 4), opener, stream);
}

u32 __stdcall Class6fdaa4_OpenOnlineSamAt(const StringView *base, u8 *opener, u32 stream) {
    return OpenAffixed(0x0078c990, base, 0x0078c738, opener, stream);
}

u32 __fastcall Class6fdaa4_OpenOnlineSamWith(u8 *object, void *, u8 *opener, u32 stream) {
    return Class6fdaa4_OpenOnlineSamAt(reinterpret_cast<const StringView *>(object + 4), opener, stream);
}

u32 __fastcall Class6fdaa4_OpenSam(u8 *object, void *, const String *name, u32 stream) {
    return OpenAssigned(object, name, 0x0078c780, stream);
}

u32 __fastcall Class6fdaa4_OpenOwnSam(u8 *object, void *, u32 stream) {
    return Class6fdaa4_OpenSam(object, nullptr, reinterpret_cast<const String *>(object + 4), stream);
}

u32 __fastcall Class6fdaa4_OpenCoasterSam(u8 *object, void *, u32 stream) {
    return Class6fdaa4_OpenSam(object, nullptr, reinterpret_cast<const String *>((uintptr_t)0x0078c708), stream);
}

u32 __fastcall Class6fdaa4_OpenEasySam(u8 *object, void *, u32 stream) { return OpenPrefixedOwn(object, 0x0078bb98, stream); }

u32 __fastcall Class6fdaa4_OpenOnlineSam(u8 *object, void *, u32 stream) { return OpenPrefixedOwn(object, 0x0078c990, stream); }

u32 __fastcall Class6fdaa4_OpenLanguageTxt(u8 *object, void *, u32 stream) {
    String language;
    String_CtorStr(&language, nullptr, Language_Buffer(reinterpret_cast<const u8 *>((uintptr_t)0x0078b530)));
    const u32 opened = OpenAssigned(object, &language, 0x0078bec8, stream);
    String_Dtor(&language);
    return opened;
}

void __fastcall Class6fdaa4_TrimName(u8 *, void *, String *s) {
    const char *const underscore = reinterpret_cast<const char *>((uintptr_t)0x0074c77c);
    const s32 first = String_Find(s, nullptr, underscore, 0);
    const s32 last = String_ReverseFind(s, nullptr, underscore, 0xffffffffu);
    if (first == last)
        return;
    if (String_Find(s, nullptr, underscore, static_cast<u32>(first + 1)) != last)
        return;
    StringView view;
    String_Assign(s, nullptr, String_Left(s, nullptr, &view, static_cast<u32>(last)));
    if (String_EqualsStr(reinterpret_cast<const String *>(String_Left(s, nullptr, &view, 1)), nullptr,
                         reinterpret_cast<const char *>((uintptr_t)0x0074c778)))
        String_Assign(s, nullptr, String_Right(s, nullptr, &view, s->length - 1));
}

u32 __fastcall Class6fdaa4_HasEmt(u8 *object) {
    struct Found {
        u8 head[0x20];
        char *name;
        char text[260];
    } found;
    found.text[0] = 0;
    found.name = found.text;
    s32 result;
    const char *const pattern = *reinterpret_cast<const char *const *>((uintptr_t)0x0078bcf0);   // the String 0x78bce8's text
    return *vcall<s32 *>(object + 0x2c, 4, &result, pattern, &found) >= 0 ? 1 : 0;
}

u8 *__fastcall Class6fda48_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return Plain(object, name, folder, 0x006fda48); }
u8 *__fastcall Class6fda68_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return Plain(object, name, folder, 0x006fda68); }
u8 *__fastcall Class6fdaa8_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return Plain(object, name, folder, 0x006fdaa8); }
u8 *__fastcall Class6fdab8_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return Plain(object, name, folder, 0x006fdab8); }
u8 *__fastcall Class6fdae8_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return Plain(object, name, folder, 0x006fdae8); }
u8 *__fastcall Class6fdaf8_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return Plain(object, name, folder, 0x006fdaf8); }
u8 *__fastcall Class6fda58_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return WithHandle(object, name, folder, 0x006fda58); }
u8 *__fastcall Class6fdad8_Ctor(u8 *object, void *, const WString *name, const WString *folder) { return WithHandle(object, name, folder, 0x006fdad8); }

u8 *__fastcall Class6fda88_Ctor(u8 *object, void *, const WString *name, const WString *folder) {
    return WithTwoHandles(object, name, folder, 0x006fda78, 0x006fda88);
}

u8 *__fastcall Class6fdac8_Ctor(u8 *object, void *, const WString *name, const WString *folder) {
    return WithTwoHandles(object, name, folder, 0x006fda58, 0x006fdac8);
}

u8 *__fastcall Class6fda98_Ctor(u8 *object, void *, const WString *name, const WString *folder) {
    WithHandle(object, name, folder, 0x006fda98);
    at<u32>(object, 0x4c) = 0;
    at<u32>(object, 0x48) = 0;
    return object;
}
}  // extern "C"
