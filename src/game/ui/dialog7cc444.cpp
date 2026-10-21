#include "dialog7cc444.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "../world/world.h"
#include "focus.h"
#include "hoverlabel.h"
#include "window.h"
#include "windowscript.h"

namespace {

u8 *&dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cc444); }

// The sample `back` before the cursor of a CircBufU32 of 144 into *value, when it is there (inlined in the original).
void sample(const u8 *buffer, s32 back, u32 *value) {
    const s32 entries = *reinterpret_cast<const s32 *>(buffer + 0x248);
    if (entries <= back) return;
    s32 at = *reinterpret_cast<const s32 *>(buffer + 0x244);
    if (at < back) {
        if (!buffer[0x24c]) return;
        at += entries;
    }
    *value = *reinterpret_cast<const u32 *>(buffer + 4 + (at - back) * 4);
}

// A child with a new part (ctor; none when the allocation fails) as its implementation and the font; returns the allocation.
u8 *dress(u8 *child, u8 *(__fastcall *ctor)(u8 *, void *, u32), u32 font) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? ctor(memory, nullptr, 0) : nullptr);
    Window_Set4c(child, nullptr, Language_Font(font));
    return memory;
}

}  // namespace

extern "C" {

void __cdecl Dialog7cc444_Create() {
    if (dialog()) return;
    Window7c24c8_Close();
    u8 *const built = WindowScript_Build(*reinterpret_cast<u8 **>((uintptr_t)0x007cb2ac), nullptr, reinterpret_cast<u8 *>(0x007579c8), 0x004b7a70);
    if (!built) return;
    dialog() = built;
    Window7c24c8_Set(built);
    HoverLabel_Attach(dialog(), 0xadb3, (u32)(uintptr_t)Language_WideText(0xba));
    u8 *const panel = Window_FindChild(dialog(), nullptr, 0xada1);
    if (!panel) return;
    static const u32 kTexts[][3] = {{0xadae, 8, 0xbc}, {0xadaf, 8, 0xbb}, {0xada9, 6, 0xbd}, {0xada3, 6, 0xbe},
                                    {0xada4, 6, 0xbf}, {0xada5, 6, 0xc0}, {0xadb0, 6, 0xc1}};
    u32 last = 0;   // (the stack slot each allocation goes to)
    for (const auto &t : kTexts) {
        u8 *const child = Window_FindChild(panel, nullptr, t[0]);
        last = (u32)(uintptr_t)dress(child, TextPart_Ctor, t[1]);
        Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(t[2]));
    }
    const u8 *const analyser =
        reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c)));
    u32 figure = last;   // (EBX: the figure before when a sample is not there, sic)
    for (u32 i = 0; i < 2; ++i) {
        u8 *const child = Window_FindChild(panel, nullptr, i ? 0xadac : 0xada8);
        sample(analyser + 0x1f350, i ? 12 : 0, &figure);
        dress(child, TextPart6ff3b0_Ctor, 6);
        vcall<void>(child, 0x1c, figure);
    }
    u32 slot = 0;   // (the dword the 0xada2 part's allocation went to, sic)
    for (u32 i = 0; i < 2; ++i) {
        u8 *const child = Window_FindChild(panel, nullptr, i ? 0xadad : 0xada2);
        u8 *const memory = dress(child, TextPart6ff350_Ctor, 6);
        if (!i) slot = (u32)(uintptr_t)memory;
        vcall<void>(child, 0x1c, 0u);
        Window_NotifyC00(child, nullptr, 2, 1);
        sample(analyser + 0x213b0, i ? 12 : 0, &slot);
        vcall<void>(child, 0x1c, slot);
    }
    static const u32 kFigures[][2] = {{0xada7, 0x1fc90}, {0xadaa, 0x1fc90}, {0xada6, 0x1f5a0}, {0xadab, 0x1f5a0}, {0xadb1, 0x1f100}, {0xadb2, 0x1f100}};
    for (u32 i = 0; i < 6; ++i) {
        u8 *const child = Window_FindChild(panel, nullptr, kFigures[i][0]);
        sample(analyser + kFigures[i][1], (i & 1) ? 12 : 0, &figure);
        dress(child, TextPart6ff3b0_Ctor, 6);
        vcall<void>(child, 0x1c, figure);
    }
}
}
