// The zlib codec (vtable 0x703128): reimplementation of 0x619250-0x619600.
//
// One of the game's codecs (the interface RefPackCodec's vtable 0x703f00 shares): it packs
// with the statically linked zlib 1.1.3 behind a 28-byte "BILZ" header. zlib allocates out of
// a work buffer (the caller's, or one of WorkSize bytes for the call) through a bump arena
// that never frees: zfree is Debug_Printf, empty in the retail build.
//
// The originals' quirks are kept: both one-shot calls size the work buffer with slot +0xc
// (the deflate size) though inflate needs less (slot +0x10); the arena moves on even when a
// request does not fit; Decompress takes windowBits from the header, DecompressStream always
// 15; the streaming compressor writes the header out with the packed size still 0 and only
// fills it in in the caller's buffer afterwards.
#pragma once

#include "../../../re/types.h"

struct ZStream {  // zlib 1.1.3 z_stream, 0x38 bytes
    const u8 *nextIn;
    u32 availIn;
    u32 totalIn;
    u8 *nextOut;
    u32 availOut;
    u32 totalOut;
    const char *msg;
    void *state;
    void *(__cdecl *zalloc)(void *opaque, u32 items, u32 size);
    void(__cdecl *zfree)(void *opaque, void *address);
    void *opaque;
    s32 dataType;
    u32 adler;
    u32 reserved;
};
static_assert(sizeof(ZStream) == 0x38, "ZStream is 0x38 bytes");

struct ZArena {  // what zalloc hands out: [cursor, end)
    u8 *cursor;
    u8 *end;
};

constexpr u32 kBilzMagic = 0x5a4c4942;  // "BILZ"
struct BilzHeader {
    u32 magic;
    u32 unpacked;
    u32 packed;      // the header included
    u32 windowBits;  // 15
    u32 memLevel;    // 9
    u32 zero[2];
};
static_assert(sizeof(BilzHeader) == 0x1c, "BilzHeader is 0x1c bytes");

struct ZlibCodec {
    void *const *vtable;
};

extern "C" {
// 0x619250, cdecl (arena, items, size): the zalloc. Moves the cursor on regardless; the old
// cursor when the request fits, else nullptr.
void *__cdecl ZlibCodec_ArenaAlloc(ZArena *arena, u32 items, u32 size);
// __thiscall. 0x619280 (+4; dst, src, size, work): src's size bytes packed into dst (header
// first, at most size bytes in all); the packed size, or 0 (size <= 0x1c, no work buffer,
// zlib failed or the output did not fit). work == nullptr: one is allocated for the call.
u32 __fastcall ZlibCodec_Compress(ZlibCodec *c, void *edx, BilzHeader *dst, const void *src, u32 size, void *work);
// 0x619380 (+8; dst, src, work): unpacks src (header.unpacked bytes) into dst; the size
// unpacked, or 0.
u32 __fastcall ZlibCodec_Decompress(ZlibCodec *c, void *edx, void *dst, const BilzHeader *src, void *work);
u32 __fastcall ZlibCodec_WorkSize(ZlibCodec *c);        // 0x619460 (+0xc): 0x62a39
u32 __fastcall ZlibCodec_InflateWorkSize(ZlibCodec *c); // 0x619470 (+0x10): 0x15388
// 0x619480 (+0x14; header): its unpacked size, 0 when it is not BILZ.
u32 __fastcall ZlibCodec_UnpackedSize(ZlibCodec *c, void *edx, const BilzHeader *header);
// 0x6194a0 (+0x18; src, size, buffer, capacity, work, writer): packs src through buffer,
// handing each full buffer to writer vtbl+4 (buffer, bytes); the packed size or 0.
u32 __fastcall ZlibCodec_CompressStream(ZlibCodec *c, void *edx, const void *src, u32 size, BilzHeader *buffer,
                                        u32 capacity, void *work, void *writer);
// 0x619600 (+0x1c; dst, buffer, capacity, work, reader): fills buffer from reader vtbl+8
// (buffer, capacity) as inflate needs; the size unpacked, or 0 (capacity <= 0x1d...).
u32 __fastcall ZlibCodec_DecompressStream(ZlibCodec *c, void *edx, void *dst, BilzHeader *buffer, u32 capacity,
                                          void *work, void *reader);
}

// The statically linked zlib, the arena callbacks and the constants it is called with.
struct ZlibEnv {
    s32(__cdecl *deflateInit2)(ZStream *z, s32 level, s32 method, s32 windowBits, s32 memLevel, s32 strategy,
                               const char *version, s32 size);
    s32(__cdecl *deflate)(ZStream *z, s32 flush);
    s32(__cdecl *deflateEnd)(ZStream *z);
    s32(__cdecl *inflateInit2)(ZStream *z, s32 windowBits, const char *version, s32 size);
    s32(__cdecl *inflate)(ZStream *z, s32 flush);
    s32(__cdecl *inflateEnd)(ZStream *z);
    void *(__cdecl *zalloc)(void *opaque, u32 items, u32 size);  // 0x619250 (hooked: ours)
    void(__cdecl *zfree)(void *opaque, void *address);           // Debug_Printf
    const char *version;   // "1.1.3"
    const u32 *inputLimit; // 0xc0000000: Decompress's avail_in
};
// The calls go through env (nullptr: the game's).
void ZlibCodec_SetEnv(const ZlibEnv *env);
