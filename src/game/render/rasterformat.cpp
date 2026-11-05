// The software rasteriser's pixel format and buffers: see rasterformat.h.
#include "rasterformat.h"

#include <cstdint>

#include "../core/crt.h"

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
    global<u32>(0x00fbe054) = redBits;
    const u32 greenBits = g_env->bitCount(green);
    global<u32>(0x00fbe058) = greenBits;
    const u32 blueBits = g_env->bitCount(blue);
    global<u32>(0x00fbe05c) = blueBits;
    const u32 redLow = g_env->lowBit(red);
    global<u32>(0x00fbe048) = redLow;
    const u32 greenLow = g_env->lowBit(green);
    global<u32>(0x00fbe04c) = greenLow;
    const u32 blueLow = g_env->lowBit(blue);
    global<u32>(0x00fbe050) = blueLow;
    global<u32>(0x00fbe064) = red;
    global<u32>(0x00fbe068) = ~red;
    global<u32>(0x00fbe06c) = green;
    global<u32>(0x00fbe070) = ~green;
    global<u32>(0x00fbe074) = blue;
    global<u32>(0x00fbe078) = ~blue;
    global<u32>(0x00fbe060) = redBits + greenBits + blueBits;
    const u32 both = green | blue;
    const u32 all = red | both;
    const u32 halves = (all << 16) | all;
    global<u32>(0x00fbe094) = both;
    global<u32>(0x00fbe098) = ~both;
    global<u32>(0x00fbe07c) = halves;
    global<u32>(0x00fbe090) = ~halves;
    const OutputDebugStringFn print = *reinterpret_cast<const OutputDebugStringFn *>((uintptr_t)0x006fd1e0);
    char text[0x74];
    Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)0x0077e5ec), redBits, greenBits, blueBits);
    print(text);
    Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)0x0077e5cc), redLow, greenLow, blueLow);
    print(text);
    for (int n = 1; n <= 4; ++n)
        global<u32>(0x00fbe07c + 4 * n) = spread(static_cast<s32>(red), static_cast<s32>(green), static_cast<s32>(blue), n);
    const bool fiveGreen = greenBits == 5;
    global<u32>(0x00fbe1a8) = fiveGreen ? 6 : 5;
    global<u32>(0x00fbe190) = global<u32>(0x00fbe194) = fiveGreen ? 0x20000008 : 0x40000008;
    global<u32>(0x00fbe198) = global<u32>(0x00fbe19c) = 0xf800f8;
    global<u32>(0x00fbe1a0) = global<u32>(0x00fbe1a4) = fiveGreen ? 0xf800 : 0xfc00;
    global<u32>(0x00fbe1ac) = 0;
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
    u32 *const shares = reinterpret_cast<u32 *>((uintptr_t)0x00fbe2e0);
    u32 *const rests = reinterpret_cast<u32 *>((uintptr_t)0x00fbe4e0);
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

u32 __cdecl RasterFormat_FrameBuffer(u32 width, u32 height) { return alloc_once(reinterpret_cast<u32 *>((uintptr_t)0x00fac52c), width * height * 4 + 0x6423, 0x3220); }

u32 __cdecl RasterFormat_ZBuffer(u32 width, u32 height) { return alloc_once(reinterpret_cast<u32 *>((uintptr_t)0x00fac534), width * height * 2 + 0x3223, 0x1920); }

static void free_once(u32 *block) {
    if (!*block) return;
    Crt().free(reinterpret_cast<void *>((uintptr_t)*block));
    block[0] = 0;
    block[1] = 0;
}

void __cdecl RasterFormat_FreeFrameBuffer() { free_once(reinterpret_cast<u32 *>((uintptr_t)0x00fac52c)); }

void __cdecl RasterFormat_FreeZBuffer() { free_once(reinterpret_cast<u32 *>((uintptr_t)0x00fac534)); }

void __cdecl RasterFormat_FreeBuffers() {
    RasterFormat_FreeZBuffer();
    RasterFormat_FreeFrameBuffer();
    --*reinterpret_cast<s32 *>((uintptr_t)0x00fac514);
}

extern "C" u32 __cdecl RasterFormat_Init(u8 *surface, u32 *zBufferOut, u32 width, u32 height) {
    if (surface) {
        u32 format[8] = {0x20, 0, 0, 0, 0, 0, 0, 0};
        const GetPixelFormat get = (*reinterpret_cast<GetPixelFormat *const *>(surface))[21];
        get(surface, format);
        if (format[3] != 8) setFormat(format[4], format[5], format[6]);
    }
    global<u32>(0x00fbe008) = 0xffff;
    g_env->blendTables();
    ++global<u32>(0x00fac514);
    if (zBufferOut) *zBufferOut = g_env->zBuffer(width, height);
    return g_env->frameBuffer(width, height);
}
