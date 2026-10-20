#include "lightsource.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/globalmem.h"
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

u32 *__cdecl LightSource_Create(u32 kind, u32 colour, const u32 *point, const u32 *direction) {
    u32 *const light = static_cast<u32 *>(GlobalMem_Alloc(0x14, 1));
    const float k = global<float>(0x006fe53c);
    switch (kind) {
    case 0:
        M3D2_DataErrorBox(reinterpret_cast<const char *>((uintptr_t)0x0074d0dc));
        return nullptr;   // (the block kept by nobody: sic)
    case 1:
        global<u32>(0x00879dd8) = 0x3f800000;   // 1.0f
        global<float>(0x00879ddc) = (float)X87_MulInt(colour >> 16 & 0xff, k);
        global<float>(0x00879de0) = (float)X87_MulInt(colour >> 8 & 0xff, k);
        global<float>(0x00879de4) = (float)X87_MulInt(colour & 0xff, k);
        break;
    case 2:
    case 3: {
        const u32 one = 0x3f800000;
        u8 *const part = static_cast<u8 *>(GameHeap_Alloc(0x68));
        const auto offset = [part](u32 table, u32 slot) { return at<s32>(at<u8 *>(part, table), slot); };
        if (part && kind == 2) {   // (the constructors inline)
            at<u32>(part, 0) = 0x006fdd00;
            at<u32>(part, 0xc) = 0x006fdcf8;
            at<u32>(part, 8) = 0x006fdce8;
            at<u32>(part, offset(0xc, 4) + 0xc) = 0x006fdcd8;
            at<s32>(part, offset(0xc, 4) + 8) = offset(0xc, 4) - 0x60;
            at<u32>(part, 0x5c) = 0;
            at<u32>(part, offset(0, 4)) = 0x006fdcc8;
            at<s32>(part, offset(0, 4) - 4) = offset(0, 4) - 8;
            at<u32>(part, offset(0, 8) + 0x50) = 2;
            Obj_SetVectorAt2c(part + offset(0, 8), nullptr, 0, 0, one);
            LightPart_SetColour(part + offset(0, 8), nullptr, one, one, one, one);
        } else if (part) {
            at<u32>(part, 0) = 0x006fe560;
            at<u32>(part, 0xc) = 0x006fe558;
            at<u32>(part, 8) = 0x006fdce8;
            Obj_CtorVirtual(part + 0xc, nullptr, 0);
            at<u32>(part, offset(0, 4)) = 0x006fe548;
            at<s32>(part, offset(0, 4) - 4) = offset(0, 4) - 8;
            at<u32>(part, offset(0, 8) + 0x50) = 4;
            u8 *const base = part + offset(0, 8);
            SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
            at<u32>(base, 0x44) = 0x42c80000;   // 100
            at<u32>(base, 0x48) = 0x461c4000;   // 10000
            at<u32>(base, 0x4c) = 0x38d1b717;   // 0.0001
            LightPart_SetColour(part + offset(0, 8), nullptr, one, one, one, one);
        }
        const float scale = global<float>(0x006fe50c);   // (no part: read through 0, sic)
        u8 *base = part + offset(0, 8);
        SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
        at<u32>(base, 0x54) = colour;
        at<float>(base, 4) = (float)(X87Reg(X87_MulInt(colour >> 24, scale)) * k);
        at<float>(base, 8) = (float)(X87Reg(X87_MulInt(colour >> 16 & 0xff, scale)) * k);
        at<float>(base, 0xc) = (float)(X87Reg(X87_MulInt(colour >> 8 & 0xff, scale)) * k);
        at<float>(base, 0x10) = (float)(X87Reg(X87_MulInt(colour & 0xff, scale)) * k);
        const u32 *const v = kind == 2 ? direction : point;
        const u32 x = v[0], y = v[1], z = v[2];
        base = part + offset(0, 8);
        SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
        u32 *const to = &at<u32>(base, kind == 2 ? 0x2c : 0x14);
        to[0] = x;
        to[1] = y;
        to[2] = z;
        Renderer_Add(global<u8 *>(0x0087a6d0), nullptr, virtualBase(part), 0x70);
        light[kind == 2 ? 1 : 3] = (u32)(uintptr_t)part;
        break;
    }
    default:
        break;
    }
    light[4] = colour;
    light[0] = kind;
    return light;
}

void __cdecl LightSource_CreateMain() {
    float *const v = &global<float>(0x007a0b30);
    const X87Reg sum = X87Reg(v[0]) * v[0] + X87Reg(v[1]) * v[1] + X87Reg(v[2]) * v[2];
    const float length = (float)X87_Sqrt(sum);
    const float x = (float)(X87Reg(v[0]) / length), y = (float)(X87Reg(v[1]) / length), z = (float)(X87Reg(v[2]) / length);
    v[0] = x;
    v[1] = y;
    v[2] = z;
    u32 *const light = LightSource_Create(2, global<u32>(0x007a0b3c), nullptr, &global<u32>(0x007a0b30));
    global<u32 *>(0x007a0b40) = light;
    u8 *const part = reinterpret_cast<u8 *>((uintptr_t)light[1]);
    at<u32>(part + at<s32>(at<u8 *>(part, 0), 8), 0x50) |= 0x20;
}

void __cdecl LightSource_UpdateMain(u32 update) {
    if (!update) return;
    float *const v = &global<float>(0x007a0b30);
    const X87Reg sum = X87Reg(v[0]) * v[0] + X87Reg(v[1]) * v[1] + X87Reg(v[2]) * v[2];
    const float length = (float)X87_Sqrt(sum);
    const u32 ambient = global<u32>(0x007a0b44);
    global<u32>(0x00879dd8) = 0x3f800000;   // 1.0f
    const float scale = (float)(X87Reg(global<float>(0x0074d0d8)) / length);
    const float k = global<float>(0x006fe53c);
    global<float>(0x00879ddc) = (float)X87_MulInt(ambient >> 16 & 0xff, k);
    global<float>(0x00879de0) = (float)X87_MulInt(ambient >> 8 & 0xff, k);
    global<float>(0x00879de4) = (float)X87_MulInt(ambient & 0xff, k);
    const float x = (float)(X87Reg(v[0]) * scale), y = (float)(X87Reg(v[1]) * scale), z = (float)(X87Reg(v[2]) * scale);
    v[0] = x;
    v[1] = y;
    v[2] = z;
    const auto base = [] {   // (the light's part, +4, at its virtual base)
        u8 *const part = reinterpret_cast<u8 *>((uintptr_t)global<u32 *>(0x007a0b40)[1]);
        return part + at<s32>(at<u8 *>(part, 0), 8);
    };
    u8 *b = base();
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<float>(b, 0x2c) = x;
    at<float>(b, 0x30) = y;
    at<float>(b, 0x34) = z;
    const float factor = global<float>(0x006fe50c);
    const u32 colour = global<u32>(0x007a0b3c);
    b = base();
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(b, 0x54) = colour;
    at<float>(b, 4) = (float)(X87Reg(X87_MulInt(colour >> 24, factor)) * k);
    at<float>(b, 8) = (float)(X87Reg(X87_MulInt(colour >> 16 & 0xff, factor)) * k);
    at<float>(b, 0xc) = (float)(X87Reg(X87_MulInt(colour >> 8 & 0xff, factor)) * k);
    at<float>(b, 0x10) = (float)(X87Reg(X87_MulInt(colour & 0xff, factor)) * k);
}
void __fastcall LightSlot_Set(u8 *slot, void *, u32 on, const u32 *colour, const u32 *direction, u32 value) {
    const auto paint = [](u8 *light, u32 c) {   // the colour's channels (high byte first) * [0x702a30] * [0x702a4c] at +4..+0x10, itself at +0x54
        const float scale = global<float>(0x00702a30), k = global<float>(0x00702a4c);
        SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
        at<u32>(light, 0x54) = c;
        at<float>(light, 4) = (float)(X87Reg(X87_MulInt(c >> 24, scale)) * k);
        at<float>(light, 8) = (float)(X87Reg(X87_MulInt(c >> 16 & 0xff, scale)) * k);
        at<float>(light, 0xc) = (float)(X87Reg(X87_MulInt(c >> 8 & 0xff, scale)) * k);
        at<float>(light, 0x10) = (float)(X87Reg(X87_MulInt(c & 0xff, scale)) * k);
    };
    const auto unit = [](const float v[3], float out[3]) {   // v over its length (the length stored as a float)
        const X87Reg sum = (X87Reg(v[0]) * v[0] + X87Reg(v[1]) * v[1]) + X87Reg(v[2]) * v[2];
        const float length = (float)X87_Sqrt(sum);
        for (int i = 0; i < 3; ++i)
            out[i] = (float)(X87Reg(v[i]) / length);
    };
    const auto base = [](u8 *light) { return light ? light + at<s32>(at<u8 *>(light, 0), 4) : nullptr; };
    if (!on) {
        if (u8 *const light = at<u8 *>(slot, 0x20)) {
            Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, (u32)(uintptr_t)base(light), 0x70);
            if (u8 *const again = at<u8 *>(slot, 0x20))
                vcall<void>(base(again), 0, 1u);
            at<u32>(slot, 0x20) = 0;
        }
        at<u32>(slot, 0x24) = value;
        return;
    }
    if (u8 *const light = at<u8 *>(slot, 0x20)) {
        if (colour)
            paint(light, *colour);
        if (direction) {
            float v[3], out[3];
            std::memcpy(v, direction, 12);
            unit(v, out);
            u8 *const again = at<u8 *>(slot, 0x20);
            SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
            std::memcpy(&at<float>(again, 0x2c), out, 12);
        }
        at<u32>(slot, 0x24) = value;
        return;
    }
    const u32 c = colour ? *colour : 0;
    float v[3] = {1.0f, 0.0f, 0.0f}, out[3];
    if (direction)
        std::memcpy(v, direction, 12);
    unit(v, out);
    u8 *const made = static_cast<u8 *>(GameHeap_Alloc(0x64));
    if (made)
        Obj_CtorVirtual(made, nullptr, 1);   // (inline)
    at<u8 *>(slot, 0x20) = made;
    at<u32>(made, 0x50) = 2;   // (no block: through 0, sic)
    paint(at<u8 *>(slot, 0x20), c);
    u8 *const light = at<u8 *>(slot, 0x20);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    std::memcpy(&at<float>(light, 0x2c), out, 12);
    Renderer_Add(global<u8 *>(0x0087a6d0), nullptr, base(at<u8 *>(slot, 0x20)), 0x70);
    at<u32>(slot, 0x24) = value;
}
}
