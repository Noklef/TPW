#include "mode.h"

namespace {

u32 mode() { return *reinterpret_cast<const u32 *>(0x0079d86c); }

}  // namespace

extern "C" {

u32 __cdecl RenderMode_Size() {
    const u32 m = mode();
    if (m & 0x2000) return 0x2000;
    if (m & 0x1000) return 0x1000;
    if (m & 0x800) return 0x800;
    return m & 0x4000 ? 0x4000 : 0x1000;
}

u32 __cdecl RenderMode_Depth() {
    const u32 m = mode();
    if (m & 8) return 8;
    if (m & 0x10) return 0x10;
    return m & 0x20 ? 0x20 : 8;
}

void __cdecl RenderMode_ResetScale() { *reinterpret_cast<float *>(0x0079d9e4) = 1.0f; }

void __cdecl RenderMode_InitEpsilon() { *reinterpret_cast<float *>(0x0079d9e8) = 1.0f / 8388608.0f; }
}
