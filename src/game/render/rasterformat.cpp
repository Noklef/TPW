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
