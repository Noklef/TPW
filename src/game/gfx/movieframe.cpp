// The video library's frame buffers and block frames: see movieframe.h.
#include "movieframe.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../files/refpack.h"
#include "movieplayer.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MovieFrameEnv kGameEnv = {reinterpret_cast<void *(__cdecl *)(u32)>((uintptr_t)0x0067dcd0), RefPack_DecodeIf};
const MovieFrameEnv *g_env = &kGameEnv;

void release(u8 *player, u32 offset) {
    if (at<void *>(player, offset)) {
        Crt().free(at<void *>(player, offset));
        at<void *>(player, offset) = nullptr;
    }
}

}  // namespace

void MovieFrame_SetEnv(const MovieFrameEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall MovieFrame_Allocate(u8 *player, const u8 *data) {
    const u32 skip = at<u32>(player, 0x62c);
    const u8 *const settings = at<u8 *>(player, 0);
    const u32 size = (at<u32>(settings, 0x60) * at<u32>(settings, 0x64) + 0xf) & 0xfffffff0;
    release(player, 0x648);
    release(player, 0x64c);
    release(player, 0x654);
    release(player, 0x650);
    at<void *>(player, 0x648) = g_env->alloc(size);
    at<void *>(player, 0x64c) = g_env->alloc(size);
    at<void *>(player, 0x654) = g_env->alloc(at<u32>(settings, 0x60) << 3);
    at<void *>(player, 0x650) = g_env->alloc((u32)((s32)size / 16) << 2);
    if (!at<void *>(player, 0x648) || !at<void *>(player, 0x64c) || !at<void *>(player, 0x654) || !at<void *>(player, 0x650)) return 0;
    u32 offset = 0u - at<u32>(settings, 0x64) * at<u32>(settings, 0x60);
    for (u32 i = 0; i < at<u32>(settings, 0x60) * 2; ++i) {
        at<u32>(at<void *>(player, 0x654), i * 4) = offset;
        offset += at<u32>(settings, 0x64);
    }
    const u32 words = skip + 4;
    g_env->refpack(data + words * 3, at<u8 *>(player, 0x648), 1);
    return 1;
}

u32 __fastcall MovieFrame_Decode(u8 *player, const u8 *data) {
    u32 ok = 1;
    u8 *const swapped = at<u8 *>(player, 0x64c);
    at<u8 *>(player, 0x64c) = at<u8 *>(player, 0x648);
    at<u8 *>(player, 0x648) = swapped;
    const u32 cells = at<u16>(data, 0), plain = at<u16>(data, 2), expanded = at<u16>(data, 4), bits = at<u16>(data, 6);
    at<u32>(player, 0x630) = cells;
    at<u32>(player, 0x634) = plain;
    at<u32>(player, 0x638) = expanded;
    at<u32>(player, 0x63c) = bits;
    if (cells) {
        if ((s32)cells > at<s32>(player, 0x640)) {
            if (at<void *>(player, 0x65c)) {
                Crt().free(at<void *>(player, 0x65c));
                at<void *>(player, 0x65c) = nullptr;
                at<u32>(player, 0x640) = 0;
            }
            release(player, 0x660);
        }
        if (!at<void *>(player, 0x65c)) {
            at<void *>(player, 0x65c) = g_env->alloc(cells * 8);
            at<u32>(player, 0x640) = cells;
        }
        if (!at<void *>(player, 0x660)) {
            at<void *>(player, 0x660) = g_env->alloc(cells * 4);
            at<u32>(player, 0x640) = cells;
        }
        if (!at<void *>(player, 0x65c)) ok = 0;
        if (!at<void *>(player, 0x660)) ok = 0;
    }
    const u32 blocks = expanded + plain;
    if (blocks) {
        if ((s32)blocks > at<s32>(player, 0x644)) release(player, 0x658);
        if (!at<void *>(player, 0x658)) {
            at<void *>(player, 0x658) = g_env->alloc(blocks << 4);
            at<u32>(player, 0x644) = blocks;
        }
        if (!at<void *>(player, 0x658)) ok = 0;
    }
    if (!ok) return 0;
    const u8 *cursor = data + 0xc;
    MoviePlayer_UnpackFields(at<s32 *>(player, 0x65c), cursor, cells * 2, 10);
    const u8 *const settings = at<u8 *>(player, 0);
    const u32 *const rows = at<const u32 *>(player, 0x654) + at<u32>(settings, 0x60);
    for (u32 i = 0; i < cells; ++i) {
        const s32 *const cell = at<const s32 *>(player, 0x65c) + i * 2;
        at<u32 *>(player, 0x660)[i] = rows[cell[1]] + (u32)cell[0];
    }
    cursor += (s32)((cells * 20 + 0x1f) >> 3) & ~3;
    const u32 copied = plain << 4;
    if (copied) std::memcpy(at<u8 *>(player, 0x658), cursor, copied);
    cursor += copied;
    if (expanded) MoviePlayer_ExpandBlocks(cursor, at<u8 *>(player, 0x658) + copied, (s32)expanded);
    cursor += expanded * 8;
    const u32 count = (at<u32>(settings, 0x64) * at<u32>(settings, 0x60)) >> 4;
    if ((bits * count + 0x1f) & 0xffffffe0) MoviePlayer_UnpackIndices(at<u32 *>(player, 0x650), cursor, (s32)count, bits);
    const u32 pitch = at<u32>(settings, 0x64);
    MoviePlayer_DrawBlocks(at<const s32 *>(player, 0x650), at<u8 *>(player, 0x64c), at<u8 *>(player, 0x648), at<s32>(player, 0x630), at<const u32 *>(player, 0x660), at<u8 *>(player, 0x658), pitch,
                           (s32)(pitch >> 2), (s32)(at<u32>(settings, 0x60) >> 2));
    return 1;
}
}
