// The TGA reader and the row reader it derives from: see tga.h.
#include "tga.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const TgaEnv kGameEnv = {
    at<void *(__cdecl *)(u32)>(0x005f5660),
    at<void *(__cdecl *)(void *)>(0x005f5680),
    at<void(__cdecl *)(u8 *, const u8 *, u32)>(0x005f7360),
    at<PixelConversion *(__fastcall *)(PixelConversion *)>(0x00630880),
    at<void(__fastcall *)(PixelConversion *, void *, const u32 *, const u32 *, const u32 *, u32)>(0x00630890),
    at<void(__fastcall *)(PixelConversion *, void *, u8 *, const u8 *, u32)>(0x00630b70),
    at<void(__fastcall *)(PixelConversion *)>(0x00630c30),
    at<const u32 *>(0x00faa298),
    at<const u32 *>(0x00faa2b0),
    at<const u32 *>(0x00faa268),
    at<const u32 *>(0x00faa280),
};
const TgaEnv *g_env = &kGameEnv;

const u32 kSeekCurrent = 1;
const u32 kFormatWords = 6;  // a SurfaceInfo's format: the masks, bits and paletted

// The reader's and the stream's virtual calls.
s32 *begin(RowReader *r, s32 *out, StreamBase *s) {
    return reinterpret_cast<s32 *(__fastcall *)(RowReader *, void *, s32 *, StreamBase *)>(r->vtable[0])(r, nullptr,
                                                                                                          out, s);
}
s32 *readRow(RowReader *r, s32 *out, StreamBase *s) {
    return reinterpret_cast<s32 *(__fastcall *)(RowReader *, void *, s32 *, StreamBase *)>(r->vtable[1])(r, nullptr,
                                                                                                          out, s);
}
u32 rowBytes(RowReader *r) { return reinterpret_cast<u32(__fastcall *)(RowReader *)>(r->vtable[2])(r); }
void copyRow(RowReader *r) { reinterpret_cast<void(__fastcall *)(RowReader *)>(r->vtable[3])(r); }
u32 streamRead(StreamBase *s, void *buffer, u32 size) {
    return reinterpret_cast<u32(__fastcall *)(StreamBase *, void *, void *, u32)>(s->vtable[2])(s, nullptr, buffer,
                                                                                               size);
}
void streamSeek(StreamBase *s, s32 offset, u32 origin) {
    reinterpret_cast<s32(__fastcall *)(StreamBase *, void *, s32, u32)>(s->vtable[4])(s, nullptr, offset, origin);
}

u32 bytesPerPixel(u32 bitCount) { return (bitCount + 7) >> 3; }

bool sameFormat(const SurfaceInfo *a, const SurfaceInfo *b) {
    return a->redMask == b->redMask && a->greenMask == b->greenMask && a->blueMask == b->blueMask &&
           a->alphaMask == b->alphaMask && a->bitCount == b->bitCount && !a->paletted == !b->paletted;
}

void xorSwap(u8 *a, u8 *b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

s32 *fail(s32 *out) {
    *out = -1;
    return out;
}

}  // namespace

void Tga_SetEnv(const TgaEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

s32 *__fastcall RowReader_Decode(RowReader *r, void *, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                                 TgaCodec *codec) {
    SurfaceInfo *const info = &codec->info;
    const u32 dstRow = bytesPerPixel(dst->bitCount) * info->width;
    const u32 own = rowBytes(r);
    r->rowSize = own > dstRow ? own : dstRow;
    r->row = static_cast<u8 *>(g_env->alloc(r->rowSize));
    if (!r->row) return fail(out);
    const u32 width = info->width < dst->width ? info->width : dst->width;
    r->copyBytes = bytesPerPixel(dst->bitCount) * width;
    r->sourceRowBytes = bytesPerPixel(info->bitCount) * info->width;
    s32 result;
    if (*begin(r, &result, stream)) {
        g_env->free(r->row);
        return fail(out);
    }
    if (!r->bottomUp) {
        r->target = static_cast<u8 *>(dst->bits);
        r->step = dst->pitch;
    } else {
        const u32 rows = info->height < dst->height ? info->height : dst->height;
        r->target = static_cast<u8 *>(dst->bits) + (rows - 1) * dst->pitch;
        r->step = -dst->pitch;
        if (dst->height < info->height) {
            for (u32 skip = info->height - dst->height; skip; --skip) {
                if (*readRow(r, &result, stream) < 0) {
                    g_env->free(r->row);
                    return fail(out);
                }
            }
        }
    }
    u32 line = 0;
    if (sameFormat(dst, info)) {
        do {
            copyRow(r);
            ++line;
        } while (*readRow(r, &result, stream) >= 0 && line < dst->height);
    } else {
        PixelConversion conversion;
        g_env->convCtor(&conversion);
        g_env->convSetup(&conversion, nullptr, &dst->redMask, &info->redMask, codec->palette, 1);
        do {
            g_env->convRow(&conversion, nullptr, r->row, r->row, info->width);
            copyRow(r);
            ++line;
        } while (*readRow(r, &result, stream) >= 0 && line < dst->height);
        g_env->convDtor(&conversion);
    }
    g_env->free(r->row);
    *out = 0;
    return out;
}

void __fastcall RowReader_CopyRow(RowReader *r) {
    std::memcpy(r->target, r->row, r->copyBytes);
    r->target += r->step;
}

s32 *__fastcall TgaReader_Load(TgaReader *r, void *, s32 *out, SurfaceInfo *dst, StreamBase *stream,
                               TgaCodec *codec) {
    s32 result;
    if (*TgaReader_ReadHeader(r, nullptr, &result, stream, codec) < 0) return fail(out);
    RowReader_Decode(&r->base, nullptr, out, dst, stream, codec);
    return out;
}

s32 *__fastcall TgaReader_ReadHeader(TgaReader *r, void *, s32 *out, StreamBase *stream, TgaCodec *codec) {
    const TgaHeader *const h = &codec->header;
    if (streamRead(stream, &codec->header, sizeof codec->header) != sizeof codec->header) return fail(out);
    r->header = *h;
    const u8 type = h->imageType;
    if (type > 11 || (type > 3 && type < 9)) return fail(out);
    const u8 mapType = h->colorMapType;
    if (mapType > 1) return fail(out);
    const bool mapped = type == 1 || type == 9;
    if (mapped && !mapType) return fail(out);
    if (mapType == 1 && (h->mapLength > 0x100 || h->mapEntryBits != 24)) return fail(out);
    r->runLength = r->header.imageType >= 9;
    r->base.bottomUp = !(h->descriptor & 0x20);
    r->mirrored = (h->descriptor & 0x10) != 0;
    streamSeek(stream, h->idLength, kSeekCurrent);
    codec->hasPalette = 0;
    if (!mapped) {
        const u16 skip = static_cast<u16>(h->mapLength * h->mapEntryBits);  // bits (sic)
        if (skip) streamSeek(stream, skip, kSeekCurrent);
    } else {
        u8 *const rgb = static_cast<u8 *>(g_env->alloc(h->mapLength * 3u));
        streamRead(stream, rgb, h->mapLength * 3u);
        std::memset(codec->palette, 0, sizeof codec->palette);
        g_env->expandRgb(reinterpret_cast<u8 *>(codec->palette), rgb, h->mapLength);
        codec->hasPalette = 1;
        u8 *entry = reinterpret_cast<u8 *>(codec->palette);
        for (u32 i = 0; i < h->mapLength; ++i, entry += 4) xorSwap(&entry[0], &entry[2]);  // BGR to RGB
        g_env->free(rgb);
    }
    const u32 *format;
    switch (h->bitCount) {
        case 8: format = g_env->pal8; break;
        case 15:
        case 16: format = g_env->rgb555; break;
        case 24: format = g_env->rgb888; break;
        case 32: format = g_env->argb8888; break;
        default: return fail(out);
    }
    SurfaceInfo *const info = &codec->info;
    std::memcpy(info, format, kFormatWords * sizeof(u32));
    info->width = h->width;
    info->pitch = bytesPerPixel(info->bitCount) * h->width;
    info->height = h->height;
    *out = 0;
    return out;
}

s32 *__fastcall TgaReader_Begin(TgaReader *r, void *, s32 *out, StreamBase *stream) {
    r->rowsLeft = r->header.height;
    r->rowBytes = r->header.width * (u32)(r->header.bitCount >> 3);
    readRow(&r->base, out, stream);
    return out;
}

s32 *__fastcall TgaReader_ReadRow(TgaReader *r, void *, s32 *out, StreamBase *stream) {
    if (r->runLength > 1) return fail(out);
    if (!r->rowsLeft--) return fail(out);
    if (r->runLength) {
        s32 result;
        if (*TgaReader_ReadRunRow(r, nullptr, &result, stream) < 0) return fail(out);
    } else if (streamRead(stream, r->base.row, r->rowBytes) != r->rowBytes) {
        return fail(out);
    }
    if (r->mirrored) return TgaReader_MirrorRow(r, nullptr, out, r->base.row);
    *out = 0;
    return out;
}

s32 *__fastcall TgaReader_MirrorRow(TgaReader *r, void *, s32 *out, u8 *row) {
    const u32 bytes = static_cast<u8>(r->header.bitCount >> 3);
    u8 *left = row;
    u8 *right = row + r->rowBytes - bytes;
    for (u32 pairs = static_cast<u16>(r->header.width >> 1); pairs; --pairs) {
        for (u32 i = 0; i < bytes; ++i) xorSwap(&left[i], &right[i]);
        left += bytes;
        right -= bytes;
    }
    *out = 0;
    return out;
}

s32 *__fastcall TgaReader_ReadRunRow(TgaReader *r, void *, s32 *out, StreamBase *stream) {
    const u32 bytes = r->header.bitCount >> 3;
    u8 *to = r->base.row;
    u32 done = 0;
    if (!r->header.width) {
        *out = 0;
        return out;
    }
    do {
        u8 packet;
        if (streamRead(stream, &packet, 1) != 1) return fail(out);
        if (packet & 0x80) {
            u8 pixel[4];  // a header ReadHeader passed has at most 4 bytes a pixel
            if (streamRead(stream, pixel, bytes) != bytes) return fail(out);
            const u32 count = (packet & 0x7f) + 1;
            done += count;
            for (u32 n = count; n; --n) {
                std::memcpy(to, pixel, bytes);
                to += bytes;
            }
        } else {
            const u32 size = (packet + 1u) * bytes;
            if (streamRead(stream, to, size) != size) return fail(out);
            to += size;
            done += packet + 1u;
        }
    } while (done < r->header.width);
    *out = 0;
    return out;
}

u32 __fastcall TgaReader_RowBytes(TgaReader *r) { return (u32)(r->header.bitCount >> 3) * r->header.width; }

}  // extern "C"
