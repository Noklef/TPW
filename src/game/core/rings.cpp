#include "rings.h"

#include <cstdint>
#include <cstring>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u16 *__fastcall Ring16_Get(u8 *object, void *, u16 *out, s32 which, s32 index) {
    const u8 *record = object + 20 * (u32)(which + 0x18d2);
    const s32 capacity = at<s32>(record, 0xc);
    bool found = false;
    u16 value = 0;
    if (index < capacity) {
        const s32 head = at<s32>(record, 8);
        if (index <= head) {
            found = true;
            value = at<u16>(record, 2 * (u32)(head - index) + 2);
        } else if (at<u8>(record, 0x10)) {
            found = true;
            value = at<u16>(record, 2 * (u32)(head - index + capacity) + 2);
        }
    }
    *out = found ? value : 0;
    return out;
}

void __fastcall Ring_PushUnique(u8 *object, void *, u32 id, u32 value) {
    const u32 position = at<u32>(object, 0x88);
    const u8 *last = position >= 1 ? object + position * 4 : object + position * 4 + 0x80;
    if (at<u16>(last, 0) == id && at<u16>(last, 2) == (u16)value)
        return;
    at<u16>(object, position * 4 + 4) = (u16)id;
    at<u16>(object, position * 4 + 6) = (u16)value;
    at<u32>(object, 0x88) = (at<u32>(object, 0x88) + 1) & 0x1f;
}

u8 *__fastcall Ring_FromEnd(u8 *object, void *, u32 index) {
    const u32 count = at<u32>(object, 0x88);
    if (count >= index)
        return object + (count - index) * 4 + 4;
    return object + (count - index) * 4 + 0x84;
}

u32 __stdcall Ring_Splice(u8 *position, u32, u8 *first, u8 *last) {
    at<u8 *>(at<u8 *>(last, 4), 0) = position;
    at<u8 *>(at<u8 *>(first, 4), 0) = last;
    at<u8 *>(at<u8 *>(position, 4), 0) = first;
    u8 *lastPrev = at<u8 *>(last, 4);
    u8 *positionPrev = at<u8 *>(position, 4);
    at<u8 *>(position, 4) = lastPrev;
    at<u8 *>(last, 4) = at<u8 *>(first, 4);
    at<u8 *>(first, 4) = positionPrev;
    return (u32)(uintptr_t)first;
}
}  // extern "C"
