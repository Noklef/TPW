#include "batchquad.h"

#include <cstdint>

#include "batch.h"
#include "clipcodes.h"
#include "drawbuffer.h"
#include "../gfx/spriteframe.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const BatchQuadEnv kPortEnv = {&Batch_Begin, &DrawBatch_AddPolygon, &DrawBuffer_End, &Batch_Flush, &Batch_DrawQuad, &SpriteFrame_FillItem};
const BatchQuadEnv *g_env = &kPortEnv;

}  // namespace

void BatchQuad_SetEnv(const BatchQuadEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

u32 __fastcall Batch_DrawScreenQuad(u8 *self) {
    static const u32 kPoints[16] = {0xbf800000, 0xbf800000, 0x3a83126f, 0x3f800000, 0x3f800000, 0xbf800000, 0x3a83126f, 0x3f800000,
                                    0x3f800000, 0x3f800000, 0x3a83126f, 0x3f800000, 0xbf800000, 0x3f800000, 0x3a83126f, 0x3f800000};
    u32 spec[27] = {0};
    at<u32>(self, 4) = 0;
    spec[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(&spec[11]));
    spec[2] = 0xffffffff;
    spec[3] = 0x9a1;
    for (u32 i = 0; i < 16; ++i) spec[11 + i] = kPoints[i];
    return g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, reinterpret_cast<u8 *>(spec), 0xffffffff);
}

u32 __fastcall SpriteFrame_DrawQuad(u8 *frame) {
    u32 spec[27] = {0};
    spec[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(&spec[11]));
    if (g_env->fillItem(frame, nullptr, reinterpret_cast<u8 *>(spec)) == 0) return 0;
    return g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, reinterpret_cast<u8 *>(spec), 0xffffffff);
}

u32 __fastcall Batch_DrawQuad(u8 *object, void *, u8 *spec, u32 mask) {
    if (global<u32>(0x008bcbc8) & 0x2000) at<u32>(spec, 0xc) |= 0x100000;
    u32 result;
    u8 codes[4];
    ClipCodes_Compute(global<u32>(0x00879a20), 0, reinterpret_cast<u8 *>(&result), codes, at<const u8 *>(spec, 4), 4, 0x10, mask);
    if (result & 0xff) return 1;
    u8 *const batch = g_env->begin(object, nullptr, 2, 4, 0x1c4, at<u32>(spec, 0xc), at<u32>(spec, 0x10));
    if (!batch) return 1;
    const u32 colour = at<u32>(spec, 8) & at<u32>(global<u8 *>(0x0087a248), 0x2c);
    at<u32>(spec, 8) = colour;
    const u32 *const points = at<const u32 *>(spec, 4);
    static const u8 kUs[4] = {0x18, 0x20, 0x20, 0x18}, kVs[4] = {0x1c, 0x1c, 0x24, 0x24};
    u32 vertices[4][8] = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) vertices[i][j] = points[i * 4 + j];
        vertices[i][4] = colour;
        vertices[i][6] = X87_FloatThrough(at<u32>(spec, kUs[i]));
        vertices[i][7] = X87_FloatThrough(at<u32>(spec, kVs[i]));
    }
    if (!g_env->addPolygon(object, nullptr, reinterpret_cast<u8 *>(vertices), 4, 0x20, reinterpret_cast<const u8 *>(&result), at<u32>(spec, 0))) return 1;
    Batch_FogVertices(object, nullptr, at<u8 *>(batch, 0x28), at<u32>(batch, 0x20));
    Batch_TransformVertices(object, nullptr, at<u32>(spec, 0x28));
    if (at<u32>(spec, 0) & 0x400) at<u32>(batch, 0) |= 0x400;
    const u8 *const last = at<const u8 *>(object, 0x20);
    if (!last || !at<u32>(last, 0x20) || !at<u32>(last, 0x24)) return 1;
    g_env->end(global<u8 *>(0x008bcbc4), nullptr, at<u32>(last, 0x28) + at<u32>(object, 0x1c) * at<u32>(last, 0x20), at<u32>(last, 0x2c) + at<u32>(last, 0x24) * 2);
    return g_env->flush(object, nullptr);
}
}
