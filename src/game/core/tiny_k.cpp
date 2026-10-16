#include "tiny_k.h"

#include <cstdint>

#include "gameheap.h"
#include "tiny_a.h"
#include "tiny_g.h"
#include "tiny_j.h"
#include "../gfx/screensurfaces.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 * __fastcall ScalarDeleteVia407c30_407c70(u8 *o, void *, u32 v0) {
    ListDtor6fd708_407c30(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia41c9a0_41c980(u8 *o, void *, u32 v0) {
    ListDtor6fd9e4_41c9a0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia41eb50_41eb30(u8 *o, void *, u32 v0) {
    ListDtor6fdb08_41eb50(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia421640_421620(u8 *o, void *, u32 v0) {
    ListDtor6fdb10_421640(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia492060_492040(u8 *o, void *, u32 v0) {
    Wrap_Fn_669fee_492060(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia4e5b90_4e5bc0(u8 *o, void *, u32 v0) {
    Vtbl700620_Set_4e5b90(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5193d0_5193b0(u8 *o, void *, u32 v0) {
    ListDtor700aa0_5193d0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5ac550_5ac530(u8 *o, void *, u32 v0) {
    ListDtor7027fc_5ac550(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5b58c0_5b5900(u8 *o, void *, u32 v0) {
    ListDtor7028a0_5b58c0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5b9a80_5b9a40(u8 *o, void *, u32 v0) {
    ListDtor7028b0_5b9a80(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5b9ac0_5b9a60(u8 *o, void *, u32 v0) {
    ListDtor7028b4_5b9ac0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5beae0_5bea80(u8 *o, void *, u32 v0) {
    ListDtor70294c_5beae0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5beb20_5beaa0(u8 *o, void *, u32 v0) {
    ListDtor702950_5beb20(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5beb60_5beac0(u8 *o, void *, u32 v0) {
    ListDtor702954_5beb60(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d5800_5d57e0(u8 *o, void *, u32 v0) {
    ListDtor7029f8_5d5800(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d8610_5d85d0(u8 *o, void *, u32 v0) {
    ListDtor702a88_5d8610(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d8650_5d85f0(u8 *o, void *, u32 v0) {
    ListDtor702a8c_5d8650(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d9f10_5d9ef0(u8 *o, void *, u32 v0) {
    ListDtor702afc_5d9f10(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5da2b0_5da290(u8 *o, void *, u32 v0) {
    ListDtor702b28_5da2b0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5dc6c0_5dc680(u8 *o, void *, u32 v0) {
    ListDtor702c0c_5dc6c0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5dc700_5dc6a0(u8 *o, void *, u32 v0) {
    ListDtor702c10_5dc700(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5e0370_5e0350(u8 *o, void *, u32 v0) {
    ListDtor702d00_5e0370(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5e3cd0_5e3cb0(u8 *o, void *, u32 v0) {
    ListDtor702d98_5e3cd0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5e40a0_5e4080(u8 *o, void *, u32 v0) {
    ListDtor702f10_5e40a0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5f3900_5f38e0(u8 *o, void *, u32 v0) {
    ListDtor703084_5f3900(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia47e390_6175f0(u8 *o, void *, u32 v0) {
    SurfaceSource_Dtor(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

}
