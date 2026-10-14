// The JPEG image codec: see jpeg.h.
#include "jpeg.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const JpegEnv kGameEnv = {
    at<void *(__cdecl *)(void *)>(0x00631280),
    at<void(__cdecl *)(JpegDecompress *, s32, u32)>(0x00630ca0),
    at<void(__cdecl *)(JpegDecompress *, JpegStream *)>(0x00631140),
    at<s32(__cdecl *)(JpegDecompress *, u8)>(0x00630d90),
    at<void(__cdecl *)(JpegDecompress *)>(0x00630d80),
    at<u8(__cdecl *)(JpegDecompress *)>(0x00631460),
    at<u32(__cdecl *)(JpegDecompress *, u8 **, u32)>(0x00631610),
    at<u8(__cdecl *)(JpegDecompress *)>(0x00631080),
    at<const u32 *>(0x00faa298),
    at<SurfaceInfo *(__fastcall *)(SurfaceInfo *)>(0x005fa680),
    at<SurfaceInfo *(__fastcall *)(SurfaceInfo *, void *, const u32 *, const u32 *, void *, s32)>(0x005fa6a0),
    at<s32 *(__fastcall *)(SurfaceInfo *, void *, s32 *)>(0x005fa6f0),
    at<void(__fastcall *)(SurfaceInfo *)>(0x005fa740),
    at<s32 *(__fastcall *)(const SurfaceInfo *, void *, s32 *, SurfaceInfo *, const s32 *, const s32 *)>(0x005fa760),
    at<s32 *(__fastcall *)(void *, void *, s32 *, SurfaceInfo *, const SurfaceInfo *, const u32 *)>(0x005fc360),
};
const JpegEnv *g_env = &kGameEnv;

const s32 kJpegLibVersion = 62;
const u32 kErrorManagerSize = 0x84;

// The stream's seek to its start: nonzero when it fails.
s32 rewind(JpegStream *s) {
    return reinterpret_cast<s32(__fastcall *)(JpegStream *, void *, s32, s32)>(s->vtable[4])(s, nullptr, 0, 0);
}

void open(JpegDecompress *ci, void *err, JpegStream *s) {
    ci->err = g_env->stdError(err);
    g_env->create(ci, kJpegLibVersion, sizeof *ci);
    g_env->source(ci, s);
}

// A scanline the destination has no row for: the original's loop goes round without reading one.
[[noreturn]] void spin() {
    volatile u32 never = 0;
    for (;;) never = never;
}

}  // namespace

void Jpeg_SetEnv(const JpegEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

s32 *__fastcall JpegCodec_GetInfo(JpegCodec *, void *, s32 *out, JpegStream *stream, SurfaceInfo *info) {
    if (rewind(stream)) {
        *out = -1;
        return out;
    }
    u8 err[kErrorManagerSize];
    JpegDecompress ci;
    open(&ci, err, stream);
    g_env->readHeader(&ci, 1);
    g_env->destroy(&ci);
    info->redMask = 0xff;
    info->greenMask = 0xff00;
    info->blueMask = 0xff0000;
    info->alphaMask = 0;
    info->bitCount = 24;
    info->paletted = 0;
    info->height = ci.imageHeight;
    info->width = ci.imageWidth;
    info->pitch = ci.imageWidth * 3;
    *out = 0;
    return out;
}

s32 *__fastcall JpegCodec_Load(JpegCodec *c, void *, s32 *out, SurfaceInfo *dst, JpegStream *stream, u32) {
    if (rewind(stream)) {
        *out = -1;
        return out;
    }
    u8 err[kErrorManagerSize];
    JpegDecompress ci;
    open(&ci, err, stream);
    g_env->readHeader(&ci, 1);
    u8 self;  // the copy's (empty) object
    SurfaceInfo row, sub;
    s32 result, place[2], size[2];
    if (dst->paletted) {
        ci.quantizeColors = 1;
        g_env->start(&ci);
        const u32 rowSize[2] = {ci.outputWidth, 1};
        g_env->infoInit(&row, nullptr, rowSize, g_env->pal8, nullptr, 0);
        g_env->infoAlloc(&row, nullptr, &result);
        u8 *rowBits = static_cast<u8 *>(row.bits);
        for (u32 line = ci.outputScanline; line < ci.outputHeight; line = ci.outputScanline) {
            if (line >= dst->height) spin();
            g_env->readScanlines(&ci, &rowBits, 1);
            g_env->infoCtor(&sub);
            size[0] = dst->width;
            place[1] = ci.outputScanline - 1;
            place[0] = 0;
            size[1] = 1;
            g_env->subRectAt(dst, nullptr, &size[1], &sub, place, size);  // the result over the height (sic)
            g_env->convert(&self, nullptr, &result, &sub, &row, nullptr);
        }
        g_env->infoFree(&row);
        const s32 colours = (s32)ci.actualNumberOfColors;
        u8 **const map = ci.colormap;
        for (s32 i = 0; i < colours; ++i) {
            c->palette[i].red = map[0][i];
            c->palette[i].green = map[1][i];
            c->palette[i].blue = map[2][i];
        }
    } else {
        g_env->start(&ci);
        const u32 rowSize[2] = {ci.outputWidth, 1};
        const u32 rgb888[6] = {0xff, 0xff00, 0xff0000, 0, 24, 0};
        g_env->infoInit(&row, nullptr, rowSize, rgb888, nullptr, 0);
        g_env->infoAlloc(&row, nullptr, &result);
        u8 *rowBits = static_cast<u8 *>(row.bits);
        for (u32 line = ci.outputScanline; line < ci.outputHeight; line = ci.outputScanline) {
            if (line >= dst->width) spin();  // the width (sic)
            g_env->readScanlines(&ci, &rowBits, 1);
            g_env->infoCtor(&sub);
            size[0] = dst->width;
            place[1] = ci.outputScanline - 1;
            size[1] = 1;
            place[0] = 0;
            g_env->subRectAt(dst, nullptr, &result, &sub, place, size);
            g_env->convert(&self, nullptr, &place[1], &sub, &row, nullptr);  // the result over the place (sic)
        }
        g_env->infoFree(&row);
    }
    g_env->finish(&ci);
    g_env->destroy(&ci);
    *out = 0;
    return out;
}

u32 __fastcall JpegCodec_IsJpeg(JpegCodec *, void *, JpegStream *stream) {
    if (rewind(stream)) return 0;
    u8 err[kErrorManagerSize];
    JpegDecompress ci;
    open(&ci, err, stream);
    const s32 header = g_env->readHeader(&ci, 1);
    g_env->destroy(&ci);
    return header == 1;  // JPEG_HEADER_OK
}

}  // extern "C"
