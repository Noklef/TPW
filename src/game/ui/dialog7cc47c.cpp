#include "dialog7cc47c.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "hoverlabel.h"
#include "window.h"
#include "windowscript.h"

namespace {

u8 *&dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cc47c); }

// A new TextPart6ff530 (none when the allocation fails) as the window's implementation.
void dress(u8 *window) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(window, nullptr, memory ? TextPart6ff530_Ctor(memory, nullptr, 0) : nullptr);
}

}  // namespace

void __cdecl Dialog7cc47c_Create() {
    if (dialog()) return;
    u8 *const parent = *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>((uintptr_t)0x00f82884) + 4);
    if (!parent) return;
    dialog() = WindowScript_Build(parent, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00757d88), 0x004b86a0);
    u8 *const label = Window_FindChild(dialog(), nullptr, 0x49d9);
    dress(label);
    Window_Set4c(label, nullptr, Language_Font(5));
    Window_NotifyC00(label, nullptr, 1, 1);
    vcall<void>(label, 0x24, 0u);
    HoverLabel_SetYellow(label);
    Window_Set4c(label, nullptr, Language_Font(0));
    Window_Notify40000(label, nullptr, 0xfc, 0xe8, 7, 0xff);
    u8 *const other = Window_FindChild(dialog(), nullptr, 0x49da);
    dress(other);
    Window_Set4c(other, nullptr, Language_Font(0));
    Window_NotifyC00(other, nullptr, 1, 1);
    Window_Notify40000(other, nullptr, 0xfc, 0xe8, 7, 0xff);
}
