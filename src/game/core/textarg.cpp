#include "textarg.h"

#include <cstdint>

#include "crt.h"
#include "vcall.h"

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

s32 __fastcall TextArgInt_Write(u8 *arg, void *, u8 *writer, void *) {
    static const u16 kFormat[] = {'%', 'd', 0};   // (0x74f9d8)
    u16 text[0x12];
    const s32 length = Crt().swprintf(text, kFormat, at<u32>(arg, 0xc));
    vcall<void>(writer, 0x14, text, length);
    return 0;
}

s32 __fastcall TextArgString_Write(u8 *arg, void *, u8 *writer, void *) {
    vcall<void>(writer, 0x14, at<u32>(arg, 0xc), -1);
    return 0;
}
}

namespace {
const TextFormatEnv kGameTextFormat = {reinterpret_cast<u32(__cdecl *)(void *, u16 *, u32, u32, ...)>((uintptr_t)0x006acd60)};
const TextFormatEnv *g_textFormat = &kGameTextFormat;
}  // namespace

const TextFormatEnv &TextFormat() { return *g_textFormat; }

void TextFormat_SetEnv(const TextFormatEnv *env) { g_textFormat = env ? env : &kGameTextFormat; }
