// The JPEG image codec (vtable 0x700140; 0x5fc740-0x5fcc4f, thiscall): reading a JPEG from a
// stream with the statically linked IJG libjpeg 6b (booleans are bytes), one scanline at a time
// into a row bitmap that is then copied into the destination.
//
// The originals' quirks are kept: the destination's palette flag picks the path (an 8-bit
// quantized decode whose colour map becomes the codec's palette, else RGB888); a scanline at or
// past the destination's height (the RGB path: its width) is not read, so the decode never ends
// (the scanline count stops moving); GetInfo always describes RGB888 with a pitch of three bytes a
// pixel; failures inside libjpeg go to its error manager (which does not return).
#pragma once

#include "../../../re/types.h"
#include "surface.h"

// The parts of IJG libjpeg 6b's jpeg_decompress_struct (0x1b0 bytes) the codec touches.
struct JpegDecompress {
    void *err;                 // jpeg_error_mgr *
    u8 unknown04[0x18];
    u32 imageWidth;            // +0x1c
    u32 imageHeight;           // +0x20
    u8 unknown24[0x26];
    u8 quantizeColors;         // +0x4a
    u8 unknown4b[0x11];
    u32 outputWidth;           // +0x5c
    u32 outputHeight;          // +0x60
    u8 unknown64[0xc];
    u32 actualNumberOfColors;  // +0x70
    u8 **colormap;             // +0x74: [3][colours]
    u32 outputScanline;        // +0x78
    u8 unknown7c[0x134];
};
static_assert(sizeof(JpegDecompress) == 0x1b0, "JpegDecompress is 0x1b0 bytes");

struct PaletteEntry {
    u8 red, green, blue, flags;
};
struct JpegCodec {
    u32 vtable;                 // 0x700140: +4 GetInfo, +8 Load, +0xc IsJpeg
    SurfaceInfo info;           // +4
    PaletteEntry palette[256];  // +0x2c
};
static_assert(sizeof(JpegCodec) == 0x42c, "JpegCodec is 0x42c bytes");

// A readable stream: only its seek (vtable +0x10, thiscall (offset, origin), nonzero on failure).
struct JpegStream {
    void *const *vtable;
};

struct JpegEnv {
    void *(__cdecl *stdError)(void *err);                       // 0x631280 jpeg_std_error
    void(__cdecl *create)(JpegDecompress *, s32 version, u32 size);  // 0x630ca0 jpeg_CreateDecompress
    void(__cdecl *source)(JpegDecompress *, JpegStream *);     // 0x631140: the stream as the source
    s32(__cdecl *readHeader)(JpegDecompress *, u8 requireImage);  // 0x630d90 jpeg_read_header
    void(__cdecl *destroy)(JpegDecompress *);                   // 0x630d80 jpeg_destroy_decompress
    u8(__cdecl *start)(JpegDecompress *);                       // 0x631460 jpeg_start_decompress
    u32(__cdecl *readScanlines)(JpegDecompress *, u8 **rows, u32 count);  // 0x631610 jpeg_read_scanlines
    u8(__cdecl *finish)(JpegDecompress *);                      // 0x631080 jpeg_finish_decompress
    const u32 *pal8;  // 0xfaa298: the 8-bit paletted format
    SurfaceInfo *(__fastcall *infoCtor)(SurfaceInfo *);         // 0x5fa680
    SurfaceInfo *(__fastcall *infoInit)(SurfaceInfo *, void *, const u32 *size, const u32 *format, void *bits,
                                        s32 pitch);  // 0x5fa6a0
    s32 *(__fastcall *infoAlloc)(SurfaceInfo *, void *, s32 *out);  // 0x5fa6f0
    void(__fastcall *infoFree)(SurfaceInfo *);                       // 0x5fa740
    s32 *(__fastcall *subRectAt)(const SurfaceInfo *, void *, s32 *out, SurfaceInfo *dst, const s32 *at,
                                 const s32 *size);  // 0x5fa760
    s32 *(__fastcall *convert)(void *, void *, s32 *out, SurfaceInfo *dst, const SurfaceInfo *src,
                               const u32 *palette);  // 0x5fc360
};
// nullptr restores the game's.
void Jpeg_SetEnv(const JpegEnv *e);

extern "C" {
// 0x5fc740 (&out, stream, &info): the image's size and RGB888 format (not its bits); -1 when the
// stream does not seek.
s32 *__fastcall JpegCodec_GetInfo(JpegCodec *c, void *edx, s32 *out, JpegStream *stream, SurfaceInfo *info);
// 0x5fc840 (&out, dst, stream, unused): the image decoded into dst, from its top left.
s32 *__fastcall JpegCodec_Load(JpegCodec *c, void *edx, s32 *out, SurfaceInfo *dst, JpegStream *stream, u32);
// 0x5fcbc0 (stream): the header reads as a JPEG's.
u32 __fastcall JpegCodec_IsJpeg(JpegCodec *c, void *edx, JpegStream *stream);
}
