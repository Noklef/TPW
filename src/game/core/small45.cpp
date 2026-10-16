#include "small45.h"

#include <cstdint>

#include "stdtree.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall StdTreeF_LowerBound(u8 *tree, void *, const u8 *key) {
    u8 *result = at<u8 *>(tree, 4);
    u8 *const nil = global<u8 *>(0x00f7c968);
    u8 *node = at<u8 *>(result, 4);
    while (node != nil) {
        if (String_Less(reinterpret_cast<const String *>(node + 0xc), nullptr, reinterpret_cast<const StringView *>(key))) {
            node = at<u8 *>(node, 8);
        } else {
            result = node;
            node = at<u8 *>(node, 0);
        }
    }
    return result;
}

u8 **__fastcall StdTreeF_FindLower(u8 *tree, void *, u8 **out, const u8 *key) {
    *out = StdTreeF_LowerBound(tree, nullptr, key);
    return out;
}

u8 __fastcall StdTreeF_AllFlagged(u8 *object) {
    u8 *const head = at<u8 *>(object, 0x3c);
    u8 *node = at<u8 *>(head, 0);
    if (node == head) return 1;
    do {
        if (at<u8>(at<u8 *>(node, 0x20), 0xb9) == 0) return 0;
        StdTree_IncF(&node);
    } while (node != at<u8 *>(object, 0x3c));
    return 1;
}

}  // extern "C"
