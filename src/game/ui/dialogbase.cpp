#include "dialogbase.h"

#include <cstdint>

#include "../core/accessors.h"
#include "focus.h"
#include "guidepth.h"
#include "guirect.h"
#include "window.h"
#include "windowfactory.h"
#include "windowreg.h"
#include "windowscript.h"

extern "C" {

s32 __cdecl Dialog_BaseHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        Window_PostMessage(reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_65d55a(window))), 4, 0, 0);
        Focus_Restore();
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

u8 *__cdecl Dialog_Create(u8 *script, u32 handler, u32 shaded) {
    u8 *const frame = Window_CreateChild(*reinterpret_cast<u8 **>(static_cast<uintptr_t>(0x00faa5d8)), nullptr, 1, 1, 1, 0, 0, 0x7ff, 0x5ff);
    if (!frame) return nullptr;
    u8 *const window = WindowScript_Build(frame, nullptr, script, handler);
    if (!window) return nullptr;
    s32 depth;
    if (shaded) {
        depth = Gui_TopDepth(nullptr, nullptr);
        Window_SetImpl(frame, nullptr,
                       reinterpret_cast<u8 *>(static_cast<uintptr_t>(GuiWindows_FindByName(reinterpret_cast<const char *>(static_cast<uintptr_t>(0x0074f9bc))))));
    } else {
        u8 rect[0xc];
        GuiRect_Ctor(rect);
        Window_GetRect(window, nullptr, rect, 1);
        depth = Gui_TopDepth(nullptr, rect);
        GuiRect_Dtor(rect);
    }
    Window_SetDepth(frame, nullptr, static_cast<u32>(depth + 1));
    if (handler) Window_SetHandler(window, nullptr, handler);
    Window_SetFocus(window, nullptr, 0);
    return window;
}
}
