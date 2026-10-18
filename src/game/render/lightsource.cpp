#include "lightsource.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../math/x87.h"
#include "../model/m3d2.h"
#include "renderer.h"
#include "state.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// A part's virtual base: the part plus the offset at +4 of its vtable (0 for no part).
u8 *virtualBase(u8 *part) { return part ? part + at<s32>(at<u8 *>(part, 0), 4) : nullptr; }

// The part at light + offset out of the renderer's tag 0x70 list, then deleted when there is one.
void releasePart(u8 *light, u32 offset) {
    Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, (u32)(uintptr_t)virtualBase(at<u8 *>(light, offset)), 0x70);
    if (u8 *const part = at<u8 *>(light, offset)) vcall<void>(virtualBase(part), 0, 1u);
}

}  // namespace

extern "C" {

void __cdecl LightSource_Destroy(u8 *light) {
    switch (at<u32>(light, 0)) {
    case 0:
        M3D2_DataErrorBox(reinterpret_cast<const char *>((uintptr_t)0x0074d0dc));
        return;
    case 1:
        global<u32>(0x00879dd8) = 0x3f800000;  // 1.0f
        global<u32>(0x00879ddc) = 0;
        global<u32>(0x00879de0) = 0;
        global<u32>(0x00879de4) = 0;
        // fall through
    case 2:
        releasePart(light, 4);
        // fall through
    case 3:
        releasePart(light, 0xc);
        return;
    default:
        return;
    }
}

void __cdecl LightSource_DestroyMain() { LightSource_Destroy(global<u8 *>(0x007a0b40)); }

void __cdecl LightSource_SetColour(u8 *const *holder, u32 r, u32 g, u32 b) {
    u8 *const object = *holder;
    u8 *const part = object + at<s32>(at<const u8 *>(object, 0), 8);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(part, 4) = 0x3f800000;  // 1.0f
    at<u32>(part, 8) = r;
    at<u32>(part, 0xc) = g;
    at<u32>(part, 0x10) = b;
    const auto byte = [](u32 bits) { return (u32)(long long)(X87Reg(reinterpret_cast<const float &>(bits)) * 255.0f).v; };  // 0x6fe540, __ftol
    const u32 green = byte(g), red = byte(r);
    at<u32>(part, 0x54) = ((red | 0xffffff00u) << 8 | green) << 8 | byte(b);
}

void __cdecl LightSource_SetColourScaled(u8 *const *light, float scale) {
    const float *const colour = reinterpret_cast<const float *>(light[1]);
    const float r = (float)(X87Reg(colour[0]) * scale), g = (float)(X87Reg(colour[1]) * scale), b = (float)(X87Reg(colour[2]) * scale);
    u8 *const object = light[0];
    u8 *const part = object + at<s32>(at<const u8 *>(object, 0), 8);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<float>(part, 8) = r;
    at<u32>(part, 4) = 0x3f800000;  // 1.0f
    at<float>(part, 0xc) = g;
    at<float>(part, 0x10) = b;
    const auto byte = [](float c) { return (u32)(long long)(X87Reg(c) * 255.0f).v; };  // 0x6fe540, __ftol
    const u32 red = byte(r);
    const u32 green = byte(g);
    at<u32>(part, 0x54) = ((red | 0xffffff00u) << 8 | green) << 8 | byte(b);
}

void __fastcall LightPart_SetColour(u8 *part, void *, u32 a, u32 r, u32 g, u32 b) {
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(part, 4) = a;
    at<u32>(part, 8) = r;
    at<u32>(part, 0xc) = g;
    at<u32>(part, 0x10) = b;
    const auto byte = [](u32 bits) { return (u32)(long long)(X87Reg(reinterpret_cast<const float &>(bits)) * 255.0f).v; };  // __ftol
    const u32 red = byte(r), alpha = byte(a);
    const u32 high = (red | alpha << 8) << 8 | byte(g);
    at<u32>(part, 0x54) = high << 8 | byte(b);
}

u8 *__fastcall Obj_CtorVirtual(u8 *object, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(object, 0) = 0x006fe570;
        at<u32>(object, 0x60) = 0x006fdce8;
    }
    at<u32>(object, at<u32>(at<u8 *>(object, 0), 4)) = 0x006fdcd8;
    const u32 base = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object, base - 4) = base - 0x60;
    at<u32>(object, 0x50) = 0;
    return object;
}

u32 __fastcall Obj_SetVectorAt2c(u8 *object, void *, u32 a, u32 b, u32 c) {
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(object, 0x2c) = a;
    at<u32>(object, 0x30) = b;
    at<u32>(object, 0x34) = c;
    return a;
}

// The part a light's object reaches: the object plus the offset at +8 of its vtable.
static u8 *RefPart(u8 *const *ref) {
    u8 *object = *ref;
    return object + at<u32>(at<u8 *>(object, 0), 8);
}

void __cdecl MeshRef_ClearFlag10000000(u8 *const *ref) {
    u8 *part = RefPart(ref);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(part, 0x50) &= 0xefffffff;
}

void __cdecl MeshRef_SetFlag10000000(u8 *const *ref) {
    u8 *part = RefPart(ref);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(part, 0x50) |= 0x10000000;
}
}
