#include "textparts.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../gfx/textlayout.h"
#include "renderer.h"

namespace {

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
}
