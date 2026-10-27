// The objects (thing model 3: rides, shops, facilities; serialized at 0x4db7d0): reimplementation
// of 0x4dd4e0, 0x4dd670, 0x4e01f0, 0x4e1670, 0x4e1690, 0x4e1c60, 0x4e1e00, 0x4e2190, 0x4e21c0, 0x4e2290, 0x4e2400, 0x4e2710 and 0x4e2b50 (thiscall).
// +0xe the definition's id; +0x18c mQualityOfGoods, +0x190 mChanceOfWinning; +0x1a0 mNumCustomers,
// +0x1a4 mNumWalkAways, and their histories, CircBufU32s of 30 at +0x1a8 and +0x230 (mTemp first,
// the count so far); +0x340 a running average of the customers' happiness changes.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// What the objects call in the game that is not yet ported, so that they can run outside it (the benches point these at recorders).
struct ObjectEnv {
    void(__fastcall *post40fb10)(u8 *centre, void *, u8 *msg);   // 0x40fb10 (agent B's; thiscall on the message centre, pops 4)
    void(__fastcall *person500870)(u8 *person);                  // 0x500870 (agent C's; thiscall on a person)
    void(__fastcall *person5012f0)(u8 *person);                  // 0x5012f0 (agent C's; thiscall on a person)
};
// The objects' calls go to env (nullptr: the game's).
void Object_SetEnv(const ObjectEnv *env);
// A message posted to the message centre (MessageCentre_Instance, made on the first call, then the env's 0x40fb10): what the things'
// code in 0x4e0000-0x530000 does inline; the staff and the research labs share the objects' env through it.
void Object_PostMessage(u8 *msg);

// 0x4e0450 (thiscall; name invented): the queue's head (the handle at +0x3c, a person of the table 0x7cfb90): when it is not element 0 of the
// object's ride record (Rides_GetElement of Rides_FindById of +0x24) and its state (+0x220) is 0xe, the env's 0x500870 on it. Otherwise, unless
// the handle is 0, it is taken off the queue (Object_QueueRemovePerson) and the env's 0x5012f0 runs on it. The debug prints are left out.
void __fastcall Object_DismissQueueHead(u8 *object);

// 0x4e0e60 (thiscall; state, pops 4; name invented): for state 1, 2 or 4, Object_DismissQueueHead, Object_RestartRide (inlined in the original)
// and a message 7, 9 or 8 of the object's handle (ThingMsg07 / 09 / 08 on the stack) posted to the message centre (MessageCentre_Instance, inlined)
// through the env's 0x40fb10. Every state is then kept at +0x19c. The debug prints (the handle and +0x6c; an unknown state's) are left out.
void __fastcall Object_SetState(u8 *object, void *, u32 state);
// 0x4e0050 (thiscall; open, pops 4; name invented): opening (the argument not 0): without the flag 4 of the byte +0x32 0; else
// Object_SetState (2), the strip animation of 0x7a4610[+0x20] grown from frame 8 (StripAnim_StartGrow), 1. Closing: unless the state
// (+0x19c) is 2, 1; else the strip animation shrunk (StripAnim_StartShrink), Object_CanOpen's checks made inline (they only pick a
// debug print, left out with the others), +0x68 = 1, the strip animation shrunk again, element 6 of the ride record (Rides_FindById of
// +0x24) = 0 and Object_SetState (0); 1.
u32 __fastcall Object_SetOpen(u8 *object, void *, u32 open);
// 0x4df390 (thiscall; name invented): the object closed: Object_CanOpen's checks made inline (they only pick a debug print, left out; their
// calls are kept), +0x68 = 1, the strip animation of 0x7a4610[+0x20] shrunk (StripAnim_StartShrink), element 6 of the ride record
// (Rides_FindById of +0x24) = 0 and Object_SetState (0).
void __fastcall Object_Close(u8 *object);
// 0x4e1220 (thiscall; name invented; called by 0x4e14e0): without +0x68 Object_DismissQueueHead. Else, with the ride record (Rides_FindById
// of +0x24): when its element 0 is 0, and its element 5 is below element 2 (signed) or the definition (ObjectDefs_Find of the id +0xe, looked
// up twice) is of kind 3 or 2 (+0x9c), and element 9 is 0 or the flag 1 of the byte +0x33 is set, and no rider (the word +0x6c): the queue's
// head (the word +0x3c; none: nothing), when Visitor_IsState11AtQueuePos0, gets Visitor_Set1f8One and becomes the rider. Otherwise a rider
// whose state (+0x220) is not 0xd is forgotten (+0x6c = 0). The debug prints are left out (element 2 is still read for the first).
void __fastcall Object_CallQueueHead(u8 *object);

// 0x4dd4e0: the object's definition, ObjectDefs_Find on gObjectDefs (0x7890a0) by its id (+0xe).
u8 *__fastcall Object_GetDef(const u8 *object);

// 0x4e1690 / 0x4e1670: a customer / a walk-away counted, in all and in its history's mTemp.
void __fastcall Object_AddCustomer(u8 *object);
void __fastcall Object_AddWalkAway(u8 *object);

// 0x4e1e00 (change): +0x340 = change when it is 0, else (+0x340 + change) / 2 (signed, toward 0).
void __fastcall Object_AddHappinessChange(u8 *object, void *, s32 change);

// 0x4e2190 / 0x4e21c0 (value): mQualityOfGoods / mChanceOfWinning = the value's low byte.
void __fastcall Object_SetQualityOfGoods(u8 *object, void *, u32 value);
void __fastcall Object_SetChanceOfWinning(u8 *object, void *, u32 value);

// 0x4e2710 (&out): the definition's u16 +0x284 to *out. Returns out.
u16 *__fastcall Object_DefWord284(const u8 *object, void *, u16 *out);

// 0x4e2b50: whether the definition's +0x9c is 3.
u32 __fastcall Object_DefIsKind3(const u8 *object);

// 0x4e2290: the object's scrap value by its age, the world's funny time now (FunnyTime_Now) less the dword pair at +0x18: the
// definition's mScrapValueYear1..4 (+0x188 + 4 * year, the last for 3 years or more; only the quotient's low dword is looked at) of the
// upgrade level the byte +0x50 picks (+0x40 each); 100 instead when under 30 days old, with no customer yet (+0x1a0) and the first
// level's first-year value positive (signed).
u32 __fastcall Object_ScrapValue(const u8 *object);

// 0x4dd670: the object's age in whole days, from its date at +0x18 (FunnyTime_DaysSince).
s32 __fastcall Object_AgeDays(const u8 *object);

// 0x4e2400: the scrap value (Object_ScrapValue, a percentage) of the definition's dword +0x1b8 (the first level's cost): their product
// (32-bit, wrapping) / 100, unsigned.
u32 __fastcall Object_ScrapRefund(const u8 *object);

// 0x535600 (thiscall; kind, pops 4): whether the kind is allowed beside the kind in the dword at +8 (see the code: 0 always, 4 only
// against anything but 4, 0x15 and 1 with their partners, equal kinds, 0 as the object's kind, 4 against 1, and 3 against 1 only while the
// dword at 0x820ab8 is 0).
u32 __fastcall Kinds_Compatible(const u8 *object, void *, u32 kind);

// 0x4d27a0 (thiscall; id, amount, kind, extra, pops 16): the selected slot (the index at +0x3cc, within 0..19, of the 48-byte slots from +0xc)
// is checked: when the byte at +0x3d9 is not 0, the slot's dword at +0 equals id, its word at +0xc equals the low 16 bits of kind, and the word at
// +0x3e0 is 0 or equal to the low 16 bits of extra, the dword at +0x3dc gains amount. Undefined return.
void __fastcall Slots_AddIfMatch(u8 *object, void *, u32 id, u32 amount, u32 kind, u32 extra);

// 0x4d2860 (thiscall (event), pops 4; through 0x50b643; name unconfirmed): by the event's type (its slot 0): 0x1a Slots_AddIfMatch with
// the event's id +8, amount +0xc and kind word +0x10; 0x11 with +0xc 0, Slots_AddIfMatch(0x10, 1, 0, 0); 0xb on the selected slot (as
// Slots_AddIfMatch's; none: nothing) by its id: 0x18 Slots_AddIfMatch(0x18, 100 - Things_AverageKind3Flag1Of100Minus44), 0x19 with
// Things_AverageStaffStat1f8Int, 0x1b with Things_AverageStat1f8OfKind of the slot's +0x10 0-4 as the staff kinds 5, 4, 6, 7, 8 (else the
// event's address, sic), the game [[0x7cf6ec]]'s GameTable7cfb90_Get1da720_519510 looked up first (unused). Any other type: nothing
// (the original's debug message). The kind's high half and the extra's are the original's stack garbage, which AddIfMatch never reads.
void __fastcall Slots_OnEvent(u8 *object, void *, u8 *event);

// 0x503a20 (thiscall; slot, pops 4): the entries (20 bytes from +0xc) are chosen by the dwords at +0x1414 + 16 * slot (0 when below 0): for slot 4
// the dword at +0x1ac of the object definition (word +0xc of the entry; ObjectDefs_Find) at 0x40 * (the byte at +0x15) + 0x1b8; for the others
// the dword at +4 of the object control record (the entry's word at +0xc; the game's table, ObjectControls_FindUncached quiet).
u32 __fastcall Slots_GetValue(const u8 *object, void *, u32 slot);

// 0x5b6230 (thiscall): 1 when any of the dwords at +0 and +0x6c, the dword at +0xc of each of the nine blocks pointed to from +0xe0..+0x100, or the
// dwords at +0x324 and +0x38c is not 0 (in that order), otherwise 0.
u32 __fastcall Obj_HasActivity(const u8 *object);

// 0x4e0220 (thiscall; out, pops 4): the word through out = the handle at +0x5e (0 for none). While the game's clock (the dword at +0x1da70c of the object
// [0x80239c]) is past the stamp at +0x60 plus 100 the handle is kept only if the thing it names (the table at 0x7cfb90) passes Person_HasWord (world/person) with
// this object's own handle (the word at +0); otherwise +0x5e and +0x60 are cleared and 0 is stored. Returns out.
u16 *__fastcall Obj_GetTrackedHandle(u8 *object, void *, u16 *out);

// 0x4e01f0 (thiscall; word, pops 4; the mechanic's 0x4da490 and 0x4d7a50 call it with a thing's handle): the tracked handle (the word at
// +0x5e that Obj_GetTrackedHandle reads) = word and its stamp, the dword +0x60, = the dword at +0x1da70c of the object at [0x80239c] (the
// world's clock); returns that dword.
u32 __fastcall Obj_StampAt5e(u8 *object, void *, u32 word);

// 0x4e1c60 (thiscall): the dword at +0x180 minus the one at +0x184.
u32 __fastcall Obj_Diff180_184(const u8 *object);

// 0x4e0370: 1 when the dword at 0x19c is 0x1, else 0.
u32 __fastcall Is19c_Eq1_4e0370(const u8 *o);

// 0x4e0380: 1 when the dword at 0x19c is 0x4, else 0.
u32 __fastcall Is19c_Eq4_4e0380(const u8 *o);

// 0x4e03e0: 1 when the dword at 0x19c is 0x2, else 0.
u32 __fastcall Is19c_Eq2_4e03e0(const u8 *o);

// 0x4e0440: 1 when the dword at 0x68 is 0, else 0.
u32 __fastcall Is68_Zero_4e0440(const u8 *o);

// 0x4dd7f0 (thiscall on the object; capacity, pops 4; the debug print is left out): the low byte of the capacity is held to the object definition's limits (the dwords at +0x124
// and +0x128 when their sum is above 0; the minimum first), set as element 2 of the object's ride record (Rides_SetElement of Rides_FindById of the dword at +0x24) and kept at +0x5d.
void __fastcall Object_SetCapacity(u8 *object, void *, u32 capacity);

// 0x4df300 (thiscall on the object; the debug prints of its handle and the word at +0x6c are left out; name unconfirmed): the dword at +0x68 and the
// word at +0x6c cleared, element 6 of the object's ride record set to 1 (Rides_SetElement of Rides_FindById of the dword at +0x24) and the strip
// animation of the ride at 0x7a4610[+0x20] grown from frame 1 (StripAnim_StartGrow).
void __fastcall Object_RestartRide(u8 *object);

// 0x52c2b0 (cdecl; the object): 1 unless the object definition's track type (+0x9c) is 1 or 2 and the cell a few steps ahead (by the angle at +0x10: 0 is (+4, +1), 0x5a (+1, -5), 0xb4
// (-5, -2), 0x10e (-2, +4) from the cell of columns/rows at +5 and +7; any other angle stays) and the one beside it (the second offsets: 0 is (-2, +1), 0x5a (+1, +1), 0xb4 (+1, -2), 0x10e (-2, -2)) each have exactly one neighbour
// bit (MapCell_NeighbourCount, on the 0x28-byte cells of the world at [0x7cf83c]).
u32 __cdecl Object_TrackNeighboursOk(const u8 *object);

// 0x4e02f0 (thiscall): 0 when Obj_GetTrackedHandle gives 0; else Person_HasWordInState (world/person) of the thing the handle at +0x5e names (the table
// at 0x7cfb90) with this object's own handle (the word at +0).
u32 __fastcall Obj_TrackedPersonInState(u8 *object);

// 0x4dad20 (thiscall, this unused; two stack arguments, unused, pops 8): for each thing of the list from the u16 at +0x1da746 of the object [0x80239c]
// (the table at 0x7cfb90; the next at +0xc) that is in state 1 or 2 (+0x19c) or has the dword at +0x64 set: when Obj_GetTrackedHandle gives a handle,
// Fn_4e0210 (core/initializers: +0x5e and +0x60 = 0) lets it go.
void __fastcall Objects_ReleaseTracked(void *self, void *, u32 unused1, u32 unused2);

// 0x4ca310 (cdecl; name unconfirmed): of the things on the list from the u16 at +0x1da746 of the object [0x80239c] (the table at 0x7cfb90; the
// next at +0xc), those whose definition (ObjectDefs_Find on 0x7890a0 by the id +0xe; it must exist) has map kind (+0x4ac) 0 and that are in
// state 1 (+0x19c, Is19c_Eq1_4e0370) counted.
s32 __cdecl Objects_CountKind0InState1();

// 0x4ca3a0 (thiscall, the analyser unused (least), pops 4; name unconfirmed): Objects_CountKind0InState1's walk, those of map kind 0 whose
// float +0x44 cut to an integer (__ftol's low byte) is below least (signed) or that are in state 1 counted.
s32 __stdcall Objects_CountKind0Below44OrState1(s32 least);

// 0x4fa770 (thiscall; name unconfirmed): whether the largest block value (Map_BlockValue of the map at [0x80239c] + 0x2d8) at the nine offsets of 0x7ced88 (dword pairs
// x, y: id + y * 0x80 + x as a word) from a cell is below the dword at +0x198 (unsigned; the search stops early once the largest is above it). The cell is the object's
// ((byte +7) * 0x80 + byte +5 + 1); when that is a type 3 or 9 cell, the last one of the chain Map_FindFacingNeighbour follows from it (forever round a loop).
u32 __fastcall Obj_BlockValuesBelowLimit(const u8 *object);

// 0x4daa90 (thiscall; out, person handle, pops 8; name unconfirmed): one round of the world's object list (head at +0x1da746 of [0x80239c], next at +0xc of each
// thing) from the cursor at +0xc: the cursor first moves past itself when it is in the list (else to the head); then each object in state 1 or 2 (+0x19c), or with the
// dword at +0x64 set and flag 1 of +0x32 clear, that tracks nobody or the person (Obj_GetTrackedHandle, asked twice) is measured from the person's cell (squared
// cell distance, from bytes +5 / +7 of both); the nearest one's own handle (+0) goes to out (0 when none or the list is empty). The cursor wraps to the head and ends
// where it started. Returns out.
u16 *__fastcall Objects_NearestFreeFor(u8 *self, void *, u16 *out, u32 person);

// 0x4de5a0 (thiscall; name unconfirmed): whether the way on from the object's exit is open. The exit cell (the id at +0x38, among [0x8023a0]'s cells)
// faces its direction (MapCell_Direction), turned round (Nibble_Scale) when the exit is the entrance (+0x36); the cell beyond it that way
// (Map_StepCellInDirection; the original reads through a null one) names a cell (its id word) from which any step but the one back (MapCell_StepBlocked
// in mode 0, directions 0-3 for 1, 4, 0x10, 0x40) open gives 1 (a debug print is left out), else 0.
u32 __fastcall Object_ExitOpen(const u8 *object);

// 0x4df450 (thiscall; a, b, an unused dword, a flag byte; pops 0x10; x87 at the game's precision, the result in ST0; name unconfirmed): a score from the object's
// definition (ObjectDefs_Find of the id at +0xe; the row at +0x40 * the byte +0x50) and its ride record's element 5 (Rides_FindById of +0x24), the load, held to
// [0, the definition's +0x128]. Two parts, each x * 0.9 + 0.1, halved after adding 1 when it is over: a / +0x138 over when the row's +0x1ac is not above a; b (with
// the flag) or the load over +0x128, over when the row's +0x19c is not above a tenth of the load. Returns (second + first) * the row's +0x1b0 * 0.5 (as ST0 holds it:
// a 24-bit significand, the double return keeps its exponent).
double __fastcall Object_DefRowScore(const u8 *object, void *, float a, float b, u32 unused, u32 useB);

// 0x4df640 (thiscall; a, b, c; pops 0xc; name unconfirmed): 100 less __ftol(Object_DefRowScore(a, b, c, with b) * 9), the low byte (AL).
u8 __fastcall Object_DefRowPercent(const u8 *object, void *, float a, float b, u32 c);

// 0x4dedf0 (thiscall; exit, x, y; pops 0xc; x87; name unconfirmed): the point a person stands at in the entrance cell (the word at +0x36; the definition's
// (+0xd4, +0xd8)) or appears at in the exit cell (with exit: +0x38; (+0xdc, +0xe0)). x[1] / y[1] = the cell's column / row ((id - 1) & 0x7f, (id - 1) >> 7, 32-bit);
// *x / *y = the low bytes of __ftol(255 times the point turned by the angle (+0x10): 0 (px, py), 0x5a (py, 1 - px), 0xb4 (1 - px, 1 - py), 0x10e (1 - py, px),
// another (px, px) (sic)). The debug prints of a point outside (0, 1) are left out.
void __fastcall Object_CellStandPoint(const u8 *object, void *, u32 exit, u8 *x, u8 *y);

// 0x4e1f20 (thiscall; name unconfirmed): 100 times the CHistory sum (CircBufU32_30_Sum, newest 30) of the ring at +0x2b8 over that of the ring at +0x1a8 (__ftol's low
// halves, unsigned, 32-bit); 0 when the latter is 0.
u32 __fastcall Object_HistoryPercent(const u8 *object);

// 0x4dda40 (thiscall; x87; name unconfirmed): 100 without flag 8 of the word at +0x32. Else [0x7cdc54] (a float) = the dword at +0x58 (unsigned) over the
// definition's row (+0x40 * the byte +0x50) +0x1a8 (signed), or 1 when it is 0; returns __ftol (the low half) of the byte +0x5d * the row's float +0x1b4 * that
// over the byte +0x5c, raised to 4 (a NaN too). The debug check of the float being above 0 is left out.
u32 __fastcall Object_FillRate(const u8 *object);

// 0x4e1920 (thiscall; amount, pops 4; name unconfirmed): amount added to the dwords +0xf8 and +0x184 and debited (Money_Debit of the game's money record, the table
// entry GameTable7cfb90_Get1da726_5195d0 gives); for a definition (+0xe) whose +0x4ac is 1 or 2, Slots_AddIfMatch(0xc or 0xd, -amount, 0, 0) on the entry
// GameTable7cfb90_Get1da736_519610 gives.
void __fastcall Object_PayRunningCost(u8 *object, void *, s32 amount);

// 0x4e1a30 (thiscall; first, second (floats), pops 8; x87; name unconfirmed): each setting (0-100) as (s - 50) * 0.005 stored to a float, held to [-0.5, 0.5] (a
// NaN to -0.5); the second negated for a definition (+0xe) of kind (+0x158) 1 or 3; returns __ftol (the low half) of (first + second + 1) * the dword at +0x188.
u32 __fastcall Object_SettingsValue(const u8 *object, void *, float first, float second);

// 0x4e1b40 (thiscall; x87; name unconfirmed): Object_SettingsValue of the bytes at +0x18c and +0x198 (inlined).
u32 __fastcall Object_CurrentSettingsValue(const u8 *object);

// 0x4e1c70 (thiscall; x87; name unconfirmed): __ftol (the low half) of the CHistory sum (CircBufU32_30_Sum, newest 30; inlined) of the ring at +0x70 less that of the
// ring at +0xf8. The debug prints of whether 30 is within each size are left out.
u32 __fastcall Object_HistoryDifference(const u8 *object);

// 0x4e1e30 (thiscall; x87; name unconfirmed): the newest (up to) 30 entries of the ring at +0x340 (CircBufU32_30_Get, inlined) summed as signed values (fiadd), over
// their count, __ftol'd (the low half) plus 0x32; 0x32 when the ring is empty. The debug print of whether 30 is within the size is left out.
u32 __fastcall Object_HistoryAverage(const u8 *object);

// 0x4dfd80 (thiscall; name unconfirmed): with flag 1 of the word at +0x32, when the low byte of __ftol(the float +0x44) is below 25, the object's cell (the bytes +5,
// +7) has spread record 6 taken back and record 1 spread (Map_SpreadRemove, Map_SpreadAdd on [0x8023a0]); then its ride record's (Rides_FindById of +0x24) element 8
// is set to 0, +0x44 = 100.0, the word +0x5e and the dword +0x64 = 0.
void __fastcall Object_ResetCondition(u8 *object);

// 0x4e0390 (thiscall; x87; name unconfirmed): with flag 1 of the byte +0x32, whether the low byte of __ftol(the condition float +0x44) is below 25
// (Object_ResetCondition's test), else 0.
u32 __fastcall Object_InPoorCondition(const u8 *object);

// 0x4e2440 (thiscall (amount), pops 4; x87; name unconfirmed): an object's wear. The mode setting 0xfb3b7c (a function-local static: guard 0xfb34b8
// bit 0, Setting_ReadMode and atexit of 0x5d1750 the first time) 1: when Object_InPoorCondition, Object_ResetCondition's steps; otherwise the
// condition +0x44 = (u64)amount * -0.05f + itself kept to 0..100 (NaN 0), and when that makes it poor (not before): the object's cell has spread
// record 1 taken back and record 6 spread.
void __fastcall Object_Wear(u8 *object, void *, u32 amount);

// 0x4db7d0 (thiscall; stream, save, version, unused): an object: Thing_Serialize first (1 if it fails); for a version of 0..1 then, saved or loaded as
// a serializer does (Save_Field; the load through the readers): mAngle, mId, the date +0x18 as a SYSTEMTIME's eight fields in dwords (saved from
// FileTime_ToSystemTime, loaded into FileTime_FromSystemTime, the day of the week skipped), MeshInstanceID, mFlags, mNameA[i] and mNameB[i] for each
// of the 33, mRideScriptHandle, mTrackRideHandle, mState, mTopLeft, mEntryPos, mNext, mAssignedStaffMember, mBackOfQueue, mCanLoad, mExitPos,
// mFirstInQ, mIsTrackRideValid, mUpgradeParent, the histories mHist1 and mHist0 (CircBufU32_30_Serialize), mNumCustomers, mHist2, mNumWalkAways,
// mHist3, mHist4, mHist5, mOperatingCapacity, mOperatingDuration, mOperatingSpeed, mPersonBeingLoaded, mCostOfGoods, mQualityOfGoods,
// mChanceOfWinning, mPricePerUse (loaded: held to 0..500, signed), mAmountOfSpecialIngredient, mQueueSizeInCells, the stats +0x4c, +0x48, +0x44
// (Save_StatFields), mRequestedService, mTimeMarkedForMaintenance, mTotalCosts, mTotalTakings, mUpgradeBalloonSprite and mUpgradeLevel (re/types.h's
// Object). 1 at the first that fails, else 0.
u32 __fastcall Object_Serialize(u8 *object, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4e0560 (thiscall; a, b, c, pops 12; x87; name unconfirmed): a rating (a byte; the definition looked up by the id at +0xe). For a definition of kind
// (+0x4ac) 2: 20 - __ftol(c * the root of a - b (wrapping; held to 0..100) * 0.1 (a double) * -0.8f). Else for track type (+0x9c) 3: 0 when
// Rides_GetE0ById of the dword +0x24 gives 0, else __ftol(50 - the coaster's +0x104 float (Coaster_GetPair's second) * -0.5). Else the base +0x13c,
// with a track ride (+0x28 nonzero) 60% of it (signed, wrapping) plus 3 * TrakRide_CountPieces' kind 11 pieces + its longest run + 2 * its others
// (held to 0..40), held to 0..100; times a over the row's (byte +0x50, 0x40 apart) limit +0x1a8 held to 0.75..1.25 (NaN 0.75) and stored to a float,
// by b over the limit +0x1a0 (stored to a float), held the same, their product stored to a float; __ftol of base times that. The x87's 24-bit
// precision throughout (X87Reg, X87_MulDouble).
u8 __fastcall Object_ComputeRating(const u8 *object, void *, s32 a, s32 b, s32 c);

// 0x4e0860 (thiscall; a flag, its low byte, pops 4; name unconfirmed): the definition's base rating (+0x13c, its low byte) for a nonzero flag, 0 for a
// base of 0; else Object_ComputeRating of a kind-2 definition's object +0x188 (mCostOfGoods), +0x194 (mPricePerUse) and the byte +0x190
// (mChanceOfWinning), of any other's +0x58 (mOperatingSpeed) and the bytes +0x5c (mOperatingDuration) and +0x5d (mOperatingCapacity).
u8 __fastcall Object_Rating(const u8 *object, void *, u32 baseOnly);
}
