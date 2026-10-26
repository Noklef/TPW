// The game's folder paths: see gamepaths.h.
#include "gamepaths.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/string.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "../core/vwrap.h"
#include "file.h"
#include "handles.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
}  // namespace

extern "C" {

void __fastcall GamePaths_MakeFolders(void *, void *, const WString *root) {
    for (const u32 *node = *at<const u32 *const *>(0x00786b90); node;) {
        const StringView *const name = reinterpret_cast<const StringView *>((uintptr_t)node[0]);
        node = reinterpret_cast<const u32 *>((uintptr_t)node[1]);
        String path;
        String_Ctor(&path);
        String_AssignFromWide(&path, root);
        String_Append(&path, nullptr, at<const StringView *>(0x00f81b48));   // (both through the String's vtable +0x28)
        String_Append(&path, nullptr, name);
        Handle folder;
        Handle_Ctor(&folder);
        folder.vtable = at<void *const *>(0x006fd9e8);
        s32 status;
        if (*Handle_Open28(&folder, nullptr, &status, reinterpret_cast<u32>(path.data)) < 0) {
            s32 made;
            Dir_Create(&made, path.data);
        }
        Handle_Dtor(&folder);
        String_Dtor(&path);
    }
}

void __cdecl GamePaths_SetSecondRoot(String drive) {
    String path;
    String_Ctor(&path);
    String unused;
    String_Ctor(&unused);
    vcall<void>(&path, 0x18, &drive);
    vcall<void>(&path, 0x20, at<const char *>(0x0077316c));
    vcall<void>(&path, 0x28, at<const String *>(0x00f79fe8));
    Obj_CallSub14Slot18(at<u8 *>(0x00f816c0), nullptr, static_cast<u32>(reinterpret_cast<std::uintptr_t>(&path)));
    String_Dtor(&unused);
    String_Dtor(&path);
    String_Dtor(&drive);
}

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

WString *__stdcall GamePaths_UsersFolder(WString *out) {
    WString path;
    GamePaths_SaveFolder(&path);
    vcall<void>(&path, 0x28, at<const WString *>(0x00f82508));
    vcall<void>(&path, 0x28, at<const WString *>(0x00f81ff8));
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}

WString *__stdcall GamePaths_OnlineFolder(WString *out) {
    WString path;
    GamePaths_SaveFolder(&path);
    vcall<void>(&path, 0x28, at<const WString *>(0x00f82508));
    vcall<void>(&path, 0x28, at<const WString *>(0x00f820e8));
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}
}  // extern "C"
