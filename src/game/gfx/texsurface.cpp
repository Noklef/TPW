#include "texsurface.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &back(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p - offset); }

}  // namespace

extern "C" {

u32 __fastcall TexSurfaceA_Field38(const u8 *sub) { return back<u32>(sub, 0x38); }

u32 __fastcall TexSurfaceA_Area(const u8 *sub) {
    const u32 off = at<u32>(back<const u8 *>(sub, 0x3c), 4);
    return back<u32>(sub + off, 0x30) * back<u32>(sub + off, 0x34);
}

u32 __fastcall TexSurfaceA_Field10(const u8 *sub) { return back<u32>(sub, 0x10); }

u32 __fastcall TexSurfaceB_FieldC(const u8 *sub) { return back<u32>(sub, 0xc); }

u32 __fastcall TexSurfaceB_Area(const u8 *sub) {
    const u32 off = at<u32>(back<const u8 *>(sub, 0x10), 4);
    return back<u32>(sub + off, 8) * back<u32>(sub + off, 4);
}
}
