#include "loading.h"

#include <cstdint>
#include <cstring>

#include "sprite.h"
#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/string.h"
#include "../files/handles.h"
#include "../core/vcall.h"
#include "../gfx/spriteframe.h"
#include "../render/renderer.h"
#include "../world/things.h"
#include "draw.h"
#include "../core/staticobjects.h"

namespace {

const SpriteLoadingEnv kGameEnv = {&NamedList_Find, &SpriteFrames_Free,
                                   reinterpret_cast<u32(__fastcall *)(u8 *, void *, StreamBase *, String *)>((uintptr_t)0x0041e740),
                                   reinterpret_cast<u32(__fastcall *)(u8 *, void *, StreamBase *, u32)>(&SpriteFrames_Load),
                                   &SpriteFrames_MarkTextures};
const SpriteLoadingEnv *g_env = &kGameEnv;
AddrCalls g_calls;

// 0x542420 / 0x542640: the frame sets swapped for the kind (0 ground, 1 park) with its two suffixes.
u8 use_frames(u32 kind, u32 suffixOn, u32 suffixOff) {
    String name, suffix;
    String_Ctor(&name);
    String_Ctor(&suffix);
    u8 *folder = nullptr;
    u8 result = 0;
    if (gLoading_763f80 != kind) {
        const u32 wide = gLoading_763f28 ? suffixOn : suffixOff;
        gLoading_763f80 = kind;
        String_AssignWide(&suffix, nullptr, reinterpret_cast<const WString *>((uintptr_t)wide));
        if (g_env->find(*reinterpret_cast<u8 ***>((uintptr_t)&gNamedObjects_786b68 + 0x34), nullptr, reinterpret_cast<const u8 *>((uintptr_t)&gString_876960),
                        &folder) == 1) {
            u8 **sprites = gSprites;
            for (s32 i = 0; i < (s32)gLoading_876b28; ++i) {
                if (!sprites[i][0x205]) continue;
                String_AssignStr(&name, nullptr, reinterpret_cast<const char *>(sprites[i] + 4));
                String_Append(&name, nullptr, reinterpret_cast<const StringView *>(&suffix));
                u8 **old = reinterpret_cast<u8 **>(sprites[i]);
                g_env->framesFree(*old);
                if (u8 *const was = *old) {
                    g_env->framesFree(was);
                    GameHeap_Free(was);
                }
                *old = nullptr;
                StreamBase *stream = static_cast<StreamBase *>(GameHeap_Alloc(8));
                if (stream) stream = StreamBase_Ctor(stream);
                if (!stream) continue;
                if (g_env->open(folder, nullptr, stream, &name) != 1) continue;  // (the stream is never freed, sic)
                u8 *const sprite = sprites[i];
                u8 *&set = *reinterpret_cast<u8 **>(sprite);
                u8 *const fresh = static_cast<u8 *>(GameHeap_Alloc(0x10));
                if (fresh) {
                    *reinterpret_cast<u32 *>(fresh) = 0;
                    *reinterpret_cast<u32 *>(fresh + 0xc) = 0;
                }
                set = fresh;
                sprite[0x204] = g_env->load(fresh, nullptr, stream, 0) == 1 ? 1 : 0;
                *reinterpret_cast<u32 *>(sprite + 0x208) = *reinterpret_cast<u32 *>(set);
                if (sprites[i][0x207]) g_env->markTextures(*reinterpret_cast<u8 **>(sprites[i]));
            }
            result = 1;
        }
    }
    String_Dtor(&suffix);
    String_Dtor(&name);
    return result;
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using FileEntry = SpriteFileEntry;

FileEntry *entries() { return gSpriteFiles_875970; }
s32 count() { return gSpriteFileCount_875430; }

const char *name(const FileEntry &e) { return *reinterpret_cast<const char *const *>(e.file + 0x20); }

// Whether the name is the first text or (the first's String alive around them) one of the others.
bool name_among(const String *folderName, u32 first, const u32 *others, u32 count) {
    String s;
    String_CtorStr(&s, nullptr, reinterpret_cast<const char *>((uintptr_t)first));
    bool same = Names_Equal(reinterpret_cast<const u8 *>(folderName), reinterpret_cast<const u8 *>(&s)) != 0;
    for (u32 i = 0; i < count && !same; i++) {
        String other;
        String_CtorStr(&other, nullptr, reinterpret_cast<const char *>((uintptr_t)others[i]));
        same = Names_Equal(reinterpret_cast<const u8 *>(folderName), reinterpret_cast<const u8 *>(&other)) != 0;
        String_Dtor(&other);
    }
    String_Dtor(&s);
    return same;
}

// The files of the path found into the table.
void list_files(u8 *folder, String *path, const String *folderName) {
    String_EnsureTrailingBackslash(path);
    vcall<void>(path, 0x28, folderName);
    String_EnsureTrailingBackslash(path);
    vcall<void>(path, 0x28, reinterpret_cast<const String *>((uintptr_t)&gString_8757f0));
    if (g_calls.Th(0x0041d570, (u32)(uintptr_t)folder, path, (u32)&gSpriteFindRecord_875038) != 1) return;
    do {
        const u32 current = Get34_41ce80(folder);
        if (gSpriteFileCount_875430 < 0x80) {
            FileEntry &e = entries()[gSpriteFileCount_875430];
            u8 *const file = static_cast<u8 *>(GameHeap_Alloc(0x128));
            if (file) {
                *reinterpret_cast<u8 **>(file + 0x20) = file + 0x24;
                file[0x24] = 0;
            }
            e.file = file;
            void *const block = GameHeap_Alloc(8);
            e.value = block ? g_calls.Th(0x005f8240, (u32)(uintptr_t)block) : 0;
            std::memcpy(file, gSpriteFindRecord_875038, 0x20);
            std::strcpy(reinterpret_cast<char *>(file + 0x24), reinterpret_cast<const char *>((uintptr_t)0x0087505c));
            *reinterpret_cast<u32 *>(file + 0x20) = (u32)(uintptr_t)file + global<u32>(0x00875058) - 0x0087505c + 0x24;
            g_calls.Th(0x005f82b0, e.value, current);
            gSpriteFileCount_875430++;
        }
    } while (g_calls.Th(0x0041d710, (u32)(uintptr_t)folder, (u32)&gSpriteFindRecord_875038));
}

// One entry's sprite made and loaded; when that fails the sprite in that place freed.
bool load_sprite(const FileEntry &e) {
    u8 **const sprites = gSprites;
    void *const block = GameHeap_Alloc(0x260);
    u8 *const sprite = block ? Sprite_Ctor(static_cast<u8 *>(block)) : nullptr;
    sprites[gLoading_87626c] = sprite;
    if (static_cast<u8>(g_calls.Th(0x00540d90, (u32)(uintptr_t)sprite, e.file, e.value))) return true;
    if (u8 *const now = sprites[gLoading_87626c]) GameHeap_Free(now);
    return false;
}

}  // namespace

void SpriteLoading_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Sprites_LoadFolder(u8 *folder, const String *folderName) {
    String path;
    String_Ctor(&path);
    s32 loaded = 0, avatarLimit = 0, staffLimit = 0;
    bool avatars = false, staff = false;
    gLoading_877354 = 1;
    static constexpr u32 kKidsHeads[] = {0x00764100};
    if (name_among(folderName, (u32)(uintptr_t)&gText_Sprites_LoadFolder_76410c, kKidsHeads, 1)) {
        avatars = true;
        avatarLimit = static_cast<s32>(g_calls.Th(0x0041a9d0, (u32)&gSprites_LoadFolder_78baec));
    }
    static constexpr u32 kStaff[] = {0x007640bc, 0x007640b4, 0x007640a8};
    if (name_among(folderName, 0x007640c8, kStaff, 3)) {
        staff = true;
        staffLimit = static_cast<s32>(g_calls.Th(0x0041aa40, (u32)&gSprites_LoadFolder_78baec));
    }
    gSpriteFileCount_875430 = 0;
    for (u32 i = 0; i < 0x80; i++) {
        entries()[i].file = nullptr;
        entries()[i].value = 0;
        entries()[i].flag = 0;
    }
    char *const buffer = gSprites_LoadFolder_876668;
    const char *const backslash = reinterpret_cast<const char *>((uintptr_t)0x007477fc);
    std::strcpy(buffer, gSprites_LoadFolder_764168);
    std::strcat(buffer, folderName->data);
    std::strcat(buffer, backslash);
    vcall<void>(&path, 0x14, reinterpret_cast<const char *>((uintptr_t)&gText_Sprites_LoadFolder_764160));
    list_files(folder, &path, folderName);
    const String *const root = reinterpret_cast<const String *>((uintptr_t)&gNamedObjects_786b68);   // (a wide string: Obj_Self; its data read as chars, sic)
    std::strcpy(buffer, root->data);
    std::strcat(buffer, backslash);
    std::strcat(buffer, folderName->data);
    std::strcat(buffer, backslash);
    vcall<void>(&path, 0x18, root);
    list_files(folder, &path, folderName);
    SpriteFiles_Sort();
    if (avatars) {
        for (u32 at = (u32)(uintptr_t)&gAvatarSprites; at < 0x00764040; at += 4) {
            for (s32 i = 0; i < gSpriteFileCount_875430; i++) {
                FileEntry &e = entries()[i];
                String wanted, have;
                String_CtorStr(&wanted, nullptr, *reinterpret_cast<const char *const *>((uintptr_t)at));
                String_CtorStr(&have, nullptr, name(e));
                const u32 same = Names_Equal(reinterpret_cast<const u8 *>(&have), reinterpret_cast<const u8 *>(&wanted));
                String_Dtor(&have);
                String_Dtor(&wanted);
                if (!same || e.flag || !load_sprite(e)) continue;
                loaded++;
                e.flag = 1;
                if (gLoading_87626c >= 0x200 || loaded >= avatarLimit) goto done;
            }
        }
    }
    for (s32 i = 0; i < gSpriteFileCount_875430; i++) {
        FileEntry &e = entries()[i];
        if (e.flag || !load_sprite(e)) continue;
        loaded++;
        e.flag = 1;
        if (gLoading_87626c >= 0x200 || (avatars && loaded >= avatarLimit) || (staff && loaded >= staffLimit)) break;
    }
done:
    for (s32 i = 0; i < gSpriteFileCount_875430; i++) {
        FileEntry &e = entries()[i];
        if (e.file) GameHeap_Free(e.file);
        if (e.value) vcall<void>(reinterpret_cast<u8 *>((uintptr_t)e.value), 0x2c, 1u);
        e.file = nullptr;
    }
    String_Dtor(&path);
    return static_cast<u32>(loaded);
}

void __cdecl SpriteFiles_Sort() {
    u32 swaps;
    do {
        swaps = 0;
        for (s32 i = 1; i < count(); i++) {
            FileEntry &cur = entries()[i], &prev = entries()[i - 1];
            String a, b;
            String_CtorStr(&a, nullptr, name(cur));
            String_CtorStr(&b, nullptr, name(prev));
            const u32 after = Names_FirstAfter(reinterpret_cast<const u8 *>(&b), reinterpret_cast<const u8 *>(&a));
            String_Dtor(&b);
            String_Dtor(&a);
            if (after) {
                const FileEntry was = cur;
                cur.file = prev.file;
                cur.value = prev.value;
                cur.flag = prev.flag;
                prev.file = was.file;
                prev.value = was.value;
                prev.flag = was.flag;
                swaps++;
            }
        }
    } while (swaps);
}

void __cdecl Sprites_FreeAll() {
    u8 **sprites = gSprites;
    for (s32 i = 0; i < gLoading_87626c; ++i) {
        u8 *sprite = sprites[i];
        if (sprite[0x204]) {
            SpriteFrames_Free(*reinterpret_cast<u8 **>(sprite));
            u8 *frames = *reinterpret_cast<u8 **>(sprite);
            if (frames) {
                SpriteFrames_Free(frames);
                GameHeap_Free(frames);
            }
            *reinterpret_cast<u8 **>(sprite) = nullptr;
        }
        if (sprites[i])
            GameHeap_Free(sprites[i]);
    }
    for (u32 k = 0; k < 16; ++k)
        TrackedArray_Free(reinterpret_cast<u8 *>((uintptr_t)(0x008751f0 + 0x24 * k)));
    TrackedArray_Free(reinterpret_cast<u8 *>((uintptr_t)&gLiveObject6ff308_876420));
}

u8 __cdecl Sprites_UseGroundFrames() { return use_frames(0, (u32)(uintptr_t)&gString_875fe8, (u32)(uintptr_t)&gString_875598); }

u8 __cdecl Sprites_UseParkFrames() { return use_frames(1, (u32)(uintptr_t)&gString_876270, (u32)(uintptr_t)&gString_8757a8); }
}

void SpriteLoading_SetEnv(const SpriteLoadingEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" bool __cdecl Sprites_LoadAll(u8 onlyLast) {
    u32 folder = 0;
    gLoading_87626c = 0;
    if (g_calls.Th(0x00421770, global<u32>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x34), (u32)(uintptr_t)&gString_876960, &folder) != 1)   // NamedList_Find
        return false;
    for (SpriteFolder *entry = &gSpriteFolders[onlyLast ? 13 : 0]; entry < gSpriteFolders + 14; ++entry) {
        entry->first = (u32)gLoading_87626c;
        String name;
        String_CtorStr(&name, nullptr, entry->name);
        entry->count = (s32)g_calls.Cd(0x00541310, folder, &name);   // Sprites_LoadFolder
        String_Dtor(&name);
    }
    gLoading_876b28 = (u32)gLoading_87626c;
    return true;
}

extern "C" bool __cdecl Sprites_Setup(u32 mode, u32 onlyLast) {
    const u32 map = global<u32>(global<u32>(0x007a0854) + 0x6c);
    global<u32>((u32)(uintptr_t)&gView_8bcbc8) |= 0x1000000;
    gSpriteDrawOn = 1;
    gThings_877350 = 0;
    reinterpret_cast<u32 &>(gSpriteHeading) = 0;
    reinterpret_cast<u32 &>(global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x928)) = 0;
    reinterpret_cast<u32 &>(global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x92c)) = 0;
    gLoading_8768fc = mode & 0xff;
    gLoading_763f80 = (mode & 0xff) == 0;
    g_calls.Cd(0x00541d90, onlyLast);
    gDraw_87734c = 0;
    const s32 width = global<s32>(map + 0x18), height = global<s32>(map + 0x1c);
    gSprites_Setup_876898 = width;
    gSprites_Setup_876894 = height;
    const u32 across = (u32)(width / 4 + 1), down = (u32)(height / 4 + 1);
    reinterpret_cast<u32 &>(gSpriteCellWidth) = across;
    reinterpret_cast<u32 &>(gSpriteCellDepth) = down;
    for (u32 y = 0; y < 4; ++y) {
        for (u32 x = 0; x < 4; ++x) {
            const u32 index = x + 4 * y, cell = 0x008751f0 + index * 0x24;
            gSpriteDrawLists[index] = reinterpret_cast<u32 *>((uintptr_t)g_calls.Th(0x0058a4f0, cell, 0x800u));
            global<u32>(cell + 4) = 0;
            const u32 right = y == 3 ? (u32)width - 1 : y * across + across - 1;
            const u32 bottom = x == 3 ? (u32)height - 1 : x * down + down - 1;
            g_calls.Th(0x00576620, reinterpret_cast<u32 &>(gRenderer_87a6d0), y * across, x * down, right, bottom, cell + global<u32>(global<u32>(cell) + 4), 0u);
        }
    }
    reinterpret_cast<u32 &>(gSpriteDrawOverlay) = g_calls.Th(0x0058a4f0, (u32)(uintptr_t)&gLiveObject6ff308_876420, 0x20u);
    global<u32>((u32)(uintptr_t)&gLiveObject6ff308_876420 + 0x4) = 0;
    g_calls.Th(0x00576410, reinterpret_cast<u32 &>(gRenderer_87a6d0), (u32)(uintptr_t)&gLiveObject6ff308_876420 + global<u32>(global<u32>((u32)(uintptr_t)&gLiveObject6ff308_876420) + 4), 0x20u);
    return true;
}

void __cdecl SpriteFolderName_AtExit() {}
