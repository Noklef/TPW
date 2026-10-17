#include "small65.h"

#include <cstdint>

#include "gameheap.h"
#include "misc.h"
#include "small.h"
#include "small20.h"
#include "../math/intpair.h"
#include "vdtor.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The words cleared before the elements are built (the others written then are overwritten later).
const u32 kZeroed[] = {0x8, 0xc, 0x10, 0x14, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x4c, 0x50, 0x54, 0x58, 0x5c};

u8 *Helper(u32 vtable) {
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (!block)
        return nullptr;
    at<u32>(block, 4) = 0;
    at<u32>(block, 8) = 0;
    at<u32>(block, 0xc) = 0;
    at<u32>(block, 0) = vtable;
    return block;
}

}  // namespace

extern "C" {

u8 *__fastcall Class702bc8_Ctor(u8 *object) {
    Class702d48_Ctor(object);
    at<u32>(object, 0) = 0x00702bc8;
    at<u8 *>(object, 8) = Helper(0x00702bc0);
    at<u8 *>(object, 0x6c) = Helper(0x00702bbc);
    at<u32>(object, 0x78) = 0;
    at<u32>(object, 0x74) = 0;
    at<u32>(object, 0x1c) = 0x3ecccccd;
    at<u32>(object, 0x20) = 0x3e19999a;
    at<u32>(object, 0x24) = 0x3f000000;
    at<u32>(object, 0x38) = 0x43960000;
    at<u32>(object, 0x28) = 0x3e4ccccd;
    at<u32>(object, 0x2c) = 0x3d99999a;
    at<u32>(object, 0x30) = 0x3e800000;
    at<u32>(object, 0x3c) = 0x43160000;
    at<u32>(object, 0x40) = 0x3fc00000;
    return object;
}

u32 *__cdecl Class7037c0_Create(u32 *result, u8 **object) {
    if (!object) {
        *result = 0x80004003;
        return result;
    }
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x18));
    if (block) {
        Class703770_Ctor(block);
        at<u32>(block, 8) = 0;
        at<u32>(block, 0) = 0x007037c0;
    }
    if (!block) {
        *result = 0xcffe0100;
        return result;
    }
    *object = block;
    *result = 0;
    return result;
}

u8 *__fastcall Class50ffe0_Ctor(u8 *object) {
    at<u32>(object, 0) = 1;
    at<u32>(object, 4) = 0xa;
    for (u32 offset : kZeroed)
        at<u32>(object, offset) = 0;
    at<u32>(object, 0x18) = 0x10000;
    at<u32>(object, 0x1c) = 0x50000;
    at<u8>(object, 0x60) = 1;
    CRT_VectorCtorIterator(object + 0x64, 8, 5, reinterpret_cast<void(__fastcall *)(void *)>(IntPair_Ctor), reinterpret_cast<void *>(0x005d1750));
    for (u32 offset = 0xa0; offset <= 0xb8; offset += 4)
        at<u32>(object, offset) = 0;
    at<u32>(object, 4) = 0x3333;
    at<u32>(object, 0) = 0x10000;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x1c) = 0x3333;
    at<u32>(object, 0x18) = 0x6666;
    return object;
}

}  // extern "C"

