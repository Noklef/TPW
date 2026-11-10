// The software rasteriser's pixel format and buffers: see rasterformat.h.
#include "rasterformat.h"

#include <cstdint>

#include "../core/crt.h"
#include "rastertriangle.h"
#include "rasterspans.h"
#include "../core/imports.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const RasterFormatEnv kGameEnv = {game<u32(__cdecl *)(u32)>(0x00679150), game<u32(__cdecl *)(u32)>(0x00679170), game<void(__cdecl *)()>(0x00679190),
                                  game<u32(__cdecl *)(u32, u32)>(0x00679230), game<u32(__cdecl *)(u32, u32)>(0x006791f0)};
const RasterFormatEnv *g_env = &kGameEnv;

using GetPixelFormat = s32(__stdcall *)(u8 *surface, u32 *format);
using OutputDebugStringFn = void(__stdcall *)(const char *text);

u32 spread(s32 red, s32 green, s32 blue, int n) { return static_cast<u32>(((blue >> n) & blue) | ((green >> n) & green) | ((red >> n) & red)); }

void setFormat(u32 red, u32 green, u32 blue) {
    const u32 redBits = g_env->bitCount(red);
    gRasterformat_fbe054 = redBits;
    const u32 greenBits = g_env->bitCount(green);
    gRasterformat_fbe058 = greenBits;
    const u32 blueBits = g_env->bitCount(blue);
    gRasterformat_fbe05c = blueBits;
    const u32 redLow = g_env->lowBit(red);
    gRasterformat_fbe048 = redLow;
    const u32 greenLow = g_env->lowBit(green);
    gRasterformat_fbe04c = greenLow;
    const u32 blueLow = g_env->lowBit(blue);
    gRasterformat_fbe050 = blueLow;
    gRasterformat_fbe064 = red;
    gRasterformat_fbe068 = ~red;
    gRasterformat_fbe06c = green;
    gRasterformat_fbe070 = ~green;
    gRasterformat_fbe074 = blue;
    gRasterformat_fbe078 = ~blue;
    gRasterformat_fbe060 = redBits + greenBits + blueBits;
    const u32 both = green | blue;
    const u32 all = red | both;
    const u32 halves = (all << 16) | all;
    gRasterformat_fbe094 = both;
    gRasterformat_fbe098 = ~both;
    gRasterformat_fbe07c[0] = halves;
    gRasterformat_fbe090 = ~halves;
    const OutputDebugStringFn print = *reinterpret_cast<const OutputDebugStringFn *>((uintptr_t)IMPORT(Kernel32_OutputDebugStringA));
    char text[0x74];
    Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)&gText_Rasterformat_77e5ec), redBits, greenBits, blueBits);
    print(text);
    Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)&gText_Rasterformat_77e5cc), redLow, greenLow, blueLow);
    print(text);
    for (int n = 1; n <= 4; ++n)
        gRasterformat_fbe07c[n] = spread(static_cast<s32>(red), static_cast<s32>(green), static_cast<s32>(blue), n);
    const bool fiveGreen = greenBits == 5;
    global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x70) = fiveGreen ? 6 : 5;
    global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x58) = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x5c) = fiveGreen ? 0x20000008 : 0x40000008;
    global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x60) = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x64) = 0xf800f8;
    global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x68) = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x6c) = fiveGreen ? 0xf800 : 0xfc00;
    global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x74) = 0;
}

}  // namespace

void RasterFormat_SetEnv(const RasterFormatEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __cdecl RasterFormat_BitCount(u32 mask) {
    u32 count = 0;
    for (u32 bit = 0; bit < 0x1f; ++bit) {
        if (mask & 1u << bit) ++count;
    }
    return count;
}

u32 __cdecl RasterFormat_LowBit(u32 mask) {
    u32 position = 0;
    while (!(mask & 1)) {
        mask = static_cast<u32>(static_cast<s32>(mask) >> 1);
        ++position;
    }
    return position;
}

void __cdecl RasterFormat_BlendTables() {
    u32 *const shares = reinterpret_cast<u32 *>((uintptr_t)&gRastertriangle_fbe240 + 0xa0);
    u32 *const rests = reinterpret_cast<u32 *>((uintptr_t)&gRasterspans_fbe4e0);
    for (s32 i = 0; i < 0x40; ++i) {
        const s32 share = i * 0x10000 / 255;
        const s32 rest = 0x4000 - share;
        shares[i * 2] = shares[i * 2 + 1] = static_cast<u32>(share) << 16 | static_cast<u32>(share);
        rests[i * 2] = rests[i * 2 + 1] = static_cast<u32>(rest) << 16 | static_cast<u32>(rest);
    }
}

static u32 alloc_once(u32 *block, u32 size, u32 margin) {
    if (*block) return 0;
    *block = static_cast<u32>(reinterpret_cast<uintptr_t>(CRT_malloc(size)));
    const u32 aligned = (*block + margin) & ~0x1fu;
    block[1] = aligned;
    return aligned;
}

u32 __cdecl RasterFormat_FrameBuffer(u32 width, u32 height) { return alloc_once(&gRasterFormat_fac52c, width * height * 4 + 0x6423, 0x3220); }

u32 __cdecl RasterFormat_ZBuffer(u32 width, u32 height) { return alloc_once(&gRasterFormat_fac534, width * height * 2 + 0x3223, 0x1920); }

static void free_once(u32 *block) {
    if (!*block) return;
    Crt().free(reinterpret_cast<void *>((uintptr_t)*block));
    block[0] = 0;
    block[1] = 0;
}

void __cdecl RasterFormat_FreeFrameBuffer() { free_once(&gRasterFormat_fac52c); }

void __cdecl RasterFormat_FreeZBuffer() { free_once(&gRasterFormat_fac534); }

void __cdecl RasterFormat_FreeBuffers() {
    RasterFormat_FreeZBuffer();
    RasterFormat_FreeFrameBuffer();
    --*reinterpret_cast<s32 *>(&gRasterFormat_fac514);
}

extern "C" u32 __cdecl RasterFormat_Init(u8 *surface, u32 *zBufferOut, u32 width, u32 height) {
    if (surface) {
        u32 format[8] = {0x20, 0, 0, 0, 0, 0, 0, 0};
        const GetPixelFormat get = (*reinterpret_cast<GetPixelFormat *const *>(surface))[21];
        get(surface, format);
        if (format[3] != 8) setFormat(format[4], format[5], format[6]);
    }
    reinterpret_cast<u32 &>(gRasterformat_fbe008) = 0xffff;
    g_env->blendTables();
    ++gRasterFormat_fac514;
    if (zBufferOut) *zBufferOut = g_env->zBuffer(width, height);
    return g_env->frameBuffer(width, height);
}
