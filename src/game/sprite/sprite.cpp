// Sprite definitions: reimplementation of 0x540bd0, 0x541f70-0x541fd0 and 0x5423a0.
#include "sprite.h"

#include "../world/random.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

u32 folderFirst(u32 folder) { return *reinterpret_cast<const u32 *>(0x00763f8c + 12u * folder); }
u32 folderCount(u32 folder) { return *reinterpret_cast<const u32 *>(0x00763f90 + 12u * folder); }
u8 *sprite(u32 index) { return reinterpret_cast<u8 **>(0x00876b48)[index]; }  // gSprites
u8 *world() { return *reinterpret_cast<u8 **>(0x007cf83c); }                  // gWorldPtr

}  // namespace

extern "C" u8 *__fastcall Sprite_Ctor(u8 *s) {
    at<u8>(s, 0x204) = 0;
    at<u32>(s, 0) = 0;
    at<u32>(s, 0x208) = 0;
    at<u16>(s, 0x20c) = 0;
    at<u16>(s, 0x21e) = 0;
    at<u8>(s, 0x207) = 1;
    for (u32 a = 0; a < 4; ++a) {  // the special actions' entry, animation and first byte
        at<u8>(s, 0x211 + 4 * a) = 0;
        at<u8>(s, 0x210 + 4 * a) = 0;
        at<u8>(s, 0x20e + 4 * a) = 0;
    }
    for (u32 n = 0; n < 16; ++n) {
        at<u16>(s, 0x220 + 4 * n) = 0;
        at<u8>(s, 0x222 + 4 * n) = 0;
        at<u8>(s, 0x223 + 4 * n) = 0;
    }
    return s;
}

extern "C" u32 __cdecl SpriteFolder_RandomSprite(u32 folder) {
    const u32 count = folderCount(folder);
    return ((u32)World_Rand(world()) >> 2) % count;
}

extern "C" u32 __cdecl Sprite_ActionCount(u32 folder, u32 index) {
    return at<u16>(sprite(folderFirst(folder) + index), 0x20c);
}

extern "C" u32 __cdecl Sprite_RandomAnim(u32 folder, u32 index) {
    const u32 count = at<u16>(sprite(folderFirst(folder) + index), 0x21e);
    return ((u32)World_Rand(world()) >> 2) % count;
}

extern "C" u32 __cdecl Sprite_FrameEntry(s32 anim, u32 frame, u32 dir) {
    u8 *const s = sprite(folderFirst(13) + (u32)(anim / 16));
    const s32 slot = anim % 16;
    const u32 first = *reinterpret_cast<const u16 *>(s + 0x220 + 4 * slot);
    const u32 d = s[0x223 + 4 * slot] ? dir & 7 : 0;
    const u32 index = s[0x222 + 4 * slot] * d + first + frame;
    return *reinterpret_cast<const u32 *>(at<u32>(s, 0) + 4) + index * 24;
}
