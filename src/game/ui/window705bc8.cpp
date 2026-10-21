#include "window705bc8.h"

#include <cstdint>

#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705bc8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705bc8;
    at<u32>(window, 0x144) = 0;
    at<u32>(window, 0x148) = 0;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 0xfff;
    at<u32>(window, 0x14c) = 0;
    at<u32>(window, 0x150) = 1;
    at<u32>(window, 0x158) = 0;
    at<u32>(window, 0x15c) = 0;
    at<u32>(window, 0x140) = at<u32>(window, 0x13c) - at<u32>(window, 0x138);
    at<u32>(window, 0x154) = at<u32>(window, 0x150) - at<u32>(window, 0x14c);
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    at<u16>(window, 0x164) = 0;
    Window_SetHandler(window, nullptr, 0x66d001);
    return window;
}

u8 *__fastcall Window705bc8_SetRange(u8 *window, void *, const s32 *low, const s32 *high) {
    if (low) at<s32>(window, 0x138) = *low;
    if (high) at<s32>(window, 0x13c) = *high;
    if (at<s32>(window, 0x138) > at<s32>(window, 0x13c)) {
        at<s32>(window, 0x138) = at<s32>(window, 0x13c);
        at<s32>(window, 0x13c) = at<s32>(window, 0x138);  // sic
    }
    at<s32>(window, 0x140) = at<s32>(window, 0x13c) - at<s32>(window, 0x138);
    return window;
}

u8 *__fastcall Window705bc8_SetRange2(u8 *window, void *, const s32 *low, const s32 *high) {
    if (low) at<s32>(window, 0x14c) = *low;
    if (high) at<s32>(window, 0x150) = *high;
    if (at<s32>(window, 0x14c) > at<s32>(window, 0x150)) {
        at<s32>(window, 0x14c) = at<s32>(window, 0x150);
        at<s32>(window, 0x150) = at<s32>(window, 0x14c);  // sic
    }
    if (at<s32>(window, 0x158) < at<s32>(window, 0x14c)) at<s32>(window, 0x158) = at<s32>(window, 0x14c);
    else if (at<s32>(window, 0x158) > at<s32>(window, 0x150)) at<s32>(window, 0x158) = at<s32>(window, 0x150);
    at<s32>(window, 0x154) = at<s32>(window, 0x150) - at<s32>(window, 0x14c);
    return window;
}

s32 __fastcall Window705bc8_FirstToSecond(u8 *window) {
    if (at<s32>(window, 0x144) == at<s32>(window, 0x13c)) return at<s32>(window, 0x150);
    if (at<s32>(window, 0x144) == at<s32>(window, 0x138)) return at<s32>(window, 0x14c);
    const s32 product = (s32)(at<u32>(window, 0x154) * at<u32>(window, 0x144));   // (wrapping, as IMUL)
    return (s32)((u32)(product / at<s32>(window, 0x140)) + at<u32>(window, 0x14c));
}

s32 __fastcall Window705bc8_SecondToFirst(u8 *window) {
    if (at<s32>(window, 0x158) == at<s32>(window, 0x150)) return at<s32>(window, 0x13c);
    if (at<s32>(window, 0x158) == at<s32>(window, 0x14c)) return at<s32>(window, 0x138);
    const s32 product = (s32)((at<u32>(window, 0x158) - at<u32>(window, 0x14c)) * at<u32>(window, 0x140));   // (wrapping, as IMUL)
    return (s32)((u32)(product / at<s32>(window, 0x154)) + at<u32>(window, 0x138));
}
}
