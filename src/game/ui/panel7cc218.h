// The game panel object at 0x7cc218 (its vtable 0x6ffbe8). Reimplementation of 0x4ad4e0, 0x4ad720, 0x4ad890, 0x4ae8e0, 0x4ae980, 0x4af980, 0x4af440, 0x4aec30. The
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Panel7cc218_ShowRide's calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7cc218ShowEnv {
    void(__fastcall *setRange)(u8 *window, void *, s32 from, s32 to);   // Window705b20_SetRange
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);           // Window_SetShown
    void(__fastcall *showSettings)(u8 *self, void *, u8 flags);         // Panel7cc218_ShowSettings
};
void Panel7cc218_SetShowEnv(const Panel7cc218ShowEnv *env);

// Panel7cc218_Refresh's calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7cc218RefreshEnv {
    u8 *(__cdecl *name)(u8 *thing);        // Thing_NameBuffer7ba058
    void(__fastcall *showRide)(u8 *self);  // Panel7cc218_ShowRide
    void(__fastcall *buttons)(u8 *self);   // Panel7cc218_UpdateButtons
};
void Panel7cc218_SetRefreshEnv(const Panel7cc218RefreshEnv *env);

// Panel7cc218_FillLevels' calls (the ports by default, 0x67c430 raw: unported, agent B's; the bench's recorders: their own benches have them).
struct Panel7cc218LevelsEnv {
    void(__fastcall *clear)(u8 *window);                                                 // Window7059f0_Clear
    u8 *(__fastcall *first)(u8 *defs);                                                   // ObjectDefs_First
    u8 *(__fastcall *next)(u8 *defs);                                                    // ObjectDefs_Next
    u8 *(__fastcall *readMode)(u8 *setting);                                             // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *f)());                                            // 0x67a7d0
    u8 *(__fastcall *find)(u8 *table, void *, u32 id, u32 quiet);                        // ObjectControls_Find
    void(__fastcall *name)(u8 *defs, void *, u32 id, u16 *out, u32 size);                // ObjectDefs_GetFullName
    s32(__fastcall *add)(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    u8 *(__fastcall *def)(const u8 *object);                                             // Object_GetDef
    u32(__fastcall *busy)(const u8 *object);                                             // Is19c_Eq2_4e03e0
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);              // Charset_FormatWide
    u16 *(__cdecl *text)(u32 index);                                                     // Language_WideText
    u16 *(__cdecl *token)(u16 *text, const u16 *separators);                             // 0x67c430
    void(__fastcall *refresh)(u8 *window);                                               // Window7059f0_RefreshRows
};
void Panel7cc218_SetLevelsEnv(const Panel7cc218LevelsEnv *env);

// Panel7cc218_RefreshFigures' calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7cc218FiguresEnv {
    u8 *(__fastcall *def)(const u8 *object);                                             // Object_GetDef
    u8 *(__fastcall *child)(u8 *window, void *, u32 id);                                 // Window_FindChild
    s32(__fastcall *age)(const u8 *object);                                              // Object_AgeDays
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);              // Charset_FormatWide
    void(__fastcall *notify)(u8 *window, void *, u32 a);                                 // Window_NotifyImpl100
    u8(__fastcall *rating)(const u8 *object, void *, s32 a, s32 b, s32 c);               // Object_ComputeRating
    u8(__fastcall *percent)(const u8 *object, void *, float a, float b, u32 c);          // Object_DefRowPercent
    u32(__fastcall *scrap)(const u8 *object);                                            // Object_ScrapRefund
    u32(__cdecl *status)(u8 *object, s32 count);                                         // Object_Status
    void(__fastcall *buttons)(u8 *self);                                                 // Panel7cc218_UpdateButtons
    void(__fastcall *showRide)(u8 *self);                                                // Panel7cc218_ShowRide
    void(__cdecl *levels)(u32 id, u8 *list, u32 clear, u8 *object);                      // Panel7cc218_FillLevels
};
void Panel7cc218_SetFiguresEnv(const Panel7cc218FiguresEnv *env);

// Panel7cc218_EnterTileMode's calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7cc218TileEnv {
    u32(__cdecl *select)(u32 value);                                    // Global7c2654_Set
    void(__cdecl *focus)(u32 record);                                   // FocusCell_Update
    u8 *(__fastcall *findDef)(u8 *defs, void *, u16 id);                // ObjectDefs_Find
    void(__cdecl *openFor)(u8 *thing);                                  // Dialog7ca1c8_OpenFor
    void *(__cdecl *alloc)(u32 size);                                   // GameHeap_Alloc
    u8 *(__fastcall *tileCtor)(u8 *self, void *, u32 arg);              // MouseModeTile_Ctor
    void(__cdecl *setRecorded)(u32 value);                              // Global81ecac_SetRecorded
    u8 *(__fastcall *baseCtor)(u8 *self);                               // MouseModeOnlineBase_Ctor
    void(__stdcall *setMode)(u8 *mode);                                 // MouseMode_Set
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);    // GameMode_MapClick
    // Panel7cc218_EnterMoveMode's others.
    u8 *(__fastcall *getDef)(const u8 *object);                         // Object_GetDef
    u8(__cdecl *traceQueue)(const u8 *thing);                           // RideQueue_Trace
    s32(__cdecl *commitPairs)();                                        // PairTable_Commit
    void(__cdecl *gameMode)(u32 mode, u32 record);                      // GameMode_Set
    u8 *(__fastcall *tileCtorKind)(u8 *self, void *, u32 arg, u16 kind);   // MouseModeTile_CtorKind
    void(__cdecl *setId)(const u16 *id, u32 flag);                      // Global8186e0_SetRecorded
    void(__cdecl *setValue)(u32 value, u32 force);                      // Global81d7a4_SetRecorded
};
void Panel7cc218_SetTileEnv(const Panel7cc218TileEnv *env);

// Panel7cc218_LevelBoxHandler's and Panel7cc218_OpenLevelBox's calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7cc218LevelBoxEnv {
    u8 *(__fastcall *child)(u8 *window, void *, u32 id);                       // Window_FindChild
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                 // Window7059f0_NthItemData
    u32(__fastcall *choose)(u8 *self, void *, s32 index, u8 close);            // Panel7cc218_Choose
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                 // Window_SendMessage
    void(__fastcall *refresh)(u8 *self);                                       // Panel7cc218_RefreshFigures
    s32(__fastcall *close)(u8 *object);                                        // WindowAt10_Message4
    u32(__fastcall *dword)(u8 *window, void *, u32 index);                     // Window_DwordA0
    u32(__fastcall *member)(u8 *object, void *, u32 a);                        // Obj_CallMember20Slot14
    s32(__fastcall *fallback)(u8 *window, void *, u32 message, u32 a, u32 b);  // Window_CallDefault
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
    void(__cdecl *levels)(u32 id, u8 *list, u32 clear, u8 *object);            // Panel7cc218_FillLevels
};
void Panel7cc218_SetLevelBoxEnv(const Panel7cc218LevelBoxEnv *env);

// Panel7cc218_Handler's calls (the ports by default, the unported 0x4e15b0 raw; the bench's recorders: their own benches have them).
struct Panel7cc218HandlerEnv {
    s32(__fastcall *close)(u8 *object);                                   // WindowAt10_Message4
    void(__fastcall *refresh)(u8 *self);                                  // Panel7cc218_RefreshFigures
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);            // ThingPanel_Handler_48d200
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);  // Window_SetTimer
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);              // Window_StartTimer
    void(__cdecl *cancel)(u32 id);                                        // Game_CancelAdvisor
    void(__cdecl *swing)();                                               // Sign_Swing
    void(__cdecl *open)(s32 tab, u32 selected);                           // Dialog7ca150_Open
    void(__fastcall *stepGroup)(u8 *owner);                               // Things_StepGroup
    void(__fastcall *stepBack)(u8 *owner);                                // Things_StepGroupBack
    void(__fastcall *restart)(u8 *panel, void *, u32 restart);            // ThingPanel_RestartOrClose_48ccf0
    void(__fastcall *goTo)(u8 *panel);                                    // ThingGoto_Start
    void(__fastcall *moveMode)(u8 *self);                                 // Panel7cc218_EnterMoveMode
    u8 *(__fastcall *def)(const u8 *object);                              // Object_GetDef
    void(__fastcall *settings)(u8 *self, void *, u8 flags);               // Panel7cc218_ShowSettings
    void(__fastcall *tile)(u8 *self, void *, u32 flag);                   // Panel7cc218_EnterTileMode
    void(__fastcall *levelBox)(u8 *self);                                 // Panel7cc218_OpenLevelBox
    void(__fastcall *setFlag)(u8 *panel, void *, u8 flag);                // ThingPanel_SetFlag64_48ce50
    u32(__fastcall *thingOp)(u8 *thing, void *, u32 a);                   // 0x4e15b0, agent A's, unported
};
void Panel7cc218_SetHandlerEnv(const Panel7cc218HandlerEnv *env);

extern "C" {

// 0x4af600 (cdecl (window, message, a, b); name invented, unconfirmed): the handler of the panel 0x7cc218. 5: closed (WindowAt10_Message4), answer 1. 0x10 with the timer 0x80080: the figures refreshed. 0x14: the advisors 0x129 and 0xcf cancelled. 0x15: the timer 0x80080 of 0xfa0 set and, when that answers (AX), started. 0x1e: Sign_Swing.
// 0x800: Object_GetDef of [0x7c2658] (not used), the slider a's value b in +0x30 (0x3e2d), +0x34 (0x3e2f), +0x2c (0x3e30), slot 5, its setting's text (2, 4, 1). 0x100, the control a: -2, -1 closed, 0; 0x3e26 stepped back, 0x3e27 forward, then the base; 0x3e29 Dialog7ca150_Open (0, the thing), 0x3e2a tile mode (1), 0x3e2b ThingGoto_Start, 0x3e2c move mode, 0x3e34 tile mode (0), 0x3e35 the level box,
// 0x3e36 SetFlag64 (b not 1), 0x3e37 0x4e15b0 on the thing (0) and slot 0xb, each 0; 0x3e38 RestartOrClose (b 1) and the figures refreshed, then the base. Each case not answering there ends in ThingPanel_Handler_48d200, whose answer it is.
s32 __cdecl Panel7cc218_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4ae430 (cdecl (window, message, a, b); name invented, unconfirmed): the level box's handler. 0x14: its owner (dword 0)'s +0x24 = 0, Obj_CallMember20Slot14 (1), then the default. 0x100 with a -2: the box sent 4 (0, 0), answer 0. 0x400 (the list a's row b chosen, the list there,
// b not -1): the row's data (Window7059f0_NthItemData; the slot left the window when none, sic) 0xffffe12e the default; 0xffffcd39 Panel7cc218_Choose (0, close) of the panel 0x7cc218, the box sent 4, Panel7cc218_RefreshFigures, answer 0; else Choose (data, no close): that done with a
// positive data WindowAt10_Message4 of the panel, answer 0; otherwise as for 0xffffcd39's last two. All else the default.
s32 __cdecl Panel7cc218_LevelBoxHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4ae560 (thiscall; name invented, unconfirmed): when +0x24 and +0xc are 0: the box built on +0x10 (script 0x755750, handler 0x4ae430 by address; not checked) into +0x24, Obj_CallMember20Slot14 (0), its depth +0x10's word +0xe0 + 0x11, its dword 0 this. Its child 0x1f4e: a TextPart6ff530
// part, font 5, the bytes 1, 1, slot 9 the text 0x1f, HoverLabel_SetYellow. Its list 0x1f4d: rows (List7cc250_NewRow, InitRow by address), the columns 0 (0) and 1 (1), the highlight "hilight" (the name at 0x74fa84), Panel7cc218_FillLevels of [0x7c2658]'s word +0xe, not cleared.
void __fastcall Panel7cc218_OpenLevelBox(u8 *self);

// 0x4af200 (thiscall (flag), pops 4; called by the handler 0x4af600; name invented): the shown thing [0x7c2658] selected (Global7c2654_Set), the focus cell updated with the record 1. With flag 0 a new tile mode of the kind 0x14. Else by the
// thing's definition's (word +0xe) track type +0x9c: 3 Dialog7ca1c8_OpenFor (the thing) and no mode; else a new tile mode of the kind 0xe for 1, 0x13 for 2, 0 otherwise, and the type given to Global81ecac_SetRecorded. A new mode made
// current (the holder made first, inline as MouseModes_EnsureHolder) and GameMode_MapClick (0, 0, 0, 0). Always this's slot 0x2c last.
void __fastcall Panel7cc218_EnterTileMode(u8 *self, void *, u32 flag);

// 0x48cfa0 (thiscall; called by the handler 0x4af600; name invented): the shown thing [0x7c2658] selected, the focus cell updated with the record 1; when its definition's +0x40 is not 0 the byte [0x81b14c] its queue line (RideQueue_Trace) and
// PairTable_Commit; GameMode_Set (0x33, 1); the map clicked at the cell before the thing's bytes +7, +5 (x = (+7 * 0x80 + +5) & 0x7f in the word, y the rest); a new tile mode (0x3b, the thing's word +0xe) current (the holder inline),
// [0x81b3ac] the thing, GameMode_Set (0x3b, 1), the record of the id (word +0xe, 0), the record of the thing's +0x10 (1), the shown thing cleared and this's slot 0x2c.
void __fastcall Panel7cc218_EnterMoveMode(u8 *self);

// 0x4af030 (thiscall; name invented): the shown thing [0x7c2658]'s definition (Object_GetDef, read first) put on the window +0x10's children (none: nothing). The child 0x3e30: the range
// +0x134 / +0x138 (Window705b20_SetRange), slot 7 given the thing's +0x58, its child 0x3e2e's slot 7 given the level table's +0x1ac << 10 over the range (idiv; no 0x3e2e or an empty range
// fault, sic). The child 0x3e2f: the definition's +0x70 kept in this +0x28, shown when that is not 0, the range +0x12c / +0x130, slot 7 given the thing's byte +0x5c. The child 0x3e2d: with the
// range +0x124 / +0x128 empty hidden, 0x3e31 too (Window_SetShown, the child not checked); else both shown and its child 0x3e2e's slot 7 given the level table's +0x19c << 10 over the range;
// either way the range, slot 7 given the thing's byte +0x5d. Finally Panel7cc218_ShowSettings (7). The level table is at the thing's byte +0x50 * 0x40 in the definition.
void __fastcall Panel7cc218_ShowRide(u8 *self);

// 0x4ae940 (thiscall; name invented, unconfirmed): the object +0x20's slot 9 given the shown thing [0x7c2658]'s name (Thing_NameBuffer7ba058), the panel's slots 6 and 5, Panel7cc218_ShowRide and Panel7cc218_UpdateButtons.
void __fastcall Panel7cc218_Refresh(u8 *self);

// 0x4ad890 (thiscall; slot 3 of the vtable 0x6ffbe8): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x3e20 text 0x11, 0x3e1a text 0x12, 0x3e1c text 0x13, 0x3e1e text 0x14, 0x3e1f text 0x15, 0x3e1d text 0x16, 0x3e22 text 0x17, 0x3e21 a
// 0x6ff350 part with 0, 0x3e1b no text, 0x3e16 a ridestatbar bar with 0, 0x3e18 a ridestatbar bar with 0, 0x3e19 a ridestatbar bar with 0,
// 0x3e17 a ridestatbar bar with 0, 0x3e23 a 0x6ff3b0 part with 0.
void __fastcall Panel7cc218_Dress(u8 *self);

// 0x4ad720 (thiscall; slot 4 of the vtable 0x6ffbe8): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7cc218_DressTip(u8 *self);

// 0x4ae980 (thiscall; called by 0x4ae8e0): a GuiRect made and destroyed about the rest (GuiRect_Ctor / GuiRect_Dtor); with the window +0x10
// set (read again for each child), its children (Window_FindChild, none checked): the gauges 0x3e2d and 0x3e30 each the range 0..0x78
// (Window705b20_SetRange) and their own child 0x3e2e a "smallguage.wct" mesh (0x124 bytes, MeshOverlay_Construct; none when the allocation
// fails) as its implementation (Window_SetImpl), then its slot 7 given 0; 0x3e2f the range 0..10; then 0x3e31, 0x3e33 and 0x3e32 each a
// TextPart_Ctor part (0x1a4 bytes; none when that fails), the bytes 1, 1 as 0xc00, font 6 (Language_Font; Window_Set4c), the bytes 0, 0,
// 0, 0xff as 0x40000 and no text (Window_NotifyImpl100(0)).
void __fastcall Panel7cc218_SetUpGauges(u8 *self);

// 0x4ae8e0 (thiscall; slot 0 of the vtable 0x6ffbe8): when the things of the panel's group (+4) number under 2 (Things_CountGroup with
// the selected thing [0x7cc208]), the window +0x10's children 0x3e26 and 0x3e27 hidden (not checked); then the gauges set up
// (Panel7cc218_SetUpGauges), the window +0x10 shown (Window_SetShown) and the hover label 0x3e28 bound (HoverLabel_Bind).
void __fastcall Panel7cc218_Open(u8 *self);

// 0x4af980 (thiscall (thing), pops 4; slot 10 of the vtable 0x6ffbe8; called by 0x4869cb / 0x4869f1 on 0x7cc218): the selected thing
// [0x7cc208] = thing, the window built (WindowAt10_Create: the script 0x755150, the handler 0x4af600, the children 0x3e24, 0x3e15 and
// 0x3e25), +0x24 = 0, and the advisor posted 0x129 when the low byte of the thing's float +0x44 (__ftol) is 1-20, else 0xcf
// (Advisor_PostId).
void __fastcall Panel7cc218_Show(u8 *self, void *, u8 *thing);

// 0x4af440 (thiscall; slot 15 of the vtable 0x6ffbe8; name invented): the shown thing [0x7c2658] given the panel's settings: the speed
// +0x2c (Obj_SetSpeed), the capacity byte +0x30 (Object_SetCapacity) and, when its definition's +0x70 is set, the duration byte +0x34
// (Obj_SetDuration).
void __fastcall Panel7cc218_ApplyToShown(u8 *self);

// 0x4aec30 (thiscall (flags: a byte), pops 4; called by 0x4af030 and the handler 0x4af600; name invented): the settings' texts, each
// formatted into the shared buffer 0x7ba058 (the formatter 0x6acd60 with the table [0x78ba94], 0x3fff characters; one number argument,
// TextArgInt_Ctor (n, value, signed)) and sent to a child of the window +0x10 (Window_FindChild, not checked; Window_NotifyImpl100):
// flag 1 the speed +0x2c (text 0x1aa, argument 5) to 0x3e33; flag 2 the capacity +0x30 (argument 6) to 0x3e31, the text 0x1cf for the
// definition's (Object_GetDef of the thing [0x7c2658], read once) track type +0x9c 3, 0x1cd for 2 (the definition looked up again),
// else 0x1ab; flag 4 the duration +0x34 (argument 7) to 0x3e32 by the definition's +0x70: 0 the child hidden (Window_SetShown) and
// the buffer sent as it is (sic), 1-4 the child shown and the text 0x1ac (the duration 1) / 0x1ad, 0x1ae, 0x1af, 0x1b0; past 4 the
// buffer sent as it is (sic).
void __fastcall Panel7cc218_ShowSettings(u8 *self, void *, u8 flags);

// 0x4ad4e0 (thiscall; called from 0x4ae0fb and by slot 1, 0x4ae940): with the thing [0x7c2658] (nothing without one) and its definition
// (Object_GetDef), the window +0x10's children (Window_FindChild, none checked): with the mode +8 3 all of 0x3e38, 0x3e34, 0x3e2a, 0x3e36,
// 0x3e2c, 0x3e37, 0x3e35 hidden; else shown (Window_SetShown) in that order, with: 0x3e38 (a 0x705a50 button) its slot 5 given 1 and the
// state 1 when the thing's +0x68 is 0 (Is68_Zero_4e0440), else 0, notifying (Window705a50_SetState), when the mode is not 0x19 / 0x1a and
// the thing either has +0x68 0 and can open (Object_CanOpen) or not 0 and Const1_424ec0 answers, else its slot 5 given 0; 0x3e2a shown
// only when the definition's track type +0x9c is not 0; 0x3e36 (a 0x705a50 button) the state 1 for the modes 0x11-0x15, else 0,
// notifying; 0x3e2c's slot 5 whether the track's neighbours are right (Object_TrackNeighboursOk, its low byte).
void __fastcall Panel7cc218_UpdateButtons(u8 *self);

// 0x4af480 (thiscall; index, close, pops 8; this not read; name invented): the shown thing [0x7c2658] and the balance of the money record
// (the game's handle +0x1da726 in the thing slots: GameTable7cfb90_Get1da726_5195d0, Source_Field0c) read first. With close (a byte) 1
// the thing closed (Object_SetOpen 0), 1. Else with index negative: opened (Object_SetOpen 1) when the balance is over its definition's
// dword at +0x1b8 - index * 0x40 (Object_GetDef; signed), 0. Else the object control index (ObjectControls_Find on the game's +0x1d943c,
// quiet 0): with the balance under its cost (+4, signed) the UI sound 0x1d (SoundUi_Play1d), 0; else a new tile mode of kind 4 for it
// (MouseModeTile_CtorKind; none when the 0x14 bytes fail) made current (the holder made first: MouseModes_EnsureHolder, inline in the
// original; MouseMode_Set), the thing selected (Global7c2654_Set) and the focus cell updated with the record 1 (FocusCell_Update), 1.
u32 __fastcall Panel7cc218_Choose(u8 *self, void *, s32 index, u8 close);

// 0x4ae140 (cdecl (id, list, clear, object); name invented, unconfirmed): the list (Window7059f0_Clear first when clear) of what the object can have: unless the mode setting 0xfb3b7c (read under the guard bit 0 of 0xfb34c5) is 2, a row for each definition
// (ObjectDefs_First / _Next of 0x7890a0) with +0x508 0 and +0x284 the id whose control record (ObjectControls_Find (its word, 0) in the game + 0x1d943c) has the byte +0x10: the full name (ObjectDefs_GetFullName into 0x7ba058, 0x3fff) and the record's +4, data the word.
// Then, with Is19c_Eq2_4e03e0: the only row (others uncounted) the text 0x1e of the object's level (byte +0x50) + 2, no second text, data -0x32c7; else below level 2, when level + 1 is at most the object's own record's +0x14: the text 0x1d of level + 2 and
// the definition's dword +0x1b8 + (level + 1) * 0x40, data -(level + 1). With no rows: the words of the language text (0x1b for mode 2, 0x1c for level 2, else 0x1a) in 0x7ba058 split by 0x67c430 at 0x7558f0, each a row with no second text, data -0x1ed2. Window7059f0_RefreshRows.
void __cdecl Panel7cc218_FillLevels(u32 id, u8 *list, u32 clear, u8 *object);

// 0x4ade40 (thiscall; name invented, unconfirmed): the panel's figures for the thing [0x7c2658] (Object_GetDef called, its answer not used). With the window +0x14, each child (Window_FindChild) only when there, through slot 7: 0x3e21 the sum as a double of the object's
// circular buffer +0x1a8 (30 when it holds 30 or more, CircBufU32_30_Get's that answer 1), through __ftol; 0x3e1b told (Window_NotifyImpl100) the text 0x1b1 of the age in days (0x1f characters on the stack; TextArgInt 8, its size, signed by its sign);
// 0x3e16 Object_ComputeRating (+0x2c, +0x34, +0x30), 0x3e18 Object_DefRowPercent (those as floats, +0x2c, +0x30, +0x34), 0x3e19 the object's float +0x44 through __ftol, 0x3e17 Object_Field48AsInt, each low byte * 1024 / 100; 0x3e23 (not checked) Object_ScrapRefund.
// Always: Object_Status (2) when it differs from +8 is kept, slot 9, Panel7cc218_UpdateButtons and _ShowRide; with the window +0x24 Panel7cc218_FillLevels (the object's word +0xe, its child 0x1f4d, 1, the object).
void __fastcall Panel7cc218_RefreshFigures(u8 *self);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[236], gPanel7cc218_OpenLevelBox_755750, 0x00755750, 0xec);
GAME_DATA(char[20], gPanel7cc218_7558f4, 0x007558f4, 0x14);
GAME_DATA(u8 *, gPanel7cc218_7c2658, 0x007c2658, 0x4);
GAME_DATA(u8, gPanel7cc218_EnterMoveMode_81b14c, 0x0081b14c, 0x1);
GAME_DATA(u8 *, gPanel7cc218_EnterMoveMode_81b3ac, 0x0081b3ac, 0x4);
