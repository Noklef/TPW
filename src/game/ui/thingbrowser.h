// The thing browser's panel (the thing shown [0x7cc308], the panel window [0x7cc304]): its labels filled for the thing, and the handler of
// its windows' radio groups. Reimplementation of 0x4b3490, 0x4b3580, 0x4b3b90, 0x4b3c10, 0x4b3cf0 and 0x4b3f60. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// ThingBrowser_SendPatrolLines's calls (the ports by default; the bench's recorders: their own benches have them).
struct ThingPatrolEnv {
    void(__cdecl *centre)(const u8 *sprite);                                       // Camera_CentreOnSprite
    u32(__cdecl *line)(u32 message, s32 x0, s32 y0, const s32 *x1, const s32 *y1);   // Map_SendLine
};
void ThingBrowser_SetPatrolEnv(const ThingPatrolEnv *env);

// ThingBrowser_Handler's calls (the ports by default; the bench's recorders: their own benches have them).
struct ThingBrowserHandlerEnv {
    void(__cdecl *sendPatrol)();                                             // ThingBrowser_SendPatrolLines
    void(__cdecl *showDetail)();                                             // ThingBrowser_ShowDetail
    void(__cdecl *step)();                                                   // ThingBrowser_Step
    void(__cdecl *showName)();                                               // ThingBrowser_ShowName
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                // Window_SetShown
    u16(__fastcall *unlink)(u8 *window);                                     // Window_Unlink
    u32(__fastcall *addChild)(u8 *window, void *, u8 *child, u32 depth, u32 words);   // Window_AddChild
    void(__cdecl *cancelAdvisor)(u32 id);                                    // Game_CancelAdvisor
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void ThingBrowser_SetHandlerEnv(const ThingBrowserHandlerEnv *env);

extern "C" {

// 0x4b3790 (cdecl (window, message, a, b); name invented): 0x14: [0x7cc2fc] = 0. 0x1e: ThingBrowser_SendPatrolLines, _ShowDetail. 0x1001d8: [0x7cc308] = 0, [0x7cc304] hidden and unlinked, [0x7cc2f0] = 0, flag 0x1000 of [0x790ab0] cleared, the advisor's 0x11f and 0xc6 cancelled.
// 0x101, with [0x7cc2f0] and b 1, a 0x95e to 0x964 but 0x963 (the modes 1, 2, 5, 4, 3, 6): [0x7cc30c] 0: the mode set, ThingBrowser_Step, and when that leaves [0x7cc308] set the panel added and the name shown; otherwise [0x7cc308] cleared for another mode, the mode set, the step: left clear: the panel hidden and unlinked, the flag cleared; set: added when it was not before, the name shown, the flag set.
// Every message then goes to Window_CallDefault, whose answer is returned.
s32 __cdecl ThingBrowser_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b3640 (cdecl; name invented): with a thing shown [0x7cc308] and the flag 0x1000 of [0x790ab0], the camera on it (Camera_CentreOnSprite); when it (the global read again) is staff (kinds 4-8)
// with a patrol region (Staff_HasPatrolRegion, Staff_GetPatrolRegion: cells, column the low 7 bits of the cell - 1, row the rest): the region's four sides sent to the map as lines of message
// 0x136 (Map_SendLine): the first row from column 0 to column 1, the column 1 from row 0 to row 1, the second row, the column 0 back; the rows' ends a column in from each side unless both are one.
void __cdecl ThingBrowser_SendPatrolLines();

// 0x4b3490 (cdecl; name unconfirmed): with a thing shown, the panel's child 0x147 given (its slot 9) the thing's name for staff (kinds
// 4-8: Staff_GetName), else the text 0x1c9 (a count +0x1d8 of 0) or 0x1c8 with the count as argument 0x18 (TextFormat into the buffer
// 0x7ba058, 0x3fff characters, the table [0x78ba94]); then the detail line (ThingBrowser_ShowDetail) and the camera on the thing
// (Camera_CentreOnSprite).
void __cdecl ThingBrowser_ShowName();

// 0x4b3580 (cdecl; name unconfirmed): with a thing shown: for staff (kinds 4-8) its mode's code (Index_ToCode485ec0 of Mode19c_ToIndex),
// when not the one shown last [0x756b98], kept there and told to the child 0x148 (Window_Notify1); for the others the child 0x14a's slot
// 7 given the count +0x30 less 1, or 0x29a for none.
void __cdecl ThingBrowser_ShowDetail();

// 0x4b3b90 (cdecl (window, message, a, b); the handler 0x4b3cf0 gives the child 0x95d of the browser's windows [0x7cc2fc] / [0x7cc300], a
// radio group; name unconfirmed): message 0x100: the group's child a (Window_FindChild) found, a selected (RadioGroup_Select), for b other
// than 1 the child set to state 1, notifying (Window705a50_SetState(1, 1); not checked for 0), and the group's parent (+0x60, Fn_65d55a)
// posted 0x101 (a, 1); 1 returned. Other messages: Window_CallDefault's result.
u32 __cdecl ThingBrowser_GroupHandler(u8 *group, u32 message, u32 a, u32 b);

// 0x4b3c10 (cdecl (window, message, a, b); the handler ThingBrowser_Create gives the panel [0x7cc304]; name invented): message 0x14 the
// panel forgotten ([0x7cc304] 0); 0x10001 / 0x10002 its child 0x149 (the "hilight" window; not checked) shown / hidden (Window_SetShown);
// 0x10006 with a 0 the thing shown [0x7cc308] (not checked) opened in the staff panel 0x7cc3b0 for the kinds 4-8 (Panel7cc3b0_Show),
// else in the panel 0x7cc408 (Panel7cc408_Show). Every message then goes to Window_CallDefault, its result returned.
u32 __cdecl ThingBrowser_PanelHandler(u8 *panel, u32 message, u32 a, u32 b);

// 0x4b3cf0 (cdecl; name unconfirmed): the browser's windows built on [0xfaa5d8] (WindowScript_Build): from the script 0x756950 (no
// handler) the windows 0x95c and 0x963 found on [0xfaa5d8] (Window_FindChild, not checked) as [0x7cc2fc] / [0x7cc300], each the handler
// 0x4b3790, unlinked (Window_Unlink), its dword +0xa0 index 0 set to 3 (Window_SetDwordA0), its child 0x95d the handler 0x4b3b90
// (ThingBrowser_GroupHandler) and hidden; then from the script 0x756ad8 (handler 0x4b3c10) the panel [0x7cc304], its cursor 2
// (Window_SetCursor), unlinked, its children (not checked): 0x147 a TextPart_Ctor part (0x1a4 bytes; none when that fails), the bytes
// 0xff x 4 as 0x40000, font 0xb (Language_Font; Window_Set4c) and no text (Window_NotifyImpl100(0)), its slot 5 given 0; 0x148's slot 5
// given 0; 0x149 the registered window "hilight" (GuiWindows_FindByName) as its object +0x98 (Window_SetObject98), slot 5 given 0 and
// hidden; the panel hidden and [0x7cc2f0] 0.
void __cdecl ThingBrowser_Create();

// 0x4b3f60 (cdecl (thing); name unconfirmed): the browser opened on a thing (or none): the window [0x7cc300] for kind 1 ([0x7cc2f0] 2),
// else [0x7cc2fc] ([0x7cc2f0] 1), posted to the dialog's child 0x21 (Dialog7cb2b8_Child21Post); with a thing: it shown [0x7cc308], the
// kinds 1, 4-8 the page [0x7cc30c] 6, 2, 1, 4, 3, 5 and their button 0x964, 0x95f, 0x95e, 0x961, 0x962, 0x960 selected in the window's
// child 0x95d (RadioGroup_Select; other kinds keep the page and select none), the panel [0x7cc304] added to the dialog [0x7cb2b8] (depth
// and words, Window_AddChild) and shown, its labels filled (ThingBrowser_ShowName) and the flag 0x1000 of [0x790ab0] set; without one
// none shown, the page 0 and the group's selection cleared (-1). Then for the page 6 the panel's child 0x148 hidden, 0x14a shown and the
// advisor told 0x11f, else the reverse and 0xc6 (Advisor_PostId); the window [0x7c24c8] closed (Window7c24c8_Close); the mode holder
// [0x7b05e8] made when missing and, when the current mode [0x7b05d8] answers 9 from its slot 9, a new CMMOnline base set (as
// Window7c24c8_Set's).
void __cdecl ThingBrowser_Open(u8 *thing);

// 0x481790 (cdecl, no arguments; name invented; called at 0x40c595 with ECX 0x7b51f0, not read): ThingBrowser_Open on the shown thing
// [0x7c2658] when [0x7cc3c0] is set, else on none.
void __cdecl ThingBrowser_OpenShown();

// 0x4b3620 (cdecl (thing); name invented): when the browser's thing [0x7cc308] is this one, Dialog7cb2b8_ResetChild21(1).
void __cdecl ThingBrowser_ResetIfThing(u8 *thing);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[192], gThingBrowser_Create_756ad8, 0x00756ad8, 0xc0);
GAME_DATA(u32, gThingbrowser_7cc2f0, 0x007cc2f0, 0x4);
GAME_DATA(u8 *, gThingbrowser_7cc2fc, 0x007cc2fc, 0x4);
GAME_DATA(u8 *, gThingbrowser_7cc300, 0x007cc300, 0x4);
GAME_DATA(u8 *, gThingbrowser_7cc304, 0x007cc304, 0x4);
GAME_DATA(u8 *, gThingbrowser_7cc308, 0x007cc308, 0x4);
GAME_DATA(u32, gThingbrowser_7cc30c, 0x007cc30c, 0x4);
