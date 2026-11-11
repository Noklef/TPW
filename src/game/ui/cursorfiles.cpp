// The game's cursor files loaded: see cursorfiles.h.
#include "cursorfiles.h"

#include <cstdint>

#include "../files/quickload.h"
#include "cursor.h"
#include "../core/imports.h"
#include "../world/objectdefs.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return GameAddress<T>(address); }

const CursorFilesEnv kGameEnv = {game<s32(__cdecl *)(char *, const char *, ...)>(0x0067a650), &Quickload_Locate, game<void *(__cdecl *)(u32)>(0x0067acc0), reinterpret_cast<u8 *(__fastcall *)(void *)>(&SysCursor_Ctor),
                                 game<u8 *(__fastcall *)(void *)>(0x0047eab0), game<void(__fastcall *)(u8 *, void *, const char *)>(0x0047ec80)};
const CursorFilesEnv *g_env = &kGameEnv;

char *locate(u32 folder, const char *name) {
    char path[0x100];
    g_env->sprintf(path, gText_Objectdefs_7488ac, global<const char *>(folder), name);
    return g_env->locate(path, 1, nullptr);
}

}  // namespace

void CursorFiles_SetEnv(const CursorFilesEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__cdecl CursorFiles_LoadSystem(const char *name) {
    char *const file = locate((u32)(uintptr_t)&gCursorFiles_SystemFolder_74fc30, name);
    if (!file) return nullptr;
    void *const room = g_env->alloc(0x18);
    u8 *const cursor = room ? g_env->systemCtor(room) : nullptr;
    if (!cursor) return nullptr;
    const u32 handle = (*reinterpret_cast<u32(__stdcall *const *)(const char *)>((uintptr_t)IMPORT(User32_LoadCursorFromFileA)))(file);
    if (handle) (*reinterpret_cast<void(__fastcall *const *)(u8 *, void *, u32, u32)>(*reinterpret_cast<u8 **>(cursor) + 4))(cursor, nullptr, handle, 0);
    return cursor;
}

u8 *__cdecl CursorFiles_LoadPicture(const char *name) {
    char *const file = locate((u32)(uintptr_t)&gCursorFiles_PictureFolder_74fc34, name);
    if (!file) return nullptr;
    void *const room = g_env->alloc(0x68);
    u8 *const cursor = room ? g_env->pictureCtor(room) : nullptr;
    if (cursor) g_env->pictureLoad(cursor, nullptr, file);
    return cursor;
}
}
