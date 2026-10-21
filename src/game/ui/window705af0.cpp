#include "window705af0.h"

#include <cstdint>

#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705af0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705af0;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 1;
    at<u32>(window, 0x140) = 0x9708cd;
    at<u32>(window, 0x144) = 0;
    at<u32>(window, 0x148) = 1;
    window[0x154] = 1;
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    Window_SetHandler(window, nullptr, 0x66aa61);
    return window;
}

u32 __fastcall Window705af0_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705af0;
    return Window_Dtor(window);
}

void __fastcall Window705af0_SetRange(u8 *window, void *, const s32 *low, const s32 *high) {
    if (low) at<s32>(window, 0x138) = *low;
    if (high) at<s32>(window, 0x13c) = *high;
    if (at<s32>(window, 0x138) > at<s32>(window, 0x13c)) {
        at<s32>(window, 0x138) = at<s32>(window, 0x13c);
        at<s32>(window, 0x13c) = at<s32>(window, 0x138);  // sic: the swap's saved low is never used, both ends become the high one
    }
    if (at<s32>(window, 0x140) < at<s32>(window, 0x138)) at<s32>(window, 0x140) = at<s32>(window, 0x138);
    else if (at<s32>(window, 0x140) > at<s32>(window, 0x13c)) at<s32>(window, 0x140) = at<s32>(window, 0x13c);
}

void __fastcall Window705af0_SetStep(u8 *window, void *, const s32 *step) {
    if (step) at<s32>(window, 0x148) = *step;
}

s32 __fastcall Window705af0_Distance(u8 *window, void *, u8 *other) {
    const u32 theirs = vcall<u32>(other, 0xc);
    if (theirs != vcall<u32>(window, 0xc)) return 0;
    return at<s32>(window, 0x140) - at<s32>(other, 0x140);
}

u16 __fastcall Window705af0_SetValue(u8 *window, void *, s32 value) {
    if (value > at<s32>(window, 0x13c)) value = at<s32>(window, 0x13c);
    else if (value < at<s32>(window, 0x138)) value = at<s32>(window, 0x138);
    if (value == at<s32>(window, 0x140) && !window[0x154]) return 0;
    window[0x154] = 0;
    at<s32>(window, 0x144) = at<s32>(window, 0x140);
    at<s32>(window, 0x140) = value;
    Window_Notify80(window, nullptr, (u32)value);
    Window_PostMessage(at<u8 *>(window, 0x60), 0x800, at<u32>(window, 0x8c), at<u32>(window, 0x140));
    return 1;
}
}
