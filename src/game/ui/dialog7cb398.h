// The window at 0x7cb398 (made by 0x4a52a0 with the handler 0x4a4830; the park statistics graph's Gauge is at 0x7cb318 beside it).
// Reimplementation of 0x4a4700, 0x4a4ab0 and 0x4a52a0. The names are unconfirmed.
#pragma once
#include "../../../re/types.h"
extern "C" {
// 0x4a4700 (cdecl (window, message, a, b): the handler 0x4a52a0 gives the top three's children): message 0xc (the tooltip asked for)
// answers the record 0x7cb310 with [0x7cb314] the widened (Language_Widen) record of the table [0x78ba9c] at the window's slot 8 value
// + 1 (Records_DataIfTerminated); any other message the default handler's answer (Window_CallDefault).
u32 __cdecl Dialog7cb398_TopHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a4ab0 (no arguments): the window's child 0x4a68d1 (Window_FindChild; nothing without it), then the three commonest values of the
// park's plain analyser things (Things_TopThreeValues; the original first looks up the game record of [0x7cf83c] for the call's ECX,
// which the callee ignores), shown by its children 0x4a68d4, 0x4a68d3 and 0x4a68d2 in turn: a value 1..0x16 goes to the child's slot 7
// as value - 1, any other value (0x17 for none) hides the child (Window_SetShown 0).
void __cdecl Dialog7cb398_ShowTopThree();

// 0x4a52a0 (no arguments; the caller's ECX unused): nothing when the window [0x7cb398] is made. Else the [0x7c24c8] window closed
// (Window7c24c8_Close), 3 stored as the select 2 (Global7cb28c_Select), the window built on [0x7cb2ac] from the script 0x7537d0 with
// the handler 0x4a4830 (WindowScript_Build; nothing more when none) and kept in [0x7cb398] and as the [0x7c24c8] one (Window7c24c8_Set);
// the timer 0x80083 of 1000 set and, when that answers (AX) not 0, started (Window_SetTimer, Window_StartTimer). Its children (looked up
// by id, not checked unless said; a "label" is a new TextPart_Ctor part (0x1a4 bytes, GameHeap_Alloc; none when the allocation fails;
// Window_SetImpl), Window_NotifyC00(1, 1), font 6 (Language_Font; Window_Set4c) and Window_Notify40000(0, 0, 0, 0xff)):
// - 0x4a68ce a label with the language text 0x9c (Window_NotifyImpl100);
// - 0x4a68d1's children 0x4a68d2..0x4a68d4 the handler 0x4a4700 (Window_SetHandler); then Dialog7cb398_ShowTopThree;
// - 0x4a68bb (checked): a RendererPart_Ctor part (0xd8 bytes) and the handler 0x4a4b90; its child 0x4a68c1 (checked; a radio group)
//   has its children 0x4a68c3, 0x4a68c4, 0x4a68c2 labelled, given no text (Window_NotifyImpl100(0)) and the texts 0x16a, 0x16b, 0x16c
//   through their slot 9, then the one [0x7cb39c] (0, 1, 2) names selected (RadioGroup_Select); its children 0x4a68bc-0x4a68c0 the
//   same with the texts 0x98, 0x99, 0x9a, 0x97, 0x9b, and for each bit 0-4 of the byte [0x753c48] set, that child set to state 1,
//   notifying (Window705a50_SetState(1, 1));
// - 0x4a68c9's children 0x4a68cb / 0x4a68cd (a TextPart_Ctor part, font 6, the texts 0x97 / 0x9e) and 0x4a68cc / 0x4a68ca (a
//   TextPart6ff350_Ctor part, font 6, slot 7 (0)); [0x7cb360] and [0x7cb378] then -1, and the people on plain cells
//   (Things_CountKind1OnPlainCell) and the analyser's total (ParkAnalyser_Total of the game record's analyser) go, when they differ
//   from those, to them and to 0x4a68cc's / 0x4a68ca's slot 7 (0x4a68c9 looked up again);
// - 0x4a68d0 a mesh of "sad_undec.wct" (0x124 bytes, MeshOverlay_Construct) as its implementation and slot 7 (0); 0x4a68cf a mesh of
//   "hap.wct" as its object +0x98 (Window_SetObject98), then a label with the text 0x9d, its depth 0x4a68d0's word +0xe0 + 2
//   (Window_SetDepth); 0x4a68d0's slot 7 the percent of people below 0x21 (Things_PercentPeopleStat19cBelow) * 1024 / 100 and
//   0x4a68cf's (100 - the percent above 0x42 (Things_PercentPeopleStat19cAbove)) * 1024 / 100 (signed, truncated).
// Then the advisor told 199 (Advisor_PostId).
void __cdecl Dialog7cb398_Create();
}
