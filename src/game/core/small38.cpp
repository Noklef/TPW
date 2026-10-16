#include "small38.h"

#include <cstdint>

#include "stdtree.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The insert over a tree whose keys are Key (the first member of the value, at +0xc of a node): Dec is the instantiation's iterator decrement and Insert its node insert.
template <typename Key, typename Value, u32 SentinelAt, void(__fastcall *Dec)(u8 **), u8 **(__fastcall *Insert)(u8 *, void *, u8 **, u8 *, u8 *, const Value *)>
void MapInsert(u8 *tree, u8 **result, const Value *value) {
    u8 *const head = at<u8 *>(tree, 4);
    u8 *const nil = global<u8 *>(SentinelAt);
    const Key key = *reinterpret_cast<const Key *>(value);
    u8 *y = head;
    u8 *x = at<u8 *>(head, 4);
    bool below = true;
    while (x != nil) {
        y = x;
        below = key < at<Key>(x, 0xc);
        x = at<u8 *>(x, below ? 0 : 8);
    }
    bool insert = at<u8>(tree, 8) != 0;
    u8 *found = y;
    if (!insert) {
        if (below) {
            if (y == at<u8 *>(at<u8 *>(tree, 4), 0))
                insert = true;
            else
                Dec(&found);
        }
        if (!insert && at<Key>(found, 0xc) < key)
            insert = true;
    }
    if (insert) {
        u8 *iterator = nullptr;
        u8 **made = Insert(tree, nullptr, &iterator, x, y, value);
        at<u8 *>(result, 0) = *made;
        at<u8>(result, 4) = 1;
        return;
    }
    at<u8 *>(result, 0) = found;
    at<u8>(result, 4) = 0;
}

}  // namespace

extern "C" {

void __fastcall StdMapA_Insert(u8 *tree, void *, u8 **result, const u32 *value) {
    MapInsert<u32, u32, 0x007b4a54, StdTree_DecA, StdTree_InsertA>(tree, result, value);
}

void __fastcall StdMapG_Insert(u8 *tree, void *, u8 **result, const u16 *value) {
    MapInsert<u16, u16, 0x00f7c970, StdTree_DecG, StdTree_InsertG>(tree, result, value);
}

}  // extern "C"
