// The game's folder paths: see gamepaths.h.
#include "gamepaths.h"

#include <cstdint>
#include <cstring>

#include "../core/app.h"
#include "../core/crt.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../core/string.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "../core/vwrap.h"
#include "dirsource.h"
#include "drives.h"
#include "file.h"
#include "handles.h"
#include "../online/postcardsend.h"

namespace {

AddrCalls g_calls;

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

void __stdcall GamePaths_MakeFoldersCopying(const WString *root, u32 copy) {
    const String *const separator = at<const String *>(0x00f81b48);
    const char *const easymode = at<const char *>(0x00773cd0);
    const String *const extension = at<const String *>(0x00f825e0);
    String base;
    String_Ctor(&base);
    String_AssignFromWide(&base, root);
    s32 result;
    Dir_Create(&result, base.data);
    for (const u32 *node = *at<const u32 *const *>(0x00786b90); node;) {
        const String *const name = reinterpret_cast<const String *>((uintptr_t)node[0]);
        node = reinterpret_cast<const u32 *>((uintptr_t)node[1]);
        String folder;
        String_CtorCopy(&folder, nullptr, reinterpret_cast<const StringView *>(&base));
        vcall<void>(&folder, 0x28, separator);
        vcall<void>(&folder, 0x28, name);
        Dir_Create(&result, folder.data);
        if (copy) {
            String from;
            String_Ctor(&from);
            vcall<void>(&from, 0x18, at<const String *>(0x00f816e8));   // (MemorySource_GetLock of 0xf816c0)
            vcall<void>(&from, 0x28, separator);
            vcall<void>(&from, 0x28, at<const String *>(0x00f81980));
            vcall<void>(&from, 0x28, separator);
            vcall<void>(&from, 0x28, name);
            vcall<void>(&from, 0x28, separator);
            vcall<void>(&from, 0x20, easymode);
            vcall<void>(&from, 0x28, extension);
            if (File_Exists(from.data)) {
                String to;
                String_Ctor(&to);
                vcall<void>(&to, 0x18, &folder);
                vcall<void>(&to, 0x28, separator);
                vcall<void>(&to, 0x20, easymode);
                vcall<void>(&to, 0x28, extension);
                File_Copy(&result, from.data, to.data);
                String_Dtor(&to);
            }
            String_Dtor(&from);
        }
        String_Dtor(&folder);
    }
    String_Dtor(&base);
}

void __cdecl GamePaths_RemoveTree(const String *path) {
    Handle folder;
    Handle_Ctor(&folder);
    folder.vtable = at<void *const *>(0x006fd9e8);
    s32 status;
    if (*Handle_Open28(&folder, nullptr, &status, reinterpret_cast<u32>(path->data)) >= 0) {
        alignas(4) u8 buffer[0x124];
        DirEntry *const entry = reinterpret_cast<DirEntry *>(buffer);
        entry->path[0] = 0;
        entry->name = entry->path;
        if (*Handle_Vfn1(&folder, nullptr, &status, *at<const u32 *>(0x00f81650), reinterpret_cast<u32>(entry)) >= 0) {
            do {
                if (entry->name[0] != '.') {
                    if (entry->attributes & 0x10) {
                        String inner;
                        String_CtorCopy(&inner, nullptr, reinterpret_cast<const StringView *>(path));
                        vcall<void>(&inner, 0x28, at<const String *>(0x00f80be0));
                        vcall<void>(&inner, 0x20, entry->name);
                        GamePaths_RemoveTree(&inner);
                        String_Dtor(&inner);
                    } else {
                        Handle_Vfn9(&folder, nullptr, &status, reinterpret_cast<u32>(entry->name));
                    }
                }
            } while (*Handle_Vfn2(&folder, nullptr, &status, reinterpret_cast<u32>(entry)) >= 0);
        }
        Dir_Remove(&status, path->data);
    }
    Handle_Dtor(&folder);
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

u32 __fastcall GamePaths_FindDisc(u8 *check) {
    WString text;
    WString_Ctor(&text);
    WString caption;
    WString_Ctor(&caption);
    vcall<void>(&text, 0x14, Language_Widen(Records_DataIfTerminated(*at<const u8 *const *>(0x0078ba88), nullptr, 9)));
    WString_AssignFromAnsi(&caption, reinterpret_cast<const String *>(0x00f7a7c8));
    using Box = s32(__stdcall *)(void *, const u16 *, const u16 *, u32);
    const Box box = *reinterpret_cast<const Box *>(0x006fd378);   // MessageBoxW (read once, as the original)
    SystemInfo *const info = reinterpret_cast<SystemInfo *>(check + 0x10);
    StringList *const list = reinterpret_cast<StringList *>(check);
    u32 found = 0;
    for (;;) {
        s32 status;
        if (*SystemInfo_ListDrives(info, nullptr, &status, list, 8) >= 0) {
            const char letter = static_cast<char>(SystemInfo_FindVolume(info, nullptr, list, *at<const char *const *>(0x00f7a7d0)));
            String drive;
            String_CtorChar(&drive, nullptr, &letter);
            GamePaths_SetSecondRoot(drive);
            found = 1;
            break;
        }
        if (box(App_GetWindow(), text.data, caption.data, 0x15) == 2)   // (MB_RETRYCANCEL | MB_ICONHAND; IDCANCEL)
            break;
    }
    WString_Dtor(&caption);
    WString_Dtor(&text);
    return found;
}

u32 __cdecl GamePaths_FontsFolder(char *out) {
    const char *const root = *reinterpret_cast<const char *const *>(Obj_Self(at<u8 *>(0x00f816c0)) + 8);
    if (root == nullptr)
        return 0;
    s32 length = 0;
    while (root[length] != 0)
        ++length;
    const char *const format = root[length - 1] == '\\' ? at<const char *>(reinterpret_cast<uintptr_t>(&gPostcardsend_74d320)) : at<const char *>(0x007488ac);   // (empty: root[-1], sic)
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

void GamePaths_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" const char *__cdecl GamePaths_Base() {
    String *const path = at<String *>(0x00788028);
    u8 &guard = *at<u8 *>(0x007887e4);
    if (!(guard & 1)) {
        guard |= 1;
        String_Ctor(path);
        g_calls.Cd(0x0067a7d0, 0x0040eff0u);   // CRT_atexit (the String's destructor)
    }
    vcall<void>(path, 0x18, at<const StringView *>(0x00f816e8));
    String_EnsureTrailingBackslash(path);
    String_EnsureTrailingBackslash(path);
    vcall<void>(path, 0x28, at<const StringView *>(0x00787f50));
    String_EnsureTrailingBackslash(path);
    vcall<void>(path, 0x28, at<const StringView *>(0x00786b68));
    return *at<const char *const *>(0x00788030);
}

s32 __cdecl GamePaths_ShareTexFolder(char *out) {
    const u32 root = g_calls.Cd(0x0040ef40);
    return Crt().sprintf(out, reinterpret_cast<const char *>((uintptr_t)0x007488ac), reinterpret_cast<const char *>((uintptr_t)root), reinterpret_cast<const char *>((uintptr_t)0x007af720));
}

u32 *__cdecl GamePaths_SetRoots(u32 *result) {
    u8 *const paths = at<u8 *>(0x00f816c0);
    String text;
    String_CtorStr(&text, nullptr, Dir_GetBase());
    Obj_CallSlot18(paths, nullptr, static_cast<u32>(reinterpret_cast<std::uintptr_t>(&text)));
    String_Dtor(&text);
    String_CtorCopy(&text, nullptr, at<const StringView *>(0x00f7eaf0));
    Obj_CallSub28Slot18(paths, nullptr, static_cast<u32>(reinterpret_cast<std::uintptr_t>(&text)));
    String_Dtor(&text);
    String_CtorStr(&text, nullptr, at<const char *>(0x00747810));
    Strings5_JoinPath(paths, nullptr, reinterpret_cast<const StringView *>(&text));
    String_Dtor(&text);
    String_CtorCopy(&text, nullptr, at<const StringView *>(0x00f7edf8));
    Strings5_JoinPathAt50(paths, nullptr, reinterpret_cast<const StringView *>(&text));
    String_Dtor(&text);
    *result = 0;
    return result;
}
}  // extern "C"

extern "C" bool __cdecl GamePaths_CheckDisc() {
    u8 *const statics = reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(0x00fb34e0));
    u8 &guard = *reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(0x00fb34ca));
    u32 *const list = reinterpret_cast<u32 *>(statics);
    if (!(guard & 1)) {
        list[1] = list[2] = list[3] = 0;
        guard |= 1;
        list[0] = 0x006fe67c;
        g_calls.Th(0x005f8790, 0x00fb34f0u);   // SystemInfo_Ctor
        g_calls.Cd(0x0067a7d0, 0x0045d810u);   // atexit: the list's destructor
    }
    u32 copy[0xe];
    copy[0] = 0x006fe67c;
    std::memcpy(copy + 1, statics + 4, 0x31);
    const bool found = g_calls.Th(0x005aa500, static_cast<u32>(reinterpret_cast<std::uintptr_t>(copy))) == 1;   // GamePaths_FindDisc
    for (u32 node = copy[1]; node;) {
        const u32 next = *reinterpret_cast<const u32 *>(static_cast<std::uintptr_t>(node + 0x14));
        g_calls.Th(0x005f61e0, node);   // String_Dtor
        g_calls.Cd(0x0067acb0, node);   // Mem_Free
        node = next;
    }
    return found;
}
