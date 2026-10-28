// The coaster editor's adjust tools: toggling the adjustment of one of the selected piece's values. Reimplementation of 0x441cd0, 0x441e80 and
// 0x442010 (names unconfirmed).
//
// The piece being edited is [0x79c640] (its handle the word at +0; +8 flags; its definition at +0x100, whose +4 byte has flag 2 for a piece
// that cannot be adjusted and whose +0x18 points at the type flags). The editor's flags are 0x79c658: bits 0-5 the tool in use (2 and 4 the
// adjusters here), 0x200 / 0x400 the value(s) being adjusted, 0x2000 set with a tool picked before a piece. Each toggle, for a piece whose type
// allows it (or with its tool on already) and that can be adjusted: with no tool on, its tool goes on (and maybe 0x2000); with its tool on, the
// tool goes off and the value is recorded (the action recorder 0x785a08) and the game's changed flag 0x2000 at [0x790fec] cleared. Then with
// the tool on: 0x79c630 / 0x79c634 take 0x790ac8 / 0x790acc, and for a piece the adjusted value's pointer (0x79c614, 0x79c6fc), its value
// (0x79c6f0, 0x79c6d4), range (0x79c61c..0x79c65c, 0x79c6e4..0x79c66c) and step (0x79c6ec, 0x79c6a8) are set and the highlight shown
// (Highlight_SetShown(1)); without a piece the tool goes off again. With the tool off, flags 0x1e00 are cleared and the highlight hidden.
// Each returns 1.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x441cd0 (cdecl): tool 4 for type flag 0x200: the value at +0xf8 (recorded as 0x3b with it, 0 and the handle). Taking it on a piece with
// flag 0x80 at +8, a value over 0.5 is first lowered by 1; its range is -0.35..0.35 with that flag, else 0.15..0.85; step 0.01.
u32 __cdecl CoasterEdit_ToggleAdjustF8();

// 0x441e80 (cdecl): only with a game ([0x790fec]) whose +0x40 is set: tool 2 for type flag 0x40: the value at +0xe0 (recorded as 0x3a with
// the handle), its range +0xd4 less and plus the type's +0x94, step 0.02; the piece's +8 gets flag 1.
u32 __cdecl CoasterEdit_ToggleAdjustE0();

// 0x442010 (cdecl): tool 4 for type flags 0x180 (0x2004 when taken with no tool): the angles at +0xd8 (type flag 0x100: 0x200, the piece's
// +8 flag 4) and +0xdc (type flag 0x80: 0x400, the piece's +8 flag 2), recorded as 0x3b with both and the handle; ranges -pi..pi
// (as floats 0xc0490e38 / 0x40490e38), step 0.02.
u32 __cdecl CoasterEdit_ToggleAdjustAngles();

// 0x4421e0 (cdecl; name unconfirmed): only with a game whose +0x40 is set and a piece whose definition lacks flag 2 (else 1):
// CoasterEdit_ToggleAdjustF8 when the piece's +8 has flag 0x40, else CoasterEdit_ToggleAdjustAngles.
u32 __cdecl CoasterEdit_ToggleAdjust();

// 0x441a00 (cdecl; name unconfirmed): the drag tool (tool 1), only with a game whose +0x40 is set, for type byte flag 0x20 (or with the
// tool on: then a missing piece faults, sic). Taken off, the piece's +0xe8 is recorded (0x39 with the handle), and when the piece's +8
// has 0x40 so is that of its neighbour (+0x64 with 0x80, else +0x68) when it has a definition (the word's high half that definition's,
// sic). With the tool on: 0x79c630 / 0x79c634 as the adjusters', the piece's Car_TravelCost (the game, no base, no positions) kept at
// 0x79c698, the value +0xe8 at 0x79c6fc / 0x79c6d4, its range from CarType_Unlerp of the type's +0x68 (0 when not above 0) to +0xe8 plus
// CarType_Scale of the type's +0x6c less the train's length (Car_TrainLength of the definition's car +0x30 + 4 * [+0x30]), its step
// 0.005, flag 0x400 and the highlight shown. With the tool off (or another tool on) and 0x785a2c clear, the money of [0x7cf83c] is
// credited the kept cost (__ftol) and debited Float_RoundUp of the cost now, and the kept cost cleared; then flags 0x1e00 are cleared
// and the highlight hidden. Returns 1.
u32 __cdecl CoasterEdit_ToggleDrag();

// 0x4423f0 (cdecl; name unconfirmed): the move-back tool (8), only with a game whose +0x40 is set: with the tool off, a selected car
// that is not its train's (+0x100) last (+4 below the count +0x30 less 1) is wanted; then (either way) the train must lack flag 2 (+4;
// with the tool on and no car, sic, it faults). No tool on: the tool goes on; this tool on: it goes off; another: no change. On: the
// move recorded (0x3d with the car's handle word, the high half the car pointer's, sic), the game's changed flag 0x2000 cleared, the
// car swapped with the one after it (Train_SwapCars) and the highlight shown, then all again while there is a game (so the tool, now
// on, goes off). Off: the highlight hidden.
void __cdecl CoasterEdit_MoveCarBack();

// 0x4425c0 (cdecl; name unconfirmed): the move-forward tool (0x10), as CoasterEdit_MoveCarBack with a car not its train's first (+4
// above 0), recorded as 0x3e, swapped with the one before it.
void __cdecl CoasterEdit_MoveCarForward();

// 0x442220 (cdecl; name unconfirmed): the editor's tool ended. With the mode 0x79c624 bit 0 and a game: the game's flag 4 cleared,
// Flags_SetChanged (game, 0) and, unless its +0x3c has bit 0 without bit 1, Coaster_FlashCar (game, 1). Then the tool in use (flags
// bits 0-5) is toggled off: 1 CoasterEdit_ToggleDrag, 2 CoasterEdit_ToggleAdjustE0, 4 CoasterEdit_ToggleAdjust (inlined), 8
// CoasterEdit_MoveCarBack, 0x10 CoasterEdit_MoveCarForward, 0x20 TrackSelection_End (inlined); another, nothing. Then 0x79c664,
// 0x79c644 and the mode cleared and flag 0x40 too.
void __cdecl CoasterEdit_EndTool();

// 0x444360 (cdecl; mode; name unconfirmed): as CoasterEdit_EndTool, but the mode (0x79c624) becomes mode; mode 1 (its low byte) then
// Coaster_FlashCar (game, 2), the game's flag 4 set (sic: before the game is tested), and with a game whose +4 has flag 2 (a byte),
// when the car at +0x58 (the game's +0x3c bit 0 clear) or at +0x60 (bits 0 and 1 set) has flag 0x80 (+8), 0x79c664 = 1 and flag 0x40.
void __cdecl CoasterEdit_SetMode(u32 mode);

// 0x444770 (cdecl; name invented, unconfirmed): CoasterEdit_Tools (0), then the editor's tool 2 ended (as CoasterEdit_EndTool; bit 2
// of the chosen tools 0x79c638 cleared) and tool 1 started: CoasterEdit_EndTool, the mode 1, Coaster_FlashCar (game, 2), the game's
// flag 4 set, CoasterLink_CheckEnd, recorded (ActionRec_Write 0x37), the game's flag 0x2000 cleared, CoasterLink_ClearEnd when
// 0x79c644 and 0x79c664 are both 2, 0x79c664 = 0, 0x79c644 = 1, CoasterEdit_SetBounds; bit 1 of 0x79c638 set. Then the 0x7ca1c8
// dialog (0x7b51f0) told the tools on offer (0x79c6e0) and the chosen ones (0x79c638). (The original's loops over its two masks,
// 2 off and 1 on, compiled in full for any bits.)
void __cdecl CoasterEdit_SwitchToTool1();

// 0x442880 (cdecl; tools; name invented, unconfirmed): the editor's tool ended (as CoasterEdit_EndTool), the offer 0x79c6e0 and the
// chosen tools 0x79c638 cleared, then each bit of tools, lowest first, started and set in 0x79c638: 1 as SwitchToTool1 starts it
// up to CoasterLink_CheckEnd, then CoasterEdit_SetTool (1); 2 CoasterEdit_SetMode (1) and CoasterEdit_SetTool (2); 4, 8, 0x10, 0x20,
// 0x40, 0x80, 0x100 CoasterEdit_SetMode (2, 4, 8, 0x10, 0x80, 0x20, 0x40); higher ones only set. Then CoasterEdit_RefreshTools.
void __cdecl CoasterEdit_SetTools(u32 tools);

// 0x445220 (cdecl; tool; name invented, unconfirmed): one tool started and set in the chosen tools 0x79c638. 1 as
// CoasterEdit_SwitchToTool1 starts it; 2 the same up to CoasterLink_CheckEnd (inlined), then ActionRec_Write (0x785a08, 0x38), the
// game's flag 0x2000 cleared and, with CoasterLink_EndMarked, 0x79c664 = 1 and flag 0x40 of 0x79c658, without CoasterLink_BeginEnd;
// 0x79c644 = 2 and CoasterEdit_SetBounds. 4 .. 0x100: CoasterEdit_SetMode (2, 4, 8, 0x10, 0x80, 0x20, 0x40; all but the last inlined
// as CoasterEdit_EndTool and the mode set); any other only set.
void __cdecl CoasterEdit_StartTool(u32 tool);
// 0x4426a0 (cdecl; name invented, unconfirmed): CoasterEdit_EndTool (inlined, TrackSelection_Clear in it too), then the offer
// 0x79c6e0 and the chosen tools 0x79c638 cleared.
void __cdecl CoasterEdit_EndAll();

// 0x4455c0 (cdecl; out; name invented, unconfirmed): the tool to start picked into *out and started (CoasterEdit_StartTool, inlined
// in full): 0x10 when the game's +0x3c has bit 0 without bit 1 (sic: the game read before it is tested); else, with
// CoasterLink_EndMarked (inlined) 0x79c664 = 1 and flag 0x40 of 0x79c658, then 2 when 0x79c664 is 1, else 1.
void __cdecl CoasterEdit_StartChosenTool(u32 *out);

// 0x446fa0 (cdecl; tool, action; name invented, unconfirmed): the editor's tool buttons (the 0x7ca1c8 dialog's handler calls it).
// Action bit 1 (its low byte tested; first): with any of tool's bits 0-8, the tool in use ended (as CoasterEdit_EndTool, inlined)
// and tool cleared from the chosen tools 0x79c638. Else bit 0: tool started as CoasterEdit_StartTool but for 2, which is started as
// tool 1 up to CoasterLink_CheckEnd and then CoasterEdit_SetTool (2); tool set in 0x79c638. Else bit 2: CoasterEdit_EndAll
// (inlined). Then, with tool not 0, CoasterEdit_Tools (0) and the dialog (0x7b51f0) told the tools on offer (0x79c6e0).
void __cdecl CoasterEdit_ToolButton(u32 tool, u32 action);
// 0x447520 (cdecl (coaster, on); name invented): recorded (0x44, the coaster's +0x14 with a coaster and on, else 0). Editing a coaster (one
// and on): with no game (0x790fec) yet, Object_RestartRide of its object (+0x28) and Coaster_SetState (2 when its +0x3c has bit 0 without
// bit 1, else 1); then it is the game (CoasterLink_SetCurrent). Else, with a game: the tool in use ended as CoasterEdit_EndTool does (inlined),
// 0x79c664, 0x79c644, 0x79c624, 0x79c6e0 and 0x79c638 cleared and flag 0x40 of 0x79c658, Flags_SetChanged (game, 0), no game, and
// Object_Close of the game's object (+0x28) when its +8 has bit 1.
void __cdecl CoasterEdit_SetEditing(u8 *coaster, u32 on);

// 0x4474d0 (cdecl (object, on); name invented): the coaster of the object's ride id (+0x20; CoasterLink_ById, none without an object); unless
// 0x785a2c is set, CoasterEdit_SetEditing (that coaster, 1) when on, else (none, 0).
void __cdecl CoasterEdit_EditRide(const u8 *object, u32 on);

// 0x446d50 (cdecl (piece); name invented, unconfirmed): a track piece selected for editing. 0 for one whose definition (+0x100) has flag 2
// (+4); else 0x79c69c cleared and, with a piece: it becomes the selection (0x79c6a4; 0x79c658's flag 0x100, its definition at 0x79c660, its
// Car_TrainLength at 0x79c694), recorded (ActionRec_WriteWWW 0x3c, 0, 0, its handle +0), the game's (0x790fec) flag 0x2000 cleared; the span's
// first end (0x79c62c) its +0x68 or itself and 0x7925d0 3; the pieces between the first end and the old other end (0x79c628, when set) lose
// flags 0x1004000 and those between the first end and the new other end (+0x64, or itself; 0x79c628) get 0x1000000 (each span from the
// lower handle (+0) along +0x64, the higher left out); Flags_SetChanged (game, 0); its definition's cell (+0xc, +0x10 to 0x79c6d8 / 0x79c6a0)
// clamped to the map (0..[0x7a0314] - 1, 0..[0x7a0374] - 1, signed) begins a path drag (PathDrag_Begin, kept with a train (+0x30) above 1);
// Highlight_SetShown (1), CoasterEdit_SetBounds, and its Car_TravelCost (game, 0, it) and its next's (+0x64) to the floats 0x79c698 /
// 0x79c64c. Returns 1 when the definition's train (+0x30) is above 1 (unsigned), else 0.
u32 __cdecl CoasterEdit_SelectPiece(u8 *piece);
}
