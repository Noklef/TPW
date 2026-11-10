#include "landscape.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/addrcalls.h"
#include "../core/crt.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/quickload.h"
#include "../gfx/texman.h"
#include "../gfx/texture.h"
#include "../math/mat4.h"
#include "../math/x87.h"
#include "../render/batch.h"
#include "../render/batchquad.h"
#include "../render/drawbuffer.h"
#include "../render/engineerrors.h"
#include "../render/quad.h"
#include "../render/meshlist.h"

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
    u8 *const textures = TexManager_Load(*reinterpret_cast<u8 *const *>(&gTexman_87a268), nullptr, located);
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

u32 __fastcall Landscape_BuildDome(u8 *landscape) {
    const float kOverFifteen = 0.0666666701f, kHalf = 0.5f, kSag = 0.200000003f, kRim = 0.600000024f, kQuarter = 0.25f, kSixteenth = 0.0625f;   // 0x701f4c, 0x701f50, 0x701f54, 0x701f58, 0x701f5c, 0x701f60
    const float kRing[3] = {-0.980000019f, -0.959999979f, -0.839999974f};                                                                       // 0x701f64, 0x701f68, 0x701f6c
    const float kPi = 3.14159274f;                                                                                                               // 0x701ee0
    float &centreX = at<float>(landscape, 0x1f64), &centreY = at<float>(landscape, 0x1f68), &centreZ = at<float>(landscape, 0x1f6c);
    const X87Reg width((double)at<s32>(landscape, 0x21e0)), height((double)at<s32>(landscape, 0x21e4));
    const float stepRow = (float)(height * kOverFifteen), stepColumn = (float)(width * kOverFifteen), halfColumn = (float)(width * kHalf);
    const float halfRow = (float)(X87Reg((float)height) * kHalf);
    for (s32 i = 0; i < 16; ++i) {
        const X87Reg rowAt = X87Reg((double)i) * stepRow - halfRow;
        const float row = (float)rowAt, rowSquared = (float)(rowAt * rowAt);
        for (s32 j = 0; j < 16; ++j) {
            float *const v = &at<float>(landscape, 0x360 + 0xc * i + 0xc0 * j);
            v[0] = (float)(X87Reg((double)j) * stepColumn - halfColumn);
            v[2] = row;
            v[1] = (float)(centreY - X87_Sqrt(X87Reg(v[0]) * X87Reg(v[0]) + rowSquared) * kSag);
            v[0] = (float)(X87Reg(v[0]) + centreX);
            v[2] = (float)(X87Reg(v[2]) + centreZ);
            const u32 grey = Crt().rand() & 0x7f;
            const u32 colour = (grey | grey << 8 | grey << 16) | 0xff808080;
            for (u32 layer = 0; layer < 4; ++layer)
                at<u32>(landscape, 0xf60 + 0x400 * layer + 4 * i + 0x40 * j) = colour;
        }
    }
    const X87Reg rim = X87Reg(halfColumn) * kRim;
    const X87Reg top = centreY - X87_Sqrt(rim * rim) * kSag;
    const float bottom = (float)X87Reg(-top.v);
    const X87Reg span = top - bottom;
    const X87Reg second = top - span * kQuarter;
    const float third = (float)(top - span * kHalf);
    float *ring = &at<float>(landscape, 0);
    for (s32 n : {0, 2, 4, 6, 8, 10, 12, 14, 16, 16, 18, 20, 22, 24, 26, 28, 30, 32}) {
        const X87Reg angle = X87Reg(kPi) * X87Reg((double)n) * kSixteenth;
        const X87Reg cosine = X87Reg(X87_Cos(angle.v)) * rim, sine = X87Reg(X87_Sin(angle.v)) * rim;
        ring[0] = (float)(cosine + centreX);
        ring[1] = (float)top;
        ring[2] = (float)(sine + centreZ);
        ring[4] = (float)second;
        ring[7] = third;
        ring[10] = bottom;
        for (u32 k = 0; k < 3; ++k) {
            ring[3 + 3 * k] = (float)(centreX - cosine * kRing[k]);
            ring[5 + 3 * k] = (float)(centreZ - sine * kRing[k]);
        }
        ring += 12;
    }
    u32 count = 0;
    for (s32 row = 0; row < 15; ++row) {
        for (s32 column = 0; column < 15; ++column) {
            const u8 *const y = &at<const u8>(landscape, 0x364 + 0xc0 * row + 0xc * column);
            const u16 at_ = (u16)(row * 16 + column);
            if (at<float>(y, 0) > bottom && at<float>(y, 0xc) > bottom && at<float>(y, 0xcc) > bottom) {
                u16 *const triangle = &at<u16>(landscape, 0x21fc + 2 * count);
                triangle[0] = (u16)(at_ + 1);
                triangle[1] = at_;
                triangle[2] = (u16)(at_ + 0x11);
                count += 3;
            }
            if (at<float>(y, 0) > bottom && at<float>(y, 0xc0) > bottom && at<float>(y, 0xcc) > bottom) {
                u16 *const triangle = &at<u16>(landscape, 0x21fc + 2 * count);
                triangle[0] = at_;
                triangle[1] = (u16)(at_ + 0x10);
                triangle[2] = (u16)(at_ + 0x11);
                count += 3;
            }
        }
    }
    at<u32>(landscape, 0x1f60) = count;
    return 1;
}

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
    u8 *const textures = TexManager_Load(*reinterpret_cast<u8 *const *>(&gTexman_87a268), nullptr, name);
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
    LoadSky(landscape, 0x1fd0, folder, gText_Landscape_LoadSky_768da0);   // "sky\sky.tga"
    if (LoadSky(landscape, 0x1fd4, folder, gText_Landscape_LoadSky_768d90) != nullptr) {   // "sky\sky_rgb.tga"
        Landscape_SampleColours(landscape);
        ReleaseSlot(landscape, 0x1fd4);
    }
    LoadSky(landscape, 0x1fd8, folder, gText_Landscape_LoadSky_768d80);   // "sky\sky_cyl.tga"
    return 1;
}
}

namespace {

template <typename T> T &G(uintptr_t address) { return *reinterpret_cast<T *>(address); }
const float &K(uintptr_t address) { return *reinterpret_cast<const float *>(address); }

const LandscapeDrawEnv kDrawPortEnv = {&Batch_Begin, &DrawBatch_AddClippedTriangles, &Batch_Close, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0)};
const LandscapeDrawEnv *g_drawEnv = &kDrawPortEnv;
const u32 kFarBits = 0x4cbebc20;  // (the depth of the sky's layers: 1e8)

// A sprite's vertices (four of x, y, z, w): the corners turned, then x scaled and all moved to the sprite's place, with the original's register and memory operands.
void PlaceSprite(float *v, const float *place) {
    const float &scale = K(0x008bcbcc);
    const X87Reg x0 = X87Reg(v[0]) * scale, x1 = X87Reg(v[4]) * scale;
    const float x2 = (float)(X87Reg(v[8]) * scale), x3 = (float)(X87Reg(v[12]) * scale);
    v[0] = (float)(X87Reg(place[0]) + x0);
    v[1] = (float)(X87Reg(place[1]) + v[1]);
    v[4] = (float)(X87Reg(place[0]) + x1);
    v[5] = (float)(X87Reg(place[1]) + v[5]);
    v[8] = (float)(X87Reg(place[0]) + x2);
    v[9] = (float)(X87Reg(place[1]) + v[9]);
    v[12] = (float)(X87Reg(place[0]) + x3);
    v[13] = (float)(X87Reg(place[1]) + v[13]);
    for (u32 i = 0; i < 4; i++) {
        at<u32>(v, i * 0x10 + 8) = at<u32>(place, 8);
        at<u32>(v, i * 0x10 + 0xc) = at<u32>(place, 0xc);
    }
}

// The two statics' matrix (0xfb3460): the identity on the first use, then the landscape's matrix through the transformer's slot 6 with `with`, kept in the landscape.
void JoinMatrix(u8 *landscape, u32 with) {
    if (!(gLandscapeMatrixGuard_fb3440 & 1)) {
        std::memset(gLandscapeMatrix_fb3460, 0, 0x40);
        gLandscapeMatrixGuard_fb3440 |= 1;
        gLandscapeMatrix_fb3460[0] = gLandscapeMatrix_fb3460[5] = gLandscapeMatrix_fb3460[10] = gLandscapeMatrix_fb3460[15] = 1.0f;
        g_drawEnv->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    u8 *const transformer = G<u8 *>(0x0087b1d8);
    at<void(__cdecl *)(u8 *, u8 *, u32, u32)>(at<const u8 *>(transformer, 0), 0x18)(transformer, landscape + 0x2ecc, with, (u32)&gLandscapeMatrix_fb3460);
    std::memcpy(landscape + 0x2ecc, gLandscapeMatrix_fb3460, 0x40);
}

void TransformPoints(u8 *landscape, u8 *out, const void *points, u32 stride, u32 count) {
    u8 *const transformer = G<u8 *>(0x0087b1d8);
    at<void(__cdecl *)(u8 *, u8 *, u8 *, const void *, u32, u32, u32, u32)>(at<const u8 *>(transformer, 0), 4)(transformer, landscape + 0x2ecc, out, points, stride, 0xc, count, 0);
}

// The rotation of the sky's texture coordinates, turned a quarter at each layer and kept from one to the next.
struct SkyTurn {
    u32 u, offsetU, v, offsetV;
};

// One layer of the sky: 16 by 16 vertices. Returns false with the answer to end the draw with.
bool SkyLayer(u8 *landscape, s32 i, const void *points, u32 flags, u32 id, u32 colourMask, u32 stride, SkyTurn &turn, u32 &answer) {
    u8 *const object = G<u8 *>(0x00879ab0);
    u8 *const batch = g_drawEnv->begin(object, nullptr, 2, 4, 0x1c4, flags, id);
    answer = 1;
    if (!batch) return false;
    const u8 *const buffer = at<const u8 *>(object, 0x20);
    const u32 indexEnd = at<u32>(buffer, 0x2c) + at<u32>(buffer, 0x24) * 2;
    const u32 vertexEnd = at<u32>(buffer, 0x28) + at<u32>(object, 0x1c) * (at<u32>(buffer, 0x20) + 0x100);
    if (at<u32>(object, 0x28) < vertexEnd || at<u32>(object, 0x2c) < indexEnd) return false;
    at<u32>(batch, 0) = 0x21;
    TransformPoints(landscape, at<u8 *>(batch, 0x28), points, stride, 0x100);
    const SkyTurn was = turn;
    turn.u = X87_FloatThrough(was.v) ^ 0x80000000;
    turn.v = X87_FloatThrough(was.u);
    turn.offsetU = X87_FloatThrough(was.offsetV) ^ 0x80000000;
    turn.offsetV = X87_FloatThrough(was.offsetU);
    const X87Reg offsetV = X87Reg(at<float>(&turn.offsetV, 0));
    const u32 base = (at<u32>(landscape, 0x2f0c) - i * at<u32>(landscape, 0x2f10)) & colourMask;
    const float &width = at<float>(landscape, 0x21ec + i * 4);
    const u8 *const centre = landscape + 0x1fa0 + i * 0xc;
    u8 *vertex = at<u8 *>(batch, 0x28);
    for (s32 row = 0; row < 16; row++) {
        for (s32 column = 0; column < 16; column++, vertex += stride) {
            at<u32>(vertex, 0x10) = base & at<u32>(landscape, 0xf60 + (i * 0x100 + row) * 4 + column * 0x40);
            at<float>(vertex, 0x18) = (float)((X87Reg((double)column) * X87Reg(width) + at<float>(centre, 0)) * at<float>(&turn.u, 0) + at<float>(&turn.offsetU, 0));
            at<float>(vertex, 0x1c) = (float)((X87Reg(width) * X87Reg((double)row) + at<float>(centre, 8)) * at<float>(&turn.v, 0) + offsetV);
        }
    }
    u32 count = 0x100;
    const s32 indices = g_drawEnv->addClipped(object, nullptr, &count, reinterpret_cast<const u16 *>(landscape + 0x21fc), at<s32>(landscape, 0x1f60), 2);
    G<u32>(0x008bcc3c) = G<u32>(0x008bcc1c);
    at<u32>(batch, 0x20) = count;
    at<s32>(batch, 0x24) = indices;
    if (!g_drawEnv->close(object, nullptr, at<float>(&kFarBits, 0))) {
        answer = 0;
        return false;
    }
    return true;
}

// The sky's rim: 0x48 vertices of the second texture, their u a sixteenth at each second column, the v bands kept to the first's or (in a reflection) the second's.
u32 SkyRim(u8 *landscape, u32 colourMask, u32 stride) {
    const u8 *const texture = at<const u8 *>(landscape, 0x1fd8);
    u8 *const object = G<u8 *>(0x00879ab0);
    u8 *const batch = g_drawEnv->begin(object, nullptr, 2, 4, 0x1c4, 0x1d56, at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0);
    if (!batch) return 1;
    at<u32>(batch, 0) = 0x21;
    TransformPoints(landscape, at<u8 *>(batch, 0x28), landscape, stride, 0x48);
    const bool reflecting = G<u32>(0x008bcbc8) & 0x2000000;
    X87Reg q = X87Reg(reflecting ? gLandscape_701f94 : gLandscape_701f88);
    const u32 low = reflecting ? 0x3c23d70a : 0x3e2e147b, high = reflecting ? 0x3e428f5c : 0x3eae147b;
    const float &e = gLandscape_701f78;
    const float a = (float)(X87Reg(at<float>(&high, 0)) - e), b = (float)(X87Reg(at<float>(&low, 0)) - e), c = (float)(q - e);
    const float qf = (float)q;
    u8 *vertex = at<u8 *>(batch, 0x28);
    const auto put = [&](float u, u32 v) {
        at<float>(vertex, 0x18) = u;
        at<u32>(vertex, 0x1c) = v;
        at<u32>(vertex, 0x10) = colourMask;
        vertex += stride;
    };
    for (s32 k = 0; k < 0x12; k += 2) {
        const float t = (float)(X87Reg((double)k) * gLandscape_701f60);
        put(t, 0x3f7d70a4);
        put(t, at<u32>(&a, 0));
        put(t, at<u32>(&b, 0));
        put(t, at<u32>(&c, 0));
    }
    for (s32 k = 0; k < 0x12; k += 2) {
        const float t = (float)(X87Reg((double)k) * gLandscape_701f60);
        put(t, 0x3efae148);
        put(t, high);
        put(t, low);
        put(t, at<u32>(&qf, 0));
    }
    u32 count = 0x48;
    const s32 indices = g_drawEnv->addClipped(object, nullptr, &count, reinterpret_cast<const u16 *>(landscape + 0x2c88), 0x120, 2);
    G<u32>(0x008bcc3c) = G<u32>(0x008bcc1c);
    at<u32>(batch, 0x20) = count;
    at<s32>(batch, 0x24) = indices;
    if (!g_drawEnv->close(object, nullptr, 0.0f)) return 0;
    return Landscape_DrawSprites(landscape, nullptr, 1) ? 1 : 0;
}

}  // namespace

namespace {
AddrCalls g_calls;
}
void Landscape_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void Landscape_SetDrawEnv(const LandscapeDrawEnv *env) { g_drawEnv = env ? env : &kDrawPortEnv; }

extern "C" {

void __fastcall Landscape_AddTextureList(u8 **list) {
    String path;
    String_Ctor(&path);
    const s32 &count = *reinterpret_cast<const s32 *>((uintptr_t)0x00785724);
    u8 *got = nullptr;
    const u32 *row = reinterpret_cast<const u32 *>((uintptr_t)0x00785728);
    for (s32 i = 0; i < count; ++i, row += 6) {
        vcall<void>(&path, 0x14, reinterpret_cast<const char *>((uintptr_t)g_calls.Cd(0x0040ef40)));
        String_EnsureTrailingBackslash(&path);
        vcall<void>(&path, 0x28, reinterpret_cast<const StringView *>((uintptr_t)0x0078aa30));
        String_EnsureTrailingBackslash(&path);
        vcall<void>(&path, 0x20, reinterpret_cast<const char *>((uintptr_t)row[5]));
        if (g_calls.Th(0x005850c0, 0x008c1828u, &got, path.data, row[0], row[1], row[2], row[3], 0u, row[4] ? 1u : 2u)) list[i] = got;
    }
    String_Dtor(&path);
}

u32 __fastcall Landscape_DrawSprites(u8 *landscape, void *, u32 mask) {
    const u32 render = G<u32>(0x008bcbc8);
    if (!(render & 0x400000) || (render & 8)) return 1;
    G<u32>(0x008bcc3c) = 0x3f800000;
    float vertices[16];
    u32 spec[11] = {};  // (+0x14 is unset in the original too; Batch_DrawQuad does not read it)
    spec[0] = 1;
    spec[1] = (u32)(uintptr_t)vertices;
    spec[2] = 0xffffffff;
    spec[10] = 1;
    for (s32 i = 0; i < at<s32>(landscape, 0x21dc); i++) {
        const u8 *const entry = landscape + 0x1fdc + i * 0x20;
        if (!(mask & at<u32>(entry, 8))) continue;
        const float half = (float)(X87Reg(at<float>(entry, 0x18)) * gLandscape_DrawSprites_701f50);
        const u8 *const texture = at<const u8 *>(entry, 0);
        if (texture) {
            spec[4] = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
            spec[3] = Texture_FormatBits(texture);
            spec[6] = X87_FloatThrough(at<u32>(texture, 0x28));
            spec[7] = X87_FloatThrough(at<u32>(texture, 0x2c));
            at<float>(spec, 0x20) = (float)(X87Reg(at<float>(texture, 0x20)) + at<float>(spec, 0x18));
            at<float>(spec, 0x24) = (float)(X87Reg(at<float>(texture, 0x24)) + at<float>(spec, 0x1c));
            if (at<u32>(texture, 8) & 0x100000) spec[0] |= 4;
        } else {
            spec[4] = 0;
            spec[3] = 0x1256;
            spec[6] = spec[7] = spec[8] = spec[9] = 0;
        }
        u32 format = at<u32>(entry, 4);
        if (!format) format = Texture_FormatBits(texture);
        spec[3] = format | 0x1800;
        float place[4];
        Mat4_Transform(reinterpret_cast<const float *>(landscape + 0x2ecc), nullptr, place, reinterpret_cast<const float *>(entry + 0xc), 0);
        vertices[0] = -half, vertices[1] = half;
        vertices[4] = half, vertices[5] = half;
        vertices[8] = half, vertices[9] = -half;
        vertices[12] = -half, vertices[13] = -half;
        Quad_TurnXY(vertices, G<float>(0x0087af60), nullptr);
        PlaceSprite(vertices, place);
        if (!Batch_DrawQuad(G<u8 *>(0x00879ab0), nullptr, reinterpret_cast<u8 *>(spec), 0xfffffffd)) return 0;
    }
    return 1;
}

u32 __fastcall Landscape_Draw(u8 *landscape) {
    const u8 *const texture = at<const u8 *>(landscape, 0x1fd0);
    if (!at<u32>(landscape, 0x2ec8) || !texture || !at<u32>(landscape, 0x1fd8) || G<u32>(0x008bd524) == 4) return 1;
    const u32 stride = gLandscape_Draw_701ed4;
    const u32 colourMask = at<u32>(gMeshlist_87a248, 0x2c);
    const u32 id = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
    const u32 flags = (G<u32>(0x008bcbc8) & 8 ? 0x1256 : 0x1556) | 0x400800;
    SkyTurn turn = {};  // (without an id the original's four are unset stack: not benched)
    if (id) {
        turn.offsetU = at<u32>(texture, 0x28);
        turn.offsetV = at<u32>(texture, 0x2c);
        turn.u = at<u32>(texture, 0x20);
        turn.v = at<u32>(texture, 0x24);
    }
    std::memcpy(landscape + 0x2ecc, G<const void *>(0x0087b154), 0x40);
    at<u32>(landscape, 0x2efc) = at<u32>(landscape, 0x2f00) = at<u32>(landscape, 0x2f04) = 0;
    JoinMatrix(landscape, 0x0087b110);
    JoinMatrix(landscape, 0x0087b0d0);
    if (!Landscape_DrawSprites(landscape, nullptr, 2)) return 0;
    const s32 layers = G<u32>(0x008bcbc8) & 8 ? 1 : at<s32>(landscape, 0x2ec8);
    u32 answer = 1;
    for (s32 i = 0; i < layers; i++)
        if (!SkyLayer(landscape, i, landscape + 0x360, flags, id, colourMask, stride, turn, answer)) return answer;
    if (G<u32>(0x008bcbc8) & 0x2000000) {
        u32 reflected[256][3];
        for (u32 p = 0; p < 256; p++) {
            const u32 *const point = reinterpret_cast<const u32 *>(landscape + 0x360 + p * 0xc);
            reflected[p][0] = X87_FloatThrough(point[0]) ^ 0x80000000;
            reflected[p][1] = X87_FloatThrough(point[1]) ^ 0x80000000;
            reflected[p][2] = point[2];
        }
        for (s32 i = 0; i < layers; i++)
            if (!SkyLayer(landscape, i, reflected, flags, id, colourMask, stride, turn, answer)) return answer;
    }
    return SkyRim(landscape, colourMask, stride);
}

u32 __fastcall Landscape_SetDomeCentre(u8 *landscape, void *, u32 a, u32 b, u32 c) {
    at<u32>(landscape, 0x1f64) = a;
    at<u32>(landscape, 0x1f68) = b;
    at<u32>(landscape, 0x1f6c) = c;
    return g_calls.Th(0x00585720, (u32)(uintptr_t)landscape);
}

u32 __fastcall Landscape_ResetDomeCentre(u8 *landscape) {
    at<u32>(landscape, 0x1f68) = 0x43960000;
    at<u32>(landscape, 0x1f64) = 0;
    at<u32>(landscape, 0x1f6c) = 0;
    return g_calls.Th(0x00585720, (u32)(uintptr_t)landscape);
}
}
