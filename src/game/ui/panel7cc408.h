// The game panel object at 0x7cc408 (its vtable 0x6fff40). Reimplementation of 0x4b75a0, 0x4b6c20, 0x4b6b90 and 0x4b79b0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Panel7cc408_Refresh's calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7cc408Env {
    s32(__fastcall *countGroup)(const u8 *self, void *, u32 thing);                // Things_CountGroup
    u8 *(__fastcall *partText)(u8 *part, void *, u32 value);                   // TextPart_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                   // Window_SetImpl
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);             // Window_NotifyC00
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);            // Window_Notify3000
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                   // Window_Set4c
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    void(__fastcall *notifyImpl100)(u8 *window, void *, u32 value);            // Window_NotifyImpl100
};
void Panel7cc408_SetEnv(const Panel7cc408Env *env);

extern "C" {

// 0x4b75a0 (thiscall; slot 0 of the vtable 0x6fff40; name invented): the group counted (Things_CountGroup of the selected thing [0x7cc430], the count dropped) and the window +0x10's child 0x3734 dressed: a TextPart_Ctor part (none when the heap has none),
// NotifyC00 (1, 1), Notify3000 (1, 1), font 6, (0, 0, 0, 0xff), NotifyImpl100 (0).
void __fastcall Panel7cc408_Refresh(u8 *self);

// 0x4b6b90 (thiscall; slot 6 of the vtable 0x6fff40): with a thing selected ([0x7c2658]), its preview shown (ThingPreview_Show) over the
// window +0x18's saved rectangle (a GuiRect on the stack: Window_GetSavedRect), at that window's depth +0xe0 (sign-extended) + 2,
// scaled 12.0.
void __fastcall Panel7cc408_ShowPreview(u8 *self);

// 0x4b79b0 (thiscall (thing), pops 4; slot 10 of the vtable 0x6fff40): the selected thing [0x7cc430] = thing, the window built
// (WindowAt10_Create: the script 0x7575e0, the handler 0x4b7720, the children 0x3722 and 0x3723, no inner one), the sound 0x81 played in
// the kids' category [0x803a24] (Sound_PlayAt(0, ..., 0, 0, 0)) and the advisor posted 0x12e (Advisor_PostId).
void __fastcall Panel7cc408_Show(u8 *self, void *, u8 *thing);

// 0x4b6c20 (thiscall; slot 3 of the vtable 0x6fff40): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x372c text 0x46, 0x3725 text 0x47, 0x3726 text 0x48, 0x3727 text 0x4b, 0x3728 text 0x49, 0x3729 text 0x4a, 0x372a text 0x4c, 0x372b
// text 0x4d, 0x3724 a 0x6ff3b0 part with 0x29a, 0x372d a 0x6ff4a0 part (no slot 7), 0x372e a 0x6ff350 part with 0, 0x372f a 0x6ff350 part
// with 0, 0x3730 a 0x6ff350 part with 0, 0x3731 a 0x6ff350 part with 0, 0x3732 (when found) a happygrad bar (no slot 7), 0x3733 no text.
void __fastcall Panel7cc408_Dress(u8 *self);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x3e8], gPanel7cc408_Show_7575e0, 0x007575e0, 0x3e8);
