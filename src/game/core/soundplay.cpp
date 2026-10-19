#include "soundplay.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

struct PlayCommand {
    u32 vtable;
    u32 category, sound;
    float x, y, z, range;
};

u32 post(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z, float range) {
    PlayCommand cmd;
    cmd.vtable = 0x00700b90;
    cmd.category = category;
    cmd.sound = sound;
    cmd.x = (float)x;   // fild; fstp
    cmd.y = (float)y;
    cmd.z = (float)z;
    cmd.range = range;
    return vcall<u32>(global<void *>(0x00802bcc), 8, owner, &cmd);
}

bool started() { return global<u32>(0x00802bc8) && global<u32>(0x00802bd4); }

}  // namespace

extern "C" {

u32 __cdecl Sound_PlayAt(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z) {
    if (!started() || !sound) return 0;
    return post(owner, category, sound, x, y, z, 0.0f);
}

u32 __cdecl Sound_PlayAtRange(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z, s32 range) {
    if (!started() || !sound) return 0;
    return post(owner, category, sound, x, y, z, (float)range);
}

u32 __cdecl SoundUi_Play(u32 sound) { return Sound_PlayAt(0, global<u32>(0x00803a2c), sound, 0, 0, 0); }

u32 __cdecl SoundUi_Play1d() { return Sound_PlayAt(0, global<u32>(0x00803a2c), 0x1d, 0, 0, 0); }

u32 __cdecl KidSound_Play(s32 level, s32 x, s32 y, s32 z) {
    if (level == 0) return 0;
    const u32 sound = level == 1 ? 0x69 : level < 4 ? 0x6a : level < 8 ? 0x6c : 0x6d;
    return Sound_PlayAt(0, global<u32>(0x00803a24), sound, x, y, z);
}

u32 __cdecl KidSound_PlayGraded(s32 level, s32 a, s32 b, s32 x, s32 y, s32 z) {
    s32 grade = (s32)((u32)a + (u32)b) / 50;
    if (grade > 3) grade = 3;
    if (level == 0 || grade < 0) return 0;
    const u32 base = level == 1 ? 0x4b : level < 4 ? 0x4f : level < 8 ? 0x53 : 0x57;
    return Sound_PlayAt(0, global<u32>(0x00803a24), base + (u32)grade, x, y, z);
}

}
