#include "spriteframelights.h"

#include <cstdint>

#include "spriteframe.h"
#include "texture.h"
#include "../math/x87.h"
#include "../render/batchquad.h"
#include "../render/lightlist.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const SpriteFrameLightsEnv kPortEnv = {&LightList_First, &LightList_Next, &SpriteFrame_GroundQuad, &Batch_DrawQuad};
const SpriteFrameLightsEnv *g_env = &kPortEnv;

u8 *const kLights = reinterpret_cast<u8 *>((uintptr_t)0x008c1658);

// The spec's extra, batch flags and UVs from a texture, or the plain ones without.
void TextureInto(u8 *spec, const u8 *texture) {
    if (texture) {
        at<u32>(spec, 0x10) = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
        at<u32>(spec, 0xc) = Texture_FormatBits(texture);
        at<float>(spec, 0x18) = (float)X87Reg(at<float>(texture, 0x28));
        at<float>(spec, 0x1c) = (float)X87Reg(at<float>(texture, 0x2c));
        at<float>(spec, 0x20) = (float)(X87Reg(at<float>(texture, 0x20)) + at<float>(spec, 0x18));
        at<float>(spec, 0x24) = (float)(X87Reg(at<float>(texture, 0x24)) + at<float>(spec, 0x1c));
        if (at<u32>(texture, 8) & 0x100000)
            at<u32>(spec, 0) |= 4;
    } else {
        at<u32>(spec, 0x10) = 0;
        at<u32>(spec, 0xc) = 0x1256;
        at<u32>(spec, 0x24) = 0;
        at<u32>(spec, 0x20) = 0;
        at<u32>(spec, 0x1c) = 0;
        at<u32>(spec, 0x18) = 0;
    }
}

bool ByLight() { return (global<u32>(0x008bcbc8) & 0x100000) != 0; }

}  // namespace

void SpriteFrameLights_SetEnv(const SpriteFrameLightsEnv *env) { g_env = env ? env : &kPortEnv; }

u32 __fastcall SpriteFrame_DrawLightQuads(u8 *frame, void *) {
    if (!(at<u32>(frame, 0x10) & 0x200) || !(at<u32>(frame, 0x10) & 0x10000000))
        return 1;
    // Sic, not kept: the original never sets the spec's +0x28, the transformer's argument, so it is what its frame held. 0 here.
    u8 spec[0x6c] = {};
    u8 *const points = spec + 0x2c;
    at<u8 *>(spec, 4) = points;
    if (!ByLight())
        TextureInto(spec, at<const u8 *>(at<u8 *>(frame, 0x28), 0));
    at<u32>(spec, 0xc) |= 0x800;
    if (at<u32>(frame, 0x10) & 0x20000)
        at<u32>(spec, 0xc) |= 0x20000;

    u8 *light = g_env->first(kLights);
    for (u32 k = 0; light; ++k) {
        if (ByLight()) {
            const u32 index = (u32)at<u8>(frame, 0x3c + k) + at<u16>(frame, 0x38);
            TextureInto(spec, at<const u8 *>(at<u8 *>(at<u8 *>(frame, 0x34), 4), index * 0x18));
        }
        at<u32>(spec, 8) = g_env->groundQuad(frame, nullptr, reinterpret_cast<float *>(points), (u32)(uintptr_t)light, 1.0f, 1.0f);
        at<u32>(spec, 0xc) &= ~0x200u;
        if (!g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, spec, 0xffffffff))
            return 0;
        light = g_env->next(kLights);
    }
    return 1;
}
