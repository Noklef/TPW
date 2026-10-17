#include "balancefile.h"

#include <cstdint>

#include "vcall.h"
#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u8 *key(u8 *self, s32 i) { return vcall<const u8 *>(self, 0, i); }

// The entry of key record i (the values at +4 of the file, re-read each time as the original does).
u8 *entry(const u8 *self, s32 i) { return at<u8 *>(self, 4) + i * 0xc; }

// Value k of an entry: its values (+0) plus k strides (+4) of dwords, in 32-bit arithmetic.
u32 &value(const u8 *e, s32 k) { return *reinterpret_cast<u32 *>(at<u32>(e, 0) + (u32)at<s32>(e, 4) * (u32)k * 4); }

void clear_flag(const u8 *self, s32 i) {
    if (u32 *const p = at<u32 *>(entry(self, i), 8)) *p = 0;
}

}  // namespace

extern "C" {

void __fastcall BalanceFile_SetDefaults(u8 *self, void *, u32 keep) {
    s32 n = 0;
    while (at<u32>(key(self, n), 0) != 0xc) ++n;
    s32 i = 0;
    while (i < n) {
        s32 count;
        if (at<u32>(key(self, i), 0) == 2) {
            s32 j = i;
            if (i < n)
                while (at<u32>(key(self, j), 0) != 3) ++j;
            count = at<s32>(key(self, j), 0x34);
        } else {
            count = 1;
        }
        ++i;
        bool stop = false;
        while (i < n && !stop) {
            switch (at<u32>(key(self, i), 0)) {
            case 1:
            case 3:
                stop = true;
                break;
            case 4:
            case 5:
            case 7:
            case 8:
                for (s32 k = 0; k < count; ++k) value(entry(self, i), k) = 0;
                clear_flag(self, i);
                break;
            case 6:
            case 9: {
                const u32 field = at<u32>(key(self, i), 0) == 6 ? 0x24 : 0x2c;
                for (s32 k = 0; k < count; ++k) {
                    u8 *const e = entry(self, i);
                    value(e, k) = at<u32>(key(self, i), field);
                }
                clear_flag(self, i);
                break;
            }
            case 10:
            case 11:
                for (s32 k = 0; k < count; ++k) {
                    u32 &v = value(entry(self, i), k);
                    if (keep == 1) {
                        v = 0;
                    } else if (v) {
                        Mem_GlobalFree(reinterpret_cast<void *>((uintptr_t)v));
                        v = 0;
                    }
                }
                clear_flag(self, i);
                break;
            default:  // (0 and 2: the original's "InitialiseBalanceValues internal" assert)
                break;
            }
            ++i;
        }
    }
}

void __fastcall BalanceFile_Release(u8 *self) {
    if (at<void *>(self, 4)) BalanceFile_SetDefaults(self, nullptr, 0);
    if (void *const values = at<void *>(self, 4)) {
        Mem_GlobalFree(values);
        at<void *>(self, 4) = nullptr;
    }
}

const u8 *__fastcall BalanceHoardings_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x0073fb40) + index * 0x3c; }
const u8 *__fastcall BalanceLevel_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x0073fc70) + index * 0x3c; }
const u8 *__fastcall BalanceOnline_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x00743f00) + index * 0x3c; }
const u8 *__fastcall BalanceGraphics_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x007442c0) + index * 0x3c; }
const u8 *__fastcall BalanceResolution_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x00744a40) + index * 0x3c; }
const u8 *__fastcall BalanceSound_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x00746a20) + index * 0x3c; }
}
