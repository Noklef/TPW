#include "moviechunks.h"

#include <cstdint>
#include <cstring>

#include "../core/imaadpcm.h"
#include "../core/xaadpcm.h"
#include "movieplayer.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

const MovieChunksEnv kGameEnv = {&XaAdpcm_DecodeMono, &XaAdpcm_DecodeStereo, &ImaAdpcm_DecodeStereo};
const MovieChunksEnv *g_env = &kGameEnv;

u8 *settings(u8 *player) { return at<u8 *>(player, 0); }
u32 file(u8 *player) { return at<u32>(settings(player), 0xa4); }

s32 mmioRead(u32 file, void *buffer, u32 size) { return (*reinterpret_cast<s32(__stdcall *const *)(u32, void *, u32)>((uintptr_t)0x006fd3e0))(file, buffer, size); }
s32 mmioSeek(u32 file, u32 offset, u32 origin) { return (*reinterpret_cast<s32(__stdcall *const *)(u32, u32, u32)>((uintptr_t)0x006fd3d8))(file, offset, origin); }
void increment(u8 *player, u32 offset) { (*reinterpret_cast<s32(__stdcall *const *)(u32 *)>((uintptr_t)0x006fd0c0))(&at<u32>(player, offset)); }

// The usual answers of a read: 0 (with +0x39c set) gives -1, -1 gives -2; 0 for one that went well.
s32 readFailed(u8 *player, s32 result) {
    if (result == 0) {
        at<u32>(player, 0x39c) = 1;
        return -1;
    }
    return result == -1 ? -2 : 0;
}

// The ring from its start, when a length from its place would pass its end.
void wrapRing(u8 *player, u32 length, u32 place, u32 start, u32 size) {
    if (length + at<u32>(player, place) >= at<u32>(player, start) + size) at<u32>(player, place) = at<u32>(player, start);
}

void decodeXa(u8 *player, const u8 *buffer, u32 size, u32 &length) {
    at<s32>(player, 0x3f0) = *reinterpret_cast<const s16 *>(buffer + 6);
    at<s32>(player, 0x3e8) = *reinterpret_cast<const s16 *>(buffer + 4);
    at<s32>(player, 0x3f4) = *reinterpret_cast<const s16 *>(buffer + 0xa);
    at<s32>(player, 0x3ec) = *reinterpret_cast<const s16 *>(buffer + 8);
    const bool twoChannels = at<u32>(player, 0x58) == 2;
    length = (size - 0xc) / (twoChannels ? 30 : 15) * 112;
    wrapRing(player, length, 0x28, 0x20, 0x493e0);
    s32 position = 0;
    s16 *out = at<s16 *>(player, 0x28);
    const s32 count = static_cast<s32>(length) / 4;
    if (twoChannels) {
        g_env->stereo(count, &position, &at<s32>(player, 0x3e8), &at<s32>(player, 0x3f0), &at<s32>(player, 0x3ec), &at<s32>(player, 0x3f4), buffer + 0xc, &out);
    } else {
        g_env->mono(count, &position, &at<s32>(player, 0x3e8), &at<s32>(player, 0x3f0), buffer + 0xc, &out);
    }
}

void decodePcm(u8 *player, const u8 *buffer, u32 &length) {
    length = *reinterpret_cast<const u32 *>(buffer) << 2;
    wrapRing(player, length, 0x28, 0x20, 0x493e0);
    const s32 count = static_cast<s32>(length) / 4;
    u8 *const out = at<u8 *>(player, 0x28);
    for (s32 i = 0; i < count; ++i) {
        if (at<u32>(player, 0x58) == 2) {
            *reinterpret_cast<u32 *>(out + i * 4) = *reinterpret_cast<const u32 *>(buffer + 4 + i * 4);
        } else {
            const u16 sample = *reinterpret_cast<const u16 *>(buffer + 4 + i * 2);
            *reinterpret_cast<u16 *>(out + i * 4 + 2) = sample;
            *reinterpret_cast<u16 *>(out + i * 4) = sample;
        }
    }
}

// A sound chunk: read when it is the one asked for, else skipped.
s32 soundChunk(u8 *player, u32 size) {
    u8 *const s = settings(player);
    if (at<u32>(s, 0x84) == 0) return -2;
    if (++at<u32>(player, 0x3b4) == at<u32>(s, 0x7c)) return MovieChunks_ReadSound(player, size);
    mmioSeek(at<u32>(s, 0xa4), size, 1);
    return 2;
}

s32 videoChunk(u8 *player, u32 size, u32 kind) {
    at<u32>(player, 0x3b4) = 0;
    return MovieChunks_ReadFrame(player, size, kind);
}

}  // namespace

void MovieChunks_SetEnv(const MovieChunksEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __fastcall MovieChunks_Next(u8 *player) {
    u32 header[2];
    const s32 result = mmioRead(file(player), header, 8);
    if (result == 0) {
        at<u32>(player, 0x39c) = 1;
        return -1;
    }
    if (result != 8) return -2;
    const u32 size = header[1] - 8;
    switch (header[0]) {
        case 0x44414553:   // SEAD
            at<u32>(player, 0x3b4) = 0;
            return MovieChunks_ReadStart(player, size);
        case 0x43444e53:   // SNDC
        case 0x534e4443:   // CDNS
        case 0x5343446c:   // lDCS
        case 0x644e5331:   // 1SNd
        case 0x6c444353:   // SCDl
            return soundChunk(player, size);
        case 0x444e4553:   // SEND
        case 0x654e5331:   // 1SNe
        case 0x6c454353:   // SCEl
            return -1;
        case 0x5447566b:   // kVGT
        case 0x6b564754:   // TGVk
            return videoChunk(player, size, 1);
        case 0x54475666:   // fVGT
        case 0x66564754:   // TGVf
            return videoChunk(player, size, 2);
        case 0x54475170:   // pQGT
        case 0x6656554d:   // MUVf
            return videoChunk(player, size, 3);
        case 0x54514970:   // pIQT
        case 0x66325655:   // UV2f
            return videoChunk(player, size, 4);
        case 0x4d41446b:   // kDAM
        case 0x6b44414d:   // MADk
            return videoChunk(player, size, 5);
        case 0x4d41446d:   // mDAM
        case 0x6d44414d:   // MADm
            return videoChunk(player, size, 6);
        case 0x4d414465:   // eDAM
        case 0x6544414d:   // MADe
            return videoChunk(player, size, 7);
        case 0x5343486c:   // lHCS
        case 0x684e5331:   // 1SNh
        case 0x6c484353:   // SCHl
            at<u32>(player, 0x3b4) = 0;
            return MovieChunks_ReadHeader(player, size);
        case 0x5343436c:   // lCCS
        case 0x53434c6c:   // lLCS
        case 0x6c434353:   // SCCl
        case 0x6c4c4353:   // SCLl
            at<u32>(player, 0x3b4) = 0;
            mmioSeek(file(player), size, 1);
            return 4;
        default:
            at<u32>(player, 0x3b4) = 0;
            return -2;
    }
}

s32 __fastcall MovieChunks_ReadFrame(u8 *player, u32 size, u32 kind) {
    wrapRing(player, size, 0x24, 0x1c, 0x124f80);
    const s32 failed = readFailed(player, mmioRead(file(player), at<void *>(player, 0x24), size));
    if (failed) return failed;
    u8 *const entry = player + (at<s32>(player, 0x380) % 24) * 16;
    at<u32>(entry, 0x1e8) = at<u32>(player, 0x24);
    at<u32>(entry, 0x1f4) = kind;
    at<u32>(entry, 0x1f0) = at<u32>(player, 0x38c);
    at<u32>(player, 0x24) += size;
    increment(player, 0x380);
    increment(player, 0x384);
    increment(player, 0x38c);
    return 1;
}

s32 __fastcall MovieChunks_ReadStart(u8 *player, u32 size) {
    u8 buffer[0xc000];
    const s32 failed = readFailed(player, mmioRead(file(player), buffer, size));
    if (failed) return failed;
    at<u32>(player, 0x60) = 0x10;
    at<u32>(player, 0x58) = 2;
    at<u32>(player, 0x5c) = 0x5622;
    at<u32>(player, 0x64) = 0xa;
    return 0;
}

s32 __fastcall MovieChunks_ReadSound(u8 *player, u32 size) {
    u8 buffer[0xc000];
    u8 *const s = settings(player);
    if (at<u32>(player, 0x390) == 0 && (at<u32>(s, 4) & 0x8000)) at<s32>(player, 0x3b8) = mmioSeek(at<u32>(s, 0xa4), 0, 1) - 8;
    const s32 failed = readFailed(player, mmioRead(at<u32>(s, 0xa4), buffer, size));
    if (failed) return failed;
    u32 length = 0;   // (another kind's: a stack dword nothing sets in the original, sic)
    switch (at<u32>(player, 0x64)) {
        case 0:
            decodePcm(player, buffer, length);
            break;
        case 7:
            decodeXa(player, buffer, size, length);
            break;
        case 0xa:
            length = size << 2;
            wrapRing(player, length, 0x28, 0x20, 0x493e0);
            g_env->ima(buffer, at<s16 *>(player, 0x28), static_cast<s32>(size), &at<u32>(player, 0x3f8), &at<u32>(player, 0x400));
            break;
        default:
            break;
    }
    if (at<u32>(s, 0x88)) increment(player, 0x38c);
    u8 *const entry = player + (at<s32>(player, 0x378) % 24) * 16;
    at<u32>(entry, 0x68) = at<u32>(player, 0x28);
    at<u32>(entry, 0x6c) = length;
    at<u32>(entry, 0x70) = at<u32>(player, 0x38c);
    at<u32>(entry, 0x74) = at<u32>(player, 0x390);
    at<u32>(player, 0x28) += length;
    at<u32>(player, 0x3a0) += length;
    increment(player, 0x378);
    increment(player, 0x388);
    increment(player, 0x390);
    return 2;
}

s32 __fastcall MovieChunks_ReadHeader(u8 *player, u32 size) {
    u8 buffer[0xc000];
    const s32 result = mmioRead(file(player), buffer, size);
    if (result == 0) return -1;
    if (result == -1) return -2;
    at<u32>(player, 0x60) = 0x10;
    at<u32>(player, 0x58) = 1;
    at<u32>(player, 0x5c) = 0x5622;
    at<u32>(player, 0x64) = 0;
    const u8 *cursor = buffer + 4;
    while (*cursor != 0xfd) MovieTags_ReadNumber(&cursor);
    ++cursor;
    while (*cursor != 0x8a) {
        const u8 tag = *cursor;
        const s32 number = MovieTags_ReadNumber(&cursor);
        if (tag == 0x81) at<s32>(player, 0x60) = number;
        if (tag == 0x82) at<s32>(player, 0x58) = number;
        if (tag == 0x83) at<s32>(player, 0x64) = number;
        if (tag == 0x84) at<s32>(player, 0x5c) = number;
    }
    return 0;
}
}
