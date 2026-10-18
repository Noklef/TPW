#include "setup.h"

#include <cstdint>
#include <initializer_list>

#include "../core/balancefile.h"
#include "../core/globalmem.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "queue.h"
#include "xsection.h"

namespace {

template <typename T> T &at(const void *p, u32 off) {
    return *reinterpret_cast<T *>(reinterpret_cast<u8 *>(const_cast<void *>(p)) + off);
}

template <typename T = u32> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

// A GlobalMem block at p + off freed and its pointer cleared.
void releaseBlock(u8 *p, u32 off) {
    GlobalMem_Free(at<void *>(p, off));
    at<u32>(p, off) = 0;
}

bool zero(float x) { return !(x < 0.0f) && !(x > 0.0f); }  // fcomp against 0: NaN is equal

const float kRadians = 0.0174532924f;  // pi / 180 (0x6fde78: 0x3c8efa35)

// A GlobalMem copy of a string.
char *copyName(const char *name) {
    u32 size = 0;
    while (name[size++]) {}
    char *const copy = static_cast<char *>(GlobalMem_Alloc(size, 1));
    for (u32 k = 0; (copy[k] = name[k]) != 0; ++k) {}
    return copy;
}

// The u32 at from + src to to + dst when it is a non-zero float.
void copyFloat(u8 *to, u32 dst, const u8 *from, u32 src) {
    if (!zero(at<float>(from, src))) at<u32>(to, dst) = at<u32>(from, src);
}

}  // namespace

extern "C" {

u32 __cdecl CoasterType_Setup(u8 *coaster, u32 id, const u8 *source) {
    copyFloat(coaster, 0x40, source, 0x3b08);
    const u32 n = at<u32>(source, 0xfc);
    if (!n) return 0;  // Debug_Printf("Failed to setup track for coaster")
    u8 *const a = static_cast<u8 *>(GlobalMem_Alloc(0x30, 1));
    u8 *const b = static_cast<u8 *>(GlobalMem_Alloc(0x40, 1));
    u8 *const c = static_cast<u8 *>(GlobalMem_Alloc(0x20, 1));
    u8 *entry = static_cast<u8 *>(GlobalMem_Alloc((n + global(0x0074ce10)) << 4, 1));
    at<u8 *>(b, 8) = a;
    at<u32>(a, 0) = 3;
    at<u32>(b, 0) = 3;
    at<u16>(b, 4) = 1;
    at<u32>(c, 0) = 0;
    at<u8 *>(c, 0x10) = entry;
    at<u16>(c, 4) = (u16)(n + global(0x0074ce10));
    at<u32>(c, 8) = id;
    const u8 *from = source + 0xc;
    for (u32 i = 0; i < n; ++i, from += 0xc, entry += 0x10) {
        at<u32>(entry, 0) = at<u32>(from, 4) ? 6 : 2;
        if (!zero(at<float>(from, 8))) {
            at<u32>(entry, 0) |= 0x20;
            at<u32>(coaster, 0) |= 4;
            at<u32>(entry, 0xc) = at<u32>(from, 8);
        }
        at<char *>(entry, 4) = copyName(at<const char *>(from, 0));
    }
    for (u32 j = 0; j < global(0x0074ce10); ++j, entry += 0x10)
        for (u32 k = 0; k < 0x10; k += 4) at<u32>(entry, k) = global(0x0074cde0 + 0x10 * j + k);
    at<u8 *>(coaster, 0x18) = b;
    at<u8 *>(coaster, 0x1c) = c;
    at<u8 *>(coaster, 0x14) = at<u8 *>(b, 8);
    at<u32>(coaster, 0x20) = n;
    if (!CrossSection_Setup(coaster, source)) return 0;  // "Failed to setup cross sections ..."
    if (!CoasterPylons_Setup(coaster, source)) return 0;  // "... pylon controls for coaster"
    if (!CoasterTrains_Setup(coaster, source)) return 0;  // "... car and trains for coaster"

    const u32 pairs = at<u32>(source, 0x3ac8);
    if (pairs) {
        at<u32>(coaster, 0xa0) = pairs;
        at<u32 *>(coaster, 0x9c) = static_cast<u32 *>(GlobalMem_Alloc(pairs * 8, 1));
        for (u32 i = 0; i < at<u32>(coaster, 0xa0); ++i) {
            at<u32 *>(coaster, 0x9c)[2 * i] = at<u32>(source, 0x3a98 + 8 * i);
            at<u32 *>(coaster, 0x9c)[2 * i + 1] = at<u32>(source, 0x3a9c + 8 * i);
        }
    }
    if (at<u32>(source, 0x3ae8)) at<u32>(coaster, 0x2e8) = at<u32>(source, 0x3ae8);
    if (at<u32>(source, 0x3aec)) at<u32>(coaster, 0x2ec) = at<u32>(source, 0x3aec);
    copyFloat(coaster, 0xa4, source, 0x3ae4);
    copyFloat(coaster, 0x8, source, 0x3acc);
    copyFloat(coaster, 0xc, source, 0x3ad0);
    for (u32 k = 0; k < 0xc; k += 4) copyFloat(coaster, 0x24 + k, source, 0x3ad8 + k);
    at<u32>(coaster, 0x31c) = at<u32>(source, 0x3b74);
    for (u32 k = 0; k < 0x10; k += 4) copyFloat(coaster, 0x44 + k, source, 0x3b0c + k);
    for (u32 k = 0; k < 0x10; k += 4) copyFloat(coaster, 0x60 + k, source, 0x3b28 + k);
    for (u32 k = 0; k < 0x3c; k += 4) copyFloat(coaster, 0x320 + k, source, 0x3b38 + k);
    if (at<u32>(source, 0x3af0)) {
        at<u32>(coaster, 0x1c8) = (at<u32>(coaster, 0x1c8) & ~0x4000u) | 0x8000;
        at<u32>(coaster, 0) |= 0x100;
    }
    for (u32 k = 0; k < 0xc; k += 4) copyFloat(coaster, 0x54 + k, source, 0x3b1c + k);
    at<u32>(coaster, 0) |= 0xa0;
    if (at<u32>(source, 0x3afc)) at<u32>(coaster, 0) |= 0x20;  // sic: already set
    if (!at<u32>(source, 0x3af8)) at<u32>(coaster, 0) |= 0x10;
    if (at<u32>(source, 0x3af4)) at<u32>(coaster, 0) = (at<u32>(coaster, 0) & ~0x10u) | 8;
    if (at<u32>(source, 0x3b78)) {
        at<u32>(coaster, 0x30) = at<u32>(source, 0x3b78);
        at<u32>(coaster, 0x34) = at<u32>(source, 0x3b78);
    }
    if (at<u32>(source, 0x3b7c)) at<u32>(coaster, 0x3c) = at<u32>(source, 0x3b7c);
    return 1;
}

u32 __cdecl CoasterPylons_Setup(u8 *coaster, const u8 *source) {
    const u32 count = at<u32>(source, 0x3a20);
    for (u32 i = 0; i < count; ++i) {
        u8 *const to = coaster + (i == 0 ? 0xa8 : 0x1c8);
        const u8 *const from = source + 0x38a8 + 0xbc * i;
        const char *const name = at<const char *>(from, 0);
        if (!name || !at<u32>(from, 4) || zero(at<float>(from, 0xc))) return 0;
        at<char *>(to, 0x10) = copyName(name);
        at<u32>(to, 0xc) = at<u32>(from, 4);
        if (at<u32>(from, 8)) at<u32>(to, 0) |= 0x10;
        at<u32>(to, 0x58) = at<u32>(from, 0xc);

        const auto copyWord = [&](u32 src, u32 dst) {
            if (at<u32>(from, src)) at<u32>(to, dst) = at<u32>(from, src);
        };
        copyFloat(to, 0xb0, from, 0x10);
        copyWord(0x14, 0xb4);
        copyWord(0x18, 0xb8);
        copyWord(0x1c, 0xcc);
        for (u32 k = 0; k < 0xc; k += 4) copyFloat(to, 0x68 + k, from, 0x20 + k);
        for (u32 k = 0; k < 0x28; k += 4)
            if (!zero(at<float>(from, 0x2c + k)))
                at<float>(to, 0x74 + k) = (float)(X87Reg(at<float>(from, 0x2c + k)) * kRadians);
        for (u32 k = 0; k < 8; k += 4) {
            const s32 v = at<s32>(from, 0x54 + k);
            if (v) {
                at<s32>(to, 0x9c + k) = (s32)((u32)v * (u32)v);
                at<float>(to, 0xa4 + k) = (float)v;
            }
        }
        if (!zero(at<float>(from, 0x5c)))
            at<float>(to, 0xac) = (float)(X87Reg(at<float>(from, 0x5c)) * 10.0f * kRadians);
        for (u32 k = 0; k < 0xc; k += 4) at<u32>(to, 0x5c + k) = at<u32>(from, 0x60 + k);
        for (u32 k = 0; k < 0x50; k += 4) copyFloat(to, 0xd0 + k, from, 0x6c + k);
    }
    return 1;
}

u32 __cdecl CoasterTrains_Setup(u8 *coaster, const u8 *source) {
    const u32 count = at<u32>(source, 0x3a78);
    if (!count) return 0;
    u8 *const cars = static_cast<u8 *>(GlobalMem_Alloc(count * 0x34, 1));
    u8 *car = cars;
    const u8 *from = source + 0x3a24;
    for (u32 i = 0; i < count; ++i, car += 0x34, from += 0x1c) {
        for (u32 k = 0; k < 0x1c; k += 4)
            if (k != 0xc) at<u32>(car, k) = 0;  // +0xc left as allocated
        at<float>(car, 0x1c) = 0.1f;
        const char *const name = at<const char *>(from, 8);
        if (!at<u32>(from, 4) || !name) return 0;  // sic: the array leaks
        if (at<u32>(from, 0)) at<u32>(car, 0) = 1;
        at<u32>(car, 4) = at<u32>(from, 4);
        at<char *>(car, 8) = copyName(name);
        for (u32 k = 0; k < 0xc; k += 4) at<u32>(car, 0x10 + k) = at<u32>(from, 0xc + k);
        copyFloat(car, 0x1c, from, 0x18);
    }
    at<u8 *>(coaster, 0x308) = cars;
    at<u32>(coaster, 0x2f0) = 0;
    at<u32>(coaster, 0x2f4) = 3;
    u8 *const first = cars + at<u32>(source, 0x3a80) * 0x34;
    u8 *const second = cars + at<u32>(source, 0x3a84) * 0x34;
    u8 *const third = cars + at<u32>(source, 0x3a88) * 0x34;
    at<u8 *>(coaster, 0x300) = second;
    at<u8 *>(coaster, 0x2fc) = first;
    at<u8 *>(coaster, 0x304) = third;
    if (first != second) at<u32>(coaster, 0x2f0) = 1;
    if (third != second) at<u32>(coaster, 0x2f0) |= 2;
    if (at<u32>(source, 0x3a7c)) at<u32>(coaster, 0x2f4) = at<u32>(source, 0x3a7c);
    for (u32 k = 0; k < 0xc; k += 4) at<u32>(coaster, 0x30c + k) = at<u32>(source, 0x3a8c + k);
    return 1;
}

void __cdecl Coaster_SetTrainCount(u8 *coaster, u32 count) {
    if (count == 0) return;
    if ((at<u8>(coaster, 8) & 0xc) != 0) {
        for (u32 i = count; i < at<u32>(coaster, 0xf4); ++i) {
            u8 *const train = at<u8 *>(coaster, 0x128) + (i << 7);
            CoasterTrain_Unload(coaster, train, 0);
            CoasterTrain_Unload(coaster, train, 1);
        }
    }
    at<u32>(coaster, 0xf4) = count;
}

void __cdecl CoasterTypes_FreeAll() {
    for (u32 i = 0; i < global(0x00790fe8); ++i) {
        u8 *&slot = global<u8 *>(0x00790b08 + 4 * i);
        u8 *const t = slot;
        u8 *const first = at<u8 *>(t, 0x300), *const last = at<u8 *>(t, 0x304), *const other = at<u8 *>(t, 0x2fc);
        releaseBlock(first, 0x2c);
        if (other != first) releaseBlock(other, 0x2c);
        if (last != first) releaseBlock(last, 0x2c);
        releaseBlock(t, 0x308);
        const u8 *const c = at<u8 *>(t, 0x1c);
        u32 n = at<u16>(c, 4) - global(0x0074ce10);
        for (u8 *e = at<u8 *>(c, 0x10) + 4; n; --n, e += 0x10) releaseBlock(e, 0);
        Dword_ClearBit0(at<u32 *>(t, 0x1c));
        Animator_Dispose(at<u8 *>(t, 0x18));
        releaseBlock(at<u8 *>(t, 0x1c), 0x10);
        releaseBlock(t, 0x1c);
        releaseBlock(t, 0x18);
        releaseBlock(t, 0x14);
        GlobalMem_Free(at<void *>(at<u8 *>(t, 0x88), 0x18 * at<u32>(t, 0x8c) + 0xc));
        GlobalMem_Free(at<void *>(t, 0x90));
        GlobalMem_Free(at<void *>(t, 0x94));
        GlobalMem_Free(at<void *>(t, 0x98));
        at<u32>(t, 0x90) = 0;
        at<u32>(t, 0x94) = 0;
        at<u32>(t, 0x98) = 0;
        releaseBlock(t, 0x88);
        releaseBlock(t, 0x78);
        slot = nullptr;
    }
    global(0x00790fe8) = 0;
}

u8 *__cdecl CoasterType_Create(u32 holder, u32, u32 id, const u8 *source) {
    u8 balance[0x3b80];   // (a track type balance file laid out and released again, used for nothing: sic)
    at<u32>(balance, 0) = 0x006fd8b0;
    BalanceFile_Init(balance);
    u8 *const t = static_cast<u8 *>(GlobalMem_Alloc(0x35c, 1));
    reinterpret_cast<u8 **>(0x00790b08)[global(0x00790fe8)] = t;
    ++global(0x00790fe8);
    const u32 one = 0x3f800000, five = 0x40a00000, ten = 0x41200000, half = 0x3f000000, pi = 0x40490fdb;
    at<u32>(t, 4) = holder;
    at<u32>(t, 0) = 0;
    at<u32>(t, 0x40) = 0;
    at<u32>(t, 8) = 0x3fa00000;   // 1.25
    at<u32>(t, 0xc) = 0x3fa00000;
    at<u32>(t, 0x10) = 0x40800000;   // 4
    at<u32>(t, 0x24) = 0x3fc00000;   // 1.5
    at<u32>(t, 0x28) = one;
    at<u32>(t, 0x2c) = one;
    at<u32>(t, 0x2e8) = 1;
    at<u32>(t, 0x2ec) = 15000;
    at<u32>(t, 0xa4) = ten;
    at<u32>(t, 0x30) = 100;
    at<u32>(t, 0x34) = 100;
    at<u32>(t, 0x38) = 0;
    at<u32>(t, 0x3c) = 0x96;
    at<u32>(t, 0x70) = one;
    at<u32>(t, 0x44) = five;
    at<u32>(t, 0x48) = five;
    at<u32>(t, 0x4c) = 0x42a00000;   // 80
    at<u32>(t, 0x50) = 0x43020000;   // 130
    at<u32>(t, 0x54) = 0x3f4ccccd;   // 0.8
    at<u32>(t, 0x58) = 0;
    at<u32>(t, 0x5c) = 0;
    at<u32>(t, 0x60) = 0;
    at<u32>(t, 0x64) = 0xbe99999a;   // -0.3
    at<u32>(t, 0x68) = 0;
    at<u32>(t, 0x6c) = 0x3e4ccccd;   // 0.2
    at<u32>(t, 0x320) = 0;
    at<u32>(t, 0x324) = one;
    at<u32>(t, 0x328) = 0;
    at<u32>(t, 0x32c) = half;
    at<u32>(t, 0x330) = 0x40400000;   // 3
    at<u32>(t, 0x334) = 0x3dcccccd;   // 0.1
    at<u32>(t, 0x338) = one;
    at<u32>(t, 0x33c) = one;
    at<u32>(t, 0x340) = five;
    at<u32>(t, 0x344) = one;
    at<u32>(t, 0x348) = 0x40000000;   // 2
    at<u32>(t, 0x34c) = half;
    at<u32>(t, 0x350) = one;
    at<u32>(t, 0x354) = ten;
    at<u32>(t, 0x358) = 0x41a00000;   // 20
    at<u32>(t, 0x9c) = global(0x0074cac8);
    at<u32>(t, 0xa0) = global(0x0074cacc);
    for (u32 k = 0; k < 5; ++k) {   // two sets of five {0, 1, 1, 0}
        for (u32 base : {0x178u, 0x298u}) {
            u32 *const e = &at<u32>(t, base + 0x10 * k);
            e[0] = 0;
            e[1] = one;
            e[2] = one;
            e[3] = 0;
        }
    }
    const float root1 = (float)X87_Sqrt(X87Reg(global<double>(0x006fde80)));
    at<u32>(t, 0xa8) = 4;
    at<u32>(t, 0x158) = 0xbf7fbe77;
    at<u32>(t, 0x15c) = 1;
    at<u32>(t, 0x160) = 1;
    at<u32>(t, 0x100) = five;
    at<u32>(t, 0x174) = 1;
    at<u32>(t, 0x110) = ten;
    at<u32>(t, 0x114) = ten;
    at<u32>(t, 0x11c) = 0x401c61aa;
    at<u32>(t, 0x120) = pi;
    at<u32>(t, 0x124) = 0x3ff5be0c;
    at<u32>(t, 0x128) = pi;
    at<u32>(t, 0x12c) = 0x3fb2b8c3;
    at<u32>(t, 0x130) = pi;
    at<u32>(t, 0x134) = 0x3f5f66f3;
    at<u32>(t, 0x138) = pi;
    at<u32>(t, 0x144) = 9;
    at<u32>(t, 0x148) = 0x40;
    at<u32>(t, 0x13c) = 0x3f490fdb;   // pi / 4
    at<u32>(t, 0x140) = 0;
    at<u32>(t, 0x154) = 0;
    at<float>(t, 0x14c) = root1;
    at<float>(t, 0x150) = (float)X87_Sqrt(X87Reg(global<double>(0x006fde88)));
    at<u32>(t, 0x230) = ten;
    at<u32>(t, 0x234) = ten;
    at<float>(t, 0x26c) = root1;
    at<u32>(t, 0x264) = 9;
    at<u32>(t, 0x23c) = 0x40060a92;
    at<u32>(t, 0x244) = 0x40060a92;
    at<u32>(t, 0x240) = 0x40425c20;
    at<u32>(t, 0x248) = 0x40425c20;
    at<u32>(t, 0x250) = 0x40425c20;
    at<u32>(t, 0x258) = 0x40425c20;
    at<u32>(t, 0x1c8) = 0x4004;
    at<u32>(t, 0x278) = 0xbf4ccccd;   // -0.8
    at<u32>(t, 0x27c) = 1;
    at<u32>(t, 0x280) = 1;
    at<u32>(t, 0x220) = five;
    at<u32>(t, 0x294) = 1;
    at<u32>(t, 0x24c) = 0;
    at<u32>(t, 0x254) = 0;
    at<u32>(t, 0x268) = 0x51;
    at<u32>(t, 0x25c) = 0x3fc90fdb;   // pi / 2
    at<u32>(t, 0x260) = 0x3fc90fdb;
    at<u32>(t, 0x274) = 0x41c80000;   // 25
    at<float>(t, 0x270) = (float)X87_Sqrt(X87Reg(global<double>(0x006fde90)));
    CoasterType_Setup(t, id, source);   // (its result unused: sic)
    at<u32>(at<u8 *>(t, 0x1c), 0xc) = 0x0074cb98;   // "Data\Generic\Dynamic\Textures"
    at<u32>(balance, 0) = 0x006fd8b0;
    BalanceFile_Release(balance);
    return t;
}
}
