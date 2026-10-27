// A train's cars: a count at +0x30 and pointers from +0x34. A car (+8 flags) has its part
// (+0x100 -> +0x18 -> a type whose flag 8 (+0) marks a rear coupling) and two couplings
// (+0x78 and +0x84), the one in use at +0x90 with its type part at +0x74.
#pragma once

#include "../../../re/types.h"

// Train_PlaceCars's call into the game: Animator_Update (0x473c70).
struct TrainEnv {
    s32(__cdecl *animate)(u8 *animator, const u8 *block, u32 mode);
};
// nullptr restores the game's.
void Train_SetEnv(const TrainEnv *e);
const TrainEnv *Train_GetEnv();

extern "C" {

// 0x432df0 (cdecl; type, train, force, keep; name unconfirmed): nothing while 0x790afc is set unless force. Each car (+0x30, +0x34):
// without keep its +8 and its piece's (+0x68, when there) +8 get 0x200000; a car with flag 0x2000 is then skipped. Its ride's (table
// 0x7a4610 [car+0x70]) model (+8) root frame (+4 -> +0x78) +0x44 takes the base: for the first car the train's +0x20; for a later one
// the car before's +0x50 plus the train type's (+0x18) gap +0xc4 when that car has flag 0x2000, else that car's ride model's top (+0x28
// -> +4, row type +0x18 of 20 bytes, +4) +0x34, or 0 with no top. Animator_Update (ride, 1, 0). A later car's model flags (+0x2c, when
// there) get 0x80000010 unless the type has flag 0x10; the first car's lose 0x10 and Model_FitToGrid (model, [0x7a0854], the car part's
// (+0x100) +0x20, 0). Then the model's frames at the type's indexes +0x1c / +0x20 / +0x24 / +0x28 past the root's +0x46 (below the
// root's +0x44 count its +0x70 frames, 0xa0 apart, else its +0x74 ones, 0x58 apart): the last car Flags_SetMoved (+0x1c, type 0x400)
// and clears 0x10 (+0x20, 0x800); the others clear 0x10 (+0x1c, 0x400, unless the next car has flag 0x2000) and Flags_SetMoved (+0x20,
// 0x800); the first car Flags_SetMoved (+0x24, 0x1000) and clears 0x10 (+0x28, 0x2000); the others clear 0x10 (+0x24, 0x1000) and set
// 0x80000010 (+0x28, 0x2000).
void __cdecl Train_PlaceCars(const u8 *type, u8 *train, u32 force, u32 keep);

// 0x433bd0 (cdecl, ten arguments; name unconfirmed): car put into the train's slot. The car already there (old; none past the count)
// and its train length (Car_TrainLength, inlined: 0 for an empty slot), and the length up to the slot before plus the type's (+0x18)
// gap +0xc4 (0 for slot 0) are taken first. Unless noInsert, the cars from the slot up move a slot up (their +4 too); the car takes the
// slot (+4, its train +0x100), the train's count (+0x30) goes up, and mark sets the train's flag 0x20 (+4). With an old car (and not
// noInsert): height is lowered to the old car's length less the type's +0x68 and gap (or taken from it when a NaN), the room above
// the slot before (height less that length, not below +0x68) unlerped against the type's +0xbc..+0xc0 and held to 0..1 (not above
// 0, or a NaN, 0) is the car's length +0xe8, height becomes the room plus the length before, Car_SetChannelRates ([0x790fec]+4, car),
// and Car_FitHeight (owner, the old car, type, its length, free). Otherwise, with a type, Car_FitHeight (owner, car, type, height,
// free). height to *out when out is given. Then (a type, not noInsert): Train_ShrinkToTop (owner, type, train) unless noShrink,
// Train_SetCouplings (inlined) and Train_PlaceCars (type, train, 0, 0). All on the x87.
void __cdecl Train_InsertCar(const u8 *owner, u8 *car, u8 *train, u32 slot, float height, float *out, u32 mark, u32 noInsert,
                             u32 noShrink, u32 free);

// 0x4424e0 (cdecl; a, b; name unconfirmed): two cars of a's train (+0x100) swapped: their slots (+4) and the train's slot pointers
// (+0x34), their rides' (table 0x7a4610 [+0x70]) root frame (+8 -> +4 -> +0x78) values +0x44 and their lengths +0xe8 (each moved
// through the x87 but b's +0xe8 to a); Car_SetChannelRates ([0x790fec]+4) for a and b, Train_SetCouplings, Flags_SetChanged
// ([0x790fec], 1) and Train_PlaceCars (the train's type, train, 0, 0).
void __cdecl Train_SwapCars(u8 *a, u8 *b);

// 0x443f40 (cdecl; what, car, coaster, unused, keepContacts; name unconfirmed): after an edit of car. With what bit 0: for the
// coaster's +0x60 car, unless the train type (car +0x100 -> +0x18) has flag 0x4000, the coaster's +0x118 = CarType_Lerp of the car's
// length +0xe8 held to 0..1 (below 0 or a NaN 0); with bit 2 instead, the coaster's +0x60 car without flag 0x40 (+8) gives its +0xd8
// to the coaster's +0x11c. Flags_SetChanged (coaster, 1); with bit 0 then Car_SetChannelRates (coaster +4, car) and Train_PlaceCars
// (type, train, 0, 0). Unless keepContacts, Contacts_MarkLostPair (car).
void __cdecl Car_Edited(u32 what, u8 *car, u8 *coaster, u32 unused, u32 keepContacts);

// 0x444150 (cdecl; length (a float's bits), index, unused, keepContacts; name unconfirmed): the length +0xe8 of the coaster's
// ([0x790fec]) piece index (Track_PieceAt of its +0x3c), then Car_Edited (1, it, the coaster, unused, keepContacts).
void __cdecl Car_SetLengthAt(u32 length, u32 index, u32 unused, u32 keepContacts);

// The coaster's ([0x790fec]) piece index (Track_PieceAt of its +0x3c) edited, then Car_Edited (what, it, the coaster, 0, keep);
// the forms of two arguments are the recorded edit events' (0x39, 0x3a, 0x3b, played back by 0x404xxx). Names invented, unconfirmed.
// 0x444190 (cdecl; length (a float's bits), index): the length +0xe8; what 1, keep 0.
void __cdecl Car_ApplyLengthAt(u32 length, u32 index);
// 0x4441d0 (cdecl; value (a float's bits), index, mark, keep): the value +0xe0, the car's +8 flag 1 with mark; what 2.
void __cdecl Car_SetE0At(u32 value, u32 index, u32 mark, u32 keep);
// 0x444220 (cdecl; value, index): the value +0xe0 and the car's flag 1; what 2, keep 0.
void __cdecl Car_ApplyE0At(u32 value, u32 index);
// 0x444270 (cdecl; a, b, index, mark, keep): a car with flag 0x40 (+8) gets +0xd8 = +0xdc = 0 and +0xf8 = a; else +0xd8 = a,
// +0xdc = b, +0xf8 = 0 and, with mark, flags 6 (the flags read before the stores); what 4.
void __cdecl Car_SetAnglesAt(u32 a, u32 b, u32 index, u32 mark, u32 keep);
// 0x4442f0 (cdecl; a, b, index): as 0x444270 with mark 1 and keep 0.
void __cdecl Car_ApplyAnglesAt(u32 a, u32 b, u32 index);

// 0x435340 (cdecl; node, range, car, other, keep, wantValue, out; name invented, unconfirmed): a value along range (+0xbc..+0xc0).
// With range flagged 0x4000 and keep 0: other's (its type +0x100 flag 1 (+4): +0x50; else its ride's (0x7a4610[+0x70] +8 -> +0x28 ->
// +4) entry the type's +0x18 -> +0x2c (0x14 apart, +4) -> +0x34, none 0.0; no other: range's own pointer bits, sic) less half the
// distance across x and z from node's +0x1c to its +0x2c node's (Vec3_Distance with y zeroed), then car's length set to its
// fraction of car's type (+0x100 -> +0x18) +0xbc..+0xc0 (Car_SetLengthAt (it, car's +0, 1, 0)); otherwise the coaster's ([0x790fec])
// +0x118. The value to *out when given; returned (st0, so double): with wantValue the value, else its fraction of range's span, held
// to 0..1 (below 0 or a NaN 0) unless node's +0x30 is set. sic: a null range faults (in the fraction).
double __cdecl Car_LengthFromNode(const u8 *node, const u8 *range, u8 *car, const u8 *other, u32 keep, u32 wantValue, u32 *out);

// 0x432d50: unless 0x790afc is set, marks each car but the last 0x100 (+8), clearing it on
// the last, and picks its coupling: +0x84 with the type's +0x44 for a marked car without
// flag 0x2000 whose type has flag 8, else +0x78 with the type's +0x30.
// (Callers pass the car's +0x18 first; it is unused.)
void __cdecl Train_SetCouplings(u32 unused, u8 *train);

// 0x433280: the length of car's train (0 for no car): over the first car+4 slots of its
// train's cars (+0x100 -> +0x34), each car's length bc + (c0 - bc) * its +0xe8 plus the gap
// +0xc4 (from the train's type, +0x18), then car's own length without a gap.
// sic: the count is car+4, not the train's +0x30; empty slots are skipped.
double __cdecl Car_TrainLength(const u8 *car);

// 0x430ca0 (cdecl; the first argument unused, car; name unconfirmed): unless the car's train (+0x100) has flag 1 (byte +4) or the car flag
// 0x2000 (+8), the rates of five channels of its ride's (table 0x7a4610 [car+0x70]) +0x10 block (0x38 bytes each: +0x20 = +0x1c times
// the rate) from the car's values, each as value * scale + offset with its range from the train's type (+0x18) +0xd0 on (min, max, scale,
// offset; below min or a NaN gives min, then above max gives max), all on the x87:
//   +0xe8 first clamped (above 1 gives 1, below 0 or a NaN 0), channel 0 from it (range +0xd0);
//   +0xf8 first wrapped (its modf fraction, plus 1 when negative or a NaN, stored as a float), channel 0xe0 from it (range +0x110);
//   channel 0x38 from +0xec (range +0xe0), first through modf(+0xec + 0.5) when the car has flag 0x10000000;
//   channels 0x70 and 0xa8 from +0xf0 and +0xf4 (ranges +0xf0, +0x100), each plus 0.5 after the offset.
void __cdecl Car_SetChannelRates(u32 unused, u8 *car);

// 0x433790 (cdecl; owner, car, type, height, free; name unconfirmed): the car's length (+0xe8, a 0..1 fraction of the type's shortest +0xbc
// to longest +0xc0) for its top to reach height: height less the train length of the car before it (its train's (+0x100) car car+4 - 1;
// Car_TrainLength, inlined in the original) and the type's gap (+0xc4), or height less the gap alone when that slot is empty, or height
// itself for the first car (+4 = 0); unless free, held to the type's +0x68..+0x6c (not above +0x68, or a NaN, gives +0x68; then from
// +0x6c up +0x6c); unlerped and held to 0..1 (not above 0, or a NaN, 0). Then Car_SetChannelRates (owner +4, car). All on the x87.
void __cdecl Car_FitHeight(const u8 *owner, u8 *car, const u8 *type, float height, u32 free);

// 0x433a90 (cdecl; owner, type, train; name unconfirmed): while the train length of the train's last car (Car_TrainLength, inlined; 0 for an
// empty slot) is above the type's +0x6c, its cars from the last down each take the length +0x68 (unlerped against the type's +0xbc..+0xc0,
// held to 0..1 as Car_FitHeight does) and Car_SetChannelRates (owner +4, car). 1 when any car changed; 0 for a train of one car or none.
// (The length is worked out again before each car: the whole train's, as the last car's slot counts them.)
u32 __cdecl Train_ShrinkToTop(const u8 *owner, const u8 *type, const u8 *train);

// 0x4338e0 (cdecl; owner, train; name unconfirmed): the train put back as 0x436230 saved it: its first 0x48 bytes from 0x790ff0 (so its
// count +0x30 and car slots +0x34..+0x44 too), then for each of the saved count (0x791020) of its cars, from the saved copy (0x104 bytes
// each from 0x791d00) at the same offsets: +4, +8 (keeping the car's bits 0x600000), +0xc..+0x6c, +0x90..+0xac and +0xd4..+0x100; +0x50
// from 0x791038 (a dword each); and Car_SetChannelRates (owner +4, car). Slots from the count to 5 are cleared; 0x791cd8 = 0.
void __cdecl Train_RestoreSaved(const u8 *owner, u8 *train);

// 0x442d70 (cdecl; owner, car; name unconfirmed): the car taken out of its train (+0x100): the count (+0x30) lowered, and from the car's
// slot (+4) on each later car moved down a slot (its +4 lowered). The car that takes the slot first (while its index is the car's +4) also
// takes the ride value of the car before it (its ride's +8, +4, +0x78, +0x44), and its length +0xe8 becomes the unlerp (CarType_Unlerp
// with the train's type +0x18) of its train length less the length up to the slot before plus the type's gap (+0xc4; 0 for slot 0),
// held to 0..1 (below 0 or a NaN 0); then Car_SetChannelRates (owner +4, car). The slot past the new count is cleared.
void __cdecl Train_RemoveCar(const u8 *owner, const u8 *car);

// 0x436750: picks car's coupling as Train_SetCouplings does, without touching its flags.
void __cdecl Car_PickCoupling(u8 *car);

// 0x447790: ride index's (table 0x7a4610) train (+0xd4 -> +0x100) and its car count (+0x30,
// less one when the train has flag 0x20 at byte +4) to *cars, when the ride has flag
// 0x80000 (+4); else 0 and 0.
u8 *__cdecl Ride_Train(u32 index, u32 *cars);

// 0x4477d0: the +0x70 of the train's car n - counting only cars without flag 0x2000 (+8),
// and 0 when there are not enough, when the train has flag 0x20 at byte +4.
u32 __cdecl Train_CarValue(const u8 *train, u32 n);

// 0x432bd0: fills the static record at 0x791ce8 with c, d, 0 and a pointer to the static
// pair {a, b} at 0x791050, and returns it.
u32 *__cdecl CoasterEvent_Make(u32 a, u32 b, u32 c, u32 d);

// 0x434650: where length lies between the type's shortest (+0xbc) and longest (+0xc0) car,
// (length - shortest) / (longest - shortest).
double __cdecl CarType_Unlerp(float length, const u8 *type);

// 0x434680: length / (longest - shortest).
double __cdecl CarType_Scale(float length, const u8 *type);

// 0x4346a0: (longest - shortest) * t + shortest.
double __cdecl CarType_Lerp(float t, const u8 *type);

// 0x4346c0: sets flag 1 of *flags, and 0x1000 too when also is set.
// (The original first passes flags to the empty Debug_Printf.)
void __cdecl Flags_SetChanged(u32 *flags, u32 also);

// 0x433220: sets flag 0x10 of *flags, and 0x80000000 too when high is set.
void __cdecl Flags_SetMoved(u32 *flags, u32 high);

// 0x435710: copies object's position (+0x30, three dwords, in order) to out.
void __cdecl Object_GetPosition(const u8 *object, u32 *out);

// 0x447830: the cost of car (+0x100 -> +0x18 its type; 0 when car or either is missing)
// reaching x, z (or its own +0x4c, +0x54, when x is null) from fromX, fromZ (or, each when
// null, its piece's (+0x68) +0x4c, +0x54): with distance = 0.1 * |dx, dz| and speed = 0.1 *
// CarType_Lerp(car +0xe8, type), (s+0x60 * distance + base) + s+0x64 * speed, base being the
// type's +0x5c when withBase is set, else 0. s is the type, unless its flags (+0) have 0xc000:
// then ride +4 -> +0xa8 when the next owner's (+0x100 -> +0x2c -> +0x18) flags lack 0xc000,
// and so too when s has 0x4000 and the piece lacks 0x80 (+8).
// sic: dz is squared by its float rounding; z is read only through x.
double __cdecl Car_TravelCost(const u8 *ride, u32 withBase, const u8 *car, const float *x, const float *z,
                              const float *fromX, const float *fromZ);

// 0x443df0 (cdecl (car, &cost, &extra or null, x, z, a, b); name unconfirmed; the edit passes 0x79c698 and 0x79c64c): with a current
// link (0x790fec), a selected car (0x79c640) and 0x785a2c clear, the change in cost of moving the car to (x, z): the new cost is
// Float_RoundUp of its Car_TravelCost (link, without the base, from its piece) to x, z, plus, with extra, that of its follower (+0x64)
// from x, z to its own place; the old one cost __ftol'd plus extra's. 0x79c69c = new - old, or 0 when (a, b) is (0x79c6d8, 0x79c6a0).
void __cdecl Car_SetMoveCostChange(const u8 *car, const float *cost, const float *extra, const float *x, const float *z, u32 a, u32 b);

// 0x4344c0: sets up a coupling {count, two arrays of 0x58-byte seats}: count from spec+0xc,
// both arrays allocated zeroed, and each seat i (in both) given the part with key
// spec+0x10 + i and flag 0x400000 of holder 0x7ae6a8[type+0x14] (+4) and the car (+0x10).
void __cdecl Car_SetupCoupling(u8 *car, u32 *coupling, const u8 *spec, const u8 *type);

// 0x433240: frees a coupling's seat arrays - the first only when the car's part (+0x100)
// lacks flag 1 at +4 - and clears both pointers.
void __cdecl Car_FreeCoupling(const u8 *car, u32 *coupling);

// 0x433300: picks the level of a pylon (a train: +0x18 its type with base +0x68, top +0x6c and
// gap +0xc4; cars counted at +0x30, from +0x34) for a brace at height wanted (first lowered to
// the top), returning 1 with the level and height out. Unless 0x79c658 has flag 0x100, a pylon
// whose first car has flag 0x40 (+8) is tried at its top first: when the last car's train length
// plus base and gap stays below the top, the level is the count. Otherwise each car is tried in
// turn (its train length as Car_TrainLength works it out): one with room for a brace (gaps and
// base, 0.01 slack) takes it there unless wanted lies below its space - then the level is the
// car's +4 and the height wanted. Past the last car the top is tried again (the height capped at
// the top); failing that, the last car that took it, or the count and gap + last length + base.
// The last argument only silences the original's Debug_Printf lines.
u32 __cdecl Pylon_FindBraceLevel(float wanted, const u8 *train, u32 *level, float *height, u32 quiet);
}
