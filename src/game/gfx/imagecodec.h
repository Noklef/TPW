// The image codecs' shared loaders and the TGA codec (0x5fa8a0-0x5fad34, thiscall). A codec
// (vtable +0 deleting destructor 0x47e750, +4 GetInfo, +8 Load, +0xc Is<format>; each from a
// stream) keeps the last image's description at +4; these read one from a file or a stream into
// it, and hand a copy back. The TGA codec (vtable 0x6fefa8) does its work with a reader object
// (vtable 0x703630) made on the stack.
//
// The originals' quirks are kept: Load only reads through a 0x400-byte buffered stream, set up
// without checking that it worked, and refuses a destination without bits; the results of
// opening go through the argument slots (here, locals); TGA's GetInfo and Load hand the reader
// the codec itself in place of their last argument, and the reader is never destroyed; IsTga
// takes the TGA 2.0 footer ("TRUEVISION-XFILE.") as proof, else checks the header's colour map
// type (0, 1), image type (0-3, 9-11) and bits a pixel (8, 15, 16, 24, 32).
#pragma once

#include "../../../re/types.h"
#include "../files/handles.h"
#include "surface.h"

struct ImageCodec {
    void *const *vtable;
    SurfaceInfo info;  // +4: the last image read
};

struct TgaReader {  // 0x44 bytes, of which the codec only sets the vtable
    u32 vtable;     // 0x703630
    u8 unknown04[0x40];
};

struct ImageCodecEnv {
    StreamBase *(__fastcall *streamCtor)(StreamBase *);  // 0x5f7180
    void(__fastcall *streamDtor)(StreamBase *);           // 0x5f71e0
    s32(__fastcall *streamSeek)(StreamBase *, void *, s32 offset, u32 origin);  // 0x5f7210
    s32 *(__fastcall *openFile)(StreamBase *, void *, s32 *out, const char *path, u32 mode);  // 0x62fa40
    s32 *(__fastcall *openBuffered)(StreamBase *, void *, s32 *out, StreamBase *source, u32 size);  // 0x62fd30
    u32 fileVtable;      // 0x7035e0
    u32 bufferedVtable;  // 0x703608
    u32 tgaVtable;       // 0x703630
    s32 *(__fastcall *tgaInfo)(TgaReader *, void *, s32 *out, StreamBase *stream, ImageCodec *codec);  // 0x630150
    s32 *(__fastcall *tgaLoad)(TgaReader *, void *, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                               ImageCodec *codec);  // 0x6300f0
    s32(__cdecl *strncmp)(const char *, const char *, u32);  // 0x67c250
    const char *footer;  // 0x775860 "TRUEVISION-XFILE."
};
// nullptr restores the game's.
void ImageCodec_SetEnv(const ImageCodecEnv *e);

extern "C" {
// 0x5fa8a0 (&out, path, mode, &info or 0): InfoFromStream on the file; -1 when it does not open.
s32 *__fastcall ImageCodec_InfoFromFile(ImageCodec *c, void *edx, s32 *out, const char *path, u32 mode,
                                        SurfaceInfo *info);
// 0x5fa970 (&out, stream, &info or 0): the stream rewound, GetInfo into c->info, copied to info.
s32 *__fastcall ImageCodec_InfoFromStream(ImageCodec *c, void *edx, s32 *out, StreamBase *stream, SurfaceInfo *info);
// 0x5faa00 (&out, dst, path, mode, &info or 0): LoadFromStream on the file.
s32 *__fastcall ImageCodec_LoadFromFile(ImageCodec *c, void *edx, s32 *out, SurfaceInfo *dst, const char *path,
                                        u32 mode, SurfaceInfo *info);
// 0x5faab0 (&out, dst, stream, &info or 0): Load into dst through a buffered stream.
s32 *__fastcall ImageCodec_LoadFromStream(ImageCodec *c, void *edx, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                                          SurfaceInfo *info);
// 0x5fabb0 (&out, stream, unused) / 0x5fabe0 (&out, dst, stream, unused): the TGA reader's.
s32 *__fastcall TgaCodec_GetInfo(ImageCodec *c, void *edx, s32 *out, StreamBase *stream, u32);
s32 *__fastcall TgaCodec_Load(ImageCodec *c, void *edx, s32 *out, SurfaceInfo *dst, StreamBase *stream, u32);
u32 __fastcall TgaCodec_IsTga(ImageCodec *c, void *edx, StreamBase *stream);  // 0x5fac10
}
