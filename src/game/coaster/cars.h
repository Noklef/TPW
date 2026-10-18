// The coaster's cars along its track. A car is 0x80 bytes, in an array at coaster+0x128
// (count at +0xf4); its position along the track (+0x30) is a fractional coaster node index.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43c160: how fast the car swings back, from its angle at +0x94: zero when the angle is
// zero (or NaN) or flag 0x100 (+8) is set; else (pi - angle, or -pi - angle when it is
// negative) * x / type+0x10 / (d1 + d2), with the coaster's type (+4), and d1 and d2 the
// distances (between the +0x4c positions) to the cars at +0x68 and +0x64. Left in st0, so
// double. (d1 is kept, as a float, in the car argument's stack slot.)
double __cdecl CoasterCar_SwingRate(const u8 *coaster, const u8 *car, float x);

// 0x43c1f0: spaces the cars along the track. Each car first takes the float at
// car+0x28 + (car+8 + car+0xc) * 0x60 - 0x20 (its spline node's predecessor's +0x40), plus
// the track length (coaster+0xa8) when negative (or NaN). Then from the second car round
// to the first, each is put one gap (type+0x2ec / 1000) behind the one before: from that
// car's position p, walk the coaster nodes (0x38 bytes at +0x9c, count +0xa4; length scale
// coaster+0xac over node+0x1c each, the first one only for 1 - frac(p)) until the sum r
// reaches the gap, then interpolate between the last two nodes: (n-1)*(1-w) + n*w with
// w = (r - gap) / (r - previous r), wrapped by fmod(length + x, length). Stores the first
// car at coaster+0x124.
// sic: no cars divides by zero, and a NaN or a node length that is zero or negative never
// reaches the gap, so the walk loops forever; node indices are not checked.
void __cdecl Coaster_SpaceCars(u8 *coaster);

// 0x43bc20: sets each of the car's segments (0x60 bytes at car+0x28, count +0xc) at its
// distance from the middle one (index car+0x2c): +0x38 the running sum of the segment
// lengths from the middle (exclusive) outwards, positive towards index 0 and negative
// past it, and +0x3c that over coaster+0xac; the middle one gets zeros. A segment's
// length is the car's type (+4) +0x1c when its +8 is 0, +0x24 when it is count - 1, else
// +0x20. The sums stay unrounded (only the stores round).
// sic: the middle index is not checked against the count.
void __cdecl CoasterCar_SetSegmentOffsets(const u8 *coaster, u8 *car);

// 0x43b750: the coaster's segment window (+0xf8, +0xfc): with a the float at
// car->0x90->4->8 for the car at coaster+0x54 and b the same for the car two along it (+0x64
// twice), and k the coaster type's (+4) +0x31c, each end is k of the way from its value to
// their midpoint: k * (a + b) / 2 + (1 - k) * a, and the same with b.
void __cdecl Coaster_SetSegmentWindow(u8 *coaster);

// 0x43b7e0: of the car's segments (index car+8 on, car+0xc of them), the first and the last
// whose span, from +0x40 - type+0x28 to +0x40 + type+0x24 (type at segment+4, both over
// coaster+0xac), reaches into the coaster's segment window (front >= +0xf8 and back not
// past +0xfc; a NaN front fails, a NaN back passes). Their indices into car+0x28 go to
// *first and *last.
// sic: with no match the index is that of a null pointer, -(car+0x28) / 0x60.
void __cdecl CoasterCar_FindSegmentsInWindow(const u8 *coaster, const u8 *car, s32 *first, s32 *last);

// 0x43bdb0: the car's extent along the track: +0x20 the front of its first segment (index
// car+8), position +0x74 plus its offset (+0x38) and its type's (+4) +0x24, and +0x24 the
// back of its last one (car+0xc on), the same less the type's +0x28; offsets and lengths
// over coaster+0xac.
// sic: with no segments the "last" is the one before the first.
void __cdecl CoasterCar_SetExtent(const u8 *coaster, u8 *car);

// 0x43bfb0: speed after the climb or drop from fromY to toY: speed + (fromY - toY) *
// params+0x24 * (+0x28 going up, else +0x2c), but at least minimum (minimum too for NaN).
// Left in st0, so double.
double __cdecl Coaster_SpeedAfterDrop(const u8 *params, float speed, float minimum, float fromY, float toY);

// 0x43ba90: sets up each of the coaster's cars (type+0x2e8 of them). A car whose flag 4
// (+0) is set, or whose segment count (+0xc) differs from a non-zero segments, takes it
// (when non-zero), restarts at segment 0 (+8) with the middle one at (count - 1) / 2
// (+0x2c), and when it has segments gets its offsets and extent again. With flag 1 the
// riders (from the first car's type+8 while zero) go to coaster+0xf0 and are shared out
// over each car's middle-type segments (+4 equal to its type's +0x10) into +0x44: each
// takes what is left over the seats still to fill (the segment count, less one each when
// the front type +0xc or the back type +0x14 is not the middle one); the rest get 0.
// coaster+0xb8 ends up a copy of +0xf0.
// sic: more middle segments than seats divides by zero.
void __cdecl Coaster_SetupCars(u8 *coaster, u32 riders, u32 segments, u32 flags);

// 0x43b330: sets coaster index's (table 0x790bd0) riders to n, clamped to 0..its type's
// +0x318 and to its first car's type +8 (signed), through Coaster_SetupCars(coaster, n, 0,
// 1) when that changes +0xf0.
void __cdecl Coaster_SetRiders(u32 index, s32 n);

// 0x43c3d0 (cdecl; coaster, value, quiet; name unconfirmed): unless quiet, the record 0x45 (ActionRec_RecordCoaster45); then +0x30 = value
// and +0x34 = the type's (+4) +0x4c and +0x50 mixed value percent of the way, on the x87: p = value (fild, unsigned) * 0.01f, then
// (1 - p) * +0x4c + +0x50 * p, stored as a float; the coaster's flags changed (Flags_SetChanged, also 0x1000), and when the type
// (read again) has flag 8 its contacts cleared (Contacts_ClearAll).
void __cdecl Coaster_SetMix(u8 *coaster, u32 value, u32 quiet);

// 0x43b2b0 (cdecl; index, value; name unconfirmed): coaster index's (table 0x790bd0) Coaster_SetMix(value, recorded) when there is
// one and value differs from its +0x30, unless the coaster has flag 0x8000 and the recorder (0x785a08) is not playing back (+0x24).
void __cdecl Coaster_SetMixAt(u32 index, u32 value);

// 0x447e30 (cdecl; index, value; name unconfirmed): the recorded action 0x45 replayed: Coaster_SetMixAt, then the coaster's (the table
// read again) flag 0x8000 set when there is one.
void __cdecl Coaster_SetMixAtPinned(u32 index, u32 value);

// 0x43b600 (cdecl; queue, coaster, first, count; name unconfirmed): boards riders while the coaster has some to take (+0x14) and count
// is not 0: one is taken from the queue's ring (+0xb4 the slots, +0xb0 their number, +0xbc the count, +0xc4 the read position, moved
// on round the ring; none when the count is 0, and then nothing more), a car of the count from first picked at random (the rand()
// seed 0x7b0958: seed * 214013 + 2531011, its high 16 bits modulo count), and from it on (round to first) the first car of the
// cars +0x28 (0x60 bytes each) with a seat free (+0x34 below +0x44) takes it: its part (the seats' parts +4->+0x2c) activated for the
// rider (PartStates_Activate with the instance +8 of the car kind's entry in 0x7a4610), the coaster's +0x10 raised, the rider
// seated (+0x2c) and +0x34 raised. sic: a rider no car has room for is dropped. Each rider lowers +0x14.
void __cdecl Coaster_BoardRiders(u8 *queue, u8 *coaster, u32 first, u32 count);

// 0x43b8b0 (cdecl; coaster, train; name unconfirmed): the train at the station. It is there when its front (+0x20) is not below the
// coaster's segment window (+0xf8; NaN: not there) and its back (+0x24) not past it (+0xfc; NaN passes).
// Arriving or staying: unless the train has flag 0x20 its riders get off (CoasterTrain_Unload, side 1); with the coaster's flag 4 (+8)
// its cars in the window (CoasterCar_FindSegmentsInWindow), widened to the last ones (+0x18 / +0x1c, unsigned), give the seats free
// (+0x14 = the sum of +0x44 - +0x34) and board (Coaster_BoardRiders), then become +0x18 / +0x1c; flag 0x20 is set.
// Leaving (away with flag 0x20): with flag 4 the cars from +0x18 to the count (+0xc) board as above; +0x18 / +0x1c = 0; each car
// from +8 (+0xc of them) swaps its rider lists (+0x28 / +0x2c), +0x30 = +0x34 and +0x34 = 0; flag 0x20 is cleared. Otherwise nothing.
void __cdecl CoasterTrain_AtStation(u8 *coaster, u8 *train);

// 0x43bbe0 / 0x43bc00 (cdecl; names unconfirmed): this unit's static initialisers of the shared header's two floats
// nothing reads (core/floatconsts): 0x7923a0 = 1 and 0x7923a4 = 1 / 2^23.
void __cdecl CoasterCars_InitUnusedOne();
void __cdecl CoasterCars_InitUnusedEpsilon();
}
