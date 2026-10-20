#include "buttontext.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../render/textparts.h"
#include "window.h"
#include "window705a50.h"
#include "windowreg.h"

extern "C" {

u32 __cdecl ButtonText_Init(u8 *window, u32 text, u32 font, u32 red, u32 green, u32 blue, u32 value) {
    u8 *const part = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(window, nullptr, part ? TextPart_Ctor(part, nullptr, 0) : nullptr);
    Window_SetObject98(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName("!cbut")));
    Window_NotifyC00(window, nullptr, 1, 1);
    Window_Notify3000(window, nullptr, 1, 1);
    Window_Set4c(window, nullptr, Language_Font(font));
    Window_Notify40000(window, nullptr, red, green, blue, 0xff);
    const u32 colour = (red & 0xff) | (green & 0xff) << 8 | (blue & 0xff) << 16 | 0xff000000u;
    Window705a50_Set14c(window, nullptr, colour);
    Window705a50_Set154(window, nullptr, colour);
    Window_NotifyImpl100(window, nullptr, text);
    *reinterpret_cast<u32 *>(window + 0x90) = value;
    return value;
}
}  // extern "C"
