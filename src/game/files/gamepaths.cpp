// The game's folder paths: see gamepaths.h.
#include "gamepaths.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/string.h"
#include "../core/trivial.h"
#include "../core/vcall.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
}  // namespace

extern "C" {

u32 __cdecl GamePaths_FontsFolder(char *out) {
    const char *const root = *reinterpret_cast<const char *const *>(Obj_Self(at<u8 *>(0x00f816c0)) + 8);
    if (root == nullptr)
        return 0;
    s32 length = 0;
    while (root[length] != 0)
        ++length;
    const char *const format = root[length - 1] == '\\' ? at<const char *>(0x0074d320) : at<const char *>(0x007488ac);   // (empty: root[-1], sic)
    Crt().sprintf(out, format, root, at<const char *>(0x0074d6c0));
    return 1;
}

s32 __cdecl GamePaths_SetTextureFolders(u32 low) {
    char *const textures = reinterpret_cast<char *>(0x007af650);
    char *const shared = reinterpret_cast<char *>(0x007af720);
    if (low) {
        Crt().sprintf(textures, reinterpret_cast<const char *>(0x0074d6f4));
        return Crt().sprintf(shared, reinterpret_cast<const char *>(0x00747e88));
    }
    Crt().sprintf(textures, reinterpret_cast<const char *>(0x0074d6e8));
    return Crt().sprintf(shared, reinterpret_cast<const char *>(0x0074d6dc));
}

WString *__stdcall GamePaths_SaveFolder(WString *out) {
    WString path;
    WString_Ctor(&path);
    WString_AssignFromAnsi(&path, reinterpret_cast<const String *>(Obj_Self(at<u8 *>(0x00f816c0))));
    vcall<void>(&path, 0x28, at<const WString *>(0x00f818d8));
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}
}  // extern "C"
