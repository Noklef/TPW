// The game's virtual file system: see vfs.h.
#include "vfs.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../core/vobj2.h"
#include "../core/vwrap.h"
#include "file.h"
#include "handles.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}

// The leading copies of the global "\" (0x78bee0) dropped from s, one at a time.
void drop_leading_backslashes(String *s) {
    const String *const backslash = at<const String *>(0x0078bee0);
    StringView view;
    while (String_Equals(reinterpret_cast<const String *>(String_Left(s, nullptr, &view, backslash->length)), nullptr,
                         reinterpret_cast<const StringView *>(backslash))) {
        StringView rest;
        vcall<void>(s, 0x1c, String_Right(s, nullptr, &rest, s->length - backslash->length));
    }
}

bool names_equal(const String *a, const String *b) {
    return Names_Equal(reinterpret_cast<const u8 *>(a), reinterpret_cast<const u8 *>(b)) == 1;
}

}  // namespace

extern "C" {

u32 __cdecl Path_StripBase(String *path, String *base, String *out) {
    const String *const wadTail = at<const String *>(0x0078bf40);
    const String *const wad = at<const String *>(0x0078c810);
    String a, b;
    String_Ctor(&a);
    String_Ctor(&b);
    vcall<void>(&a, 0x18, String_SkipDotPrefix(path));
    vcall<void>(&b, 0x18, String_SkipDotPrefix(base));
    drop_leading_backslashes(&a);
    StringView view;
    String part;
    String_CtorCopy(&part, nullptr, String_Right(&b, nullptr, &view, wadTail->length));
    const bool archive = names_equal(&part, wad);
    String_Dtor(&part);
    if (archive) {
        vcall<void>(&b, 0x1c, String_Left(&b, nullptr, &view, b.length - wad->length));
        String_EnsureTrailingBackslash(&b);
    }
    u32 result = 0;
    if (wad->length + a.length >= b.length) {
        String_CtorCopy(&part, nullptr, String_Left(&a, nullptr, &view, b.length));
        const bool under = names_equal(&part, &b);
        String_Dtor(&part);
        if (under) {
            vcall<void>(out, 0x1c, String_Right(&a, nullptr, &view, a.length - b.length));
            drop_leading_backslashes(out);
            result = 1;
        }
    }
    if (!result) vcall<void>(out, 0x18, at<const String *>(0x0078be50));
    String_Dtor(&b);
    String_Dtor(&a);
    return result;
}

StreamBase *__cdecl VFS_Open(const char *name) {
    void *const block = GameHeap_Alloc(8);
    StreamBase *const stream = block ? StreamBase_Ctor(static_cast<StreamBase *>(block)) : nullptr;
    u8 *const source = reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(StreamBase_GetSource(*at<u8 **>(0x00786b9c))));
    if (source) {
        u8 *const archive = reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(Get38_41ce90(source)));
        if (archive) {
            String relative, root, path;
            String_Ctor(&relative);
            String_Ctor(&root);
            String_CtorStr(&path, nullptr, name);
            vcall<void>(&root, 0x18, source + 0x18);
            bool opened = false;
            if (Path_StripBase(&path, &root, &relative) == 1) {
                s32 result;
                opened = *vcall<s32 *>(archive, 0xc, &result, stream, relative.data, 0x80000001u, 0u) >= 0;
            }
            String_Dtor(&path);
            String_Dtor(&root);
            String_Dtor(&relative);
            if (opened) return stream;
        }
        String path;
        String_CtorStr(&path, nullptr, name);
        const bool opened = vcall<u32>(source, 8, stream, &path, 0x80000001u) == 1;
        String_Dtor(&path);
        if (opened) return stream;
    }
    String path;
    String_CtorStr(&path, nullptr, name);
    const bool opened = ListRef_AnyHandles(*at<u8 ***>(0x00786b9c), nullptr, static_cast<u32>(reinterpret_cast<std::uintptr_t>(stream)),
                                           static_cast<u32>(reinterpret_cast<std::uintptr_t>(&path))) == 1;
    String_Dtor(&path);
    return opened ? stream : nullptr;
}

u32 __cdecl VFS_FileExists(const char *name) {
    StreamBase *const stream = VFS_Open(name);
    if (!stream) return 0;
    Object_FinishAndDelete(reinterpret_cast<u8 *>(stream));
    return 1;
}

u32 __cdecl VFS_ExtractFile(const char *from, const char *to) {
    StreamBase *const in = VFS_Open(from);
    if (!in) return 0;
    const u32 size = vcall<u32>(in, 0x14);
    void *const buffer = Mem_GlobalAlloc(size);
    if (!buffer) return 0;
    vcall<u32>(in, 8, buffer, size);
    vcall<u32>(in, 4);
    vcall<void>(in, 0, 1u);
    void *const block = GameHeap_Alloc(8);
    StreamBase *out = nullptr;
    if (block) {
        out = StreamBase_Ctor(static_cast<StreamBase *>(block));
        *reinterpret_cast<u32 *>(out) = 0x006fd930;   // FileStream_vtable
    }
    s32 result;
    if (*vcall<s32 *>(out, 0x24, &result, to, 0xc0000010u) < 0) return 0;
    vcall<u32>(out, 0xc, buffer, size);
    vcall<u32>(out, 4);
    if (out) vcall<void>(out, 0, 1u);
    return 1;
}
}
