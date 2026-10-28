// The track rides' module (TRAK, saved by 0x5428e0, loaded by 0x543560): reimplementation of
// its chunk write 0x543490, its stream read 0x54bbb0, the free record scan 0x549b50 and the
// handle lookup 0x549d30, and the lookups 0x5452a0 and 0x54ab40, the setters 0x544c00, 0x545100 and 0x54abd0, and the chunk reads 0x544160 and 0x5441f0. The module is loaded while 0x877b58 holds 'GFEJ' (0x4a454647); its
// rides are 0xd0-byte records from [0x877b60], named by the handle index | serial (+0) << 8.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x543490 (chunk, buffer, count): Stream_Write of buffer to the chunk's stream (+4); 0 when
// its result is negative (never: it is 0 or 1), else the chunk's count (+0x10) grows by count
// and the bytes written are returned. The chunk (0x18 bytes) starts with a 12-byte header at
// +0xc: version 1, the count, 0.
s32 __cdecl TrakChunk_Write(u8 *chunk, const void *buffer, s32 count);

// The chunk: +0 the stream's position where it starts, +4 the stream, +8 the read position, +0xc the header (version, count, total).

// 0x543400 (size; name unconfirmed): GlobalAlloc(GMEM_ZEROINIT, size) through its import slot; 0 for a size of 0.
void *__cdecl TrakMem_Alloc(u32 size);

// 0x543420 (version, stream; name unconfirmed): a new chunk (GlobalAlloc(GMEM_ZEROINIT, 0x18); none: 0) starting at the stream's position
// (Stream_Tell), the header (version, 0, 0) written to the stream (Stream_Write); unless that result is negative (never) the count grows by
// the header's 12 bytes. Returns the chunk.
u8 *__cdecl TrakChunk_Begin(u32 version, Stream *stream);

// 0x5434d0 (chunk; name unconfirmed): the total (+0x14) becomes the stream's position (Stream_Tell) less the chunk's start; the stream is
// sought back to the start (origin 0), the header written again (as Begin writes it, the count growing by 12 again) and the stream sought to
// where it was; the chunk is freed (GlobalFree). Returns the total, read after the free (sic). (A Debug_Printf is left out.)
u32 __cdecl TrakChunk_End(u8 *chunk);

// 0x544290 (chunk; name unconfirmed): the stream sought to the chunk's start plus its total (origin 0), the chunk freed (GlobalFree).
// Returns the total. (A Debug_Printf is left out.)
u32 __cdecl TrakChunk_Close(u8 *chunk);

// 0x54bbe0 (cdecl (&handle, ride handle); name unconfirmed): the first thing in use (World_FirstThing / World_NextThing on [0x7cf83c],
// newest first) that is an object (+2 is 3) of the ride (its dword +0x28 the handle): its handle (Thing_GetHandle), else 0. (Each step's
// empty Debug_Printf left out.) Returns &handle.
u16 *__cdecl TrakRide_FindThing(u16 *out, u32 ride);

// 0x54bbb0 (&result, stream, buffer, count, &done): Stream_Read; returns &result.
s32 *__cdecl TrakStream_Read(s32 *result, Stream *stream, void *buffer, s32 count, s32 *done);

// 0x549b50: the first of the 0x100 0xac-byte records at [0x877b68] (none: 0) with bit 0 of +0
// clear, or 0.
u8 *__cdecl TrakPool_FirstFree();

// 0x549d30 (handle): the ride's link (+0x58) +8 when the module is loaded, the handle names
// the ride and the link is set with flag 0x400000 in its +0; else 0. (Not loaded, a handle of
// 0 reads the link from address 0x58 and faults.)
u32 __cdecl TrakRide_LinkedValue(u32 handle);

// 0x5452a0 (handle): the ride's dword at +0x5c when the handle names it, else 0 (not loaded, a
// handle of 0 reads address 0x5c and faults).
u32 __cdecl TrakRide_Field5c(u32 handle);

// 0x54ab40 (handle): when the module is loaded and the handle names the ride, its first node
// (the list head at +0xc8) goes back on the free list at 0x877b8c and the node's first dword
// is returned; 0 for an empty list, a stale handle or no module.
u32 __cdecl TrakRide_PopNode(u32 handle);

// 0x544c00 (handle): the ride's dword at +0x54 = 1 when the module is loaded and the handle names the ride. (Not loaded, a handle of 0 writes to address 0x54 and faults.)
void __cdecl TrakRide_MarkWornOut(u32 handle);

// 0x545100 (handle, value): the same for the dword at +4 = value.
void __cdecl TrakRide_SetField4(u32 handle, u32 value);

// 0x54abd0 (handle, value): when the module is loaded, the handle names the ride and the free list at 0x877b8c has a node: the node (+0x10 the next free one) is taken
// off it, put at the head of the ride's list at +0xc8, given the handle at +4 and the value at +0.
void __cdecl TrakRide_PushNode(u32 handle, u32 value);

// 0x544160 (chunk, buffer, count): count bytes (at most those left in the chunk: +0x10 - +8) are read from the chunk's stream (+4) into the buffer; the chunk's position (+8)
// advances by what was read and the rest of the count is zeroed in the buffer. Returns the bytes read (0 when the read failed).
u32 __cdecl TrakChunk_Read(u8 *chunk, u8 *buffer, u32 count);

// 0x5441f0 (chunk): the same for one dword: its value when all four bytes were read, otherwise 0.
u32 __cdecl TrakChunk_ReadDword(u8 *chunk);

// 0x544a10 (handle): closes the ride when the module is loaded, the handle names it and its dword at +0x50 is not 0: that is cleared; the list at +0xc8 gets the list at +0xc4
// appended (or becomes it when empty) and +0xc4 is cleared; for the five ride kinds (the dword at +0 is -14, -11, -6, -3 or -1) the pool records at [0x877b68] (0xac bytes
// each, as many used ones - bit 0 of +0 - as the dword at 0x877b6c counts) that belong to the ride (+0x9c) get +0x88 cleared and bits 0x10020 set, 0x405e cleared in +0.
void __cdecl TrakRide_Close(u32 handle);

// 0x544e50 (handle): when the module is loaded and the handle names the ride, its dword at +0x54 is cleared; if it was 2 (the ride was worn out and is fixed now), for the kinds -14
// to -3 and -1 (+0) the pool records of the ride (as in TrakRide_Close) whose dword at +0x2c is not -1 end that particle system (ParticleSystem_End) and get -1 there; for the
// kind -1, when bit 0x4000 of the record is set, +0x10 = 5 and the object the table at 0x7a4610 holds for the record's dword at +8 (when it is not 0) has its layer queue cleared
// and animation bank 5 entry 0 started in mode 1 at speed 1.
void __cdecl TrakRide_Fixed(u32 handle);

// 0x545180 (handle, percent): the ride's performance (+0x1c) is set to the percent (below 0: 0, above 99: 100) and its four values at +0x20..+0x2c are set to the point that
// far (of 100) from the dwords at +0x30..+0x3c to those at +0x40..+0x4c.
void __cdecl TrakRide_SetPerformance(u32 handle, s32 percent);

// 0x545310 (handle, &a, &b, &c, &d): counts the pieces of the ride's chain (the head at +0xbc, +0x24 the next; the kind in the word at +4) twice over and stores (each pointer when not
// 0): a = the pieces of any other kind / 2, b = the longest run of kinds 9 to 13, c = kind 11 pieces / 2, d = the pieces of kind 12 or 13 with +8 zero (sic: twice
// over). Returns all the pieces / 2; 0 when the handle does not name the ride.
s32 __cdecl TrakRide_CountPieces(u32 handle, s32 *others, s32 *longestRun, s32 *kind11, s32 *free);

// 0x54aa80 (handle, value): when the module is loaded, the handle names the ride, its dword at +0x50 is not 0 and the free node list at 0x877b8c has a node: the node is taken off
// it, put at the head of the ride's list at +0xc4, given the value at +0, the handle at +4 and -1 at +8. Returns 1, otherwise 0.
u32 __cdecl TrakRide_AddNextCar(u32 handle, u32 value);

// 0x544840 (handle): opens the ride when the module is loaded and the handle names it. For the kinds -13, -12, -8, -5 (+0) with +0x50 clear: the first piece of the chain at
// +0xa8 with flag 4 is found and the first piece after it without flag 2 (the chain read round) becomes the ride's current one (+0xa4), +0x50 = 2; for the kinds -10, -9, -7,
// -4 the same with +0x50 = 1; the others only get +0x50 = 1. The performance is then set again from +0x1c (TrakRide_SetPerformance). (A chain without a piece with flag 4,
// or with every piece flagged 2, never ends.)
void __cdecl TrakRide_Open(u32 handle);

// 0x545890 (kind, x, y, orientation, handle, value): when the module is loaded, makes a ride of the kind (-1 to -14; a handle names the record, else the first free one of the 0x40
// at [0x877b60] is taken - none: faults) from the template record 0xd0 bytes apart from 0x764178, the ride count [0x877b64] raised, with the orientation at +0xb0, the position
// at +0xb4 / +0xb8 and the value at +4, no chain; then by the template's kind: the five kinds with collision nodes (-14, -11, -6, -3, -1) get one collision node (sizes 0x1600,
// 0x1100, 0x1600, 0x1600 and 0x1200, the node pushed on the list at +0xc0) and eight pieces round it; -13, -12, -8 and -5 one end piece (flag 4); -10, -9, -7 and -4 as many
// pieces (flag 2) as +0x64 says along a line and an end piece; -2 three collision nodes and fourteen pieces. The performance is set from +0x1c (not for -2); returns the
// ride's handle (index | kind << 8), 0 when the module is not loaded.
u32 __cdecl TrakRide_Create(s32 kind, s32 x, s32 y, u32 orientation, u32 handle, u32 value);

// 0x548e80 (cdecl; ride, asleep; name unconfirmed): asleep and the ride's flag 0x200 (+0) clear: the flag is set and the instance [0x7a4610 + 4 * (+8)] leaves the
// renderer (its holder at +8 by Obj_RemoveFromRect over the words at +0xc0 / +0xc4 when the instance's flags (+4) have 0x4000, else Holder_Sleep), so does the instance
// of +0xc when the flags have 0x2002000 and +0xc is not 0, and the parts of the node list at +0x30 (next at +0x10, part at +8; -1 none) are released
// (PartStates_Release on the first instance's holder). Not asleep with the flag set: the flag is cleared and the same in reverse (Holder_AddToGrid / Holder_Wake,
// PartStates_Activate with handle 0). Otherwise nothing.
void __cdecl TrakRide_SetAsleep(u8 *ride, s32 asleep);

// 0x54b2f0 (cdecl; handle, x, y, piece, a, b, c; name unconfirmed): a track section added to the ride when the module is loaded and the
// handle names it; 0 otherwise, for a section of the ride's chain (+0xbc, the next at +0x24) already at (x, y) (after "Duplicate track
// section - IGNORED!" to the empty Debug_Printf), or when the section pool (0x400 records of 0x28 bytes from [0x877b80], bit 0 of +0 in use;
// none: 0) is full. The first free section: +0 = 1 (the count [0x877b84] up), +4 the piece, +8/+0xc/+0x10 a, b, c, +0x14/+0x18 the
// position, +0x20 the ride, +0x24 = 0, appended to the chain; its node list +0x1c is not cleared (sic). By the piece's kind (its low word)
// collision nodes (CollNode_Alloc, put at the head of that list: +0x14 the next, +0x18 the section; w = the ride's +0xc - +8): kinds 1-8,
// in pairs (1 and 5, ...), two at a corner of the 0x1800 square (1: (x + 0x1800, y); 2: (x, y); 3: (x, y + 0x1800); 4: both) of sizes
// w + 0xc00 and 0xc00 - w, both with that corner's flags (0x14, 0x12, 0xa, 0xc) cleared and the second a blocker (0x40); 9 and 12 a box
// (0x20; 12 also 0x100) at the centre of size 0xc20 by w; 10 and 13 the same of w by 0xc20; 11 a circle at the centre of radius
// sqrt(w^2 + 0x900000) (truncated; flag 0x80 for the ride kinds -5, -8, -12 and -13) and the four corners' blockers of 0xc00 - w. Unless the
// piece has 0x10000 a buoy (Buoy_Alloc) at (x + ox, y + 0x1800 - oy) (1: 0xf84, 0xf84; 2: 0x87c, 0xf84; 3: 0x87c, 0x87c; 4: 0xf84, 0x87c;
// 9-13: 0xc00, 0xc00; other kinds y + 0x1800 for both, sic) joins the ride's buoy chain (+0xa8, the next at +0x14; the count +0xac up),
// its +0x10 the ride and +0x18 the first node, whose +0x1c is the buoy (no node, as for the other kinds, faults). A node the pool cannot
// give faults at its flags. Returns 1.
s32 __cdecl TrakRide_AddSection(u32 handle, s32 x, s32 y, u32 piece, u32 a, u32 b, u32 c);

// 0x54ac70 (cdecl; ride, effect; name unconfirmed): empties the ride's node list +0x30 (the next at +0x10): each node goes to the head of
// the list +0xc8 of the record at +0x9c, whose count +0x60 goes down; a node with a part (+8, -1 none) has it released (PartStates_Release
// on the holder +8 of the instance [0x7a4610 + 4 * (+8)]) and set to -1, and with effect (any dword but 0) Particle_Spawn(0x13) at 1024
// times the position +0x40..+0x48 of that holder's model's root (+4 -> +0x78; z through __ftol first). Returns the record's count then.
// (Its Debug_Printf of the node's +0 and the record's handle only reads.)
s32 __cdecl TrakRide_Unload(u8 *ride, u32 effect);

// 0x54ae50 (cdecl (car): a 0xac-byte record of the pool [0x877b68]; called by 0x545450 / 0x545610; name invented): the car taken off its
// ride (+0x9c): TrakRide_Unload (car, 0); flag 0x400000 cleared (and the ride's +0x58 with it); unless the ride's state (+0x50) is 4,
// Particle_Spawn (0xf) at 1024 times the car's instance's position (as Unload's; the original reads it either way); Ride_Destroy 0x463c80 (through
// the queue pieces' env) of +8, and of +0xc with the flags 0x2002000 (then cleared); with a sound (+0x20) and the state not 4,
// for the ride kinds (+0) -10, -9, -7 and -4 Sound_PlayRideElement (the ride's object's +0x24 (the inlined search of TrakRide_FindThing),
// the sound, [0x803a3c], 1, 0, 0, 0) and, when it plays, Sound_SetPosition at the car's +0x34 / +0x38 (as TrakCar_PlaySounds'); +0x20
// left 0; other kinds Sound_Stop (the sound). A particle system (+0x2c) ended (ParticleSystem_End) and set to -1; flag 1 cleared, the
// count 0x877b6c down, the ride's +0x5c down: at 0 its state becomes 1 (loading), except for the kinds -13, -12, -8 and -5. Returns the
// ride's +0x5c. (Its Debug_Printf lines left out.)
s32 __cdecl TrakCar_Remove(u8 *car);

// 0x549c60 (cdecl; ride, effect; name unconfirmed): Unload's reverse for the node list +0x30 (the next at +0x10): each node in turn is
// given the seat number +0xc (1, 2, ...) and the part +8 Model_FindPartByKey4(the instance [0x7a4610 + 4 * (+8)], 0x80, that number) finds;
// a part found (not -1) is activated (PartStates_Activate on the instance's holder +8, with the node's +0 as the handle) and with effect
// (any dword but 0) Particle_Spawn(0x13) at 1024 times the holder's model's root position (as Unload). The +0x9c record's count +0x60 goes
// up for every node.
void __cdecl TrakRide_Load(u8 *ride, u32 effect);

// 0x547170 (cdecl; car: a 0xac-byte record of the pool [0x877b68]; called at 0x547117 after the area and blocker passes; name
// unconfirmed): the car's sounds. A next state (+0x14) above the state (+0x18, unsigned) replaces it, and +0x14 is cleared; then by the
// state, at x = __ftol(+0x34 * 0.0033036) and z = __ftol(+0x38 * 0.0033036) (fild; fmul 0x700f20), y 0: 1 Sound_PlayAt(0, [0x803a28],
// 0x18, ...); 3 and 5 Sound_PlayAt(0, [0x803a28], 0xc, ...); 2, when the game's rand (0x877b90 = * 0x343fd + 0x269ec3) has none of
// the bits 0x70000, and 4: Sound_PlayRideElement(the dword +0x24 of the thing found, 0, [0x803a3c], 3 or 4, ...), the thing (0: none)
// the ride's object (state 2: TrakRide_FindThing; state 4: the same search inlined, through Thing_ByHandle on [[0x7cf6ec]]) for the
// handle of the car's ride (+0x9c; 0 while the module is not loaded); after each the state is 0 (2: also when the rand skips). Other
// states leave it. The original's Debug_Printf calls are left out.
void __cdecl TrakCar_PlaySounds(u8 *car);

// 0x547f50 (cdecl; car, as TrakCar_PlaySounds'; called at 0x546cd2 for each car in use; name invented, unconfirmed): the car's step. The
// velocity +0x3c / +0x40 times the ride's (+0x9c) rate +0x24 (car flag 0x80000: 7/10 of it; else 1000 when the ride's +0x54 is 2) over
// 1024 (signed, truncated) is the new velocity (also +0x44 / +0x48), its length (fsqrt of the 32-bit sum of squares, __ftol) the speed
// +0x4c, and the place +0x34 / +0x38 moves by it. By the ride's kind: -14, -11, -6, -3, -1 (ride +0x50 2: the heading +0x58 (+0x50 for
// -1, else 256 - atan2(vx, -vy) / pi * 256) and the turn +0x5c = ((heading - +0x54) & 0x1ff as -0xff..0x100) * speed / 1000; else turn
// 0), with ride +0x50 2, car flag 0x4000, the frame (0x877b5c) & 0x1f 0, part +0x28 not -1, the game's rand's (0x877b90) bits 16-17 2,
// the kind (read again) not -1 and +0x14 0: Particle_SpawnWithVelocity(1) at that part of the instance's holder's part state entries
// (1024 times its place, along its direction normalised) and +0x14 = 4; the sound below (the thing by TrakRide_FindThing); the particle
// system +0x2c (not -1) at part +0x24 (not -1). -13, -12, -8, -5: a turn +0x58 past 0x30 either way scaled by 99.8 * 0.01 (__ftol); +0x5c
// = +0x58 / 128 clamped to -16..16; ride +0x50 2, the frame and the rand's bits 16-18 2: Particle_Spawn(0x60) at 1024 times the
// instance's model's root place; the sound. -10, -9, -7, -4: +0x14 = 2 when 0 with ride +0x50 2, the frame and speed above 0xe6; the
// animation +0x10 12 becomes 5 above speed 0x28 (Layer_ClearQueue, Animator_Play(5, 0, mode 1) on the instance +8 when not 0), 5 becomes
// 12 at or below it unless the car's flags had 0x400 (Animator_Play(0xc, 0, mode 2)); with ride +0x50 2, +0x54 1, the frame, speed above
// 0x50 and part +0x24 not -1, the rand's bits 16-18 0 or 1 spawn particle 1 at that part, 2 (and +0x14 0) particle 10 and +0x14 = 1;
// the sound (the thing by the park's things walked); the particle system; then as -2 and any other kind: the heading 256 - atan2(+0x3c,
// -+0x40) / pi * 256, +0x60 = the new turn - the old. The sound +0x20 (not 0): Sound_SetPosition(x, 0, z) at +0x34 / +0x38 * 0.0033036,
// then Sound_SetParam(Rides_GetLinkedElement(the ride's thing's id, 10), speed / 3). +0x54 += the turn (& 0x1ff). The link +0x98: without
// flag 0x100 a sleeping car (flag 0x200) is woken (TrakRide_SetAsleep); with it, its gate +0x18 (marked 0xdac at +0x10) on axis x (+4
// 0xc: +0x34 - link +4) or y (+0x38 - link +8): gate +8 0, an awake car within 0x4b0 gets flag 0x800000 for a negative distance (else
// cleared) and sleeps, the gate's animator (+0xc) told 5 (Animator_Play mode 0) unless Layer_GetFlags says it plays bank 5, and the gate
// flag 2 set; gate +8 not 0, a sleeping car past 0x4b0 on its side wakes; a gate with flag 2 whose queued bank (Layer_GetQueued) is 0xc
// plays bank 2 (mode 1) and loses the flag. The original's argument slot holds the car's flags, then the latest of the values written
// into it (the length of a part's direction, a handle found, the heading's fild operands); with no animator the gate tests read that
// (sic). The original's Debug_Printf calls are left out.
void __cdecl TrakCar_Move(u8 *car);
// 0x545450 (cdecl (handle); name invented, unconfirmed): for a handle that names its ride (the module not loaded: only 0, then the null ride
// is read), the pool's cars (0x877b68, 0xac bytes each; the used ones, bit 0, as many as 0x877b6c counts) on the ride (+0x9c) taken off
// (TrakCar_Remove), the ride closed (TrakRide_Close), its +0xa8 pieces cut to those with flag 2 or 4 (Collider_KeepA8 (ride, 6), inlined)
// and its +0xbc chunks freed (Pool_FreeAll, inlined).
void __cdecl TrakRide_Strip(u32 handle);

// 0x545610 (cdecl (handle); name invented, unconfirmed): as TrakRide_Strip, but every +0xa8 piece dropped (Collider_KeepA8 (ride, 0)), then
// the +0xc0 list released (Collider_ReleaseC0), the ride's +0 cleared and the ride count 0x877b64 down (all inlined).
void __cdecl TrakRide_Destroy(u32 handle);

// 0x5445d0 (cdecl; name invented, unconfirmed): with the module loaded ('JEFG' at 0x877b58), each used ride (+0 set) of the pool
// [0x877b60] (0xd0 bytes each), as many as [0x877b64] counts at the start, gets state 4 (+0x50) and is destroyed as TrakRide_Destroy
// destroys its name (inlined in the original); then the 0x400 free records of [0x877b88] (0x14 bytes) chained again by +0x10 (the last
// none) and [0x877b8c] = the first.
void __cdecl TrakRides_DestroyAll();
}
