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
void __cdecl FloatConsts_InitOne_438760();      // 0x792224 (after CoasterLink_Nodes, before coaster/helpers)
void __cdecl FloatConsts_InitEpsilon_438780();  // 0x792228
void __cdecl FloatConsts_InitOne_43afd0();      // 0x792390 (after Contacts_ClearAll, before another pair)
void __cdecl FloatConsts_InitEpsilon_43aff0();  // 0x792394
void __cdecl FloatConsts_InitOne_43dbb0();      // 0x7923b8 (after RideObject_Orient, before coaster/rating)
void __cdecl FloatConsts_InitEpsilon_43dbd0();  // 0x7923d4
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
}
