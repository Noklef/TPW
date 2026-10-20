#include "lists.h"

#include <cmath>
#include <cstdint>

#include "../math/x87.h"
#include "../model/holder.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__cdecl CoasterList_Append(u8 *list) {
    u8 *mesh = at<u8 *>(at<u8 *>(at<u8 *>(global<u8 *>(0x00792218), 0x38), 0x24), 4);
    u8 *item = global<u8 *>(0x0079c604);
    at<u32>(item, 8) = 0;
    if (!at<u8 *>(list, 8)) at<u8 *>(list, 8) = item;
    if (at<u8 *>(list, 0xc)) at<u8 *>(at<u8 *>(list, 0xc), 8) = item;
    at<u8 *>(item, 0xc) = at<u8 *>(list, 0xc);
    at<u8 *>(list, 0xc) = item;
    if (!at<u8 *>(list, 0x10)) {
        u8 *slot = at<u8 *>(at<u8 *>(mesh, 0x78), 0x6c) + (global<u32>(0x007925c8) << 4);
        at<u8 *>(list, 0x10) = slot;
        at<u8>(slot, 0xc) = list[0] & 1;
        ++at<u16>(mesh, 0x3c);
        ++at<u16>(at<u8 *>(mesh, 0x78), 0x5a);
        ++global<u32>(0x007925c8);
    }
    ++at<u32>(list, 4);
    global<u8 *>(0x0079c604) += 0x10;
    return item;
}

void __cdecl CoasterMesh_Finish(u32 keep) {
    const auto coaster = []() { return global<u8 *>(0x00792218); };
    const auto copy3 = [](u8 *to, u32 from) {
        for (u32 k = 0; k < 0xc; k += 4)
            at<u32>(to, k) = global<u32>(from + k);
    };
    const auto tenth = [](u32 address) { return (u32)(long long)(X87Reg(global<float>(address)) * 0.100000001f).v; };   // 0x6fe144, __ftol
    const u8 *const build = at<const u8 *>(coaster(), 4);
    const u8 *const source = at<const u8 *>(build, 0x18);
    const u8 *const sizes = at<const u8 *>(build, 0x14);
    if (at<u8 *>(coaster(), 0x38)) {
        u8 *const mesh = at<u8 *>(at<u8 *>(at<u8 *>(coaster(), 0x38), 0x24), 4);
        copy3(mesh + 0x80, 0x007925b8);
        copy3(mesh + 0x8c, 0x007925a8);
        u8 *const part = at<u8 *>(mesh, 0x78);
        copy3(part + 0x78, 0x007925b8);
        copy3(part + 0x84, 0x007925a8);
        const u32 a = tenth(0x007925b8);
        const u32 b = tenth(0x007925c0);
        const u32 c = tenth(0x007925a8);
        const u32 d = tenth(0x007925b0);
        at<u32>(mesh, 0xa8) = (a & 0xff) << 24 | (c & 0xff) << 16 | (b & 0xff) << 8 | (d & 0xff);
        at<u32>(mesh, 0x30) |= 0xc00;
        u8 *const lists = global<u8 *>(0x007925dc);
        u8 *const animator = at<u8 *>(coaster(), 0x38);
        u32 slot = at<u32>(at<u8 *>(at<u8 *>(animator, 0x24), 4), 0x50);
        u8 *pairs = at<u8 *>(animator, 0x1c);
        const u32 count = at<u16>(at<u8 *>(at<u8 *>(coaster(), 4), 0x1c), 4);
        u32 first = 0;
        for (u32 i = 0; i < count; ++i, slot += 8) {
            u8 *const list = lists + i * 0x1c;
            if (!at<u8 *>(list, 0x10))
                continue;
            at<u16>(at<u8 *>(list, 0x10), 8) = at<u16>(list, 0x14);
            at<u16>(at<u8 *>(list, 0x10), 0xa) = at<u16>(list, 0x18);
            at<u32>(at<u8 *>(list, 0x10), 0) = slot;
            at<u16>(at<u8 *>(list, 0x10), 6) = at<u16>(list, 4);
            at<u16>(at<u8 *>(list, 0x10), 4) = (u16)first;
            for (const u8 *item = at<const u8 *>(list, 8); item; item = at<const u8 *>(item, 8)) {
                at<u32>(pairs, 0) = at<u32>(item, 0);
                at<u32>(pairs, 4) = at<u32>(item, 4);
                pairs += 8;
            }
            first += at<u32>(list, 4);
        }
        u8 *const built = at<u8 *>(at<u8 *>(at<u8 *>(coaster(), 0x38), 0x24), 4);
        at<u16>(built, 0x38) = (u16)((s32)(global<u32>(0x0079c600) + 3) >> 2);
        u8 *const first_part = at<u8 *>(built, 0x78);
        at<u16>(built, 0x3a) = (u16)((s32)(global<u32>(0x007925c4) + 3) >> 2);
        at<u16>(built, 0x3e) = global<u16>(0x007923e8);
        at<u16>(first_part, 0x58) = global<u16>(0x0079c600);
        at<u16>(first_part, 0x5c) = global<u16>(0x007923e8);
        at<u16>(first_part, 0x5e) = global<u16>(0x007925c4);
        Animator_Wake(at<u8 *>(coaster(), 0x38));
    }
    u8 *list = global<u8 *>(0x007925dc);
    for (u32 i = at<u16>(at<u8 *>(at<u8 *>(coaster(), 4), 0x1c), 4); i != 0; --i, list += 0x1c) {
        at<u32>(list, 4) = 0;
        at<u32>(list, 8) = 0;
        at<u32>(list, 0xc) = 0;
        at<u32>(list, 0x10) = 0;
        at<u32>(list, 0x14) = 10000;
        at<u32>(list, 0x18) = 0;
    }
    global<u32>(0x0079c604) = global<u32>(0x00792408);
    double whole;
    const double fraction = std::modf((double)at<float>(global<u8 *>(0x00790fe4), 0x38), &whole);   // CRT modf (0x67b280)
    at<float>(global<u8 *>(0x00790fe4), 0x38) = (float)fraction;
    if (keep)
        return;
    u8 *const old = at<u8 *>(coaster(), 0x38);
    Animator_Clone(source, 0, 0x21ef, reinterpret_cast<u8 **>(coaster() + 0x38));
    at<u8 *>(at<u8 *>(coaster(), 0x38), 0x38) = old;
    Holder_SetPosition(at<u8 *>(at<u8 *>(coaster(), 0x38), 0x24), 0, 0, 0);
    u8 *const mesh = at<u8 *>(at<u8 *>(at<u8 *>(coaster(), 0x38), 0x24), 4);
    u8 *const part = at<u8 *>(mesh, 0x78);
    at<u16>(mesh, 0x3c) = 0;
    at<u16>(part, 0x5a) = 0;
    global<u32>(0x007923e0) = at<u16>(sizes, 6);
    global<u32>(0x007925d4) = at<u16>(sizes, 8);
    global<u32>(0x007925cc) = at<u16>(sizes, 0xc);
    global<u32>(0x00792410) = at<u16>(sizes, 4);
    global<u32>(0x0079c600) = 0;
    global<u32>(0x007925c4) = 0;
    global<u32>(0x007925d8) = 0;
    global<u32>(0x007923e8) = 0;
    global<u32>(0x007925c8) = 0;
    for (u32 k = 0; k < 0xc; k += 4)
        global<u32>(0x007925b8 + k) = 0x49742400;   // 1e6
    for (u32 k = 0; k < 0xc; k += 4)
        global<u32>(0x007925a8 + k) = 0xc9742400;   // -1e6
}
}
