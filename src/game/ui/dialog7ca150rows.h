// The objects' lists dialog's rows: the callbacks that add a thing to the list [0x7ca14c] (src/game/ui/dialog7ca150's tabs pass them to Things_ForEachSelected). Reimplementation of 0x493cd0-0x4941e0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca150RowsEnv {
    u8 *(__fastcall *def)(const u8 *object);                                                // Object_GetDef
    u8(__fastcall *rating)(const u8 *object, void *, u32 baseOnly);                         // Object_Rating
    u32(__fastcall *field48)(const u8 *object);                                             // Object_Field48AsInt
    u32(__fastcall *difference)(const u8 *object);                                          // Object_HistoryDifference
    u32(__fastcall *diff180)(const u8 *object);                                             // Obj_Diff180_184
    u32(__fastcall *average)(const u8 *object);                                             // Object_HistoryAverage
    u32(__stdcall *control)(u32 id);                                                        // ObjectControls_GetWord18
    s32(__fastcall *find)(u8 *list, void *, u32 data);                                      // Window7059f0_FindItemData
    s32(__fastcall *add)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
};
void Dialog7ca150_SetRowsEnv(const Dialog7ca150RowsEnv *env);

extern "C" {

// 0x493cd0 (cdecl (thing, list, figures)): the thing's row on list. The figures come from the object figures, or the thing when that is 0 (Object_GetDef of it asked first, unused). The texts: the thing's name (Thing_NameBuffer7ba058), the
// newest-30 sum of the ring +0x1a8 (CircBufU32_30_Sum; the debug print left out), then Object_Rating (0), the float +0x44 through __ftol and Object_Field48AsInt, each its low byte * 1024 / 100. Without figures the row is added at the end (parent -1);
// with them the parent is figures' item (Window7059f0_FindItemData), none added and that negative answer returned when it has none. Otherwise the row's answer. The row's data is the thing, not refreshed.
s32 __cdecl Dialog7ca150_AddRow(u8 *thing, u8 *list, u8 *figures);

// 0x493e90 (cdecl (thing)): Dialog7ca150_AddRow on [0x7ca14c], no figures; answers 1.
u32 __cdecl Dialog7ca150_AddPlainRow(u8 *thing);

// 0x493eb0 (cdecl (thing)): as 0x493e90 with the figures from the table 0x7cfb90's entry (0x14 each) of the thing's word +0x30, when that is not 0 (Fn_4e0df0, inlined; the debug print left out).
u32 __cdecl Dialog7ca150_AddLinkedRow(u8 *thing);

// 0x493f30 (cdecl (thing)): a row on [0x7ca14c] at the end: the name, the sum, Object_HistoryDifference, Obj_Diff180_184, Object_HistoryAverage * 1024 / 100 (signed). Answers 1.
u32 __cdecl Dialog7ca150_AddHistoryRow(u8 *thing);

// 0x494070 (cdecl (thing)): the same with Object_Rating (0)'s low byte * 1024 / 100 after the sum. Answers 1.
u32 __cdecl Dialog7ca150_AddRatedHistoryRow(u8 *thing);

// 0x4941e0 (cdecl (thing)): a row on [0x7ca14c] at the end: the name and ObjectControls_GetWord18 of its word +0xe (GameTable7cfb90_Get1da720_519510 of [0x7cf83c] called first for the ECX of that stdcall; the id's upper half pushed is the
// thing's address, which the lookup ignores). Answers 1.
u32 __cdecl Dialog7ca150_AddControlRow(u8 *thing);

}
