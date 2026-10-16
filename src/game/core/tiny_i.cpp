#include "tiny_i.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 * __fastcall ScalarDelete6fd7a0_40f270(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7a0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7a8_40f2b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7a8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7b0_40f2e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7b0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7c0_40f360(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7c0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7c8_40f3b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7c8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7d0_40f400(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7d0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7d8_40f450(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7d8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7e0_40f4a0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7e0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7e8_40f4e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7e8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7f0_40f520(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7f0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7f8_40f560(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7f8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd800_40f5c0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd800; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd808_40f610(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd808; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd810_40f660(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd810; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd818_40f6b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd818; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd820_40f720(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd820; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd828_40f780(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd828; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd830_40f820(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd830; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd838_40f890(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd838; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd840_40f8e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd840; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd848_40f930(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd848; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fdce8_429b60(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fdce8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fef90_47e3a0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fef90; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fefb8_47e730(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fefb8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete709b44_4f18b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00709b44; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete7007a8_4ff500(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x007007a8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete700868_5061b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00700868; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete701178_564710(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00701178; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete701260_565210(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00701260; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete702198_58b310(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00702198; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete7026e8_591980(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x007026e8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete7029fc_5d5c30(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x007029fc; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete703748_617930(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00703748; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete703770_617ae0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00703770; if (v0 & 1) GameHeap_Free(o); return o;
}

}
