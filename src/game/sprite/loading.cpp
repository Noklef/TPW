#include "loading.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/string.h"
#include "../files/handles.h"
#include "../gfx/spriteframe.h"

namespace {

const SpriteLoadingEnv kGameEnv = {&NamedList_Find, &SpriteFrames_Free,
                                   reinterpret_cast<u32(__fastcall *)(u8 *, void *, StreamBase *, String *)>((uintptr_t)0x0041e740),
                                   reinterpret_cast<u32(__fastcall *)(u8 *, void *, StreamBase *, u32)>((uintptr_t)0x00587db0),
                                   &SpriteFrames_MarkTextures};
const SpriteLoadingEnv *g_env = &kGameEnv;

// 0x542420 / 0x542640: the frame sets swapped for the kind (0 ground, 1 park) with its two suffixes.
u8 use_frames(u32 kind, u32 suffixOn, u32 suffixOff) {
    String name, suffix;
    String_Ctor(&name);
    String_Ctor(&suffix);
    u8 *folder = nullptr;
    u8 result = 0;
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00763f80) != kind) {
        const u32 wide = *reinterpret_cast<u8 *>((uintptr_t)0x00763f28) ? suffixOn : suffixOff;
        *reinterpret_cast<u32 *>((uintptr_t)0x00763f80) = kind;
        String_AssignWide(&suffix, nullptr, reinterpret_cast<const WString *>((uintptr_t)wide));
        if (g_env->find(*reinterpret_cast<u8 ***>((uintptr_t)0x00786b9c), nullptr, reinterpret_cast<const u8 *>((uintptr_t)0x00876960),
                        &folder) == 1) {
            u8 **sprites = reinterpret_cast<u8 **>((uintptr_t)0x00876b48);
            for (s32 i = 0; i < *reinterpret_cast<s32 *>((uintptr_t)0x00876b28); ++i) {
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

struct FileEntry {
    const u8 *file;  // the name at +0x20
    u32 value;
    u8 flag;
    u8 pad[3];
};
static_assert(sizeof(FileEntry) == 12, "the table's entries are 12 bytes");

FileEntry *entries() { return reinterpret_cast<FileEntry *>((uintptr_t)0x00875970); }
s32 count() { return global<s32>(0x00875430); }

const char *name(const FileEntry &e) { return *reinterpret_cast<const char *const *>(e.file + 0x20); }

}  // namespace

extern "C" {

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
    u8 **sprites = reinterpret_cast<u8 **>((uintptr_t)0x00876b48);
    for (s32 i = 0; i < global<s32>(0x0087626c); ++i) {
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
    TrackedArray_Free(reinterpret_cast<u8 *>((uintptr_t)0x00876420));
}

u8 __cdecl Sprites_UseGroundFrames() { return use_frames(0, 0x00875fe8, 0x00875598); }

u8 __cdecl Sprites_UseParkFrames() { return use_frames(1, 0x00876270, 0x008757a8); }
}

void SpriteLoading_SetEnv(const SpriteLoadingEnv *env) { g_env = env ? env : &kGameEnv; }
