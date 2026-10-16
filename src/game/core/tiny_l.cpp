#include "tiny_l.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 * __fastcall ListScalarDelete6fd9e4_41f6a0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fd9e4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete700a9c_51aeb0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00700a9c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u32 __cdecl FreeGlobal879a20_561a00() {
    const u32 p = global<u32>(0x00879a20); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x00879a20) = 0; return 1;
}

u32 __cdecl FreeGlobal879d54_56b760() {
    const u32 p = global<u32>(0x00879d54); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x00879d54) = 0; return 1;
}

u32 __cdecl FreeGlobal87a248_5740e0() {
    const u32 p = global<u32>(0x0087a248); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x0087a248) = 0; return 1;
}

u32 __cdecl FreeGlobal8bcbb8_580500() {
    const u32 p = global<u32>(0x008bcbb8); if (!p) return 0; GameHeap_Free(reinterpret_cast<void *>(p)); global<u32>(0x008bcbb8) = 0; return 1;
}

u8 * __fastcall ListScalarDelete7028a0_5ba7c0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028a0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete7028b4_5ba810(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028b4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete70294c_5beba0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x0070294c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702950_5bebf0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702950; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702a88_5d9750(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702a88; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702afc_5da4d0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702afc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702c0c_5ddaf0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702c0c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702f10_5e4320(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702f10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete703084_5f46e0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00703084; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

}
