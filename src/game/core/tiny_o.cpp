#include "tiny_o.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall ListRemovec_5d8a80(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0xc); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove58_5da180(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x58); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove4_5da450(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x4); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove6c_5dd4b0(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x6c); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove8_5e15c0(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x8); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove18_5e4250(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x18); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

}
