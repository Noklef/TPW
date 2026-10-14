// The zlib codec: see zlibcodec.h.
#include "zlibcodec.h"

#include <cstdint>

#include "../core/vcall.h"
#include "handles.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const ZlibEnv kGameEnv = {
    at<s32(__cdecl *)(ZStream *, s32, s32, s32, s32, s32, const char *, s32)>(0x006351a0),
    at<s32(__cdecl *)(ZStream *, s32)>(0x00635450),
    at<s32(__cdecl *)(ZStream *)>(0x006357c0),
    at<s32(__cdecl *)(ZStream *, s32, const char *, s32)>(0x00636600),
    at<s32(__cdecl *)(ZStream *, s32)>(0x00636710),
    at<s32(__cdecl *)(ZStream *)>(0x006365b0),
    at<void *(__cdecl *)(void *, u32, u32)>(0x00619250),
    at<void(__cdecl *)(void *, void *)>(0x005da3c0),
    at<const char *>(0x007780f0),
    at<const u32 *>(0x007780ec),
};
const ZlibEnv *g_env = &kGameEnv;

constexpr s32 kOk = 0;           // Z_OK
constexpr s32 kStreamEnd = 1;    // Z_STREAM_END
constexpr s32 kSyncFlush = 2;    // Z_SYNC_FLUSH
constexpr s32 kFinish = 4;       // Z_FINISH
constexpr s32 kWindowBits = 15;
constexpr s32 kMemLevel = 9;

// The work buffer for a call: the caller's, or one allocated (and freed by the destructor).
struct Work {
    u8 *block;
    u8 *owned = nullptr;
    ZArena arena{};
    Work(ZlibCodec *c, void *given) : block(static_cast<u8 *>(given)) {
        const u32 size = vcall<u32>(c, 0xc);
        if (!block) block = owned = static_cast<u8 *>(Handles_GetEnv()->alloc(size));
        arena = {block, block + size};
    }
    ~Work() {
        if (owned) Handles_GetEnv()->free(owned);
    }
};

ZStream stream(ZArena *arena) {
    ZStream z{};
    z.zalloc = g_env->zalloc;
    z.zfree = g_env->zfree;
    z.opaque = arena;
    return z;
}

s32 deflate_init(ZStream *z) {
    return g_env->deflateInit2(z, -1, 8, kWindowBits, kMemLevel, 0, g_env->version, sizeof(ZStream));
}

void header(BilzHeader *h, u32 unpacked, u32 packed) {
    h->magic = kBilzMagic;
    h->unpacked = unpacked;
    h->packed = packed;
    h->windowBits = kWindowBits;
    h->memLevel = kMemLevel;
    h->zero[0] = 0;
    h->zero[1] = 0;
}

}  // namespace

void ZlibCodec_SetEnv(const ZlibEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void *__cdecl ZlibCodec_ArenaAlloc(ZArena *arena, u32 items, u32 size) {
    u8 *const p = arena->cursor;
    arena->cursor = p + items * size;
    return arena->end >= arena->cursor ? p : nullptr;
}

u32 __fastcall ZlibCodec_Compress(ZlibCodec *c, void *, BilzHeader *dst, const void *src, u32 size, void *work) {
    if (size <= sizeof(BilzHeader)) return 0;
    Work w(c, work);
    if (!w.block) return 0;
    u32 packed = 0;
    ZStream z = stream(&w.arena);
    if (deflate_init(&z) == kOk) {
        z.nextIn = static_cast<const u8 *>(src);
        z.availIn = size;
        z.nextOut = reinterpret_cast<u8 *>(dst + 1);
        z.availOut = size - sizeof(BilzHeader);
        if (g_env->deflate(&z, kFinish) == kStreamEnd) {
            packed = z.totalOut + sizeof(BilzHeader);
            header(dst, size, packed);
        }
        g_env->deflateEnd(&z);
    }
    return packed;
}

u32 __fastcall ZlibCodec_Decompress(ZlibCodec *c, void *, void *dst, const BilzHeader *src, void *work) {
    Work w(c, work);
    if (!w.block) return 0;
    u32 unpacked = 0;
    ZStream z = stream(&w.arena);
    if (src->magic == kBilzMagic && g_env->inflateInit2(&z, static_cast<s32>(src->windowBits), g_env->version,
                                                        sizeof(ZStream)) == kOk) {
        z.nextIn = reinterpret_cast<const u8 *>(src + 1);
        z.availIn = *g_env->inputLimit;
        z.nextOut = static_cast<u8 *>(dst);
        z.availOut = src->unpacked;
        if (g_env->inflate(&z, kFinish) == kStreamEnd) unpacked = z.totalOut;
        g_env->inflateEnd(&z);
    }
    return unpacked;
}

u32 __fastcall ZlibCodec_WorkSize(ZlibCodec *) { return 0x62a39; }

u32 __fastcall ZlibCodec_InflateWorkSize(ZlibCodec *) { return 0x15388; }

u32 __fastcall ZlibCodec_UnpackedSize(ZlibCodec *, void *, const BilzHeader *header) {
    return header->magic == kBilzMagic ? header->unpacked : 0;
}

u32 __fastcall ZlibCodec_CompressStream(ZlibCodec *c, void *, const void *src, u32 size, BilzHeader *buffer,
                                        u32 capacity, void *work, void *writer) {
    Work w(c, work);
    if (!w.block) return 0;
    u32 packed = 0;
    ZStream z = stream(&w.arena);
    if (deflate_init(&z) == kOk) {
        z.nextIn = static_cast<const u8 *>(src);
        z.availIn = size;
        z.nextOut = reinterpret_cast<u8 *>(buffer + 1);
        z.availOut = capacity - sizeof(BilzHeader);
        header(buffer, size, 0);
        s32 r = g_env->deflate(&z, kFinish);
        while (r == kOk) {
            vcall<void>(writer, 4, static_cast<void *>(buffer), capacity - z.availOut);
            z.nextOut = reinterpret_cast<u8 *>(buffer);
            z.availOut = capacity;
            r = g_env->deflate(&z, kFinish);
        }
        if (capacity != z.availOut) vcall<void>(writer, 4, static_cast<void *>(buffer), capacity - z.availOut);
        if (r == kStreamEnd) {
            packed = z.totalOut + sizeof(BilzHeader);
            buffer->packed = packed;
        }
        g_env->deflateEnd(&z);
    }
    return packed;
}

u32 __fastcall ZlibCodec_DecompressStream(ZlibCodec *c, void *, void *dst, BilzHeader *buffer, u32 capacity,
                                          void *work, void *reader) {
    if (capacity <= sizeof(BilzHeader) + 1) return 0;
    Work w(c, work);
    if (!w.block) return 0;
    u32 unpacked = 0;
    ZStream z = stream(&w.arena);
    vcall<void>(reader, 8, static_cast<void *>(buffer), capacity);
    if (buffer->magic == kBilzMagic && g_env->inflateInit2(&z, kWindowBits, g_env->version, sizeof(ZStream)) == kOk) {
        z.availIn = capacity - sizeof(BilzHeader);
        z.nextIn = reinterpret_cast<const u8 *>(buffer + 1);
        z.nextOut = static_cast<u8 *>(dst);
        z.availOut = buffer->unpacked;
        s32 r = g_env->inflate(&z, kSyncFlush);
        while (r == kOk) {
            vcall<void>(reader, 8, static_cast<void *>(buffer), capacity);
            z.nextIn = reinterpret_cast<const u8 *>(buffer);
            z.availIn = capacity;
            r = g_env->inflate(&z, kSyncFlush);
        }
        if (r == kStreamEnd) unpacked = z.totalOut;
        g_env->inflateEnd(&z);
    }
    return unpacked;
}

}  // extern "C"
