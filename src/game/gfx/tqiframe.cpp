// The second DCT video's frames: see tqiframe.h.
#include "tqiframe.h"

#include <cstdint>

#include "tgqvideo.h"
#include "tqivideo.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using Macroblock = void(__cdecl *)(u8 *, u32, u8 *);

const TqiFrameEnv kGameEnv = {reinterpret_cast<Macroblock>(&TqiVideo_DecodeMacroblockRgb16), reinterpret_cast<Macroblock>(&TqiVideo_DecodeMacroblockTable16), reinterpret_cast<Macroblock>(&TqiVideo_DecodeMacroblockYuy2)};
const TqiFrameEnv *g_env = &kGameEnv;

// The rows of macroblocks; settings() is read again each pass, as the original does.
template <typename Pick> void rows(u8 *player, u8 *out, u32 pitch, Pick pick) {
    for (u32 y = 0; y < at<u32>(at<u8 *>(player, 0), 0x60); y += 16) {
        u8 *block = out;
        for (u32 x = 0; x < at<u32>(at<u8 *>(player, 0), 0x64); x += 16, block += 0x20) pick()(block, pitch, player + 0x408);
        out += (pitch >> 1) << 5;
    }
}

}  // namespace

void TqiFrame_SetEnv(const TqiFrameEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall TqiVideo_DecodeFrame16(u8 *player, u32 value, u8 *out, u32, u32 pitch) {
    TgqVideo_SetBlockBase(value, player);
    rows(player, out, pitch, [player]() -> Macroblock { return at<u32>(player, 0x40) ? g_env->rgb16 : g_env->table16; });
}

void __fastcall TqiVideo_DecodeFrameYuy2(u8 *player, u32 value, u8 *out, u32, u32 pitch) {
    TgqVideo_SetBlockBase(value, player);
    at<u32>(player, 0x510) = 0;
    at<u32>(player, 0x50c) = 0;
    at<u32>(player, 0x508) = 0;
    rows(player, out, pitch, []() -> Macroblock { return g_env->yuy2; });
}
