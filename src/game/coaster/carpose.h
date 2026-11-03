// A coaster's cars and pieces posed for the frame: reimplementation of 0x430ed0.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x430ed0 (cdecl, no arguments; name unconfirmed). On the coaster [0x790fe4], with the settings [[0x792218] + 4]:
// - Each group (first +0x10, next +0x28) and its cars (count +0x30, pointers +0x34) whose kind (+0x100; flag +4 bit 0) is not off, with no car flag 0x2000
//   and a type (kind +0x18) whose +0x2c is not -1: position +0x4c = the kind's +0x1c, x and z + 5; the height from the type's +0xbc to +0xc0 by +0xe8: with a
//   seat +4, that + type +0xc4 + the +0x50 of the kind's car (+0x30 by seat); else + its own +0x50.
// - TrackPiece_Handles on each piece (first +0x18, next +0x64).
// - Each piece: TrackOrientation_Init of its direction +0x34 gives the angles +0xd4 (and, flags 1, 2 and 4 not set: +0xe0, +0xdc, +0xd8 cleared). With settings
//   flag 0x10, no flag 4 and a next piece: +0xd8 = CoasterCar_SwingRate, the running speed (from 0) by Coaster_SpeedAfterDrop. +0xec, +0xf0, +0xf4 = those / 2 pi,
//   the first and last wrapped. A piece with kind and flags on: Car_SetChannelRates, Animator_Update of its ride (+0x70 in 0x7a4610).
// - Its nodes (+0x90: count, in, out; 0x58 bytes each): on, the bone (the animator's by node +4) gives the position in +0x40; flag 0x10000000 turns the bone by -pi.
//   With flag 0x40: out +0x40 = position + 2 * the bone's +0x10; else out = position with y + 1, and +0xe4 = the bone's +0x10 and +0x20 turned to give the roll.
//   Off: out = position + up, or with flag 0x40 a sideways -1 turned by the angle; sic, these do not step on to the next node, so the first is written each time.
// - TrackPiece_NodeHandles on each piece, with its animator when on.
void __cdecl Coaster_PoseCars();
}

// The calls with large worlds of their own (the bench's recorders; null: the ports).
struct CoasterPoseEnv {
    void(__cdecl *handles)(u8 *piece, const u8 *rules, const u8 *coaster);
    void(__cdecl *channelRates)(u32 unused, u8 *car);
    s32(__cdecl *animate)(u8 *animator, const u8 *block, u32 mode);
    void(__cdecl *nodeHandles)(const u8 *piece, const u8 *source);
};
void CoasterPose_SetEnv(const CoasterPoseEnv *env);
