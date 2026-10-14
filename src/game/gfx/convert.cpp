// Copying one bitmap's pixels into another: see convert.h.
#include "convert.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const ConvertEnv kGameEnv = {
    at<PixelConversion *>(0x00fa7d18),
    at<PixelConversion *(__fastcall *)(PixelConversion *)>(0x00630880),
    at<void(__fastcall *)(PixelConversion *)>(0x00630c30),
    at<void(__fastcall *)(PixelConversion *, void *, const u32 *, const u32 *, const u32 *, u32)>(0x00630890),
    at<void(__fastcall *)(PixelConversion *, void *, u8 *, const u8 *, s32, s32, u32, u32)>(0x00630ba0),
    at<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
    at<void(__cdecl *)()>(0x005fc350),
};
const ConvertEnv *g_env = &kGameEnv;

bool sameFormat(const SurfaceInfo *a, const SurfaceInfo *b) {
    return a->redMask == b->redMask && a->greenMask == b->greenMask && a->blueMask == b->blueMask &&
           a->alphaMask == b->alphaMask && a->bitCount == b->bitCount && !a->paletted == !b->paletted;
}

// rep movsd, rep movsb: forwards, a dword at a time.
void copyForwards(u8 *dst, const u8 *src, u32 n) {
    for (u32 i = n >> 2; i; --i, dst += 4, src += 4) {
        u32 d;
        std::memcpy(&d, src, 4);
        std::memcpy(dst, &d, 4);
    }
    for (u32 i = n & 3; i; --i) *dst++ = *src++;
}

}  // namespace

void Convert_SetEnv(const ConvertEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __cdecl Convert_StaticInit() {
    Convert_Construct();
    Convert_RegisterExit();
}

PixelConversion *__cdecl Convert_Construct() { return g_env->ctor(g_env->conversion); }

void __cdecl Convert_RegisterExit() { g_env->atExit(g_env->onExit); }

void __cdecl Convert_AtExit() { g_env->dtor(g_env->conversion); }

s32 *__fastcall SurfaceInfo_Convert(void *, void *, s32 *out, SurfaceInfo *dst, const SurfaceInfo *src,
                                    const u32 *palette) {
    if (!dst->bits || !src->bits) {
        *out = -1;
        return out;
    }
    if (sameFormat(src, dst)) {
        if (dst->bits != src->bits) {
            const u32 rowBytes = ((dst->bitCount + 7) >> 3) * (dst->width < src->width ? dst->width : src->width);
            u32 rows = dst->height < src->height ? dst->height : src->height;
            u8 *d = static_cast<u8 *>(dst->bits);
            const u8 *s = static_cast<const u8 *>(src->bits);
            for (; rows; --rows) {
                copyForwards(d, s, rowBytes);
                d += dst->pitch;
                s += src->pitch;
            }
        }
        *out = 0;
        return out;
    }
    const u32 srcBytes = (src->bitCount + 7) >> 3;
    const u32 dstBytes = (dst->bitCount + 7) >> 3;
    if (dst->bits == src->bits) {
        const u32 rowBytes = src->width * srcBytes;
        dst->width = ((u32)dst->pitch < rowBytes ? (u32)dst->pitch : rowBytes) / dstBytes;
    }
    const u32 width = dst->width < src->width ? dst->width : src->width;
    const u32 rows = dst->height < src->height ? dst->height : src->height;
    g_env->setup(g_env->conversion, nullptr, &dst->redMask, &src->redMask, palette, width);
    g_env->rect(g_env->conversion, nullptr, static_cast<u8 *>(dst->bits), static_cast<const u8 *>(src->bits),
                dst->pitch, src->pitch, rows, width);
    *out = 0;
    return out;
}

}  // extern "C"
