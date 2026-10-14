// The image codecs' shared loaders and the TGA codec: see imagecodec.h.
#include "imagecodec.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const ImageCodecEnv kGameEnv = {
    at<StreamBase *(__fastcall *)(StreamBase *)>(0x005f7180),
    at<void(__fastcall *)(StreamBase *)>(0x005f71e0),
    at<s32(__fastcall *)(StreamBase *, void *, s32, u32)>(0x005f7210),
    at<s32 *(__fastcall *)(StreamBase *, void *, s32 *, const char *, u32)>(0x0062fa40),
    at<s32 *(__fastcall *)(StreamBase *, void *, s32 *, StreamBase *, u32)>(0x0062fd30),
    0x007035e0,
    0x00703608,
    0x00703630,
    at<s32 *(__fastcall *)(TgaReader *, void *, s32 *, StreamBase *, ImageCodec *)>(0x00630150),
    at<s32 *(__fastcall *)(TgaReader *, void *, s32 *, SurfaceInfo *, StreamBase *, ImageCodec *)>(0x006300f0),
    at<s32(__cdecl *)(const char *, const char *, u32)>(0x0067c250),
    at<const char *>(0x00775860),
};
const ImageCodecEnv *g_env = &kGameEnv;

const u32 kTgaHeaderSize = 0x12;  // and the footer's

// The codec's own and the stream's virtual calls.
s32 *getInfo(ImageCodec *c, s32 *out, StreamBase *stream) {
    return reinterpret_cast<s32 *(__fastcall *)(ImageCodec *, void *, s32 *, StreamBase *, SurfaceInfo *)>(
        c->vtable[1])(c, nullptr, out, stream, &c->info);
}
s32 *load(ImageCodec *c, s32 *out, SurfaceInfo *dst, StreamBase *stream) {
    return reinterpret_cast<s32 *(__fastcall *)(ImageCodec *, void *, s32 *, SurfaceInfo *, StreamBase *,
                                                 SurfaceInfo *)>(c->vtable[2])(c, nullptr, out, dst, stream,
                                                                               &c->info);
}
u32 streamRead(StreamBase *s, void *buffer, u32 size) {
    return reinterpret_cast<u32(__fastcall *)(StreamBase *, void *, void *, u32)>(s->vtable[2])(s, nullptr, buffer,
                                                                                               size);
}
s32 streamSeek(StreamBase *s, s32 offset, u32 origin) {
    return reinterpret_cast<s32(__fastcall *)(StreamBase *, void *, s32, u32)>(s->vtable[4])(s, nullptr, offset,
                                                                                            origin);
}
s32 streamSize(StreamBase *s) { return reinterpret_cast<s32(__fastcall *)(StreamBase *)>(s->vtable[5])(s); }

void copyInfo(SurfaceInfo *to, const SurfaceInfo *from) {
    if (to) *to = *from;
}

}  // namespace

void ImageCodec_SetEnv(const ImageCodecEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

s32 *__fastcall ImageCodec_InfoFromFile(ImageCodec *c, void *, s32 *out, const char *path, u32 mode,
                                        SurfaceInfo *info) {
    StreamBase file;
    g_env->streamCtor(&file);
    file.vtable = at<void *const *>(g_env->fileVtable);
    s32 opened;
    if (*g_env->openFile(&file, nullptr, &opened, path, mode) < 0)
        *out = -1;
    else
        ImageCodec_InfoFromStream(c, nullptr, out, &file, info);
    g_env->streamDtor(&file);
    return out;
}

s32 *__fastcall ImageCodec_InfoFromStream(ImageCodec *c, void *, s32 *out, StreamBase *stream, SurfaceInfo *info) {
    s32 result;
    if (streamSeek(stream, 0, 0) || *getInfo(c, &result, stream) < 0) {
        *out = -1;
        return out;
    }
    copyInfo(info, &c->info);
    *out = 0;
    return out;
}

s32 *__fastcall ImageCodec_LoadFromFile(ImageCodec *c, void *, s32 *out, SurfaceInfo *dst, const char *path,
                                        u32 mode, SurfaceInfo *info) {
    StreamBase file;
    g_env->streamCtor(&file);
    file.vtable = at<void *const *>(g_env->fileVtable);
    s32 opened;
    if (*g_env->openFile(&file, nullptr, &opened, path, mode) < 0)
        *out = -1;
    else
        ImageCodec_LoadFromStream(c, nullptr, out, dst, &file, info);
    g_env->streamDtor(&file);
    return out;
}

s32 *__fastcall ImageCodec_LoadFromStream(ImageCodec *c, void *, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                                          SurfaceInfo *info) {
    StreamBase buffered;
    g_env->streamCtor(&buffered);
    buffered.vtable = at<void *const *>(g_env->bufferedVtable);
    s32 result;
    g_env->openBuffered(&buffered, nullptr, &result, stream, 0x400);
    if (!g_env->streamSeek(&buffered, nullptr, 0, 0) && dst->bits && *load(c, &result, dst, &buffered) >= 0) {
        copyInfo(info, &c->info);
        *out = 0;
    } else {
        *out = -1;
    }
    g_env->streamDtor(&buffered);
    return out;
}

s32 *__fastcall TgaCodec_GetInfo(ImageCodec *c, void *, s32 *out, StreamBase *stream, u32) {
    TgaReader reader;
    reader.vtable = g_env->tgaVtable;
    g_env->tgaInfo(&reader, nullptr, out, stream, c);
    return out;
}

s32 *__fastcall TgaCodec_Load(ImageCodec *c, void *, s32 *out, SurfaceInfo *dst, StreamBase *stream, u32) {
    TgaReader reader;
    reader.vtable = g_env->tgaVtable;
    g_env->tgaLoad(&reader, nullptr, out, dst, stream, c);
    return out;
}

u32 __fastcall TgaCodec_IsTga(ImageCodec *, void *, StreamBase *stream) {
    if (streamSize(stream) < (s32)kTgaHeaderSize) return 0;
    const s32 footerAt = streamSize(stream) - (s32)kTgaHeaderSize;
    if (streamSeek(stream, -(s32)kTgaHeaderSize, 2) != footerAt) return 0;
    u8 header[kTgaHeaderSize], footer[kTgaHeaderSize];
    if (streamRead(stream, footer, kTgaHeaderSize) != kTgaHeaderSize) return 0;
    if (!g_env->strncmp(reinterpret_cast<const char *>(footer), g_env->footer, kTgaHeaderSize)) return 1;
    if (streamSeek(stream, 0, 0)) return 0;
    if (streamRead(stream, header, kTgaHeaderSize) != kTgaHeaderSize) return 0;
    const u8 mapType = header[1], imageType = header[2], bits = header[16];
    const u32 mapOk = mapType == 0 || mapType == 1;
    const u32 typeOk = imageType <= 3 || (imageType >= 9 && imageType <= 11);
    const u32 bitsOk = bits == 8 || bits == 15 || bits == 16 || bits == 24 || bits == 32;
    return mapOk & typeOk & bitsOk;
}

}  // extern "C"
