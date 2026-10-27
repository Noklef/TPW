#include "shufflebag.h"

#include <cstdint>
#include <cstring>

#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *entry(u32 address) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(address)); }

}  // namespace

extern "C" {

void __fastcall ShuffleBag_Shuffle(u8 *bag) {
    at<u32>(bag, 8) = 0;
    const u32 seed = at<u32>(bag, 4) * 0x19660d + 0x3c6ef35f;
    at<u32>(bag, 4) = seed;
    const u32 k = ((seed >> 13) | (seed << 19)) % at<u32>(at<u8 *>(bag, 0), 4);
    u32 first = at<u32>(at<u8 *>(bag, 0), 8);
    u32 a = first + k * 0x2a - 0x2a;
    u32 b = first + k * 0x2a;
    while (a >= first) {
        const u8 *list = at<u8 *>(bag, 0);
        if (b >= first + at<u32>(list, 4) * 0x2a - 0x2a) return;
        u8 swap[0x2a];
        memcpy(swap, entry(a), 0x2a);
        memcpy(entry(a), entry(b), 0x2a);
        memcpy(entry(b), swap, 0x2a);
        a -= 0x2a;
        bag[0xe] = !bag[0xe];
        b += bag[0xc + bag[0xe]] * 0x2a;
        first = at<u32>(at<u8 *>(bag, 0), 8);
    }
}

void __fastcall ShuffleBag_SetList(u8 *bag, void *, u8 *list) {
    at<u8 *>(bag, 0) = list;
    ShuffleBag_Shuffle(bag);
}

u8 *__fastcall ShuffleBag_Ctor(u8 *bag, void *, u8 *list) {
    at<u32>(bag, 8) = 0;
    bag[0xe] = 0;
    bag[0xc] = 1;
    bag[0xd] = 2;
    at<u32>(bag, 4) = Time_GetMs();
    ShuffleBag_SetList(bag, nullptr, list);
    return bag;
}

u8 *__fastcall ShuffleBag_Next(u8 *bag, void *, u32 refill) {
    if (at<u32>(bag, 8) >= at<u32>(at<u8 *>(bag, 0), 4)) {
        if (!refill) return nullptr;
        ShuffleBag_Shuffle(bag);
    }
    const u32 i = at<u32>(bag, 8);
    at<u32>(bag, 8) = i + 1;
    return entry(at<u32>(at<u8 *>(bag, 0), 8) + i * 0x2a);
}

}  // extern "C"
