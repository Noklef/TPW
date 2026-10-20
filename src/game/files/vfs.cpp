// The game's virtual file system: see vfs.h.
#include "vfs.h"

#include <cstdint>

#include "../core/string.h"
#include "../core/vcall.h"

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
}
