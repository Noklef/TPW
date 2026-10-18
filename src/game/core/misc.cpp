#include "misc.h"

#include <cstdint>
#include <initializer_list>

#include "../render/renderer.h"
#include "../render/state.h"
#include "accessors.h"
#include "gameheap.h"
#include "initializers.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Class703c18_Ctor(u8 *object) {
    Class703770_Ctor(object);
    at<u32>(object, 0) = 0x00703c18;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    return object;
}

u8 *__fastcall Class703ca0_Ctor(u8 *object) {
    Class703770_Ctor(object);
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0x00703748;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x10) = 0x00703744;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0) = 0x00703ca0;
    at<u32>(object, 0xc) = 0x00703c98;
    return object;
}

u8 *__cdecl Records32_MaxWeight(u8 *records, s32 count) {
    u8 *best = nullptr;
    s32 weight = 0;
    for (s32 i = 0; i < count; i++) {
        u8 *record = records + 0x20 * (u32)i;
        if (at<s32>(record, 0x1c) > weight && at<s32>(record, 0x18) > 0) {
            best = record;
            weight = at<s32>(record, 0x1c);
        }
    }
    return best;
}

u8 *__fastcall Class703770_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00703770;
    at<u32>(object, 4) = 1;
    return object;
}

u8 *__fastcall Class703c80_Ctor(u8 *object) {
    at<u32>(object, 4) = 5;
    at<u32>(object, 8) = 5;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x10) = 1;
    at<u32>(object, 0x14) = 0xffffffff;
    at<u32>(object, 0x18) = 0xffffffff;
    at<u32>(object, 0x1c) = 0xffffffff;
    at<u32>(object, 0) = 0x00703c80;
    return object;
}

u8 *__cdecl Codec631280_Init(u8 *object) {
    at<u32>(object, 0) = 0x6312d0;
    at<u32>(object, 4) = 0x631340;
    at<u32>(object, 8) = 0x631300;
    at<u32>(object, 0xc) = 0x631380;
    at<u32>(object, 0x10) = 0x631440;
    at<u32>(object, 0x68) = 0;
    at<u32>(object, 0x6c) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x70) = 0x7039d0;
    at<u32>(object, 0x74) = 0x7b;
    at<u32>(object, 0x78) = 0;
    at<u32>(object, 0x7c) = 0;
    at<u32>(object, 0x80) = 0;
    return object;
}

}
