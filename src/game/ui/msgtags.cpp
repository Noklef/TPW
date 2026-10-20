#include "msgtags.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "window.h"
#include "windowreg.h"

extern "C" {

u8 *__cdecl MsgTag_NewWindow(u32 id) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x138));
    u8 *const window = memory ? Window_Ctor(memory, nullptr, nullptr, 0, id, 0, 0, 0x46, 0x41) : nullptr;
    if (window) {
        Window_SetObject98(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName("msgtag")));
        Window_SetHandler(window, nullptr, 0x0048dae0);
        *reinterpret_cast<u32 *>(window + 0x90) = 0x1db;
    }
    return window;
}

void __cdecl MsgTags_Create() {
    for (u32 i = 0; i < 10; i++) {
        u8 *const record = reinterpret_cast<u8 *>((uintptr_t)(0x007c26b0 + 0x208 * i));
        *reinterpret_cast<u16 *>(record + 4) = 0;
        u8 *const window = MsgTag_NewWindow(0x384 + i);
        *reinterpret_cast<u8 **>(record) = window;
        if (window) Window_SetDwordA0(window, nullptr, 2, (u32)(uintptr_t)(record + 6));
    }
}
}  // extern "C"
