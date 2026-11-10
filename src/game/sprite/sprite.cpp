// Sprite definitions: reimplementation of 0x540bd0, 0x541f70-0x541fd0 and 0x5423a0.
#include "sprite.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/handles.h"
#include "../gfx/spriteframe.h"
#include "../world/random.h"
#include "loading.h"
#include "draw.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

u32 folderFirst(u32 folder) { return gSpriteFolders[folder].first; }
u32 folderCount(u32 folder) { return (u32)gSpriteFolders[folder].count; }
u8 *sprite(u32 index) { return gSprites[index]; }
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

extern "C" u32 __fastcall Sprite_LoadEsp(u8 *s, void *, const u8 *entry, u8 *files) {
    const auto read = [](StreamBase *stream, void *buffer, u32 count) { return vcall<u32>(stream, 8, buffer, count); };
    const auto open = [files](s32 *result, StreamBase *stream, const char *path) { return *vcall<s32 *>(files, 0xc, result, stream, path, 0x80000001u, 0u) >= 0; };
    const StringView *const suffix = reinterpret_cast<const StringView *>(0x008757a8);
    String path;
    const char *const name = reinterpret_cast<const char *>(entry + 0x24);
    String_CtorStr(&path, nullptr, name);
    u32 answer = 0;
    void *const block = GameHeap_Alloc(8);
    StreamBase *const stream = block ? StreamBase_Ctor(static_cast<StreamBase *>(block)) : nullptr;
    s32 result;
    if (stream && open(&result, stream, name)) {
        u8 skipped[0x100];
        read(stream, skipped, 8);
        read(stream, skipped, 4);
        read(stream, skipped, 0x100);
        read(stream, s + 0x205, 1);
        read(stream, s + 0x206, 1);
        {
            StringView view;
            String_Left(&path, nullptr, &view, path.length - suffix->length);
            String stem;
            String_CtorCopy(&stem, nullptr, &view);
            char *to = reinterpret_cast<char *>(s + 4);
            const char *from = stem.data;
            while ((*to++ = *from++) != 0) {}
            String_Dtor(&stem);
        }
        at<u16>(s, 0x21e) = 0;
        for (u32 i = 0; i < 16; i++) {
            u8 *const animation = s + 0x222 + i * 4;
            read(stream, animation - 2, 2);
            read(stream, animation, 1);
            read(stream, animation + 1, 1);
            if (*animation)
                at<u16>(s, 0x21e)++;
        }
        at<u8>(s, 0x216) = 0;
        at<u8>(s, 0x21a) = 0;
        at<u8>(s, 0x20e) = 0;
        at<u8>(s, 0x212) = 0;
        at<u16>(s, 0x20c) = 0;
        if (read(stream, s + 0x20e, 1)) {
            for (u32 field = 0x20f; field <= 0x21d; field++)
                read(stream, s + field, 1);
        }
        for (u32 field = 0x20e; field <= 0x21a; field += 4) {
            if (at<u8>(s, field))
                at<u16>(s, 0x20c)++;
        }
        vcall<void>(stream, 4);
        {
            StringView view;
            String_Left(&path, nullptr, &view, path.length - (*reinterpret_cast<u32 *>((uintptr_t)0x008757ac)));
            String_Assign(&path, nullptr, &view);
        }
        const bool first = gLoading_8768fc && at<u8>(s, 0x205);
        const u32 from = gLoading_763f28 ? (first ? 0x00875fe8 : 0x00876270) : (first ? 0x00875598 : 0x008757a8);
        String_Append(&path, nullptr, reinterpret_cast<const StringView *>((uintptr_t)from));
        if (open(&result, stream, path.data)) {
            u32 *const set = static_cast<u32 *>(GameHeap_Alloc(0x10));
            if (set) {
                set[0] = 0;
                set[3] = 0;
            }
            at<u32 *>(s, 0) = set;
            at<u8>(s, 0x204) = SpriteFrames_Load(reinterpret_cast<u8 *>(set), nullptr, reinterpret_cast<u8 *>(stream), 0) == 1 ? 1 : 0;
            at<u32>(s, 0x208) = set[0];
            const u8 mark = gLoading_877354;
            at<u8>(s, 0x207) = mark;
            if (mark)
                SpriteFrames_MarkTextures(reinterpret_cast<u8 *>(set));
            ++*reinterpret_cast<u32 *>(&gLoading_87626c);
            answer = 1;
        }
    }
    String_Dtor(&path);
    return answer;
}

extern "C" u32 __cdecl Sprite_FrameEntry(s32 anim, u32 frame, u32 dir) {
    u8 *const s = sprite(folderFirst(13) + (u32)(anim / 16));
    const s32 slot = anim % 16;
    const u32 first = *reinterpret_cast<const u16 *>(s + 0x220 + 4 * slot);
    const u32 d = s[0x223 + 4 * slot] ? dir & 7 : 0;
    const u32 index = s[0x222 + 4 * slot] * d + first + frame;
    return *reinterpret_cast<const u32 *>(at<u32>(s, 0) + 4) + index * 24;
}
