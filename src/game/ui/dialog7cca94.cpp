#include "dialog7cca94.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../render/textparts.h"
#include "window.h"
#include "windowscript.h"

namespace {

u8 *&dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cca94); }

u8 *text() { return Window_FindChild(dialog(), nullptr, 0x2241); }

}  // namespace

extern "C" {

s32 __cdecl Dialog7cca94_OnMessage(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) dialog() = nullptr;
    return Window_CallDefault(window, nullptr, message, a, b);
}

void __cdecl Dialog7cca94_Show(const u16 *string) {
    if (dialog()) {
        vcall<void>(text(), 0x24, string);
        return;
    }
    dialog() = WindowScript_Build(*reinterpret_cast<u8 **>((uintptr_t)0x00faa5d8), nullptr, reinterpret_cast<u8 *>(0x00759548), 0x004c23c0);
    u8 *memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
    Window_SetImpl(dialog(), nullptr, memory ? MeshOverlay_Construct(memory, nullptr, reinterpret_cast<const char *>(0x007595a0)) : nullptr);
    vcall<void>(dialog(), 0x1c, 0u);
    memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    u8 *const part = memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr;
    Window_SetImpl(text(), nullptr, part);
    Window_NotifyC00(text(), nullptr, 1, 1);
    Window_Notify3000(text(), nullptr, 1, 1);
    u8 *const font = Language_Font(6);
    Window_Set4c(text(), nullptr, font);
    Window_Notify40000(text(), nullptr, 0, 0, 0, 0xff);
    Window_NotifyImpl100(text(), nullptr, (u32)(uintptr_t)string);
    Window_SetDepth(dialog(), nullptr, 0x23);
    vcall<void>(dialog(), 0x14, 0u);
}

void __cdecl Dialog7cca94_ShowText(u32 index) {
    if (!dialog() || index != *reinterpret_cast<u32 *>((uintptr_t)0x007cca9c)) Dialog7cca94_Show(Language_WideText(index));
    *reinterpret_cast<u32 *>((uintptr_t)0x007cca9c) = index;
}

void __cdecl Dialog7cca94_SetProgress(u32 percent) {
    if (u8 *const w = dialog()) vcall<void>(w, 0x1c, (u32)((s32)(percent << 10) / 100));
}
}
