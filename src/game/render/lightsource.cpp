#include "lightsource.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../model/m3d2.h"
#include "renderer.h"

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
}
