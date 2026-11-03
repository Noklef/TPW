// A scape's objects (0x550e00): Scape.omp's "OBJ_" records, each a point sound, a particle effect or an area sound.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x550e00 (cdecl (path); called at 0x407f82): nothing when the file does not open or its first dword is not "OBJ_". Then the record count, the record size and the records: up to 0x3c bytes of each read over zeros, any more
// passed over. +0 the kind: 1 Sound_PlayAtRange(0, category, +8, +0xc / 1024, +0x10 / 1024, +0x14 / 1024, +0x18 * 2 / 1024); 2 Particle_Spawn(+4, +0xc, +0x10, +0x14), not scaled; 3 Sound_PlayAt with the same first six, then
// Sound_SetArea of its answer with +0xc, +0x14, +0x1c and +0x20 each / 1024. The category is +4's: not 0 [0x803a38], 0 [0x803a20]. The divisions truncate toward 0. The stream is closed at the end.
void __cdecl Scape_LoadObjects(const char *path);
}

struct Stream;

// What it calls (defaults the game's): the driver records these.
struct ScapeObjectsEnv {
    s32 *(__cdecl *open)(s32 *result, Stream **out, const char *path, u32 mode);  // File_Open (0x5f5a20)
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // 0x51bfc0
    u32(__cdecl *playAtRange)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z, s32 range);  // 0x51c130
    void(__cdecl *setArea)(u32 sound, u32 x, u32 z, u32 w, u32 d);  // 0x51c5d0
    s32(__cdecl *spawn)(s32 effect, s32 x, s32 y, s32 z);  // 0x521e60
};
void ScapeObjects_SetEnv(const ScapeObjectsEnv *e);
