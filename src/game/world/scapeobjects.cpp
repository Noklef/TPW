// A scape's objects: see scapeobjects.h.
#include "scapeobjects.h"

#include <cstdint>

#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../files/file.h"
#include "../render/particles.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ScapeObjectsEnv kGameEnv = {File_Open, Sound_PlayAt, Sound_PlayAtRange, Sound_SetArea, Particle_Spawn};
const ScapeObjectsEnv *g_env = &kGameEnv;

}  // namespace

void ScapeObjects_SetEnv(const ScapeObjectsEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __cdecl Scape_LoadObjects(const char *path) {
    s32 result;
    Stream *stream;
    g_env->open(&result, &stream, path, 0x80000000);
    if (!stream) return;
    u32 word;
    Stream_Read(&result, stream, &word, 4, nullptr);
    if (word != 0x5f4a424f) return;   // "OBJ_"
    s32 count;
    u32 size;
    Stream_Read(&result, stream, &count, 4, nullptr);
    Stream_Read(&result, stream, &size, 4, nullptr);
    for (s32 i = 0; i < count; ++i) {
        s32 record[0xf] = {};
        if (size > 0x3c) {
            Stream_Read(&result, stream, record, 0x3c, nullptr);
            Stream_Seek(stream, static_cast<s32>(size - 0x3c), 1);
        } else {
            Stream_Read(&result, stream, record, static_cast<s32>(size), nullptr);
        }
        const u32 category = record[1] ? gSoundCategories.themeAmbient : gSoundCategories.ambient;
        const u32 sound = static_cast<u32>(record[2]);
        const s32 x = record[3] / 1024, y = record[4] / 1024, z = record[5] / 1024;
        if (record[0] == 1) {
            g_env->playAtRange(0, category, sound, x, y, z, static_cast<s32>(static_cast<u32>(record[6]) * 2) / 1024);
        } else if (record[0] == 2) {
            g_env->spawn(record[1], record[3], record[4], record[5]);
        } else if (record[0] == 3) {
            g_env->setArea(g_env->playAt(0, category, sound, x, y, z), x, z, record[7] / 1024, record[8] / 1024);
        }
    }
    Stream_Close(&result, stream);
}

}  // extern "C"
