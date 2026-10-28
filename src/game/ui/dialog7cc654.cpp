#include "dialog7cc654.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../gfx/guitext.h"
#include "guirect.h"
#include "window.h"
#include "window705b20.h"

namespace {

s32 word(const u8 *p, u32 offset) { return *reinterpret_cast<const s16 *>(p + offset); }

}  // namespace

extern "C" {

void __cdecl Dialog7cc654_FitText() {
    u8 *const window = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc654), nullptr, 0xee2);
    u8 *const bar = Window_FindChild(window, nullptr, 0xee3);
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetSavedRect(window, nullptr, rect);
    const s32 gw = *reinterpret_cast<const s16 *>((uintptr_t)0x00faa5c4), gh = *reinterpret_cast<const s16 *>((uintptr_t)0x00faa5c0);
    const s32 area[4] = {0, 0, (word(rect, 8) - word(rect, 4)) * gw / 2048, (word(rect, 0xa) - word(rect, 6)) * gh / 1536};
    s32 box[4] = {0, 0, 0, 0};
    GuiText_Measure(reinterpret_cast<const u16 *>((uintptr_t)0x007cb4d0), area, 6, 0, 0, 1, box);
    const s32 height = word(rect, 0xa) - word(rect, 6);
    const s32 needed = (s32)((u32)box[3] * 1536u) / gh;
    if (needed > height) {
        Window_SetShown(bar, nullptr, 1);
        Window705b20_SetRange(bar, nullptr, (s32)((u32)needed - (u32)height), 0);
    } else {
        Window_SetShown(bar, nullptr, 0);
    }
    if (!(u16)vcall<u32>(bar, 0x1c, 0u)) vcall<void>(window, 0x24, 0x007cb4d0u);
    GuiRect_Dtor(rect);
}
}
