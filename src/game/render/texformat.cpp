#include "texformat.h"

#include <cstdint>

#include "../gfx/texture.h"
#include "../math/bits.h"
#include "engineerrors.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall TexFormat_Record(void *, void *, u32 *out, s32 index) {
    if (index < 4) {
        u8 *const record = reinterpret_cast<u8 *>((uintptr_t)(0x008bd558 + (u32)index * 0x20));
        if (*reinterpret_cast<const u32 *>(record)) {
            *out = (u32)(uintptr_t)record;
            return 1;
        }
    }
    EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754003);
    return 0;
}

void __fastcall TexFormat_FillTables(u8 *settings) {
    for (u32 k = 0; k < 4; k++) {
        const u8 *format = settings + 0x20 * k;
        const u32 masks[4] = {at<u32>(format, 0x24), at<u32>(format, 0x18), at<u32>(format, 0x1c), at<u32>(format, 0x20)};
        u32 *scaled = reinterpret_cast<u32 *>(settings + 0x4088 + 0x10 * k);
        for (u32 c = 0; c < 4; c++)
            scaled[c] = (u32)(((unsigned long long)(masks[c] << 16) * 0x80808081u) >> 39);
        if (at<u32>(format, 8) != 0x20)
            continue;
        u32 width[4], shift[4];
        for (u32 c = 0; c < 4; c++) {
            width[c] = Bits_Count(masks[c]);
            shift[c] = Bits_Shift(masks[c]);
        }
        u32 *table = reinterpret_cast<u32 *>(settings + 0x88 + 0x1000 * k);
        for (u32 i = 0; i < 0x100; i++)
            for (u32 c = 0; c < 4; c++)
                table[4 * i + c] = (i >> ((8 - width[c]) & 31)) << (shift[c] & 31);
    }
}

s32 __stdcall TexFormat_Classify(const u32 *pixels) {
    u32 mask, class0, class1, class2;
    if (*reinterpret_cast<u32 *>(0x008bd524) == 4) {
        mask = 0x808080;
        class0 = 0x800000;
        class1 = 0x8000;
        class2 = 0x80;
    } else {
        mask = *reinterpret_cast<u32 *>(0x008bd4e0);
        class0 = *reinterpret_cast<u32 *>(0x008bd4c8);
        class1 = *reinterpret_cast<u32 *>(0x008bd4d0);
        class2 = *reinterpret_cast<u32 *>(0x008bd4d8);
    }
    u8 kind[60];
    for (u32 i = 0; i < 60; i++) {
        const u32 pixel = pixels[i], masked = mask & pixel;
        kind[i] = masked == (class0 & pixel) ? 0 : masked == (class1 & pixel) ? 1 : masked == (class2 & pixel) ? 2 : 3;
    }
    u32 fit = kind[0] == 1 ? 1 : 0;
    if (kind[59] == 1)
        fit++;
    for (u32 i = 1; i < 59; i += 2) {
        if (kind[i] == 3)
            fit++;
        if (kind[i + 1] == 2)
            fit++;
    }
    *reinterpret_cast<u32 *>(0x008c1644) = fit;
    if (fit == 60)
        return 0;
    return kind[0] != 0 && kind[59] != 2 ? -1 : 1;
}

u32 __fastcall TexLoad_DriverFlags(const u8 *object, void *, const u8 *request, const u8 *texture) {
    const u32 scene = *reinterpret_cast<u32 *>(0x008bcbc8);
    u32 bits = 0;
    if (scene & 0x2000)
        bits = 0x100000;
    if (scene & 0x400)
        bits |= 0x80000;
    const u32 flags = at<u32>(at<const u8 *>(object, 4), 0);
    if (flags & 2)
        bits |= 0x800;
    if (flags & 0x2000000)
        bits |= 0x5156;
    if (flags & 0x8000000)
        bits |= 0x20000;
    const u32 first = at<u32>(request, 0);
    if (at<u8>(request, 0xc) & 2)
        bits |= 0x800000;
    const u32 more = at<u32>(object, 8);
    if (more & 0x10000000)
        bits |= 0x10000000;
    if (more & 0x4000000)
        bits |= 0x30000000;
    if (more & 0x8000000)
        bits |= 0x50000000;
    if (!first || !texture)
        return bits | 0x1256;
    if (at<u8>(reinterpret_cast<const u8 *>(first), 1) & 8)
        bits |= 0x1256;
    else
        bits |= Texture_FormatBits(texture);
    const u32 tex = at<u32>(texture, 8);
    if (tex & 0x100000)
        bits |= 0x40000;
    if (tex & 0x30000)
        bits |= 0x400000;
    if ((tex & 0x400) && at<u32>(texture, 0x3c) == 0)
        bits |= 0x80;
    return bits;
}

void __fastcall TexFormat_SetArgb32(u8 *settings) {
    at<u32>(settings, 0x6c) = 0x41;
    at<u32>(settings, 0x68) = 0x20;
    at<u32>(settings, 0x70) = 0;
    at<u32>(settings, 0x74) = 0x20;
    at<u32>(settings, 0x78) = 0xff0000;
    at<u32>(settings, 0x7c) = 0xff00;
    at<u32>(settings, 0x80) = 0xff;
    at<u32>(settings, 0x84) = 0xff000000;
}

u8 *__fastcall TexFormat_Table(u8 *settings) { return settings + 0x88 + (at<u32>(settings, 4) << 12); }

void __cdecl TexFormat_ConvertSquare(u16 *to, const u32 *from, u32 side) {
    const u32 n = side * side;
    switch (*reinterpret_cast<const u32 *>((uintptr_t)0x008bd554)) {
    case 0:
        for (u32 i = 0; i < n; ++i) {
            const u32 c = from[i];
            to[i] = (u16)(((c >> 8) & 0xf800) + ((c >> 4) & 0xfc0) + ((c >> 3) & 0x1f));
        }
        return;
    case 1:
        for (u32 i = 0; i < n; ++i) {
            const u32 c = from[i];
            to[i] = (u16)(((c >> 16) & 0x8000) + ((c >> 9) & 0x7c00) + ((c >> 6) & 0x3e0) + ((c >> 3) & 0x1f));
        }
        return;
    case 2:
        for (u32 i = 0; i < n; ++i) {
            const u32 c = from[i];
            to[i] = (u16)(((c >> 4) & 0xf00) + ((c >> 16) & 0xf0) + ((c >> 28) & 0xf));
        }
        return;
    case 3: {
        u32 *const out = reinterpret_cast<u32 *>(to);
        for (u32 i = 0; i < side; ++i) out[i] = from[i];  // side dwords, not side * side (sic)
        return;
    }
    default: EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754003);
    }
}

u32 __fastcall Masks_TopBits(u8 *object) {
    auto top = [](u32 mask) {
        u32 bit = 0x80000000;
        while (!(mask & bit) && (bit >>= 1)) {
        }
        return bit;
    };
    const u32 red = top(at<u32>(object, 0x8fc));
    at<u32>(object, 0x900) = red;
    const u32 green = top(at<u32>(object, 0x904));
    at<u32>(object, 0x908) = green;
    const u32 blue = top(at<u32>(object, 0x90c));
    at<u32>(object, 0x910) = blue;
    at<u32>(object, 0x918) = blue | green | red;
    return blue | green | red;
}
}
