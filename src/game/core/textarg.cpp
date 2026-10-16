#include "textarg.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u32 kTag = 0x5641524d;  // 'MRAV'

}  // namespace

extern "C" {

u8 *__fastcall TextArg_Ctor(u8 *arg, void *, u32 n) {
    at<u32>(arg, 0) = 0x006ffb34;
    at<u32>(arg, 4) = kTag;
    at<u32>(arg, 8) = n;
    return arg;
}

u8 *__fastcall TextArgString_Ctor(u8 *arg, void *, u32 n, u32 string) {
    at<u32>(arg, 4) = kTag;
    at<u32>(arg, 8) = n;
    at<u32>(arg, 0xc) = string;
    at<u32>(arg, 0) = 0x006ff174;
    return arg;
}

u8 *__fastcall TextArgInt_Ctor(u8 *arg, void *, u32 n, s32 value, u32 isSigned) {
    at<u32>(arg, 8) = n;
    at<u32>(arg, 4) = kTag;
    at<u32>(arg, 0) = 0x006ff170;
    if (isSigned != 0 && value < 0) {
        at<u32>(arg, 0xc) = 0u - (u32)value;
        at<u32>(arg, 0x10) = 1;
    } else {
        at<u32>(arg, 0xc) = (u32)value;
        at<u32>(arg, 0x10) = 0;
    }
    return arg;
}
}
