#include "textparts.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../gfx/textlayout.h"
#include "../math/x87.h"
#include "renderer.h"

namespace {

const u32 kInv255 = 0x3b808081, k255 = 0x437f0000;

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *Construct(u8 *part, u32 value, u32 vtable, bool layoutFlag) {
    RendererPart_Ctor(part, nullptr, value);
    TextLayout_Ctor(part + 0xe4);
    at<u8>(part, 0x48) = 0xff;
    at<u8>(part, 0x49) = 0xff;
    at<u8>(part, 0x4a) = 0xff;
    if (layoutFlag) at<u8>(part, 0x178) = 1;
    at<u8>(part, 0xd8) = 0;
    at<u8>(part, 0xd9) = 1;
    at<u8>(part, 0xda) = 0;
    at<u8>(part, 0xdb) = 0;
    at<u16>(part, 0xde) = 0;
    at<u16>(part, 0xe0) = 0;
    at<u8>(part, 0xdc) = 0xff;
    at<u8>(part, 0xdd) = 0;
    at<u32>(part, 0) = vtable;
    return part;
}

// a colour byte through the float constants 0x6ff2bc (1 / 255) and 0x6ff2c0 (255), at the x87's precision; __ftol's low half
u32 ColourByte(u8 c) {
    const X87Reg x = X87Reg((double)c) * X87Reg((double)reinterpret_cast<const float &>(kInv255)) * X87Reg((double)reinterpret_cast<const float &>(k255));
    return (u32)(long long)x.v;
}

void Destroy(u8 *part) {
    at<u32>(part, 0) = 0x006ff320;
    TextLayout_Dtor(part + 0xe4);
    RendererPart_Dtor(part);
}
}

extern "C" {
u8 *__fastcall TextPart_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff320, false); }

void __fastcall TextPart_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff350_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff350, false); }

void __fastcall TextPart6ff350_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff350_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff350_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff380_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff380, true); }

void __fastcall TextPart6ff380_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff380_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff380_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff3b0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff3b0, false); }

void __fastcall TextPart6ff3b0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff3b0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff3b0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff3e0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff3e0, false); }

void __fastcall TextPart6ff3e0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff3e0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff3e0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff410_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff410, false); }

void __fastcall TextPart6ff410_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff410_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff410_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff440_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff440, false); }

void __fastcall TextPart6ff440_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff440_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff440_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff470_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff470, false); }

void __fastcall TextPart6ff470_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff470_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff470_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff4a0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff4a0, false); }

void __fastcall TextPart6ff4a0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff4a0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff4a0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff4d0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff4d0, true); }

void __fastcall TextPart6ff4d0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff4d0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff4d0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff500_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff500, false); }

void __fastcall TextPart6ff500_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff500_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff500_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff530_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff530, false); }

void __fastcall TextPart6ff530_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff530_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff530_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

void __fastcall TextPart_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!info || !flags) return;
    RendererPart_SetLayout(part, nullptr, flags, info);
    if (!(flags & 0x7fd00)) return;
    const u8 *const text = at<const u8 *>(const_cast<u8 *>(info), 0x10);
    if (!text) return;
    if (flags & 0x400) at<u8>(part, 0xd8) = text[4];
    if (flags & 0x800) at<u8>(part, 0xd9) = text[5];
    if (flags & 0x1000) at<u8>(part, 0xda) = text[6];
    if (flags & 0x2000) at<u8>(part, 0xdb) = text[7];
    if (flags & 0x4000) at<u16>(part, 0xde) = *reinterpret_cast<const u16 *>(text + 0xa);
    if (flags & 0x8000) at<u16>(part, 0xe0) = *reinterpret_cast<const u16 *>(text + 0xc);
    if (flags & 0x10000) at<u8>(part, 0xdc) = text[8];
    if (flags & 0x20000) at<u8>(part, 0xdd) = text[9];
    if (flags & 0x40000) {   // (the colour bytes +0x48..+0x4b, then +0x78 = +0x4b, +0x48, +0x49, +0x4a from the top, each through the floats)
        at<u32>(part, 0x48) = *reinterpret_cast<const u32 *>(text + 0x10);
        u32 colour = ColourByte(at<u8>(part, 0x4b));
        colour = colour << 8 | ColourByte(at<u8>(part, 0x48));
        colour = colour << 8 | ColourByte(at<u8>(part, 0x49));
        colour = colour << 8 | ColourByte(at<u8>(part, 0x4a));
        at<u32>(part, 0x78) = colour;
    }
    if (flags & 0x100) {
        const u32 object = *reinterpret_cast<const u32 *>(text);
        if (object) vcall<void>(part, 0x24, object);   // (slot 9; 0x48f830 in this class)
    }
}
}
