// The TGA reader (0x630150-0x6306b1, thiscall) and the row reader it derives from (0x62fe00,
// 0x6300c0). A row reader (vtable +0 Begin, +4 ReadRow, +8 RowBytes, +0xc CopyRow) decodes an
// image into a destination a row at a time: it reads one row ahead into a buffer, converts it to
// the destination's format if that differs (a PixelConversion on the codec's palette), and copies
// as much of each row as both widths allow, top down or bottom up. The TGA reader (vtable
// 0x703630, 0x44 bytes, made on the stack by the TGA codec) reads the header and the colour map
// into the codec, then raw or run-length rows, mirrored when the descriptor says right to left.
//
// The originals' quirks are kept:
// - Decode copies a first row even into a destination with no height, and a row that fails to
//   read ends the image without failing it. Begin's result fails it when nonzero, not negative.
// - A bottom-up image taller than the destination has its extra rows read first and dropped
//   (they are the bottom ones); a wider one is cut on the right.
// - ReadHeader skips a colour map it does not use by its length times its entry bits (not
//   bytes), as a 16-bit product; it ignores the seeks' and the map read's results; it accepts
//   image type 0 (no image data); 15-bit pixels are one byte each to the rows (bits / 8) though
//   the format is RGB555.
// - ReadRow takes one from the rows left even when there are none, so after a failed read past
//   the end the count has wrapped and later reads go on; the run-length rows are not bounded by
//   the row buffer (a packet past the width writes on), only ended once the width is reached.
// - The row mirror swaps by XOR, byte by byte.
#pragma once

#include "../../../re/types.h"
#include "../files/handles.h"
#include "pixconv.h"
#include "surface.h"

#pragma pack(push, 1)
struct TgaHeader {  // as in the file
    u8 idLength;
    u8 colorMapType;  // 0 none, 1 present
    u8 imageType;     // 1-3 raw (mapped, true colour, grey), 9-11 run-length
    u16 mapFirst;     // +3
    u16 mapLength;    // +5
    u8 mapEntryBits;  // +7
    u16 xOrigin, yOrigin;
    u16 width;     // +0xc
    u16 height;    // +0xe
    u8 bitCount;   // +0x10
    u8 descriptor; // +0x11: 0x10 right to left, 0x20 top down
};
#pragma pack(pop)
static_assert(sizeof(TgaHeader) == 0x12, "TgaHeader is 0x12 bytes");

struct RowReader {
    void *const *vtable;
    u8 *row;              // +4: the row read ahead (Mem_GlobalAlloc)
    u32 rowSize;          // +8
    u32 copyBytes;        // +0xc: of each row to the destination
    u32 sourceRowBytes;   // +0x10: the image's row in its own format (unused)
    u32 bottomUp;         // +0x14
    u8 *target;           // +0x18: the destination row the next copy goes to
    s32 step;             // +0x1c
};

struct TgaReader {
    RowReader base;
    u32 runLength;   // +0x20
    u32 mirrored;    // +0x24
    u32 rowsLeft;    // +0x28
    u32 rowBytes;    // +0x2c
    TgaHeader header;  // +0x30
    u8 pad42[2];
};
static_assert(sizeof(TgaReader) == 0x44, "TgaReader is 0x44 bytes");

struct TgaCodec {  // an ImageCodec (vtable 0x6fefa8, info) with a palette and the header read
    void *const *vtable;
    SurfaceInfo info;   // +4
    u32 palette[256];   // +0x2c: RGBX
    u32 hasPalette;     // +0x42c
    TgaHeader header;   // +0x430
};

struct TgaEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_GlobalAlloc 0x5f5660
    void *(__cdecl *free)(void *block);  // Mem_GlobalFree 0x5f5680
    void(__cdecl *expandRgb)(u8 *dst, const u8 *src, u32 count);  // Palette_ExpandRgb 0x5f7360
    PixelConversion *(__fastcall *convCtor)(PixelConversion *);  // 0x630880
    void(__fastcall *convSetup)(PixelConversion *, void *, const u32 *target, const u32 *source, const u32 *palette,
                                u32 flags);  // 0x630890
    void(__fastcall *convRow)(PixelConversion *, void *, u8 *dst, const u8 *src, u32 count);  // 0x630b70
    void(__fastcall *convDtor)(PixelConversion *);  // 0x630c30
    const u32 *pal8, *rgb555, *rgb888, *argb8888;  // 0xfaa298, 0xfaa2b0, 0xfaa268, 0xfaa280
};
// nullptr restores the game's.
void Tga_SetEnv(const TgaEnv *e);

extern "C" {
// 0x62fe00 (&out, dst, stream, codec): the image decoded into dst; -1 without a row buffer or when
// Begin or skipping fails.
s32 *__fastcall RowReader_Decode(RowReader *r, void *edx, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                                 TgaCodec *codec);
void __fastcall RowReader_CopyRow(RowReader *r);  // 0x6300c0 (vtable +0xc)
// 0x6300f0 (&out, dst, stream, codec): ReadHeader, then Decode.
s32 *__fastcall TgaReader_Load(TgaReader *r, void *edx, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                               TgaCodec *codec);
// 0x630150 (&out, stream, codec): the header and colour map into codec, its format into codec->info.
s32 *__fastcall TgaReader_ReadHeader(TgaReader *r, void *edx, s32 *out, StreamBase *stream, TgaCodec *codec);
s32 *__fastcall TgaReader_Begin(TgaReader *r, void *edx, s32 *out, StreamBase *stream);    // 0x630400 (+0)
s32 *__fastcall TgaReader_ReadRow(TgaReader *r, void *edx, s32 *out, StreamBase *stream);  // 0x630440 (+4)
s32 *__fastcall TgaReader_MirrorRow(TgaReader *r, void *edx, s32 *out, u8 *row);           // 0x630500
s32 *__fastcall TgaReader_ReadRunRow(TgaReader *r, void *edx, s32 *out, StreamBase *stream);  // 0x630580
u32 __fastcall TgaReader_RowBytes(TgaReader *r);  // 0x6306a0 (+8)
}
