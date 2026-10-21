#include "soundchannel.h"

#include <cstdint>

#include "soundeax.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 *__fastcall SoundChannel_Apply(u8 *channel, void *, u32 *out, u8 *functor) {
    vcall<void>(functor, 0, out, channel);
    return out;
}

u32 *__fastcall SoundChannel_GetSettings(u8 *channel, void *, u32 *out, u8 *settings) {
    u8 *const record = settings;   // (the argument's slot is EaxBuffer_QueryAll's out below)
    if (at<u32>(record, 4) & 0x100) {
        bool clear = at<u32>(channel, 0x20) != 0;
        if (!clear) {
            u8 *const source = at<u8 *>(channel, 0x18);
            clear = source && !vcall<u8>(source, 0x4c);
        }
        if (clear) at<u32>(record, 4) &= ~0x100u;
    }
    if ((at<u32>(record, 4) & 0x200) && !at<u32>(channel, 0x20)) at<u32>(record, 4) &= ~0x200u;
    const u32 asked = at<u32>(record, 8);
    if ((asked & 0x100) && at<u8 *>(channel, 0x14)) {
        const u32 value = at<u32>(at<u8 *>(channel, 0x14), 0);
        const u32 flags = at<u32>(record, 0x4c) | 0x100;
        at<u32>(record, 0x44) = value;
        at<u32>(record, 0x4c) = flags;
    }
    if ((asked & 0x80) && at<u8 *>(channel, 0x14)) {
        at<u32>(record, 0x40) = at<u32>(at<u8 *>(channel, 0x14), 8);
        at<u32>(record, 0x4c) |= 0x80;
    }
    if (asked & 1) {
        vcall<void>(at<u8 *>(channel, 0x18), 0xc, record);
        if (at<u32>(record, 0)) at<u32>(record, 0x4c) |= 1;
    }
    if ((record[8] & 0x40) && at<u8 *>(channel, 0x20)) {
        const u32 *const status = EaxBuffer_QueryAll(at<u8 *>(channel, 0x20), nullptr, reinterpret_cast<u32 *>(&settings), 0);
        if (static_cast<s32>(*status) >= 0) at<u32>(record, 0x4c) |= 0x40;
    }
    if (at<u32>(record, 8) & 0x400) {
        at<u32>(record, 0x6c) = global<u32>(0x00fb2058);
        at<u32>(record, 0x4c) |= 0x400;
    }
    *out = 0;
    return out;
}

}  // extern "C"
