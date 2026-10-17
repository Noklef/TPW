// The shared header's two float statics, 1 and 1 / 2^23 (the float epsilon), that every translation unit including it instantiates: each
// unit gets its own copy of both and a pair of at-start initialisers after its last function, and nothing reads them. Reimplementation of
// the pairs whose unit is unknown (the code around them is unported or of another subject); pairs whose unit is clear live in that unit's
// module (GameClock_InitUnusedOne and others).
#pragma once

#include "../../../re/types.h"

extern "C" {

// Each pair (cdecl, a static initialiser; names unconfirmed): FloatConsts_InitOne_<addr> sets its unit's copy of 1 (the first global) and
// FloatConsts_InitEpsilon_<addr + 0x20> its copy of 1 / 2^23 (the second), both read by nothing. The unit is unknown for each.
void __cdecl FloatConsts_InitOne_407b20();      // 0x78639c (after the unported 0x405700-0x407b10)
void __cdecl FloatConsts_InitEpsilon_407b40();  // 0x786aec
void __cdecl FloatConsts_InitOne_409f60();      // 0x78736c (after the unported 0x409cf0-0x409db0)
void __cdecl FloatConsts_InitEpsilon_409f80();  // 0x787a5c
void __cdecl FloatConsts_InitOne_40d110();      // 0x788394 (after the unported 0x40d070 and 0x40d0e0)
void __cdecl FloatConsts_InitEpsilon_40d130();  // 0x788a84
void __cdecl FloatConsts_InitOne_4148d0();      // 0x78a3e0 (after the unported 0x4145f0-0x4148c1)
void __cdecl FloatConsts_InitEpsilon_4148f0();  // 0x78a458
void __cdecl FloatConsts_InitOne_41c870();      // 0x78c02c (after Graphics_Level (core/options), before core/class6fd9d8)
void __cdecl FloatConsts_InitEpsilon_41c890();  // 0x78c71c
void __cdecl FloatConsts_InitOne_423620();      // 0x78de9c (after Set4_4217f0 (core/accessors), before unported code)
void __cdecl FloatConsts_InitEpsilon_423640();  // 0x78e58c
void __cdecl FloatConsts_InitOne_424e80();      // 0x78e848 (after String_StripSuffix (core/string), before core/trivial)
void __cdecl FloatConsts_InitEpsilon_424ea0();  // 0x78e84c
void __cdecl FloatConsts_InitOne_427c00();      // 0x78fc74 (after TextScan_Read (files/textscan), before unported code)
void __cdecl FloatConsts_InitEpsilon_427c20();  // 0x790364
void __cdecl FloatConsts_InitOne_42d0f0();      // 0x790fdc (after Camera_Save (core/savemods), before coaster/xsection)
void __cdecl FloatConsts_InitEpsilon_42d110();  // 0x791cdc
void __cdecl FloatConsts_InitOne_438760();      // 0x792224 (after CoasterLink_Nodes, before coaster/helpers)
void __cdecl FloatConsts_InitEpsilon_438780();  // 0x792228
void __cdecl FloatConsts_InitOne_4398e0();      // 0x792234 (after Obj_FillLinkedElements (world/ridelist), before coaster/contacts)
void __cdecl FloatConsts_InitEpsilon_439900();  // 0x7922e0
void __cdecl FloatConsts_InitOne_43afd0();      // 0x792390 (after Contacts_ClearAll, before another pair)
void __cdecl FloatConsts_InitEpsilon_43aff0();  // 0x792394
void __cdecl FloatConsts_InitOne_43b010();      // 0x792398 (after Contacts_ClearAll (coaster/contacts), before coaster/registry)
void __cdecl FloatConsts_InitEpsilon_43b030();  // 0x79239c
void __cdecl FloatConsts_InitOne_43dbb0();      // 0x7923b8 (after RideObject_Orient, before coaster/rating)
void __cdecl FloatConsts_InitEpsilon_43dbd0();  // 0x7923d4
void __cdecl FloatConsts_InitOne_441800();      // 0x79c63c (after TrackCursor_Point (coaster/cursor), before coaster/registry)
void __cdecl FloatConsts_InitEpsilon_441820();  // 0x79c6cc
void __cdecl FloatConsts_InitOne_447e60();      // 0x79ce9c (after ActionRec_RecordCoaster45 (core/actionrec), before unported code)
void __cdecl FloatConsts_InitEpsilon_447e80();  // 0x79d58c
void __cdecl FloatConsts_InitOne_44bae0();      // 0x79df2c (after PartStates_Release (model/partstate), before unported code)
void __cdecl FloatConsts_InitEpsilon_44bb00();  // 0x79e62c
void __cdecl FloatConsts_InitOne_45f950();      // 0x7a3f1c (after a String destructor thunk)
void __cdecl FloatConsts_InitEpsilon_45f970();  // 0x7ae610
void __cdecl FloatConsts_InitOne_470990();      // 0x7b4970 (after Vec3_Distance, before motion/animator)
void __cdecl FloatConsts_InitEpsilon_4709b0();  // 0x7b4978
void __cdecl FloatConsts_InitOne_4740e0();      // 0x7b4984 (after Motion_SetFlags)
void __cdecl FloatConsts_InitEpsilon_474100();  // 0x7b498c
void __cdecl FloatConsts_InitOne_475fe0();      // 0x7b4a24 (right after SpriteScripts' own pair: a unit of no functions of its own?)
void __cdecl FloatConsts_InitEpsilon_476000();  // 0x7b4a28
void __cdecl FloatConsts_InitOne_476a30();      // 0x7b4a2c (after the sprite script ops, before sprite/screenmatrix)
void __cdecl FloatConsts_InitEpsilon_476a50();  // 0x7b4a30
void __cdecl FloatConsts_InitOne_48c9a0();      // 0x7c265c (after the 0x7c264c globals' accessors, before core/initializers)
void __cdecl FloatConsts_InitEpsilon_48c9c0();  // 0x7c2664
void __cdecl FloatConsts_InitOne_491fa0();      // 0x7ca0b8 (after a constant-wrapping thunk)
void __cdecl FloatConsts_InitEpsilon_491fc0();  // 0x7ca0dc
void __cdecl FloatConsts_InitOne_44e610();      // 0x79ee24 (after render/frame, before terrain/land)
void __cdecl FloatConsts_InitEpsilon_44e630();  // 0x79f514
void __cdecl FloatConsts_InitOne_450e20();      // 0x79f7d8 (after terrain/land, before model/scratch)
void __cdecl FloatConsts_InitEpsilon_450e40();  // 0x79fb14
void __cdecl FloatConsts_InitOne_4549d0();      // 0x79fb30 (after model/stripanim, before model/clone)
void __cdecl FloatConsts_InitEpsilon_4549f0();  // 0x79fb34
void __cdecl FloatConsts_InitOne_468910();      // 0x7afd98 (after core/idtable, before model/parts)
void __cdecl FloatConsts_InitEpsilon_468930();  // 0x7afda8
void __cdecl FloatConsts_InitOne_474fd0();      // 0x7b4a04 (after motion/path, before sprite/script)
void __cdecl FloatConsts_InitEpsilon_474ff0();  // 0x7b4a0c
void __cdecl FloatConsts_InitOne_47dd80();      // 0x7b4a58 (after core/buffers, before gfx/palette)
void __cdecl FloatConsts_InitEpsilon_47dda0();  // 0x7b4a5c
void __cdecl FloatConsts_InitOne_480190();      // 0x7b4bf0 (after gfx/gauge's drawing, before core/circbuf)
void __cdecl FloatConsts_InitEpsilon_4801b0();  // 0x7b4bf4
void __cdecl FloatConsts_InitOne_4808f0();      // 0x7b51e4 (after the unported 0x480810-0x4808e0, before core/ime)
void __cdecl FloatConsts_InitEpsilon_480910();  // 0x7b51e8
void __cdecl FloatConsts_InitOne_48d870();      // 0x7c26a8 (after the vcall wrapper 0x48d860)
void __cdecl FloatConsts_InitEpsilon_48d890();  // 0x7c26ac
void __cdecl FloatConsts_InitOne_4b4490();      // 0x7cc31c (after the 0x7cc2f0 getter, before an unported constructor)
void __cdecl FloatConsts_InitEpsilon_4b44b0();  // 0x7cc320
}
