// The Direct3D driver probes: see d3dprobe.h.
#include "d3dprobe.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/cpuinfo.h"
#include "../core/vcall.h"
#include "../core/vregs.h"
#include "../math/x87.h"
#include "drawbuffer.h"
#include "d3dresult.h"

using namespace d3dresult;

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kTwenty = 0x41a00000;  // 20.0f
const u32 kHundred = 0x42c80000;  // 100.0f

// The probe's virtual base (its offset is the second dword of the table at +0), found again for each use.
u8 *base(u8 *probe) { return probe + at<u32>(at<u8 *>(probe, 0), 4); }
u8 *device(u8 *probe) { return at<u8 *>(probe, 0x24); }
u8 *caps() { return global<u8 *>(0x00879da8); }
u32 &renderCaps() { return global<u32>(0x008bcbc8); }

void setFlags(u8 *probe, u32 flags) {
    at<u32>(base(probe), 0x14) = ~flags;
    vcall<u32>(base(probe), 8, flags);
}
void clear(u8 *probe) { vcall<u32>(base(probe), 0xc, 0u, 0u); }
void draw(u8 *probe, u32 colour, u32 specular) { vcall<u32>(base(probe), 0x44, kTwenty, colour, specular, 0u, 0u, 0u, 0u); }
u32 readPixel(u8 *probe, u32 *pixel) { return vcall<u32>(base(probe), 0x40, 0u, 0u, pixel); }
void renderState(u8 *probe, u32 state, u32 value) { comcall<u32>(device(probe), 0x58, state, value); }
void stageState(u8 *probe, u32 stage, u32 type, u32 value) { comcall<u32>(device(probe), 0xa0, stage, type, value); }

u32 &shadow(u8 *probe) { return at<u32>(base(probe), 0x14); }

// A flag that differs from the flags set last sets its render state (on: the flag's bit, or its complement for `inverted`), then that bit of the shadow.
void applyFlag(u8 *probe, u32 flags, u32 bit, u32 state, bool inverted) {
    if (!((shadow(probe) ^ flags) & bit)) return;
    renderState(probe, state, ((inverted ? ~flags : flags) & bit) != 0 ? 1 : 0);
    u32 &s = shadow(probe);
    s = ((flags ^ s) & bit) ^ s;
}

const D3dProbeEnv kGameEnv = {Cpu_Ticks16};
const D3dProbeEnv *g_env = &kGameEnv;

const u16 kQuad[6] = {0, 2, 1, 0, 3, 2};   // two triangles of a quad's four vertices

u32 bitsOf(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

u32 drawIndexed(u8 *probe, u32 type, const u32 *vertices, u32 vertexCount, const u16 *indices, u32 indexCount, u32 flags) {
    return comcall<u32>(device(probe), 0x74, type, 0x1c4u, vertices, vertexCount, indices, indexCount, flags);
}

}  // namespace

void D3dProbe_SetEnv(const D3dProbeEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

u32 __fastcall D3dProbe_AlphaTest(u8 *probe) {
    u32 pixel = static_cast<u32>(reinterpret_cast<uintptr_t>(probe));  // (sic: the slot PUSH ECX made)
    if (!(at<u8>(caps(), 0xa4) & 0x40)) {
        at<u32>(base(probe), 0xc) = 0x1000;
        return 1;
    }
    at<u32>(probe, 8) = 0;
    setFlags(probe, 0x9256);
    clear(probe);
    draw(probe, 0, 0);
    vcall<u32>(base(probe), 8, 0x8556u);  // (the kept complement is not updated)
    renderState(probe, 0x19, 7);     // ALPHAFUNC: GREATEREQUAL
    renderState(probe, 0xf, 1);      // ALPHATESTENABLE
    renderState(probe, 0x18, 0xff);  // ALPHAREF
    stageState(probe, 0, 1, 2);      // COLOROP: SELECTARG1
    stageState(probe, 0, 4, 2);      // ALPHAOP: SELECTARG1
    stageState(probe, 0, 2, 1);      // COLORARG1: TEXTURE
    stageState(probe, 0, 5, 1);      // ALPHAARG1: TEXTURE
    draw(probe, 0x80ffffff, 0);
    if (!readPixel(probe, &pixel)) return 0;
    if (pixel == 0) at<u32>(probe, 8) = 2;
    return 1;
}

u32 __fastcall D3dProbe_FogTest(u8 *probe) {
    renderCaps() &= ~0xc000u;
    if (!(at<u32>(caps(), 0xa8) & 0x80000)) {
        renderCaps() |= 0x4000;
        return 1;
    }
    u32 pixel;  // (sic: not set; read only after a read that succeeded)
    const u32 fogStart = 0, fogEnd = kHundred;
    setFlags(probe, 0x109256);
    clear(probe);
    renderState(probe, 0x22, 0);  // FOGCOLOR
    renderState(probe, 0x23, 3);  // FOGTABLEMODE: LINEAR
    renderState(probe, 0x24, fogStart);
    renderState(probe, 0x25, fogEnd);
    draw(probe, 0xffffffff, 0);
    if (!readPixel(probe, &pixel)) {
        renderCaps() |= 0x8000;
        return 0;
    }
    const u32 mask = global<u32>(0x008bd4dc);
    pixel &= mask;
    if (pixel == 0 || pixel == mask) {
        setFlags(probe, 0x109256);
        clear(probe);
        renderState(probe, 0x23, 0);         // FOGTABLEMODE: NONE
        renderState(probe, 0x22, 0xffffff);  // FOGCOLOR
        draw(probe, 0, 0x80000000);
        if (readPixel(probe, &pixel)) {
            const u32 m = global<u32>(0x008bd4dc);
            const u32 v = pixel & m;
            renderCaps() |= v == 0 || v == m ? 0x4000 : 0x8000;
        }
    }
    if (!(renderCaps() & 0xc000)) renderCaps() |= 0x800000;
    return 1;
}

void __fastcall D3dProbe_SetTextureStages(u8 *probe, void *, u32 mode, const u8 *textures) {
    if (at<u32>(probe, 0x18) != 4) return;
    struct Stage {
        u32 stage, type, value;
    };
    const Stage *first, *second = nullptr;
    u32 firstCount, secondCount = 0, firstTexture = 0, secondTexture = 0;
    static const Stage kOne[] = {{0, 1, 2}, {0, 4, 2}, {0, 2, 0x22}, {0, 0xb, 1}};
    static const Stage kOneB[] = {{1, 1, 8}, {1, 4, 8}, {1, 2, 0x32}, {1, 3, 1}, {1, 5, 0x12}, {1, 6, 1}, {1, 0xb, 0}};
    static const Stage kTwo[] = {{0, 1, 3}, {0, 4, 3}, {0, 2, 2}, {0, 3, 0}, {0, 5, 2}, {0, 6, 0}, {0, 0xb, 0}};
    static const Stage kTwoB[] = {{1, 1, 4}, {1, 4, 4}, {1, 2, 2}, {1, 3, 1}, {1, 5, 2}, {1, 6, 1}, {1, 0xb, 0}};
    static const Stage kZero[] = {{0, 1, 4}, {0, 4, 4}, {0, 2, 2}, {0, 3, 0}, {0, 5, 2}, {0, 6, 0}, {1, 0xb, 1},
                                  {1, 1, 1}, {1, 4, 1}, {1, 2, 2}, {1, 3, 0}, {1, 6, 2}, {1, 6, 0}};
    switch (mode) {
    case 0:
        for (const Stage &s : kZero) stageState(probe, s.stage, s.type, s.value);
        return;
    case 1:
        if (shadow(probe) & 0x100) {
            renderState(probe, 0x1b, 0);
            shadow(probe) &= ~0x100u;
        }
        first = kOne, firstCount = 4, second = kOneB, secondCount = 7;
        firstTexture = 8, secondTexture = 0xc;
        break;
    case 2:
        if (~shadow(probe) & 0x100) {
            renderState(probe, 0x1b, 1);
            shadow(probe) |= 0x100;
        }
        if ((shadow(probe) ^ 0xffffff93) & 0xff) {
            renderState(probe, 0x13, 9);
            renderState(probe, 0x14, 3);
            shadow(probe) = (shadow(probe) & 0xffffff93) | 0x93;
        }
        first = kTwo, firstCount = 7, second = kTwoB, secondCount = 7;
        firstTexture = 0xc, secondTexture = 8;
        break;
    default:
        return;
    }
    for (u32 i = 0; i < firstCount; ++i) stageState(probe, first[i].stage, first[i].type, first[i].value);
    vcall<u32>(base(probe), 0xc, 0u, at<u32>(textures, firstTexture));
    for (u32 i = 0; i < secondCount; ++i) stageState(probe, second[i].stage, second[i].type, second[i].value);
    vcall<u32>(base(probe), 0xc, 1u, at<u32>(textures, secondTexture));
    stageState(probe, 2, 1, 1);  // stage 2 COLOROP: DISABLE
}

u32 __fastcall D3dProbe_SetFlags(u8 *self, void *, u32 flags) {
    u8 *const probe = self - 0x2c;
    applyFlag(probe, flags, 0x400, 0xf, false);  // ALPHATESTENABLE
    {
        const u32 index = at<u32>(probe, 8) * 4;
        const u32 off = global<u32>(0x007012d8 + index), on = global<u32>(0x007012dc + index);  // (both read first)
        if ((shadow(probe) ^ flags) & 0x200) {
            renderState(probe, 0x18, flags & 0x200 ? on : off);  // ALPHAREF
            u32 &s = shadow(probe);
            s = ((flags ^ s) & 0x200) ^ s;
        }
    }
    applyFlag(probe, flags, 0x8000, 7, true);      // ZENABLE
    applyFlag(probe, flags, 0x800, 0xe, true);     // ZWRITEENABLE
    applyFlag(probe, flags, 0x80000, 0x1d, false);  // SPECULARENABLE
    applyFlag(probe, flags, 0x100000, 0x1c, false);  // FOGENABLE
    applyFlag(probe, flags, 0x1000000, 0x1a, true);  // DITHERENABLE
    applyFlag(probe, flags, 0x2000000, 0x1f, true);
    applyFlag(probe, flags, 0x4000000, 4, true);   // TEXTUREPERSPECTIVE
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x20000, 0x10, 1, 2);
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x40000, 0x12, 1, at<u32>(probe, 0xc));
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x10000, 1, 5, 4);
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x400000, 0xc, 1, 3);
    Shadow_SetBits3(probe, nullptr, flags, 1, 0x4000, 1, at<u32>(probe, 0x18), 1);
    Shadow_SetBits3(probe, nullptr, flags, 1, 0x4000, 4, 3, 1);  // never sets anything: the call above already brought the shadow's 0x4000 into line (sic)
    stageState(probe, 0, 0x11, flags & 0x400000 ? 2 : 1);
    if (renderCaps() & 0x80000000) {
        Shadow_SetBits2(probe, nullptr, 0xffffffff, 0x100, 0x1b, 1, 0);
        Shadow_SetByte(probe, nullptr, (flags & 0xff) == 0x52 ? 0x52 : 0x22);
        return 1;
    }
    if (shadow(probe) != flags) {
        Shadow_SetByte(probe, nullptr, static_cast<s32>(flags));
        Shadow_SetBits2(probe, nullptr, flags, 0x100, 0x1b, 1, 0);
    }
    return 1;
}

u32 __fastcall D3dProbe_Draw(u8 *self, void *, u32 size, u32 diffuse, u32 specular, u32 u0, u32 v0, u32 u1, u32 v1) {
    u8 *const probe = self - 0x2c;
    vcall<u32>(base(probe), 0x1c);
    float s;
    std::memcpy(&s, &size, 4);
    const float edgeF = static_cast<float>(X87Reg(s) - X87Reg(0.5f));
    u32 edge;
    std::memcpy(&edge, &edgeF, 4);
    const u32 half = 0xbf000000, z = 0x42480000, rhw = 0x3ca3d70a;   // -0.5, 50.0, 0.02
    u32 vertices[24] = {half, half, z, rhw, diffuse, specular, X87_FloatThrough(u0), X87_FloatThrough(v0),
                        edge, half, z, rhw, diffuse, specular, X87_FloatThrough(u1), X87_FloatThrough(v0),
                        half, edge, z, rhw, diffuse, specular, X87_FloatThrough(u0), X87_FloatThrough(v1)};
    comcall<u32>(at<u8 *>(probe, 0x24), 0x70, 4u, 0x1c4u, vertices, 3u, 1u);
    vcall<u32>(base(probe), 0x20);
    return 1;
}

u32 __fastcall D3dProbe_BeginScene(u8 *self) {
    u8 *const probe = self - 0x2c;
    stageState(probe, 0, 0x13, global<u32>(0x008bd4a8));
    keep(comcall<u32>(device(probe), 0x24));
    return pushed(3);
}

u32 __fastcall D3dProbe_EndScene(u8 *self) {
    keep(comcall<u32>(device(self - 0x2c), 0x28));
    return pushed(3);
}

u32 __fastcall D3dProbe_ReadPixel(u8 *, void *, s32 x, s32 y, u32 *out) {
    u8 *const surface = at<u8 *>(global<u8 *>(0x00879a24), 0x14);
    u32 desc[31] = {0x7c};
    keep(comcall<u32>(surface, 0x64, 0u, desc, 1u, 0u));
    if (result() != 0) return 0;
    const s32 bits = global<s32>(0x008bcbe4);
    const u32 pixel = desc[9] + static_cast<u32>(bits >> 3) * static_cast<u32>(x) + desc[4] * static_cast<u32>(y);
    *out = bits == 0x10 ? global<u16>(pixel) : global<u32>(pixel);
    *out &= global<u32>(0x008bd4dc);
    comcall<u32>(surface, 0x80, 0u);
    return 1;
}

u32 __fastcall D3dProbe_SetTexture(u8 *self, void *, u32 stage, u32 texture) {
    u8 *const probe = self - 0x2c;
    if (at<u32>(probe, 0x20) == texture) return 1;
    keep(comcall<u32>(device(probe), 0x98, stage, texture));
    if (result() != 0) return 0;
    at<u32>(probe, 0x20) = texture;
    return 1;
}

u32 __fastcall D3dProbe_SetFog(u8 *self, void *, u32 start, u32 end, u32 colour) {
    u8 *const probe = self - 0x2c;
    Obj_SetColour8b4(reinterpret_cast<u8 *>((uintptr_t)0x008bcbc8), nullptr, colour);
    float s, e;
    std::memcpy(&s, &start, 4);
    std::memcpy(&e, &end, 4);
    const X87Reg range = X87Reg(e) - s;
    if (range.v > 0)
        global<float>(0x008bd498) = static_cast<float>(X87Reg(1.0) / range);
    else
        global<u32>(0x008bd498) = 0;
    renderState(probe, 0x30, 0);
    renderState(probe, 0x22, colour);   // FOGCOLOR
    if (renderCaps() & 0xc000) {
        renderState(probe, 0x23, 0);    // FOGTABLEMODE: NONE
    } else {
        renderState(probe, 0x23, 3);    // FOGTABLEMODE: LINEAR
        renderState(probe, 0x24, start);
        renderState(probe, 0x25, end);
    }
    return 1;
}

u32 __fastcall D3dProbe_DrawTestQuad(u8 *self) {
    u8 *const probe = self - 0x2c;
    const u32 half = 0x3f000000, one = 0x3f800000;
    const u32 vertices[32] = {0x41200000, 0x41200000, half, one, 0xffffffff, 0, 0, 0, 0x441d8000, 0x43870000, half, one, 0xffff0000, 0, 0, 0,
                              0x43f50000, 0x43eb0000, half, one, 0xff00ff00, 0, 0, 0, 0x41f00000, 0x43c80000, half, one, 0xff0000ff, 0, 0, 0};
    vcall<u32>(base(probe), 0x1c);
    clear(probe);
    drawIndexed(probe, 4, vertices, 4, kQuad, 6, 0);
    return vcall<u32>(base(probe), 0x20);
}

u32 __fastcall D3dProbe_DrawQuad(u8 *self, void *, u32 x0, u32 y0, u32 x1, u32 y1, u32 colour, u8 *texture) {
    u8 *const probe = self - 0x2c;
    const u32 z = 0x3a83126f, one = 0x3f800000;   // 0.001, 1.0
    const u32 vertices[32] = {x0, y0, z, one, colour, 0, 0, 0, x1, y0, z, one, colour, 0, one, 0, x1, y1, z, one, colour, 0, one, one, x0, y1, z, one, colour, 0, 0, one};
    if ((colour & 0xff000000) != 0xff000000) {
        if ((shadow(probe) & 0xff) != 0x56) {
            renderState(probe, 0x13, 5);
            renderState(probe, 0x14, 6);
            shadow(probe) = (shadow(probe) & ~0xffu) | 0x56;
        }
        if (!(shadow(probe) & 0x100)) {
            renderState(probe, 0x1b, 1);
            shadow(probe) |= 0x100;
        }
        const u32 reference = global<u32>(0x007012d8 + at<u32>(probe, 8) * 4);
        if (shadow(probe) & 0x200) {
            renderState(probe, 0x18, reference);   // ALPHAREF
            shadow(probe) &= ~0x200u;
        }
    } else if (shadow(probe) & 0x100) {
        renderState(probe, 0x1b, 0);
        shadow(probe) &= ~0x100u;
    }
    if (shadow(probe) & 0x8000) {
        renderState(probe, 7, 1);     // ZENABLE
        shadow(probe) &= ~0x8000u;
    }
    if (shadow(probe) & 0x400) {
        renderState(probe, 0xf, 0);   // ALPHATESTENABLE
        shadow(probe) &= ~0x400u;
    }
    if (texture)
        vcall<u32>(base(probe), 0xc, 0u, vcall<u32>(texture, 8));
    else
        clear(probe);
    keep(drawIndexed(probe, 4, vertices, 4, kQuad, 6, 0));
    return pushed(2);
}

u32 __fastcall D3dProbe_DrawLines(u8 *self, void *, const float *points, s32 count, u32 colour, float scale) {
    u8 *const probe = self - 0x2c;
    clear(probe);
    u32 vertices[16 * 8];
    u32 *v = vertices;
    for (s32 i = 0; i < count; ++i, v += 8) {
        const float x = static_cast<float>(X87Reg(scale) * points[2 * i]), y = static_cast<float>(X87Reg(points[2 * i + 1]) * scale);
        std::memcpy(v, &x, 4);
        std::memcpy(v + 1, &y, 4);
        v[2] = 0x358637bd;   // 1e-6
        v[3] = 0x3f800000;
        v[4] = colour;
        v[5] = v[6] = v[7] = 0;
    }
    keep(comcall<u32>(device(probe), 0x70, 3u, 0x1c4u, vertices, count, 0u));
    return pushed(2);
}

u32 __fastcall D3dProbe_DrawFrame(u8 *self) {
    u8 *const probe = self - 0x2c;
    const double width = global<s32>(0x008bcbd0), height = global<s32>(0x008bcbd4);
    const u32 w = bitsOf(static_cast<float>(width)), h = bitsOf(static_cast<float>(height));
    const u32 innerW = bitsOf(static_cast<float>(X87Reg(width) - 5.0f)), innerH = bitsOf(static_cast<float>(X87Reg(height) - 5.0f));
    const bool flat = (renderCaps() & 0x8000000) != 0;
    const u32 outer = flat ? 0x80000000 : 0xff000000, inner = flat ? 0x80000000 : 0;
    const u32 z = 0x38d1b717, one = 0x3f800000, five = 0x40a00000;   // 0.0001, 1.0, 5.0
    const u32 vertices[64] = {0, 0, z, one, outer, 0, 0, 0,
                              five, five, z, one, inner, 0, 0, 0,
                              innerW, five, z, one, inner, 0, 0, 0,
                              w, 0, z, one, outer, 0, 0, 0,
                              0, h, z, one, outer, 0, 0, 0,
                              five, innerH, z, one, inner, 0, 0, 0,
                              innerW, innerH, z, one, inner, 0, 0, 0,
                              w, h, z, one, outer, 0, 0, 0};
    static const u16 kIndices[24] = {0, 3, 2, 0, 2, 1, 2, 3, 7, 2, 7, 6, 5, 6, 7, 5, 7, 4, 0, 1, 5, 0, 5, 4};
    clear(probe);
    vcall<u32>(base(probe), 8, 0x9d56u);
    keep(drawIndexed(probe, 4, vertices, 8, kIndices, 24, 0));
    pushed(2);
    return 1;
}

u32 __fastcall D3dProbe_DrawBox(u8 *self, void *, const u32 *corners, u32, u32 flags) {
    u8 *const probe = self - 0x2c;
    u32 vertices[64] = {};
    for (u32 i = 0; i < 8; ++i) {
        std::memcpy(vertices + 8 * i, corners + 4 * i, 16);
        vertices[8 * i + 4] = 0xffffffff;
    }
    // The corners of each of the six sides.
    static const u8 kSides[6][4] = {{6, 4, 2, 0}, {7, 5, 3, 1}, {3, 2, 1, 0}, {7, 6, 5, 4}, {7, 6, 3, 2}, {5, 4, 1, 0}};
    for (u32 side = 0; side < 6; ++side)
        if (flags & 1u << side)
            for (const u8 corner : kSides[side]) vertices[8 * corner + 4] = 0xffff0000;
    u8 *const transform = global<u8 *>(0x0087b1d8);
    (void)reinterpret_cast<float(__cdecl *)(u8 *, u32 *, u32, u32, u32)>(at<uintptr_t>(at<u8 *>(transform, 0), 0xc))(transform, vertices, 0x20, 8, 0);
    clear(probe);
    vcall<u32>(base(probe), 8, 0x56u);
    if (shadow(probe) & 0x100) {
        renderState(probe, 0x1b, 0);
        shadow(probe) &= ~0x100u;
    }
    if (shadow(probe) & 0x8000) {
        renderState(probe, 7, 1);      // ZENABLE
        shadow(probe) &= ~0x8000u;
    }
    if (shadow(probe) & 0x800) {
        renderState(probe, 0xe, 1);    // ZWRITEENABLE
        shadow(probe) &= ~0x800u;
    }
    if (shadow(probe) & 0x80000) {
        renderState(probe, 0x1d, 0);   // SPECULARENABLE
        shadow(probe) &= ~0x80000u;
    }
    if (shadow(probe) & 0x100000) {
        renderState(probe, 0x1c, 0);   // FOGENABLE
        shadow(probe) &= ~0x100000u;
    }
    static const u16 kIndices[16] = {0, 2, 6, 4, 0, 1, 5, 4, 6, 7, 5, 1, 3, 2, 3, 7};
    keep(drawIndexed(probe, 3, vertices, 8, kIndices, 16, 9));
    return 1;
}

u32 __fastcall D3dProbe_DrawBatchTinted(u8 *probe, void *, u8 *batch) {
    if (!(at<u8>(probe, 4) & 1) && !(at<u32>(batch, 4) & 0x100)) return 1;
    u8 *vertices = reinterpret_cast<u8 *>((uintptr_t)DrawBuffer_Vertices(global<u8 *>(0x008bcbc4)));
    const u32 fvf = at<u32>(batch, 0x1c);
    const u32 stride = fvf == 0x1c4 ? 0x20 : fvf == 0xc4 ? 0x18 : fvf == 0x2c4 ? 0x28 : fvf == 0x3c4 ? 0x30 : 0;
    const u8 *source = at<u8 *>(batch, 0x28);
    if (vertices == source) vertices += at<u32>(batch, 0x20) * stride;
    const u32 size = stride * at<u32>(batch, 0x20);
    for (u32 i = 0; i < size >> 2; ++i) at<u32>(vertices, 4 * i) = at<u32>(source, 4 * i);   // (forward, as rep movs: the two may overlap; the size is always whole dwords)
    for (s32 i = 0; i < at<s32>(batch, 0x20); ++i) at<u32>(vertices, static_cast<u32>(i) * stride + 0x10) = 0xffff0000;
    if (!(shadow(probe) & 0x100)) {
        renderState(probe, 0x1b, 1);
        shadow(probe) |= 0x100;
    }
    const u32 flags = at<u32>(batch, 4);
    const u32 index = at<u32>(probe, 8);
    const auto blended = [&] {
        if (!(shadow(probe) & 0x400)) {
            renderState(probe, 0xf, 1);
            shadow(probe) |= 0x400;
        }
        const u32 reference = global<u32>(0x007012dc + index * 4);
        if (!(shadow(probe) & 0x200)) {
            renderState(probe, 0x18, reference);
            shadow(probe) |= 0x200;
        }
    };
    const auto opaqueBlend = [&] {
        if ((shadow(probe) & 0xff) != 0x91) {
            renderState(probe, 0x13, 9);
            renderState(probe, 0x14, 1);
            shadow(probe) = (shadow(probe) & ~0xffu) | 0x91;
        }
    };
    const auto drawn = [&] {
        keep(comcall<u32>(device(probe), 0x74, at<u32>(batch, 0x18), fvf, vertices, at<u32>(batch, 0x20), at<u32>(batch, 0x2c), at<u32>(batch, 0x24), 0xcu));
    };
    if ((flags & 0x556) == 0x556) {
        blended();
        vcall<u32>(base(probe), 0xc, 0u, at<u32>(batch, 8));
        opaqueBlend();
        stageState(probe, 0, 1, 6);
        stageState(probe, 0, 2, 0x22);
        const s32 r = global<s32>(0x007012d8 + index * 4);
        renderState(probe, 0x18, static_cast<u32>(((r * 2) >> 2) + r));
        drawn();
        stageState(probe, 0, 1, 4);
        stageState(probe, 0, 2, 2);
        const u32 reference = global<u32>(0x007012d8 + index * 4);
        if (shadow(probe) & 0x200) {
            renderState(probe, 0x18, reference);
            shadow(probe) &= ~0x200u;
        }
        return 1;
    }
    // (the original's next test, of 0x2756, can never hold here: it includes all of 0x556)
    if ((flags & 0xff) == 0x22 || (flags & 0xff) == 0x52) return 1;
    vcall<u32>(base(probe), 0xc, 0u, 0u);
    opaqueBlend();
    drawn();
    return 1;
}

float __fastcall D3dProbe_FillRate(u8 *self) {
    u8 *const probe = self - 0x2c;
    const s32 width = global<s32>(0x008bcbd0), height = global<s32>(0x008bcbd4);
    const s32 pixels = static_cast<s32>(static_cast<u32>(height) * static_cast<u32>(width) * 50);
    const u32 w = bitsOf(static_cast<float>(width)), h = bitsOf(static_cast<float>(height));
    const u32 z = 0x3f000000, one = 0x3f800000;   // 0.5, 1.0
    const u32 vertices[32] = {0, 0, z, one, 0xffffffff, 0, 0, 0, w, 0, z, one, 0xffff0000, 0, 0, 0, w, h, z, one, 0xff00ff00, 0, 0, 0, 0, h, z, one, 0xff0000ff, 0, 0, 0};
    u16 indices[48];
    for (u32 i = 0; i < 48; ++i) indices[i] = kQuad[i % 6];   // (the original builds twice these and draws these)
    clear(probe);
    if (!vcall<u32>(base(probe), 0x1c)) return 0.0f;
    vcall<u32>(base(probe), 8, 0x9256u);
    const u32 begun = g_env->ticks();
    for (u32 i = 0; i < 3; ++i) {
        drawIndexed(probe, 4, vertices, 4, indices, 48, 0);
        if (result() != 0) {
            vcall<u32>(base(probe), 0x20);
            return 0.0f;
        }
    }
    const u32 taken = g_env->ticks() - begun;
    if (!vcall<u32>(base(probe), 0x20)) return 0.0f;
    const s32 scaled = static_cast<s32>(taken << 4);
    float rate;
    __asm {
        fild pixels
        fidiv scaled
        fstp rate
    }
    return rate;
}

}  // extern "C"
