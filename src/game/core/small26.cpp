#include "small26.h"

#include <cstdint>

#include "small23.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 **__fastcall Tree16a_Find(u8 *tree, void *, u8 **out, const u16 *key) {
    u8 *found = Tree16a_LowerBound(tree, nullptr, key);
    u8 *end = at<u8 *>(tree, 4);
    *out = (found == end || *key < at<u16>(found, 0xc)) ? end : found;
    return out;
}

u8 **__fastcall Tree32_Find(u8 *tree, void *, u8 **out, const u32 *key) {
    u8 *found = Tree32_LowerBound(tree, nullptr, key);
    u8 *end = at<u8 *>(tree, 4);
    *out = (found == end || *key < at<u32>(found, 0xc)) ? end : found;
    return out;
}

u8 **__fastcall Tree16b_Find(u8 *tree, void *, u8 **out, const u16 *key) {
    u8 *found = Tree16b_LowerBound(tree, nullptr, key);
    u8 *end = at<u8 *>(tree, 4);
    *out = (found == end || *key < at<u16>(found, 0xc)) ? end : found;
    return out;
}

u32 __fastcall Obj_HasKey5af780(u8 *object, void *, u32 key) {
    u16 word = (u16)key;
    u8 *tree = object + 0x28;
    u8 *found = Tree16b_LowerBound(tree, nullptr, &word);
    u8 *end = at<u8 *>(tree, 4);
    u8 *it = (found == end || word < at<u16>(found, 0xc)) ? end : found;
    return it != at<u8 *>(object, 0x2c);
}

}  // extern "C"
