#include "landscape.h"

#include <cstdint>
#include <cstring>

#include "../files/quickload.h"
#include "../gfx/texman.h"
#include "../gfx/texture.h"
#include "../math/x87.h"
#include "../render/engineerrors.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

float Bits(u32 b) {
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

// The 8 x 3 cells' two triangles (a, a + 5, a + 4 and a, a + 1, a + 5, a = 4 row + column + first) from +at.
void Cells(u8 *landscape, u32 at, u32 first) {
    for (u32 row = 0; row < 8; ++row)
        for (u32 column = 0; column < 3; ++column) {
            u16 *const cell = reinterpret_cast<u16 *>(landscape + at + 0x24 * row + 0xc * column);
            const u32 a = column + row * 4 + first;
            cell[0] = (u16)a;
            cell[1] = (u16)(a + 5);
            cell[2] = (u16)(a + 4);
            cell[3] = (u16)a;
            cell[4] = (u16)(a + 1);
            cell[5] = (u16)(a + 5);
        }
}

// The texture array at +offset released (Texture_Release on its first texture, the array destroyed when still set) and cleared.
void ReleaseSlot(u8 *landscape, u32 offset) {
    Texture_Release(at<u8 *>(landscape, offset));
    if (u8 *const array = at<u8 *>(landscape, offset))
        Texture_DestroyArray(array);
    at<u32>(landscape, offset) = 0;
}

// One of the sky's textures: folder + "\" (0x7477fc) + file (120 bytes, unchecked) located (Quickload_Locate(path, 1, 0)), the old
// array at +offset released when set, the path loaded (TexManager_Load at [0x87a268]) into +offset: none pushes the error (1,
// 0xb0754001) twice (sic), got has its flags' 0x100 cleared and 0x30200 set. Returns the array.
u8 *LoadSky(u8 *landscape, u32 offset, const char *folder, const char *file) {
    char path[120];
    std::strcpy(path, folder);
    std::strcat(path, reinterpret_cast<const char *>((uintptr_t)0x007477fc));
    std::strcat(path, file);
    const char *const located = Quickload_Locate(path, 1, nullptr);
    if (at<u8 *>(landscape, offset) != nullptr)
        ReleaseSlot(landscape, offset);
    u8 *const textures = TexManager_Load(*reinterpret_cast<u8 *const *>((uintptr_t)0x0087a268), nullptr, located);
    at<u8 *>(landscape, offset) = textures;
    if (textures == nullptr) {
        u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
        EngineErrors_Push(errors, nullptr, 1, 0xb0754001);
        EngineErrors_Push(errors, nullptr, 1, 0xb0754001);
    } else {
        at<u32>(textures, 8) = (at<u32>(textures, 8) & ~0x100u) | 0x30200;
    }
    return textures;
}

}  // namespace

extern "C" {

u8 *__fastcall Landscape_Init(u8 *landscape) {
    float *const matrix = &at<float>(landscape, 0x2ecc);
    std::memset(matrix, 0, 0x40);
    matrix[15] = matrix[10] = matrix[5] = matrix[0] = 1.0f;
    at<u32>(landscape, 0x21e8) = 0x46bb8000;   // 24000.0
    at<u32>(landscape, 0x21e0) = 0x960;
    at<u32>(landscape, 0x21e4) = 0x960;
    at<u32>(landscape, 0x1f68) = 0x43960000;   // 300.0
    at<u32>(landscape, 0x2ec8) = *reinterpret_cast<const u32 *>(0x008bd524) != 4 ? 4 : 1;
    at<u32>(landscape, 0x1f64) = 0;
    at<u32>(landscape, 0x1f6c) = 0;
    for (s32 i = 0; i < 4; ++i) {
        u32 *const zero = &at<u32>(landscape, 0x1fa0 + 12 * i);
        zero[0] = zero[1] = zero[2] = 0;
        const X87Reg n = (double)i;
        at<float>(landscape, 0x1f70 + 12 * i) = (float)(X87Reg(Bits(0x3a51b717)) - n * Bits(0xba03126f));   // 0x701f3c, 0x701f38
        at<u32>(landscape, 0x1f74 + 12 * i) = 0;
        at<float>(landscape, 0x1f78 + 12 * i) = (float)(X87Reg(Bits(0x38bcbe62)) - n * Bits(0x37fba882));   // 0x701f44, 0x701f40
        at<float>(landscape, 0x21ec + 4 * i) = (float)(X87Reg((double)(7 - i)) * Bits(0x40490fdb) * Bits(0x3c124925));   // 0x701ee0, 0x701f48
    }
    at<u32>(landscape, 0x1fd0) = 0;
    at<u32>(landscape, 0x1fd4) = 0;
    at<u32>(landscape, 0x1fd8) = 0;
    Cells(landscape, 0x2c88, 0);
    Cells(landscape, 0x2da8, 0x24);
    at<u32>(landscape, 0x21dc) = 0;
    at<u32>(landscape, 0x2f0c) = 0xcfffffff;
    at<u32>(landscape, 0x2f10) = 0x38000000;
    return landscape;
}

void __fastcall Landscape_SetPoint(u8 *landscape, void *, s32 i, u32 x, u32 y, u32 z) {
    u32 *point = reinterpret_cast<u32 *>(landscape + 0x1f70 + i * 12);
    point[0] = x;
    point[1] = y;
    point[2] = z;
}

void __fastcall Landscape_SetDetail(u8 *landscape, void *, u32 level) { at<u32>(landscape, 0x2ec8) = level; }

u32 __fastcall Landscape_List(const u8 *landscape) { return at<u32>(landscape, 0x1fd0); }

u8 *__fastcall Landscape_Point(u8 *landscape) { return landscape + 0x1fa0; }

void __fastcall Landscape_SampleColours(u8 *landscape) {
    u8 *const texture = at<u8 *>(landscape, 0x1fd4);
    if (!texture)
        return;
    const s32 stepX = (s32)at<s16>(texture, 0x10) / 16;
    const s32 stepY = (s32)at<s16>(texture, 0x12) / 16;
    const u32 x0 = (u32)(stepX / 2);
    u32 y = (u32)(stepY / 2);
    const uintptr_t pixels = (uintptr_t)Texture_Lock(texture);
    u8 *column = landscape + 0xf60;
    for (u32 row = 0; row < 16; row++) {
        u8 *cell = column;
        u32 x = 0;
        for (u32 i = 0; i < 16; i++) {
            const u32 format = *reinterpret_cast<const u32 *>((uintptr_t)0x008bd554);
            const u32 index = (u32)(s32)at<s16>(texture, 0x34) * y + x0 + x;
            if (format == 2) {
                const u32 c = *reinterpret_cast<const u16 *>(pixels + index * 2);
                at<u32>(cell, 0) = (c & 0xff80) << 12 | (c & 0xf0) << 8 | (c & 0xf) << 4 | 0xff080808;   // (its AND AL,0x80 keeps bit 7, sic)
            } else if (format == 3) {
                at<u32>(cell, 0) = *reinterpret_cast<const u32 *>(pixels + index * 4);
            }
            const u32 colour = at<u32>(cell, 0);
            at<u32>(cell, 0x400) = colour;
            at<u32>(cell, 0x800) = colour;
            at<u32>(cell, 0xc00) = colour;
            x += (u32)stepX;
            cell += 0x40;
        }
        column += 4;
        y += (u32)stepY;
    }
}

u32 __fastcall Landscape_FreeTextures(u8 *landscape) {
    for (u32 offset = 0x1fd0; offset <= 0x1fd8; offset += 4) {
        if (!at<u8 *>(landscape, offset))
            continue;
        Texture_Release(at<u8 *>(landscape, offset));
        if (u8 *const array = at<u8 *>(landscape, offset))
            Texture_DestroyArray(array);
        at<u32>(landscape, offset) = 0;
    }
    return 1;
}

u32 __fastcall Landscape_RemoveTexture(u8 *landscape, void *, u8 *textures) {
    s32 i = at<s32>(landscape, 0x21dc);   // (from the one past the last, sic)
    while (i >= 0 && at<u8 *>(landscape, 0x1fdc + 0x20 * i) != textures)
        --i;
    if (i < 0)
        return 0;
    u8 *const entry = landscape + 0x1fdc + 0x20 * i;
    u8 *const array = at<u8 *>(entry, 0);
    at<u32>(entry, 0) = 0;
    if (array) {
        Texture_Release(array);
        Texture_DestroyArray(array);
    }
    const s32 last = --at<s32>(landscape, 0x21dc);
    if (last != 0) {
        u32 *const a = reinterpret_cast<u32 *>(entry);
        u32 *const b = reinterpret_cast<u32 *>(landscape + 0x1fdc + 0x20 * last);
        for (u32 k = 0; k < 8; ++k) {
            const u32 t = a[k];
            a[k] = b[k];
            b[k] = t;
        }
    }
    return 1;
}

u32 __fastcall Landscape_AddTexture(u8 *landscape, void *, u8 **out, const char *name, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f) {
    u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
    const s32 count = at<s32>(landscape, 0x21dc);
    if (count >= 0x10) {
        EngineErrors_Push(errors, nullptr, 1, 0xb0754012);
        return 0;
    }
    u8 *const entry = landscape + 0x1fdc + 0x20 * count;
    at<s32>(landscape, 0x21dc) = count + 1;
    if (at<u8 *>(entry, 0) != nullptr) {
        Texture_Release(at<u8 *>(entry, 0));
        if (u8 *const array = at<u8 *>(entry, 0))
            Texture_DestroyArray(array);
        at<u32>(entry, 0) = 0;
    }
    u8 *const textures = TexManager_Load(*reinterpret_cast<u8 *const *>((uintptr_t)0x0087a268), nullptr, name);
    at<u8 *>(entry, 0) = textures;
    if (textures == nullptr) {
        EngineErrors_Push(errors, nullptr, 1, 0xb0754001);
        EngineErrors_Push(errors, nullptr, 1, 0xb0754001);
        *out = nullptr;
        return 0;
    }
    at<u32>(textures, 8) = (at<u32>(textures, 8) & ~0x100u) | 0x30200;
    at<u32>(entry, 0xc) = a;
    at<u32>(entry, 0x10) = b;
    at<u32>(entry, 0x14) = c;
    at<u32>(entry, 0x18) = d;
    at<u32>(entry, 4) = e;
    at<u32>(entry, 8) = f;
    *out = textures;
    return 1;
}

u32 __fastcall Landscape_RemoveTextureList(u8 **list) {
    const s32 &count = *reinterpret_cast<const s32 *>(0x00785724);
    for (s32 i = 0; i < count; i++)
        Landscape_RemoveTexture(reinterpret_cast<u8 *>(0x008c1828), nullptr, list[i]);
    return static_cast<u32>(count);
}

u32 __fastcall Landscape_LoadSky(u8 *landscape, void *, const char *folder) {
    LoadSky(landscape, 0x1fd0, folder, reinterpret_cast<const char *>((uintptr_t)0x00768da0));   // "sky\sky.tga"
    if (LoadSky(landscape, 0x1fd4, folder, reinterpret_cast<const char *>((uintptr_t)0x00768d90)) != nullptr) {   // "sky\sky_rgb.tga"
        Landscape_SampleColours(landscape);
        ReleaseSlot(landscape, 0x1fd4);
    }
    LoadSky(landscape, 0x1fd8, folder, reinterpret_cast<const char *>((uintptr_t)0x00768d80));   // "sky\sky_cyl.tga"
    return 1;
}
}
