#include "dialog7cb2b8.h"

#include <cstdint>

#include "../core/accessors.h"
#include "guirect.h"
#include "window.h"

extern "C" {

void __cdecl Dialog7cb2b8_ShowRightOf(u8 *window, u32 limit) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    if (window) {
        Window_GetRect(window, nullptr, rect, 1);
        Window_SetShown(window, nullptr, *reinterpret_cast<const s16 *>(rect + 8) > (s16)limit ? 1 : 0);
    }
    for (u8 *child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(window, nullptr, 0)); child;
         child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(child))) {
        Dialog7cb2b8_ShowRightOf(child, limit);
    }
    GuiRect_Dtor(rect);
}
}  // extern "C"
