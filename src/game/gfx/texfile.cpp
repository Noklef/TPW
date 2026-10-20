#include "texfile.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../core/vregs.h"
#include "../core/vwrap.h"
#include "../files/vfs.h"
#include "../render/texformat.h"
#include "color.h"
#include "wctimage.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

u32 &state() { return global<u32>(0x0087a24c); }
s32 levels() { return global<s32>(0x0087a264); }

// The TGA header (0x12 bytes read; the dword at +0x10 is read whole by the game, only its two low bytes used).
struct Header {
    u8 bytes[0x14];
    s32 width() const { return *reinterpret_cast<const s16 *>(bytes + 0xc); }
    s32 height() const { return *reinterpret_cast<const s16 *>(bytes + 0xe); }
    u32 bits() const { return bytes[0x10]; }
    u32 descriptor() const { return bytes[0x11]; }
};

u8 *open(const char *name) { return reinterpret_cast<u8 *>(VFS_Open(name)); }

// The rows swapped as the game's inline assembly does: `count` dwords (LOOP: 0 is 2^32).
void swap_dwords(u32 *a, u32 *b, u32 count) {
    do {
        const u32 t = *a;
        *a++ = *b;
        *b++ = t;
    } while (--count);
}

}  // namespace

extern "C" {

u8 *__cdecl TexFile_Load(const char *name, u32 *width, u32 *height, u32 flags, const u32 *table, s32 pixelSize) {
    const u32 *const formats =
        table ? table : reinterpret_cast<const u32 *>(TexFormat_Table(reinterpret_cast<u8 *>((uintptr_t)0x008bd550)));
    state() = 0;
    Header header;
    if (flags & 1) {
        u8 *data = nullptr;
        if (u8 *const stream = open(name)) {
            Object_CallSlot8Product((u32)(uintptr_t)header.bytes, 0x12, 1, stream);
            const s32 pixels = header.width() * header.height();
            Stream_SeekFailed(stream, (u32)((s32)(pixels * header.bits()) >> 3), 1);
            data = reinterpret_cast<u8 *>((uintptr_t)TrackedGlobal_Alloc((u32)pixels));
            if (data) Object_CallSlot8Product((u32)(uintptr_t)data, (u32)pixels, 1, stream);
            Object_FinishAndDelete(stream);
        }
        if (!data) return nullptr;
        if (header.descriptor() & 0x20) {
            const s32 step = (s32)(header.width() * header.bits()) >> 5;
            for (s32 i = 0; i < header.height(); ++i)
                swap_dwords(reinterpret_cast<u32 *>(data + i * step), reinterpret_cast<u32 *>(data + (header.height() - i - 1) * step),
                            (u32)step);
        }
        global<u32>(0x0087a25c) = 0xffffffff;
        state() = 0;
        return data;
    }
    char path[0x104];
    {
        u32 n = 0;
        while (name[n]) ++n;
        for (u32 k = 0; k <= n; ++k) path[k] = name[k];
    }
    char *first = path;
    while (*first == '.') ++first;
    char *ext = first;
    while (*ext) ++ext;
    if (*ext == '.') {
        ++ext;
    } else {
        while (ext > first)
            if (*--ext == '.') break;
        ext = *ext == '.' ? ext + 1 : nullptr;
    }
    if (*reinterpret_cast<volatile char *>(ext)) {
        for (const char *wct = "WCT"; *wct;) {
            *ext = *wct++;
            if (!*++ext) break;
        }
    }
    void *const block = GameHeap_Alloc(0x30);
    u8 *const image = block ? Obj_CtorVirtual564ba0(static_cast<u8 *>(block), nullptr, 1) : nullptr;
    if (WctImage_Load(image, nullptr, path)) {
        state() = 0x24;
        if (at<u32>(image, 0x20)) {
            state() = 0x25;
            if (at<u32>(image, 0x20) && at<u8>(image, 5)) state() |= 2;
        }
        *width = at<u16>(image, 8);
        *height = at<u16>(image, 0xa);
        state() |= (u32)at<u8>(image, 7) << 8;
        global<u32>(0x0087a254) = image ? (u32)(uintptr_t)(image + at<s32>(at<u8 *>(image, 0), 4)) : 0;
        return image;
    }
    if (image) vcall<void>(image + at<s32>(at<u8 *>(image, 0), 4), 0, 1u);
    u8 *data = nullptr;
    if (u8 *const stream = open(name)) {
        Object_CallSlot14(stream);
        Object_CallSlot8Product((u32)(uintptr_t)header.bytes, 0x12, 1, stream);
        const s32 size = (s32)(header.bits() * header.width() * header.height()) >> 3;
        data = reinterpret_cast<u8 *>((uintptr_t)TrackedGlobal_Alloc((u32)((levels() + 1) * size)));
        if (data) Object_CallSlot8Product((u32)(uintptr_t)data, (u32)size, 1, stream);
        Object_FinishAndDelete(stream);
    }
    if (!data) return nullptr;
    const s32 pixel = pixelSize ? pixelSize : reinterpret_cast<const s32 *>((uintptr_t)0x007016f8)[global<u32>(0x008bd554)];
    *width = (u32)header.width();
    *height = (u32)header.height();
    const u32 flip = header.descriptor() >> 5 & 1;
    if (flags & 2) {
        const s32 bytes = ((s32)(header.bits() * header.width()) >> 3) * header.height();
        s32 n = header.height() * header.width();
        const s32 kind = at<s32>(global<u8 *>(0x00879ab0), 0x24);
        if (kind == 3) {
            if (n > 0) {
                u8 *from = data + 3, *to = data + bytes;
                do {
                    const u8 alpha = *from;
                    to[3] = alpha;
                    to[2] = alpha;
                    to[1] = alpha;
                    to[0] = alpha;
                    from += 4;
                    to += 4;
                } while (--n);
            }
        } else if (kind == 1) {
            if (n > 0) {
                u8 *from = data + 3, *half = data + bytes, *rest = data + 2 * bytes;
                do {
                    const u8 alpha = static_cast<u8>(*from >> 1);
                    half[3] = alpha;
                    half[2] = alpha;
                    half[1] = alpha;
                    half[0] = alpha;
                    const u8 other = static_cast<u8>(static_cast<u8>(~*from) >> 1);
                    rest[3] = other;
                    rest[2] = other;
                    rest[1] = other;
                    rest[0] = other;
                    from += 4;
                    half += 4;
                    rest += 4;
                } while (--n);
            }
        }
        state() |= 8;
    }
    const s32 pixels = header.width() * header.height();
    const s32 fromLevel = (s32)(header.bits() * pixels) >> 3;
    const s32 toLevel = pixel * pixels;
    state() |= 0x300;
    u8 *const out = reinterpret_cast<u8 *>((uintptr_t)TrackedGlobal_Alloc((u32)((levels() + 1) * toLevel + 4)));
    if (!out) {
        TrackedGlobal_Free((u32)(uintptr_t)data);
        return nullptr;
    }
    const u8 *from = data;
    u8 *to = out;
    for (s32 i = 0; i <= levels(); ++i) {
        state() |= Format_ConvertPixels(to, from, header.width(), header.height(), formats, pixel, (s32)(header.bits() >> 3), flip);
        from += fromLevel;
        to += toLevel;
    }
    TrackedGlobal_Free((u32)(uintptr_t)data);
    return out;
}
}
