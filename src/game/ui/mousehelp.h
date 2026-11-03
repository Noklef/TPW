// The help line and cursor for what the mouse is over. Reimplementation of 0x486d90; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x4e2780 agent A's, by address until ported; the bench's recorders: their own benches have them).
struct MouseHelpEnv {
    u8 *(__cdecl *ensureModes)();                                                // MouseModes_EnsureHolder
    u32(__cdecl *tool)();                                                        // Global7cc2f0_Get_4b4480
    u32(__cdecl *other)();                                                       // Global81ae2c_Get_52f840
    u32(__cdecl *rideThing)();                                                   // Global7c2654_Get
    void(__cdecl *highlight)();                                                  // Highlight_Update
    void(__cdecl *help)(const u16 *text);                                        // HelpBg_SetText
    void(__cdecl *cursor)(u32 id);                                               // GameCursor_Set
    u16 *(__fastcall *entrance)(const u8 *map, void *, u16 *out, u32 b);         // Map_EntrancePos
    u16 *(__fastcall *booth)(const u8 *map, void *, u16 *out, u32 b);            // Map_TicketBoothPos
    u16 *(__fastcall *firstObject)(const u8 *cell, void *, u16 *out);            // Cell_FirstObject
    u16 *(__cdecl *objectAt)(u16 *out, s32 x, s32 y);                            // Map_ObjectOrEntranceAt
    u8 *(__fastcall *def)(const u8 *object);                                     // Object_GetDef
    u16 *(__fastcall *word284)(const u8 *object, void *, u16 *out);              // Object_DefWord284
    void(__fastcall *copyName)(u8 *thing, void *, u16 *buffer, u32 size);        // 0x4e2780 (agent A's)
    u16 *(__fastcall *staffName)(u8 *staff);                                     // Staff_GetName
    void(__fastcall *settle)(u8 *staff);                                         // Staff_Settle
    u32(__fastcall *isType9)(const u8 *cell);                                    // MapCell_IsType9
    u32(__fastcall *isTypeA)(const u8 *cell);                                    // MapCell_IsTypeA
    u32(__fastcall *isType4)(const u8 *cell);                                    // MapCell_IsType4
    u32(__fastcall *isType0)(const u8 *cell);                                    // MapCell_IsType0
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const u32 *argument);   // Charset_FormatWide (no argument: null, the original leaves it out)
    u8 *(__fastcall *records)(const u8 *object, void *, u32 index);              // Records_DataIfTerminated
    u32 *(__fastcall *widen)(const u8 *object, void *, u32 *status, u16 *dest, const u8 *source, u32 count);   // Wide_FromNarrow
};
void MouseHelp_SetEnv(const MouseHelpEnv *env);

extern "C" {

// 0x486d90 (cdecl (forced); name invented, unconfirmed): the help line (HelpBg_SetText; the texts of the charset [0x78ba98] and buffer 0x7ba058, 0x78a374 to clear it) and the cursor (GameCursor_Set) for what is under the mouse; [0x7c24f4] the kind shown, kept to tell a change, [0x7c24f8] the thing named. The mode: 1 when forced, else the mode holder [0x7b05d8]'s slot 9 (none: 0).
// Mode 8, or 3 with [0x81ae2c] 0x15 or 0x16: Highlight_Update alone, unless [0x7a1c7c] clear and the tile's ride queue's first thing not the current ride. Mode 9: the guest or object text of the tile. Other modes but 0 and 1: cleared. Modes 0 and 1, by the tool [0x7cc2f0] and modifiers, the object [0x7b05ac] and the tile [0x7b05cc]: text entries 0x229/0x22a (a person),
// 0x22b (staff, settled), 0x1cb/0x1cc (the entrance and booth tiles), 0x1be (a ride's queue), 0x1b9, 0x1ba, 0x1bc, 0x226, 0x1c0 by the cell's type.
void __cdecl MouseHelp_Update(u32 forced);
}
