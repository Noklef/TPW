// An object's script loaded: see objectscript.h.
#include "objectscript.h"

#include <cstdint>

#include "../files/quickload.h"
#include "objectdefs.h"
#include "objectscript.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

u8 *const kDefs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);

const ObjectScriptEnv kGameEnv = {&ObjectDefs_FindAt390, &ObjectDefs_Find, game<s32(__cdecl *)(char *, const char *, ...)>(0x0067a650), &Quickload_Locate,
                                  game<u16 *(__fastcall *)(const u8 *, void *, u16 *)>(0x0050b350), game<u32(__cdecl *)(const char *, u32, u32)>(0x005587f0), game<void(__cdecl *)(u32, u32)>(0x004685c0)};
const ObjectScriptEnv *g_env = &kGameEnv;

}  // namespace

void ObjectScript_SetEnv(const ObjectScriptEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __fastcall Object_LoadScript(u8 *object) {
    char path[0x100];
    const u8 *const name = g_env->findAt390(kDefs, nullptr, at<u16>(object, 0xe));
    const u8 *const definition = g_env->find(kDefs, nullptr, at<u16>(object, 0xe));
    g_env->sprintf(path, gText_Object_LoadScript_75b66c, definition + 0x28c, name, gText_Object_LoadScript_700540);
    char *file = g_env->locate(path, 1, nullptr);
    if (!file) {
        g_env->find(kDefs, nullptr, at<u16>(object, 0xe));   // (for the Debug_Printf left out)
        file = g_env->locate(gText_Object_LoadScript_75b61c, 1, nullptr);
    }
    const u32 quiet = at<u32>(reinterpret_cast<void *>((uintptr_t)0x00785a2c), 0) ? 1 : 0;
    u16 handle;
    const u16 id = *g_env->handle(object, nullptr, &handle);
    const u32 script = g_env->loadScript(file, id, quiet);
    at<u32>(object, 0x24) = script;
    g_env->setRecord(at<u32>(object, 0x20), script);
}
