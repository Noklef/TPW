#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs all but RSSE_ExecuteInstruction as recorders).
struct RsseTickEnv {
    void(__cdecl *execute)(RsseContext *ctx);                                  // RSSE_ExecuteInstruction (0x551cb0)
    void(__cdecl *setSpeed)(u8 *animator, float speed);                        // Animator_SetQueuedSpeed (0x474020)
    void(__cdecl *setCap)(s32 index, s32 value);                               // FlyingRide_SetCap (0x55d950)
    void(__cdecl *setTenth)(s32 index, s32 value);                             // FlyingRide_SetTenth (0x55d9b0)
    void(__cdecl *setMix)(u32 index, u32 value);                               // Coaster_SetMixAt (0x43b2b0)
    void(__cdecl *setPerformance)(u32 handle, s32 percent);                    // TrakRide_SetPerformance (0x545180)
    void(__cdecl *partAxes)(s32 index, const u32 *part, float *dir, float *up); // Ride_PartAxes (0x551b30)
    void(__cdecl *setVector)(u32 handle, s32 x, s32 y, s32 z);                 // ParticleSystem_SetVectorA (0x51fe30)
    void(__cdecl *setScaled)(u32 handle, s32 x, s32 y, s32 z);                 // ParticleSystem_SetScaled (0x51fd90)
    u32(__cdecl *setPosition)(u32 sound, s32 x, s32 y, s32 z);                 // Sound_SetPosition (0x51c270)
    void(__cdecl *kill)(s32 id, u32 flags);                                    // RSSE_KillScript (0x559060)
};
void RsseTick_SetEnv(const RsseTickEnv *env);

extern "C" {
// 0x5516b0 (cdecl; ()): the scripts' tick. Off (0x8791a0 zero) it only returns [0x8791b4]; else the counter 0x8791a4 goes up by one and each script of the list (0x8791b0) in turn: with
// +0xb8 set, or its id's low three bits the counter's, it runs (below); then its object nodes (+0xb0, next at +0, those whose +0x10 is negative passed over) follow it, by their +8:
// 1 ParticleSystem_SetVectorA of +0xc at the position of the part +0x14 (ride [0x7a4610 + owner +0xc8 * 4] +8 -> +0x28 -> +4, 0x14 each, +4: its +0x30, +0x34, +0x38, or 0 for none), each
// __ftol'd (z first) and << 10; 2 the same (no 0.0 float loaded: the z dword moved) and Ride_PartAxes' direction * 1024 __ftol'd for ParticleSystem_SetScaled; 3-10 on the script's every
// eighth tick (its id's low three bits only: +0xb8 does not count) Sound_SetPosition of +0xc at the part's position, __ftol'd; another kind nothing. A script whose pc is negative by then
// is killed (RSSE_KillScript(id, 0); its next read first). Returns [0x8791b4].
// Running: with code (+0x18), the lock 0x87919c cleared, +0x98 = the budget +0x94, and while the budget was above 0, up to +0x98 instructions (RSSE_ExecuteInstruction) while pc is not
// negative: one that leaves the lock set is not counted. Then the s16 speed +0xc0 goes out: with an owner +0xc8, Animator_SetQueuedSpeed of [0x7a4610 + owner * 4] at (0.5 - speed *
// -0.01) * s16 +0xe4 * 0.001; a flying ride +0x9c (FlyingRide_SetCap at var 3 * 1000 when +0x8c is above 3, then FlyingRide_SetTenth); a coaster +0xe0 (Coaster_SetMixAt); the word +0xac,
// a thing of 0x7cfb90 (20 bytes each), when it is there and its +0x28 too (TrakRide_SetPerformance); and the child +0xc (id) found in the list, its +0xc0 too: no such script, or the VM
// off, writes at 0xc0: a fault (sic). The original's Debug_Printf calls (empty) are left out.
u32 __cdecl RSSE_Tick();
}
