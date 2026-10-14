// Coaster ratings: reimplementation of 0x43dbf0, 0x43dc50 and 0x43df90, which the test run
// (0x43c450) calls to rate a coaster's track node by node.
//
// The coaster's type (coaster +4) holds the BalanceTrack values it was made from, copied
// by 0x42f390 when non-zero: +0x320 mfInitialSickness, +0x324 mfInitialExcitement,
// +0x32c mfGForceSicknessModifier, +0x330 mfUpsideDownSicknessModifier,
// +0x334 mfSpeedExcitementModifier, +0x338 mfGForceExcitementModifier,
// +0x340 mfBankExcitementModifier, +0x344 mfXGForceUncomfortableModifier,
// +0x348 mfInvertedUncomfortableModifier, +0x34c/+0x350 mfNormal/TerrorScreamStartExciteDelta,
// +0x354/+0x358 mfNormal/TerrorScreamSustainExcitement (BalanceTrack offset - 0x3818).
// The running totals live in statics at 0x7923a8 (CoasterRatingState).
#pragma once

#include "../../../re/types.h"

struct CoasterRatingState {    // at 0x7923a8
    u32 unknownA8;             // zeroed by the reset, read elsewhere
    u32 screaming;             // +0x04 (0x7923ac): the last node's riders scream
    float maxSickness;         // +0x08
    float sickness;            // +0x0c (0x7923b4)
    u32 unknownB8;             // +0x10: not the ratings' (0x43d8f0 writes it)
    float maxExcitement;       // +0x14
    float lastExcitement;      // +0x18 (0x7923c0)
    u32 terrified;             // +0x1c (0x7923c4): the last node's riders scream in terror
    float lastForce[3];        // +0x20 (0x7923c8): only ever zeroed (sic)
    u32 unknownD4;             // +0x2c: not the ratings'
    float excitement;          // +0x30 (0x7923d8)
    float lastBank;            // +0x34 (0x7923dc): only ever zeroed (sic)
};
static_assert(sizeof(CoasterRatingState) == 0x38, "CoasterRatingState covers 0x7923a8-0x7923df");

extern "C" {
// 0x43dbf0 (coaster): sickness and excitement from the type's initial values, the rest 0.
void __cdecl CoasterRating_Reset(const u8 *coaster);

// 0x43dc50 (coaster, node, unused, speed, bank, force, unused): rates node `node` (the
// nodes are 0x38 bytes at coaster +0x9c), given the train's speed there, its bank angle
// and the g-force vector. Both totals decay by half each node and add:
//   sickness   (upside + |bank| - pi/2 past the vertical) * upside-down modifier
//              + |force - last force| (summed over x, y, z) * g-force modifier
//   excitement speed * speed modifier + |bank - last bank| * bank modifier
//              + |force| * g-force modifier - |force.x| * x g-force modifier
//              - the inverted term, at least 0.
// where upside is 1 past the vertical, else 0, and the inverted term is upside + past,
// times the inverted modifier when force.y < 0.1 and 3 times that when force.y < 0.
// Clears the node's flags 1, 2 and 4, then from node 3 on sets 4 (scream) when excitement rose
// by more than the normal start delta (or stays above the normal sustain level while
// screaming) and 2 (terror) likewise for the terror values.
// - "last force" and "last bank" are never stored, so both terms take the node's own value.
// - The arithmetic is the x87's, operand for operand: wide intermediates (only the stores
//   to locals and statics round to a float), and of two NaNs the larger significand.
// - Each comparison is the x87's: a NaN makes the sickness and excitement maxima take the
//   new value and the excitement term 0.
void __cdecl CoasterRating_Step(u8 *coaster, u32 node, u32 unused, float speed, float bank, const float *force,
                                float unused2);

// 0x43df90 (coaster, max height, top speed, unused): the ratings into the coaster: +0x100
// sickness (peak + current), +0x104 excitement (current / 2 + peak * 1.7), both clamped to
// 0..100 (NaN as 100); +0x108 the cars (list at +0x54, next +0x64) with flags 0x40 and
// 0x80; +0x10c = +0xac * +0xa8; +0x110 the height, +0x114 the speed.
void __cdecl CoasterRating_Finish(u8 *coaster, float maxHeight, float topSpeed, float unused);
}
