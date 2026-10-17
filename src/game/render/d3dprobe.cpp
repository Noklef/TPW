// The Direct3D driver probes: see d3dprobe.h.
#include "d3dprobe.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../core/vregs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kTwenty = 0x41a00000;  // 20.0f
const u32 kHundred = 0x42c80000;  // 100.0f

// The probe's virtual base (its offset is the second dword of the table at +0), found again for each use.
u8 *base(u8 *probe) { return probe + at<u32>(at<u8 *>(probe, 0), 4); }
u8 *device(u8 *probe) { return at<u8 *>(probe, 0x24); }
u8 *caps() { return global<u8 *>(0x00879da8); }
u32 &renderCaps() { return global<u32>(0x008bcbc8); }

void setFlags(u8 *probe, u32 flags) {
    at<u32>(base(probe), 0x14) = ~flags;
    vcall<u32>(base(probe), 8, flags);
}
void clear(u8 *probe) { vcall<u32>(base(probe), 0xc, 0u, 0u); }
void draw(u8 *probe, u32 colour, u32 specular) { vcall<u32>(base(probe), 0x44, kTwenty, colour, specular, 0u, 0u, 0u, 0u); }
u32 readPixel(u8 *probe, u32 *pixel) { return vcall<u32>(base(probe), 0x40, 0u, 0u, pixel); }
void renderState(u8 *probe, u32 state, u32 value) { comcall<u32>(device(probe), 0x58, state, value); }
void stageState(u8 *probe, u32 stage, u32 type, u32 value) { comcall<u32>(device(probe), 0xa0, stage, type, value); }

u32 &shadow(u8 *probe) { return at<u32>(base(probe), 0x14); }

// A flag that differs from the flags set last sets its render state (on: the flag's bit, or its complement for `inverted`), then that bit of the shadow.
void applyFlag(u8 *probe, u32 flags, u32 bit, u32 state, bool inverted) {
    if (!((shadow(probe) ^ flags) & bit)) return;
    renderState(probe, state, ((inverted ? ~flags : flags) & bit) != 0 ? 1 : 0);
    u32 &s = shadow(probe);
    s = ((flags ^ s) & bit) ^ s;
}

}  // namespace

extern "C" {

u32 __fastcall D3dProbe_AlphaTest(u8 *probe) {
    u32 pixel = static_cast<u32>(reinterpret_cast<uintptr_t>(probe));  // (sic: the slot PUSH ECX made)
    if (!(at<u8>(caps(), 0xa4) & 0x40)) {
        at<u32>(base(probe), 0xc) = 0x1000;
        return 1;
    }
    at<u32>(probe, 8) = 0;
    setFlags(probe, 0x9256);
    clear(probe);
    draw(probe, 0, 0);
    vcall<u32>(base(probe), 8, 0x8556u);  // (the kept complement is not updated)
    renderState(probe, 0x19, 7);     // ALPHAFUNC: GREATEREQUAL
    renderState(probe, 0xf, 1);      // ALPHATESTENABLE
    renderState(probe, 0x18, 0xff);  // ALPHAREF
    stageState(probe, 0, 1, 2);      // COLOROP: SELECTARG1
    stageState(probe, 0, 4, 2);      // ALPHAOP: SELECTARG1
    stageState(probe, 0, 2, 1);      // COLORARG1: TEXTURE
    stageState(probe, 0, 5, 1);      // ALPHAARG1: TEXTURE
    draw(probe, 0x80ffffff, 0);
    if (!readPixel(probe, &pixel)) return 0;
    if (pixel == 0) at<u32>(probe, 8) = 2;
    return 1;
}

u32 __fastcall D3dProbe_FogTest(u8 *probe) {
    renderCaps() &= ~0xc000u;
    if (!(at<u32>(caps(), 0xa8) & 0x80000)) {
        renderCaps() |= 0x4000;
        return 1;
    }
    u32 pixel;  // (sic: not set; read only after a read that succeeded)
    const u32 fogStart = 0, fogEnd = kHundred;
    setFlags(probe, 0x109256);
    clear(probe);
    renderState(probe, 0x22, 0);  // FOGCOLOR
    renderState(probe, 0x23, 3);  // FOGTABLEMODE: LINEAR
    renderState(probe, 0x24, fogStart);
    renderState(probe, 0x25, fogEnd);
    draw(probe, 0xffffffff, 0);
    if (!readPixel(probe, &pixel)) {
        renderCaps() |= 0x8000;
        return 0;
    }
    const u32 mask = global<u32>(0x008bd4dc);
    pixel &= mask;
    if (pixel == 0 || pixel == mask) {
        setFlags(probe, 0x109256);
        clear(probe);
        renderState(probe, 0x23, 0);         // FOGTABLEMODE: NONE
        renderState(probe, 0x22, 0xffffff);  // FOGCOLOR
        draw(probe, 0, 0x80000000);
        if (readPixel(probe, &pixel)) {
            const u32 m = global<u32>(0x008bd4dc);
            const u32 v = pixel & m;
            renderCaps() |= v == 0 || v == m ? 0x4000 : 0x8000;
        }
    }
    if (!(renderCaps() & 0xc000)) renderCaps() |= 0x800000;
    return 1;
}

u32 __fastcall D3dProbe_SetFlags(u8 *self, void *, u32 flags) {
    u8 *const probe = self - 0x2c;
    applyFlag(probe, flags, 0x400, 0xf, false);  // ALPHATESTENABLE
    {
        const u32 index = at<u32>(probe, 8) * 4;
        const u32 off = global<u32>(0x007012d8 + index), on = global<u32>(0x007012dc + index);  // (both read first)
        if ((shadow(probe) ^ flags) & 0x200) {
            renderState(probe, 0x18, flags & 0x200 ? on : off);  // ALPHAREF
            u32 &s = shadow(probe);
            s = ((flags ^ s) & 0x200) ^ s;
        }
    }
    applyFlag(probe, flags, 0x8000, 7, true);      // ZENABLE
    applyFlag(probe, flags, 0x800, 0xe, true);     // ZWRITEENABLE
    applyFlag(probe, flags, 0x80000, 0x1d, false);  // SPECULARENABLE
    applyFlag(probe, flags, 0x100000, 0x1c, false);  // FOGENABLE
    applyFlag(probe, flags, 0x1000000, 0x1a, true);  // DITHERENABLE
    applyFlag(probe, flags, 0x2000000, 0x1f, true);
    applyFlag(probe, flags, 0x4000000, 4, true);   // TEXTUREPERSPECTIVE
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x20000, 0x10, 1, 2);
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x40000, 0x12, 1, at<u32>(probe, 0xc));
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x10000, 1, 5, 4);
    Shadow_SetBits3(probe, nullptr, flags, 0, 0x400000, 0xc, 1, 3);
    Shadow_SetBits3(probe, nullptr, flags, 1, 0x4000, 1, at<u32>(probe, 0x18), 1);
    Shadow_SetBits3(probe, nullptr, flags, 1, 0x4000, 4, 3, 1);
    stageState(probe, 0, 0x11, flags & 0x400000 ? 2 : 1);
    if (renderCaps() & 0x80000000) {
        Shadow_SetBits2(probe, nullptr, 0xffffffff, 0x100, 0x1b, 1, 0);
        Shadow_SetByte(probe, nullptr, (flags & 0xff) == 0x52 ? 0x52 : 0x22);
        return 1;
    }
    if (shadow(probe) != flags) {
        Shadow_SetByte(probe, nullptr, static_cast<s32>(flags));
        Shadow_SetBits2(probe, nullptr, flags, 0x100, 0x1b, 1, 0);
    }
    return 1;
}

}  // extern "C"
