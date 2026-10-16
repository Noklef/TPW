#include "small38.h"

#include <cstdint>

#include "stdtree.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall StdMapA_Insert(u8 *tree, void *, u8 **result, const u32 *value) {
    u8 *const head = at<u8 *>(tree, 4);
    u8 *const nil = global<u8 *>(0x007b4a54);
    u8 *y = head;
    u8 *x = at<u8 *>(head, 4);
    bool below = true;
    while (x != nil) {
        y = x;
        below = *value < at<u32>(x, 0xc);
        x = at<u8 *>(x, below ? 0 : 8);
    }
    bool insert = at<u8>(tree, 8) != 0;
    u8 *found = y;
    if (!insert) {
        if (below) {
            if (y == at<u8 *>(at<u8 *>(tree, 4), 0))
                insert = true;
            else
                StdTree_DecA(&found);
        }
        if (!insert && at<u32>(found, 0xc) < *value)
            insert = true;
    }
    if (insert) {
        u8 *iterator = nullptr;
        u8 **made = StdTree_InsertA(tree, nullptr, &iterator, x, y, value);
        at<u8 *>(result, 0) = *made;
        at<u8>(result, 4) = 1;
        return;
    }
    at<u8 *>(result, 0) = found;
    at<u8>(result, 4) = 0;
}

}  // extern "C"
