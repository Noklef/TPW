#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs them as recorders, except RSSE_FetchWord, RSSE_OperandValue, Thing_ByHandle and World_Rand, which run as they are).
struct RsseExecEnv {
    u32(__fastcall *clock)(u8 *clock);                                                      // Clocks_Now (0x402d70)
    u32(__cdecl *alloc)(u32 size);                                                          // Mem_AllocLogged (0x556ab0)
    void *(__cdecl *free)(void *block);                                                     // Mem_GlobalFree (0x5f5680)
    s32(__cdecl *trigger)(u8 *vm, s32 kind, s32 key, u32 what, u32 extra);                  // Rsse_Trigger (0x5573d0)
    u32(__cdecl *triggerNode)(u8 *vm, u8 *node, u32 what, u32 extra, u32 value);            // Rsse_TriggerNode (0x557970)
    void(__cdecl *setAge)(u32 handle, u32 age);                                             // ParticleSystem_SetAge (0x51ff70)
    void(__cdecl *soundForward6)(u32 value);                                                // Sound_Forward6 (0x51c2c0)
    void(__cdecl *soundStop)(u32 id);                                                       // Sound_Stop (0x51c300)
    u32(__cdecl *soundSetParam)(u32 sound, u32 parameter, u32 value);                       // Sound_SetParam (0x51bc40)
    s32(__cdecl *clearQueue)(u8 *animator, u32 index);                                      // Layer_ClearQueue (0x473270)
    u32(__cdecl *play)(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index); // Animator_Play (0x4732a0)
    u32(__cdecl *getFlags)(const u8 *animator, u32 *bank, u32 *entry, u32 index);           // Layer_GetFlags (0x473fb0)
    void(__cdecl *entryPosition)(u32 index, const u32 *slot, float *out);                   // Ride_EntryPosition (0x4386d0)
    u32(__cdecl *findPart)(const u8 *holder, u32 mask, u32 key);                            // Model_FindPartByKey4 (0x44b220)
    s32(__cdecl *activate)(u8 *instance, u32 part, u32 handle);                             // PartStates_Activate (0x44b410)
    void(__cdecl *release)(u8 *instance, u32 part);                                         // PartStates_Release (0x44b4c0)
    u32(__cdecl *slotKey)(u32 ride, u32 i);                                                 // Ride_SlotKey (0x4587b0)
    void(__cdecl *meshClear)(u8 *const *ref);                                               // MeshRef_ClearFlag10000000 (0x458910)
    void(__cdecl *meshSet)(u8 *const *ref);                                                 // MeshRef_SetFlag10000000 (0x458940)
    void(__cdecl *colourScaled)(u8 *const *light, float scale);                             // LightSource_SetColourScaled (0x4587e0)
    void(__cdecl *colour)(u8 *const *holder, u32 r, u32 g, u32 b);                          // LightSource_SetColour (0x458890)
    u32(__cdecl *centre)(const s32 *handle, float *x, float *y, float *z);                  // Ride_Centre (0x556af0)
    u32(__cdecl *getVector)(const u8 *frame, u32 *x, u32 *y, u32 *z);                       // Rsse_GetVector (0x556a80)
    u32(__cdecl *startScream)(u32 scream, s32 level, s32 a, s32 b, s32 x, s32 y, s32 z);    // RSSE_StartScream (0x551130)
    u32(__cdecl *screamVolume)(u32 scream, s32 a, s32 b);                                   // RSSE_SetScreamVolume (0x551290)
    u32(__cdecl *kidPlay)(s32 level, s32 x, s32 y, s32 z);                                  // KidSound_Play (0x551560)
    u32(__cdecl *kidGraded)(s32 level, s32 a, s32 b, s32 x, s32 y, s32 z);                  // KidSound_PlayGraded (0x551320)
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);          // Sound_PlayAt (0x51bfc0)
    void(__cdecl *soundSet)(u32 value);                                                     // Sound_Set803ad0 (0x51e710)
    s32(__cdecl *flyCreate)(u32 object, u32 sound, u32 x, u32 y, u32 z, u32 turn, u32 quick); // FlyingRide_Create (0x55a620)
    void(__cdecl *flyFree)(s32 index);                                                      // FlyingRide_Free (0x55d3d0)
    u32(__cdecl *flyAddToSlot)(s32 index, u32 value);                                       // FlyingRide_AddToSlot (0x55d830)
    u32(__cdecl *flyStep)(s32 index);                                                       // FlyingRide_StepInSlot (0x55d610)
    void(__cdecl *flySet4)(s32 index, u32 value);                                           // FlyingRide_Set4 (0x55dcf0)
    void(__cdecl *flyCap)(s32 index, s32 value);                                            // FlyingRide_SetCap (0x55d950)
    void(__cdecl *flyMode)(s32 index, s32 mode);                                            // FlyingRide_SetMode (0x55da70)
    u32(__cdecl *flyGet3c)(s32 index);                                                      // FlyingRide_Get3c (0x55dd20)
    u32(__cdecl *flyTake3c)(s32 index);                                                     // FlyingRide_Take3c (0x55dd80)
    void(__cdecl *flySet44)(s32 index, u32 value);                                          // FlyingRide_Set44 (0x55ddd0)
    void(__cdecl *flyOn)(s32 index, u32 on);                                                // FlyingRide_SetOn (0x55d9f0)
    u32(__cdecl *flyReady)(s32 index);                                                      // FlyingRide_FirstSlotReady (0x55dd40)
    void(__cdecl *flySet20)(s32 index, u32 value);                                          // FlyingRide_Set20 (0x55ddf0)
    void(__cdecl *flySet24)(s32 index, u32 value);                                          // FlyingRide_Set24 (0x55de10)
    u32(__cdecl *flyBusy)(s32 index);                                                       // FlyingRide_Busy (0x55de30)
    u32(__cdecl *trakAddNext)(u32 handle, u32 value);                                       // TrakRide_AddNextCar (0x54aa80)
    u32(__cdecl *trakPop)(u32 handle);                                                      // TrakRide_PopNode (0x54ab40)
    void(__cdecl *trakBump)(u32 handle);                                                    // TrakRide_StartBump (0x544f90)
    u8 *(__cdecl *trakAddCar)(u32 handle, u32 flags);                                       // TrakRide_AddCar (0x549db0)
    void(__cdecl *trakClose)(u32 handle);                                                   // TrakRide_Close (0x544a10)
    void(__cdecl *trakOpen)(u32 handle);                                                    // TrakRide_Open (0x544840)
    void(__cdecl *trakBreak)(u32 handle);                                                   // TrakRide_Break (0x544c80)
    void(__cdecl *trakFixed)(u32 handle);                                                   // TrakRide_Fixed (0x544e50)
    void(__cdecl *trakWorn)(u32 handle);                                                    // TrakRide_MarkWornOut (0x544c00)
    void(__cdecl *trakRemoveCars)(u32 handle);                                              // TrakRide_RemoveCars (0x544b50)
    u32(__cdecl *trakField5c)(u32 handle);                                                  // TrakRide_Field5c (0x5452a0)
    u32(__cdecl *trakLoadCar)(u32 handle);                                                  // TrakRide_LoadCar (0x549b80)
    void(__cdecl *trakSetField4)(u32 handle, u32 value);                                    // TrakRide_SetField4 (0x545100)
    u32(__cdecl *trakRemoveCar)(u32 handle, u32 empty);                                     // TrakRide_RemoveCar (0x54ad90)
    u32(__cdecl *animateA)(const u8 *thing, u32 start);                                     // TrackRun_AnimateA (0x52a490)
    u32(__cdecl *animateB)(const u8 *thing, u32 start);                                     // TrackRun_AnimateB (0x52a700)
    u32(__cdecl *linkFind)(u32 key);                                                        // CoasterLink_Find (0x43b050)
    void(__cdecl *queuePush)(u32 index, u32 peep);                                          // Coaster_QueuePush (0x43b0e0)
    s32(__cdecl *queueRoom)(u32 index);                                                     // Coaster_QueueRoom (0x43b1f0)
    u32(__cdecl *boardingPop)(u32 index);                                                   // Coaster_BoardingPop (0x43b220)
    void(__cdecl *setModeAt)(u32 index, u32 which);                                         // Coaster_SetModeAt (0x43b270)
    void(__cdecl *setMixAt)(u32 index, u32 value);                                          // Coaster_SetMixAt (0x43b2b0)
    void(__cdecl *stepStateAt)(u32 index, u32 on);                                          // Coaster_StepStateAt (0x43b2f0)
    void(__cdecl *setRiders)(u32 index, s32 n);                                             // Coaster_SetRiders (0x43b330)
    u32(__cdecl *loadScript)(const char *name, u32 handle, u32 quiet);                      // RSSE_LoadScriptFile (0x5587f0)
    void(__cdecl *killScript)(s32 id, u32 flags);                                           // RSSE_KillScript (0x559060)
    s32(__cdecl *startWalker)(u8 *vm, u32 value, s32 from, s32 to, u32 a, u32 b, u32 kind, u32 last); // Rsse_StartWalker (0x556f40)
    void(__cdecl *walkerBack)(u8 *vm, u32 value);                                           // Rsse_WalkerBack (0x5571a0)
    u32(__cdecl *takeFinished)(u8 *vm);                                                     // Rsse_TakeFinished (0x557110)
    u32(__cdecl *startTimer)(RsseContext *ctx, u32 ms, u32 a, u32 b);                       // RSSE_StartTimer (0x557160)
    void(__cdecl *setEffect)(u8 *context, u32 on);                                          // Ride_SetEffect (0x556de0)
    u8 *(__cdecl *findScript)(s32 id);                                                      // Rides_FindById (0x55a070)
    u32(__cdecl *partPlace)(const u8 *obj, float *pos, float *dir, s32 key, u32 mask);      // Ride_PartPlace (0x556b90)
    s32(__cdecl *time)(s32 *t);                                                             // the CRT's time (0x67d3f0)
    const u8 *(__cdecl *localTime)(const s32 *t);                                           // the CRT's localtime (0x67d8e0)
};
// The calls go to env (nullptr: the ports).
void RsseExec_SetEnv(const RsseExecEnv *env);

extern "C" {
// 0x551cb0 (cdecl (context); its one caller, 0x55171f, ignores what EAX is left with): one instruction of a ride script. A word is fetched (RSSE_FetchWord; pc -10000 past the code); with pc dead
// nothing else, with a top byte other than 0x80 or an opcode above 0x69 pc becomes -10000. Operands are fetched in order; "value" is RSSE_OperandValue's (a variable, or the low 16 bits signed); a word
// tagged 0x40 names a variable, written or read at vars + word * 4 (the 32-bit wrap taking the tag off), and an instruction whose word lacks the tag does nothing with it.
// The time factor f is 0.5 - speed (s16 +0xc0) * -0.01, a stored float taken before the fetch; each "end" below is __ftol(n / f) + the game clock.
//  0 nothing. 1, 2 the lock 0x87919c set, cleared (2 also ends the slice: +0x98 = 0); 6 ends the slice; 0x34 pc -10000.
//  3 var = value, result too. 4 result = word. 5 result and var = value - value. 7 result and var = the clock. 0x1c result and var = |(World_Rand >> 1) % (s16 word + 1)|.
//  8, 9 a node (Mem_AllocLogged 0x1c; first on the list +0xb0; count 0x8791b4 up) with +8 and +0x10 values, Rsse_TriggerNode(what, extra 1000 or a value, value). 0xa, 0xb the nodes whose +0x18 is the
//  value: kinds 1, 2 ParticleSystem_SetAge(+0xc, -2), kinds 3-10 Sound_Forward6 (0xa) or Sound_Stop (0xb), unlinked, freed, counted down. 0xc those nodes of kinds 3-10: +0xc = Sound_SetParam(+0xc, value, value).
//  0xd, 0xe Rsse_Trigger(value, value, value, 1000 or a value). 0xf, 0x16 Layer_ClearQueue(ride, 0 or a value) with an owner (+0xc8).
//  0x10, 0x13, 0x15, 0x17, 0x1a Animator_Play(bank, entry, 0, ...) (0 with no owner), result = its answer - 300, at least 300, the word's var = result. 0x10 +0xa4 the end, +0xa8 0xffff. 0x17 the same with an
//  index and no +0xa8. 0x15 a rate value c: speed c * f * 0.001, +0xe4 = c, +0xa4 = result * 1000 / c + clock, +0xa8 0xffff. 0x13, 0x1a (no +0xa8; index value) are two-step, +0xbc holding bank + 1:
//  first call plays, +0xa4 the end, +0xbc set, pc back 4 without ending the slice; later Layer_GetFlags into the word (left raw with no owner): word + 1 = +0xbc clears +0xbc, else pc back 4, slice ended.
//  The others set +0xe4 = 1000. 0x12 (mode 1) only when +0xa8 differs from bank + entry * 0x10000, then +0xa4 = 0, +0xa8 = that. 0x19 (mode 1, raw index) +0xa4 = 0.
//  0x11, 0x18 (raw index) wait: +0xa0 set and reached (unsigned) clears it; set and not reached pc back 3, slice ended; else played, +0xa0 = __ftol((u32)(answer - 300) / f) (0x18: answer - 300) at
//  least 300 unsigned, plus the clock, pc back 3, slice ended, +0xa4 = 0 (0x11 also +0xa8 0xffff). 0x14, 0x1b (index value) result = the bank, -1 with flag 4 of Layer_GetFlags, 0 flags with no owner; var = result.
//  0x1d word, push pc | 0x20000000 at sp (down; out of range pc -10000), a label word (tag 0x20) jumps. 0x1e pop, a label jumps. 0x1f, 0x20-0x23 jump always, on result == 0, != 0, < 0, > 0: not a label pc -10000.
//  0x24 skip a word. 0x25 name +0x74 = a string word. 0x26 result = var. 0x27 result = var - value. 0x28 push value, result. 0x29 pop to result and var. 0x2a, 0x2b the same from the bottom by hp +0x44.
//  0x2c, 0x2d wait a value (0x2c: end from it; 0x2d value + clock) in +0xa0: pc back 2 and slice ended till reached, then +0xa0 = 0. 0x2e wait for +0xa4 the same, pc back 1. 0x2f var += value, result.
//  0x30, 0x31, 0x32 result and var = value * / % value (0 for a zero divisor). 0x33 turbo +0xb8 = the word.
//  0x35 flying rides by sub-opcode, on +0x9c: 1 FlyingRide_Create at Ride_EntryPosition of part 99 (mask 0x800) * 300, turn (+8's +0x10 << 12) / 360, into +0x9c (no part: nothing). 2 Free, 3 AddToSlot,
//  4 StepInSlot, 5 Set4, 8 SetCap (value * 1000), 9 SetMode, 0xa Get3c, 0xb Take3c, 0xc Set44, 0xe SetOn, 0xf FirstSlotReady, 0x10 Busy, 0x11 Set20, 0x12 Set24.
//  0x36 track rides by sub-opcode, on the +0x28 of the thing of word +0xac: 1 AddNextCar, 2 PopNode, 3 StartBump, 4 AddCar, 5 result = +0x2c, 6 Close, 7 Open, 8 Break or Fixed, 9 MarkWornOut or Fixed, 0xa
//  RemoveCars, 0xb Field5c, 0xc LoadCar, 0xd SetField4 (value * 30), 0xe SetField4 (-value), 0x10 RemoveCar, 0x11 when s16 +0xe6 differs from a value: stored, TrackRun_AnimateA and B (thing, value == 0).
//  0x37 coasters by sub-opcode on +0xe0 (the thing is looked up too and not used): 1 QueuePush, 2 QueueRoom, 3 BoardingPop, 4 SetModeAt, 5 StepStateAt, 6 SetRiders, 7 a value dropped, 8 +0xe0 = CoasterLink_Find(id), SetMixAt (speed).
//  0x38 a value in a random empty entry of +0x30 (+0x4c of them), PartStates_Activate of the ride's part entry + 1 under 0x80. 0x39 entries of a value released and cleared. 0x3a, 0x3b, 0x3c limbo (+0x24): a guest in the first
//  free slot till value * 1000 + clock; the first whose time has passed out (signed); the first out. 0x3d, 0x3e the count, the room. 0x3f, 0x40 RSSE_LoadScriptFile(+0x38 + the string word's text at +0x34, 0, 0): 0x3f +0xc = its id
//  and the new script (found in the list, null with the VM off: a fault) takes +0x10 = id, word +0xac, +0x9c, +0xb4, +0xc8; 0x40 +0x14 = the answer. 0x41 RSSE_KillScript(+0xc, 0), cleared.
//  0x42, 0x43 another script's var by index (below its +0x8c only) written, read: script +0xc; 0x44, 0x45 script +0x10. 0x46 +0x70 = word. 0x47 s16 +0x6e = var or word. 0x48 riders (+0x28): first free gets a guest, +0x70 + i, value
//  * 1000 + clock, the clock. 0x49, 0x4a the first (0x49: whose time has passed the clock read first) with (clock - start) % 1000 / 200 == 0 out. 0x4b s16 +0x6c. 0x4c Rsse_StartWalker, 0x4d Rsse_WalkerBack, 0x4e Rsse_TakeFinished.
//  0x4f RSSE_StartTimer (value * 1000), 0x50 result = +0x84, 0x51 with +0x84: +0x80 = clock - (clock * 1000 - +0x80 * 1000) / +0x84, +0x84 = 1000. 0x52-0x55 of Ride_SlotKey(owner, part of a value under 0x20000): MeshRef clear, set,
//  colour scaled (value * 0.01), colour (three of them). 0x56 +0xd0 and result = RSSE_StartScream(+0xd0, value, value, speed, Ride_Centre). 0x57 Sound_Stop(+0xd0), cleared. 0x58 KidSound_Play, or Graded for a value not negative.
//  0x59 +0xd0 = RSSE_SetScreamVolume(+0xd0, value, speed). 0x5a result 0, then of the scripts whose name equals the string word's, a random one's id in result and the word's var. 0x5b, 0x5c a var of Rides_FindById's script read
//  (else 0), written. 0x5d Ride_SetEffect, then with a value Sound_PlayAt of 0x6f and 0xd3 at Rsse_GetVector, else with more than 5 vars KidSound_PlayGraded(var 5, 1, 1). 0x5e 0 to result and var. 0x5f +0xc4 = value + clock.
//  0x60 result and var = +0xc4 - clock, at least 0. 0x61-0x66 result and var = year, month + 1, day, hour, minute, second of localtime(time). 0x67 +0xd8 = value. 0x68 byte +0xb9 = value, Sound_Set803ad0. 0x69 +0xdc, +0xde = values,
//  two values dropped, Ride_PartPlace (ctx, keys +0xdc, +0xde, mask 0x100) with their answers unused.
// The original's Debug_Printf calls are left out, and so is its reading of the pointer [0x7cf6ec] for Thing_ByHandle's unused ECX.
void __cdecl RSSE_ExecuteInstruction(RsseContext *ctx);
}
