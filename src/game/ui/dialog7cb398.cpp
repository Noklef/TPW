#include "dialog7cb398.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../render/renderer.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "../world/analyser.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "focus.h"
#include "radiogroup.h"
#include "window.h"
#include "window705a50.h"
#include "windowscript.h"
#include "windowtimers.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *dialog() { return global<u8 *>(0x007cb398); }

// The game record's analyser (GameTable7cfb90_Get1da720_519510 of [0x7cf83c]).
u8 *analyser() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c))); }

// A new part of 0x1a4 bytes made by ctor with 0 (none when the allocation fails).
u8 *part(u8 *(__fastcall *ctor)(u8 *, void *, u32)) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    return memory ? ctor(memory, nullptr, 0) : nullptr;
}

u8 *mesh(const char *name) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
    return memory ? MeshOverlay_Construct(memory, nullptr, name) : nullptr;
}

// A TextPart_Ctor part, the bytes 1, 1 as 0xc00, font 6 and the bytes 0, 0, 0, 0xff as 0x40000.
void label(u8 *child) {
    Window_SetImpl(child, nullptr, part(TextPart_Ctor));
    Window_NotifyC00(child, nullptr, 1, 1);
    Window_Set4c(child, nullptr, Language_Font(6));
    Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
}

// The child `id` of window labelled (label()), given no text (Window_NotifyImpl100(0)) and the language text `text` through its slot 9.
void titled(u8 *window, u32 id, u32 text) {
    u8 *const child = Window_FindChild(window, nullptr, id);
    label(child);
    Window_NotifyImpl100(child, nullptr, 0);
    vcall<void>(child, 0x24, (u32)(uintptr_t)Language_WideText(text));
}

// The child `id` of window given a part made by ctor and font 6; returns it.
u8 *plain(u8 *window, u32 id, u8 *(__fastcall *ctor)(u8 *, void *, u32)) {
    u8 *const child = Window_FindChild(window, nullptr, id);
    Window_SetImpl(child, nullptr, part(ctor));
    Window_Set4c(child, nullptr, Language_Font(6));
    return child;
}

// The value through the child's slot 7 when it differs from the one kept at `kept` (kept then).
void figure(u8 *window, u32 id, u32 kept, s32 value) {
    if (global<s32>(kept) == value) return;
    global<s32>(kept) = value;
    vcall<void>(Window_FindChild(window, nullptr, id), 0x1c, global<s32>(kept));
}

}  // namespace

extern "C" {

u32 __cdecl Dialog7cb398_TopHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message != 0xc) return (u32)Window_CallDefault(window, nullptr, message, a, b);
    const u32 index = vcall<u32>(window, 0x20) + 1;
    global<u16 *>(0x007cb314) = Language_Widen(Records_DataIfTerminated(global<const u8 *>(0x0078ba9c), nullptr, index));
    return 0x007cb310;
}

void __cdecl Dialog7cb398_ShowTopThree() {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007cb398), nullptr, 0x4a68d1);
    if (!panel) return;
    u8 top[3];
    GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c));   // (the original's ECX for the call below, which ignores it)
    Things_TopThreeValues(&top[0], &top[1], &top[2]);
    static const u32 kIds[3] = {0x4a68d4, 0x4a68d3, 0x4a68d2};
    for (int k = 0; k < 3; ++k) {
        u8 *const child = Window_FindChild(panel, nullptr, kIds[k]);
        if (top[k] != 0 && top[k] < 0x17) vcall<void>(child, 0x1c, (u32)top[k] - 1);
        else Window_SetShown(child, nullptr, 0);
    }
}

void __cdecl Dialog7cb398_Create() {
    if (dialog()) return;
    Window7c24c8_Close();
    Global7cb28c_Select(2, 3);
    u8 *const built = WindowScript_Build(global<u8 *>(0x007cb2ac), nullptr, reinterpret_cast<u8 *>(0x007537d0), 0x004a4830);
    if (!built) return;
    global<u8 *>(0x007cb398) = built;
    Window7c24c8_Set(built);
    if ((u16)Window_SetTimer(dialog(), nullptr, 0x80083, 1000)) Window_StartTimer(dialog(), nullptr, 0x80083);
    u8 *const title = Window_FindChild(dialog(), nullptr, 0x4a68ce);
    label(title);
    Window_NotifyImpl100(title, nullptr, (u32)(uintptr_t)Language_WideText(0x9c));
    u8 *const top = Window_FindChild(dialog(), nullptr, 0x4a68d1);
    for (u32 id = 0x4a68d2; id <= 0x4a68d4; ++id) Window_SetHandler(Window_FindChild(top, nullptr, id), nullptr, 0x004a4700);
    Dialog7cb398_ShowTopThree();
    if (u8 *const panel = Window_FindChild(dialog(), nullptr, 0x4a68bb)) {
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0xd8));
        Window_SetImpl(panel, nullptr, memory ? RendererPart_Ctor(memory, nullptr, 0) : nullptr);
        Window_SetHandler(panel, nullptr, 0x004a4b90);
        if (u8 *const radio = Window_FindChild(panel, nullptr, 0x4a68c1)) {
            titled(radio, 0x4a68c3, 0x16a);
            titled(radio, 0x4a68c4, 0x16b);
            titled(radio, 0x4a68c2, 0x16c);
            static const u32 kChoice[3] = {0x4a68c3, 0x4a68c4, 0x4a68c2};
            const u32 choice = global<u32>(0x007cb39c);
            if (choice < 3) RadioGroup_Select(radio, nullptr, kChoice[choice]);
        }
        static const u32 kLabels[5][2] = {{0x4a68bc, 0x98}, {0x4a68bd, 0x99}, {0x4a68be, 0x9a}, {0x4a68bf, 0x97}, {0x4a68c0, 0x9b}};
        for (const auto &l : kLabels) titled(panel, l[0], l[1]);
        for (u32 k = 0; k < 5; ++k)
            if (global<u8>(0x00753c48) & (1u << k)) Window705a50_SetState(Window_FindChild(panel, nullptr, kLabels[k][0]), nullptr, 1, 1);
    }
    u8 *const counts = Window_FindChild(dialog(), nullptr, 0x4a68c9);
    Window_NotifyImpl100(plain(counts, 0x4a68cb, TextPart_Ctor), nullptr, (u32)(uintptr_t)Language_WideText(0x97));
    vcall<void>(plain(counts, 0x4a68cc, TextPart6ff350_Ctor), 0x1c, 0u);
    Window_NotifyImpl100(plain(counts, 0x4a68cd, TextPart_Ctor), nullptr, (u32)(uintptr_t)Language_WideText(0x9e));
    vcall<void>(plain(counts, 0x4a68ca, TextPart6ff350_Ctor), 0x1c, 0u);
    global<s32>(0x007cb360) = -1;
    global<s32>(0x007cb378) = -1;
    u8 *const again = Window_FindChild(dialog(), nullptr, 0x4a68c9);
    analyser();   // (the original's ECX for the call below, which ignores it)
    const s32 people = Things_CountKind1OnPlainCell();
    const s32 total = (s32)ParkAnalyser_Total(analyser());
    figure(again, 0x4a68cc, 0x007cb360, people);
    figure(again, 0x4a68ca, 0x007cb378, total);
    u8 *const sad = Window_FindChild(dialog(), nullptr, 0x4a68d0);
    Window_SetImpl(sad, nullptr, mesh(reinterpret_cast<const char *>(0x00753c54)));   // "sad_undec.wct"
    vcall<void>(sad, 0x1c, 0u);
    u8 *const happy = Window_FindChild(dialog(), nullptr, 0x4a68cf);
    Window_SetObject98(happy, nullptr, mesh(reinterpret_cast<const char *>(0x00753c4c)));   // "hap.wct"
    label(happy);
    Window_NotifyImpl100(happy, nullptr, (u32)(uintptr_t)Language_WideText(0x9d));
    Window_SetDepth(happy, nullptr, (u16)(*reinterpret_cast<const u16 *>(Window_FindChild(dialog(), nullptr, 0x4a68d0) + 0xe0) + 2));
    const s32 below = Things_PercentPeopleStat19cBelow(analyser(), nullptr, 0x21);
    const s32 above = Things_PercentPeopleStat19cAbove(analyser(), nullptr, 0x42);
    vcall<void>(Window_FindChild(dialog(), nullptr, 0x4a68d0), 0x1c, (s32)((u32)below << 10) / 100);
    vcall<void>(Window_FindChild(dialog(), nullptr, 0x4a68cf), 0x1c, (s32)((u32)(100 - above) << 10) / 100);
    Advisor_PostId(199);
}

}
