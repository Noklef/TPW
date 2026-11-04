#include "numput.h"

#include <cstdint>

#include "crt.h"
#include "crtoutput.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

// The put area's pointer (through +0x20) and its count (through +0x30), as streambuf::sputc reads them. The vtable's slot 1 is overflow.
u32 overflow(u8 *buffer, u32 c) {
    using Overflow = u32(__fastcall *)(u8 *, void *, u32);
    return reinterpret_cast<const Overflow *>((uintptr_t)at<u32>(buffer, 0))[1](buffer, nullptr, c);
}

void put_narrow(u32 &failed, u8 *buffer, u32 c) {
    if (buffer) {
        u32 *const pointer = at<u32 *>(buffer, 0x20);
        const u32 p = *pointer;
        u32 result;
        if (p) {
            s32 *const count = at<s32 *>(buffer, 0x30);
            const u32 left = static_cast<u32>(*count);
            if (p < p + left) {
                *count = static_cast<s32>(left - 1);
                *at<u32 *>(buffer, 0x20) = p + 1;
                *reinterpret_cast<u8 *>((uintptr_t)p) = static_cast<u8>(c);
                result = c & 0xff;
            } else {
                result = overflow(buffer, c & 0xff);
            }
        } else {
            result = overflow(buffer, c & 0xff);
        }
        if (result != 0xffffffff) return;
    }
    failed = (failed & 0xffffff00) | 1;
}

void put_wide(u32 &failed, u8 *buffer, u32 c) {
    if (buffer) {
        u32 *const pointer = at<u32 *>(buffer, 0x20);
        const u32 p = *pointer;
        u32 result;
        if (p) {
            s32 *const count = at<s32 *>(buffer, 0x30);
            const u32 left = static_cast<u32>(*count);
            if (p < p + left * 2) {
                *count = static_cast<s32>(left - 1);
                *at<u32 *>(buffer, 0x20) = p + 2;
                *reinterpret_cast<u16 *>((uintptr_t)p) = static_cast<u16>(c);
                result = c;
            } else {
                result = overflow(buffer, c);
            }
        } else {
            result = overflow(buffer, c);
        }
        if (static_cast<u16>(result) != 0xffff) return;
    }
    failed = (failed & 0xffffff00) | 1;
}

template <typename Pad, typename Digit> u32 *do_put(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 value, Pad pad, Digit digit) {
    char text[0x44];
    const s32 length = CRT_sprintf(text, "%p", value);
    const s32 width = at<s32>(ios, 0x14);
    u32 padding = width > 0 && width > length ? static_cast<u32>(width - length) : 0;
    if ((at<u32>(ios, 0xc) & 0x1c0) != 0x40) {
        for (; padding; --padding) pad(failed, buffer);
    }
    for (s32 i = 0; static_cast<u32>(i) < static_cast<u32>(length); ++i) digit(failed, buffer, text[i]);
    at<u32>(ios, 0x14) = 0;
    for (; padding; --padding) pad(failed, buffer);
    out[0] = failed;
    out[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(buffer));
    return out;
}

}  // namespace

extern "C" {

u32 *__fastcall NumPut_DoPutPointer(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) {
    return do_put(out, failed, buffer, ios, value, [fill](u32 &f, u8 *b) { put_narrow(f, b, fill); }, [](u32 &f, u8 *b, char c) { put_narrow(f, b, static_cast<u8>(c)); });
}

u32 *__fastcall NumPutW_DoPutPointer(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) {
    return do_put(out, failed, buffer, ios, value, [fill](u32 &f, u8 *b) { put_wide(f, b, fill); },
                  [](u32 &f, u8 *b, char c) { put_wide(f, b, CRT_btowc(static_cast<u32>(static_cast<s32>(c)))); });
}

}  // extern "C"
