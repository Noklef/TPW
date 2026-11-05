// The screen's surface as a drawing target, the pixel formats' tables: see screenview.h.
#include "screenview.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

AddrCalls g_calls;

constexpr u32 kGetDirectDraw = 0x005fa040, kSurfaceRefresh = 0x0061b190;

u32 &global(u32 address) { return *reinterpret_cast<u32 *>(static_cast<uintptr_t>(address)); }

void table(u32 address, u32 r, u32 g, u32 b, u32 a, u32 bits, u32 paletted) {
    global(address) = r;
    global(address + 4) = g;
    global(address + 8) = b;
    global(address + 0xc) = a;
    global(address + 0x10) = bits;
    global(address + 0x14) = paletted;
}

}  // namespace

void ScreenView_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __cdecl PixelFormats_InitPal8() { table(0x00faa298, 0, 0, 0, 0, 8, 1); }
void __cdecl PixelFormats_Init555() { table(0x00faa2b0, 0x7c00, 0x3e0, 0x1f, 0, 0x10, 0); }
void __cdecl PixelFormats_Init888() { table(0x00faa268, 0xff0000, 0xff00, 0xff, 0, 0x18, 0); }
void __cdecl PixelFormats_Init8888() { table(0x00faa280, 0xff0000, 0xff00, 0xff, 0xff000000, 0x20, 0); }

u32 __cdecl ScreenView_Surface() { return 0x00fa7780; }
u32 __cdecl ScreenView_Field164() { return global(0x00fa78e4); }

u32 *__fastcall ScreenView_Rect(u8 *self, void *, u32 *out) {
    if (!g_calls.Cd(kGetDirectDraw, 0u)) {
        out[0] = out[1] = out[2] = out[3] = 0;
        return out;
    }
    const u32 surface = vcall<u32>(self, 8);
    const u32 *const size = reinterpret_cast<const u32 *>(static_cast<uintptr_t>(g_calls.Th(kSurfaceRefresh, surface) + 0x1c));
    out[0] = 0;
    out[1] = 0;
    out[2] = size[0];
    out[3] = size[1];
    return out;
}

u32 __cdecl ScreenView_Flag1() { return global(0x00fa779c) >> 1 & 1; }

}  // extern "C"
