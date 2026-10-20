#include "texman.h"

#include <cstdint>

#include "texfile.h"
#include "texture.h"
#include "../core/crt.h"
#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 &levels() { return global<s32>(0x0087a264); }

// The game's inline strcmp, only ever tested for 0 (a byte at a time).
bool same(const char *a, const char *b) {
    for (;; ++a, ++b) {
        if (*a != *b)
            return false;
        if (!*a)
            return true;
    }
}

// strcat as the game's inline copy does it (the string's end found, then its bytes with the terminator).
void append(char *to, const char *from) {
    while (*to)
        ++to;
    do
        *to++ = *from;
    while (*from++);
}

}  // namespace

extern "C" {

u32 __cdecl TexManager_Destroy() {
    u8 *const manager = global<u8 *>(0x0087a268);
    if (!manager)
        return 0;
    if (at<s32>(manager, 0x10000) > 0) {
        s32 i = 0;
        do {
            u8 *const slot = manager + 8 + 0x10 * i;
            if (u8 *const textures = at<u8 *>(slot, 0)) {
                // (the original first walks to the last texture chained by 0x800000 and does nothing with it)
                Texture_Release(textures);
                if (u8 *const array = at<u8 *>(slot, 0))
                    Texture_DestroyArray(array);
                at<u32>(slot, 0) = 0;
            }
            at<u32>(slot, 4) = 0;
        } while (++i < at<s32>(manager, 0x10000));
    }
    at<u32>(manager, 0x10000) = 0;
    GameHeap_Free(global<u8 *>(0x0087a268));
    global<u32>(0x0087a268) = 0;
    return 1;
}

u32 __fastcall TexManager_ReleaseTexture(u8 *manager, void *, u8 *textures) {
    s32 i = at<s32>(manager, 0x10000) - 1;
    while (i >= 0 && at<u8 *>(manager, 0x10 * i + 8) != textures)
        --i;
    if (i < 0)
        return 0;
    u8 *const slot = manager + 0x10 * i;
    if (--at<s32>(slot, 0xc) != 0)
        return 0;
    Texture_Release(at<u8 *>(slot, 8));
    if (u8 *const array = at<u8 *>(slot, 8))
        Texture_DestroyArray(array);
    const s32 last = --at<s32>(manager, 0x10000);
    for (u32 offset = 0; offset < 0x10; offset += 4)
        at<u32>(slot, offset) = at<u32>(manager, 0x10 * last + offset);
    return 1;
}

void __fastcall TexManager_SetLoadFlags(u8 *manager, void *, u32 flags) { at<u32>(manager, 0x10008) = flags; }

u8 *__fastcall TexManager_Load(u8 *manager, void *, const char *name) {
    levels() = 0;
    u32 flags = at<u32>(manager, 0x10008);
    at<u32>(manager, 0x10008) = 0;
    if (flags & 2) {
        switch (at<u32>(global<u8 *>(0x00879ab0), 0x24)) {
        case 0:
            flags &= ~2u;
            break;
        case 1:
        case 2:
            levels() = 2;
            break;
        case 3:
            levels() = 1;
            break;
        default:
            break;
        }
    }
    const s32 count = levels() + 1;
    u8 *textures = nullptr;
    if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc((u32)(count * 0x60 + 4)))) {
        at<s32>(block, 0) = count;
        textures = block + 4;
        CRT_VectorVbaseCtorIterator(textures, 0x60, count, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                    reinterpret_cast<void *>((uintptr_t)0x00590a70));
    }
    if (!textures)
        return nullptr;
    u32 width, height;
    u8 *const data = TexFile_Load(name, &width, &height, flags, nullptr, 0);
    if (!data) {
        Texture_DestroyArray(textures);
        return nullptr;
    }
    const u32 state = global<u32>(0x0087a24c);
    u32 format = state & 4 ? 0x11000000 : 0x1000000;
    if (state & 8)
        format |= 0x30400;
    else if (flags & 4)
        format |= 0x8030000;
    else if (state & 2)
        format |= 0x200;
    else if (state & 1)
        format |= 0x100;
    if (flags & 8)
        format |= 0x300;
    if (state & 0x20)
        format |= 0x40000;
    switch (state & 0x300) {
    case 0x100:
        format |= 0x40;
        break;
    case 0x200:
        format |= 0x80;
        break;
    case 0x300:
        format |= 0xc0;
        break;
    default:
        break;
    }
    const s32 level = reinterpret_cast<const s32 *>((uintptr_t)0x007016f8)[global<u32>(0x008bd554)] * (s32)height * (s32)width;
    u8 *pixels = data;
    for (s32 i = 0; i <= levels(); ++i) {
        u8 *const texture = textures + 0x60 * i;
        at<u32>(texture, 8) = (i < levels() ? 0x800000u : 0u) | format;
        at<u8 *>(texture, 0x40) = format & 0x10000000 ? global<u8 *>(0x0087a254) : pixels;
        Texture_SetSize(texture, nullptr, width, height);
        at<u16>(texture, 0x34) = (u16)width;
        pixels += level;
    }
    return textures;
}

u8 *__fastcall TexManager_Acquire(u8 *manager, void *, const char *folder, const char *name) {
    s32 i = at<s32>(manager, 0x10000) - 1;
    for (; i >= 0; --i) {
        const char *const its = at<const char *>(manager, 0x10 * i);
        if (folder != its && (!folder || !its || !same(folder, its)))
            continue;
        if (same(name, at<const char *>(manager, 0x10 * i + 4)))
            break;
    }
    if (i >= 0) {
        ++at<s32>(manager, 0x10 * i + 0xc);
        return at<u8 *>(manager, 0x10 * i + 8);
    }
    if (at<s32>(manager, 0x10000) >= 0x1000)
        return nullptr;
    char path[0x104];
    path[0] = 0;
    append(path, reinterpret_cast<const char *>((uintptr_t)0x00786de4));
    if (folder) {
        append(path, folder);
        s32 n = 0;
        while (path[n])
            ++n;
        if (n > 0 && path[n - 1] != '\\')
            append(path, reinterpret_cast<const char *>((uintptr_t)0x007477fc));
    }
    append(path, name);
    if (u8 *const textures = TexManager_Load(manager, nullptr, path)) {
        at<const char *>(manager, 0x10 * at<s32>(manager, 0x10000)) = folder;
        at<const char *>(manager, 0x10 * at<s32>(manager, 0x10000) + 4) = name;
        at<u8 *>(manager, 0x10 * at<s32>(manager, 0x10000) + 8) = textures;
        at<s32>(manager, 0x10 * at<s32>(manager, 0x10000) + 0xc) = 1;
        ++at<s32>(manager, 0x10000);
        return textures;
    }
    const s32 fallback = at<s32>(manager, 0x10004);
    if (fallback == -1)
        return nullptr;
    ++at<s32>(manager, 0x10 * fallback + 0xc);
    return at<u8 *>(manager, 0x10 * at<s32>(manager, 0x10004) + 8);
}
}
