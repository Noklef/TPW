#include "tiny_j.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall ListDtor6fd708_407c30(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fd708; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor6fd9e4_41c9a0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fd9e4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor6fdb08_41eb50(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fdb08; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor6fdb10_421640(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fdb10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor700aa0_5193d0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00700aa0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7027fc_5ac550(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007027fc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7027fc_5ac590(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007027fc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7028a0_5b58c0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028a0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7028b0_5b9a80(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028b0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7028b4_5b9ac0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028b4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor70294c_5beae0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x0070294c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702950_5beb20(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702950; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702954_5beb60(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702954; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor7029f8_5d5800(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007029f8; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702a88_5d8610(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702a88; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702a8c_5d8650(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702a8c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702afc_5d9f10(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702afc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702b28_5da2b0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702b28; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702c0c_5dc6c0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702c0c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702c10_5dc700(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702c10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702d00_5e0370(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702d00; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702d98_5e3cd0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702d98; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor702f10_5e40a0(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702f10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

void __fastcall ListDtor703084_5f3900(u8 *o) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00703084; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0;
}

}
