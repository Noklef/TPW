// The movie's file opened and closed: see moviefile.h.
#include "moviefile.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "moviechunks.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename F> F import(u32 slot) { return *reinterpret_cast<F const *>((uintptr_t)slot); }

u32 mmioOpen(const char *file) { return import<u32(__stdcall *)(const char *, void *, u32)>(0x006fd3e4)(file, nullptr, 0); }
s32 mmioSetBuffer(u32 file) { return import<s32(__stdcall *)(u32, void *, u32, u32)>(0x006fd3e8)(file, nullptr, 0x2000, 0); }
s32 mmioRead(u32 file, void *buffer, u32 size) { return import<s32(__stdcall *)(u32, void *, u32)>(0x006fd3e0)(file, buffer, size); }
s32 mmioSeek(u32 file, u32 offset, u32 origin) { return import<s32(__stdcall *)(u32, u32, u32)>(0x006fd3d8)(file, offset, origin); }
void mmioClose(u32 file) { import<s32(__stdcall *)(u32, u32)>(0x006fd3f0)(file, 0); }

const MovieFileEnv kGameEnv = {reinterpret_cast<void *(__cdecl *)(u32, u32)>((uintptr_t)0x0067fcb0), &MovieChunks_Next};
const MovieFileEnv *g_env = &kGameEnv;

u8 *settings(u8 *player) { return at<u8 *>(player, 0); }
u32 file(u8 *player) { return at<u32>(settings(player), 0xa4); }

// A table's 24 entries of 0x10 bytes: the first dword of each.
void clearPlaces(u8 *player, u32 table) {
    for (u32 i = 0; i < 0x18; ++i) at<u32>(player, table + i * 0x10) = 0;
}

// A chunk's header: its tag, and its length less these 8 bytes. A failed read: no tag, 1.
void readHeader(u8 *player, u32 &tag, u32 &size) {
    u32 header[2];
    if (mmioRead(file(player), header, 8) == 8) {
        tag = header[0];
        size = header[1] - 8;
    } else {
        tag = 0;
        size = 1;
    }
}

}  // namespace

void MovieFile_SetEnv(const MovieFileEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall MovieFile_CountHeaders(u8 *player) {
    at<u32>(settings(player), 0x80) = 0;
    at<u32>(settings(player), 0x84) = 0;
    u32 tag, size, offset = 0xfffffff8;
    readHeader(player, tag, size);
    while (tag == 0x44414553 || tag == 0x6c484353 || tag == 0x684e5331) {
        mmioSeek(file(player), size, 1);
        offset += 0xfffffff8 - size;
        readHeader(player, tag, size);
        at<u32>(settings(player), 0x84) = 1;
        ++at<u32>(settings(player), 0x80);
    }
    mmioSeek(file(player), offset, 1);
    return at<u32>(settings(player), 0x84);
}

u32 __fastcall MovieFile_Open(const char *name, u8 *player) {
    at<u32>(player, 0x3a4) = 5;
    at<u32>(player, 0x39c) = 0;
    if (!(at<u32>(settings(player), 4) & 0x80000)) at<u32>(settings(player), 0xa4) = mmioOpen(name);
    if (!file(player)) return 1;
    if (mmioSetBuffer(file(player))) return 1;
    MovieFile_CountHeaders(player);
    u8 *const s = settings(player);
    if (!(at<u32>(s, 4) & 0x4000) || at<s32>(s, 0x7c) < 1 || at<s32>(s, 0x7c) > at<s32>(s, 0x80)) at<u32>(s, 0x7c) = 1;
    clearPlaces(player, 0x68);
    clearPlaces(player, 0x1e8);
    at<void *>(player, 0x1c) = at<void *>(player, 0x24) = g_env->calloc(1, 0x124f80);
    if (!at<void *>(player, 0x1c)) return 2;
    at<void *>(player, 0x20) = at<void *>(player, 0x28) = g_env->calloc(1, 0x493e0);
    if (!at<void *>(player, 0x20)) return 2;
    std::memset(at<void *>(player, 0x1c), 0, 0x124f80);
    std::memset(at<void *>(player, 0x20), 0, 0x493e0);
    at<u32>(player, 0x3a0) = 0;
    at<u32>(player, 0x38c) = 0;
    at<u32>(player, 0x390) = 0;
    at<u32>(player, 0x37c) = 0;
    at<u32>(player, 0x380) = 0;
    at<u32>(player, 0x384) = 0;
    at<u32>(player, 0x388) = 0;
    at<u32>(player, 0x3ac) = 4;
    if (at<u32>(settings(player), 0x84)) {
        do {
            while (g_env->next(player) == 2) {
            }
        } while (at<s32>(player, 0x388) < 0x15);
        at<s32>(player, 0x3a4) = 0x8000 / (at<s32>(player, 0x3a0) / at<s32>(player, 0x390));
    } else {
        do {
            if (g_env->next(player) == -1) break;
        } while (at<s32>(player, 0x384) < 0x14);
    }
    at<u32>(settings(player), 0x88) = at<u32>(player, 0x38c) == 0;
    return 0;
}

u32 __fastcall MovieFile_Close(u8 *player) {
    u8 *const s = settings(player);
    if (!(at<u32>(s, 4) & 0x80000) && at<u32>(s, 0xa4)) mmioClose(at<u32>(s, 0xa4));
    if (at<void *>(player, 0x1c)) Crt().free(at<void *>(player, 0x1c));
    if (at<void *>(player, 0x20)) Crt().free(at<void *>(player, 0x20));
    clearPlaces(player, 0x1e8);
    if (at<u32>(settings(player), 0x84)) clearPlaces(player, 0x68);
    return 0;
}
}
