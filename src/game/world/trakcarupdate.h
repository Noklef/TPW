// A track ride's car (trakcarroute.h) stepping each frame. Reimplementation of 0x5474b0 (name invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs them as recorders). IntArea_Contains and World_Rand run as they are.
struct TrakCarUpdateEnv {
    s32(__cdecl *steerTo)(u8 *car, s32 x, s32 y);                                      // Collider_SteerTo (0x547c60)
    void(__cdecl *chooseRoute)(u8 *ride, u8 *car);                                     // TrakCar_ChooseRoute (0x54a040)
    u8 *(__cdecl *findNearest)(const s32 *area, s32 x, s32 y, u32 self);               // CollArea_FindNearest (0x549070)
    s32(__cdecl *unload)(u8 *car, u32 effect);                                         // TrakRide_Unload (0x54ac70)
    s32(__cdecl *remove)(u8 *car);                                                     // TrakCar_Remove (0x54ae50)
    void(__cdecl *stopSound)(u32 id);                                                  // Sound_Stop (0x51c300)
    s32(__cdecl *clearQueue)(u8 *animator, u32 index);                                 // Layer_ClearQueue (0x473270)
    u32(__cdecl *play)(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index);  // Animator_Play (0x4732a0)
};
// The calls go to env (nullptr: the ports).
void TrakCarUpdate_SetEnv(const TrakCarUpdateEnv *env);

extern "C" {
// 0x5474b0 (cdecl (car); name invented, unconfirmed): +0x90 counts up. A counter +0x88 not negative runs down while the ride (+0x9c) is in state 2 (+0x50) and +0x54 is not 2;
// at 0 the car is sent off: flags & 0xffffffa1 | 0x10020 (kinds -14, -11, -6, -3, -1: 0x4000 cleared first). Then by flags:
// - 4, chasing the car +0x80 of the pool [0x877b68]: +0x8c counts down; run out or that car not (flags & 0x40021) == 1: TrakCar_ChooseRoute. Else flag 0x10000 and it steers for where the other
//   will be: up to 0x18 of the other's steps (its +0x3c, +0x40) on, by their distance over this car's speed (each fild; fsqrt; __ftol of values halved until below 0x8000, shifted back; a speed
//   under 1 is 1); a place outside the other's area +0x98 and all the module's areas ([0x877b70], 0x20 each, [0x877b74] of them with flag 1, not with 0x40) goes to the nearest area's
//   rim (CollArea_FindNearest). At the place (the steer's answer within the ride's +0x68) flag 0x20000 is cleared.
// - 0x20, leaving: kinds -14, -11, -6, -3, -1: TrakRide_Unload (car, 1): none left: the ride's state 1, flags & 0xffffff81, its motion 0xc when not already (+0x10; on its instance +8 when set:
//   Layer_ClearQueue, Animator_Play (0xc, 0, 2, 1.0, 0)). Sound_Stop of its sound +0x20 when set. Other kinds steer for the ride's +0x90, +0x94: there with flag 0x20000: that flag for 0x10000; else
//   below +0x68: TrakCar_Remove.
// - 0x58 with nodes [0x877b78]: its node +0x7c (0x1c bytes each) with flag 1: steer for the node's +8, +0xc; there with flag 0x20000: that flag for 0x10000. Flags 0x48000 both: TrakCar_ChooseRoute,
//   0x10000. There (within +0x68): +0x8c counted down, at 0 as just said; flag 8 clear: with 0x10, a random node (World_Rand as unsigned, mod [0x877b7c]). Flag 8, 0x40: within a quarter of +0x68, flags
//   0x90000 and the node the ride's +0xa4's +4, done. Flag 8 alone follows the route: a node with flag 4 in ride state 3, 0 or +0x88 counted up to 0 ends the run (kinds -10, -9, -7, -4 in state 2: state
//   3; +0x74, +0x78 zero, sent off as above), otherwise TrakCar_ChooseRoute. A node's area +0x18, not holding the car: +0x74, +0x78 zero, 0x10000, done; else it is the car's +0x98. The next node
//   (+0x14; at the end the ride's +0xa8) without flag 2 is +0x7c, +0x78 = +0x74, +0x74 a random offset within +0x68, 0x10000, done. Otherwise 0x10000.
void __cdecl TrakCar_Update(u8 *car);
}
