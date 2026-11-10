// The movie player's DCT video frames: see madvideo.h.
#include "madvideo.h"

#include <cstdint>
#include <initializer_list>

#include "dctmacroblock.h"
#include "dctvlc.h"
#include "moviepixels.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const MadVideoEnv kGameEnv = {game<void *(__cdecl *)(u32, u32)>(0x0067fcb0), reinterpret_cast<void(__fastcall *)(const u8 *, u32, s32)>(&DctVlc_BeginFrame),
                              reinterpret_cast<void(__fastcall *)(u8 *, u8 *, s32)>(&DctVideo_DecodeMacroblock), reinterpret_cast<void(__cdecl *)(const u8 *, u8 *, u32)>(&MoviePixels_FromYuv)};
const MadVideoEnv *g_env = &kGameEnv;

u8 *settingsOf(u8 *player) { return at<u8 *>(player, 0); }
u32 heightOf(u8 *player) { return at<u32>(settingsOf(player), 0x60); }
u32 widthOf(u8 *player) { return at<u32>(settingsOf(player), 0x64); }

}  // namespace

void MadVideo_SetEnv(const MadVideoEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" u32 __fastcall MadVideo_DecodeFrame(u8 *player, const u8 *data, u8 *out, u32 pitch, u32 kind) {
    if (!gMadVideo_DecodeFrame_faac64) {
        gMadVideo_DecodeFrame_faac64 = 1;
        for (u32 offset : {0x664u, 0x668u, 0x66cu}) at<void *>(player, offset) = g_env->calloc(1, widthOf(player) * heightOf(player) * 4);
    }
    g_env->beginFrame(data + 0x10, kind != 5, static_cast<s8>(data[0xd]));
    u8 *frame = at<u8 *>(player, kind == 7 ? 0x66c : 0x668);
    for (u32 y = 0; y < heightOf(player); y += 16) {
        for (u32 x = 0; x < widthOf(player); x += 16) {
            const u32 place = ((y * widthOf(player) + x) >> 1) * 4;
            g_env->macroblock(at<u8 *>(player, 0x664) + place, frame + place, widthOf(player));
        }
    }
    if (heightOf(player) != 1) {
        u32 row = 0;
        do {
            g_env->fromYuv(frame, out, widthOf(player) >> 1);
            out += pitch;
            frame += (widthOf(player) >> 1) * 4;
            ++row;
        } while (row < heightOf(player) - 1);
    }
    if (kind != 7) {
        u8 *const last = at<u8 *>(player, 0x664);
        at<u8 *>(player, 0x664) = at<u8 *>(player, 0x668);
        at<u8 *>(player, 0x668) = last;
    }
    return 0;
}
