// A coaster's car placed at a map square while editing. Reimplementation of 0x435020. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

// Its calls (the ports by default, 0x498780 by address; the bench runs them as recorders).
struct CoasterPlaceCarEnv {
    u8 *(__cdecl *addEntry)(s32 x, s32 y, u32 flags, u32 unused, u32 value, const float *position);   // CoasterLink_AddEntry (0x432c10)
    u8 *(__cdecl *addCar)(u8 *train, u32 a, u32 b, float height, u32 flags, const u8 *desc, u32 half, u32 slot, u32 claim);  // CoasterLink_AddCar (0x434060)
    double(__cdecl *lengthFromNode)(const u8 *node, const u8 *range, u8 *car, const u8 *other, u32 keep, u32 wantValue, u32 *out);  // Car_LengthFromNode (0x435340)
    u32(__cdecl *findBraceLevel)(float wanted, const u8 *train, u32 *level, float *height, u32 quiet);  // Pylon_FindBraceLevel (0x433300)
    void(__cdecl *setChannelRates)(u32 unused, u8 *car);                                              // Car_SetChannelRates (0x430ca0)
    double(__cdecl *travelCost)(const u8 *ride, u32 withBase, const u8 *car, const float *x, const float *z, const float *fromX, const float *fromZ);  // Car_TravelCost (0x447830)
    void(__cdecl *resumeRunning)(u8 *coaster);    // Coaster_ResumeRunning (0x442c70)
    void(__cdecl *setTools)(u32 tools);           // CoasterEdit_SetTools (0x442880)
    void(__cdecl *afterResume)();                 // 0x498780 (unported)
    void(__cdecl *switchToTool1)();               // CoasterEdit_SwitchToTool1 (0x444770)
    void(__cdecl *refreshTools)();                // CoasterEdit_RefreshTools (0x444740)
    void(__cdecl *setBounds)();                   // CoasterEdit_SetBounds (0x434f10)
    u32(__fastcall *money)(u8 *table);            // GameTable7cfb90_Get1da726_5195d0 (0x5195d0)
    u32(__fastcall *sourceField)(u32 source);     // Source_Field0c (0x6ad810)
    void(__fastcall *debit)(u32 money, void *, s32 amount);  // Money_Debit (0x4d01f0)
};
void CoasterPlaceCar_SetEnv(const CoasterPlaceCarEnv *env);

extern "C" {

// 0x435020 (cdecl): the coaster being edited ([0x790fec]). Its range = its +4 + 0x1c8 with [0x79c658] bit 0x40, else + 0xa8. The first node of the links ([0x790fe0],
// by +0x18; their nodes from +0x4c by +0x28) with byte +4 & 5 clear and +8 = the square (the world's width * y + x; [[0x7a0854] + 0x6c] + 0x18): with the coaster's bit 2
// of +0x3c and its current car ([+0x64]) on the node (the +0x30 dwords from +0x34): it resumes (Coaster_ResumeRunning), the outs are set, CoasterEdit_SetTools(0x10) and
// 0x498780 are called, and the answer is 1. With no height given (0): a node whose last car's +0 is the coaster's +0x44 - 1 only sets the node's out and answers 0. No such
// node: CoasterLink_AddEntry. With no height given, Car_LengthFromNode and Pylon_FindBraceLevel find it and the level (the kind, when 0xff). Then CoasterLink_AddCar and
// Car_SetChannelRates. With no height given also: the coaster's +0 gets 0x1001 and, unless [0x785a2c], the car's cost (Car_TravelCost, 1 less on x and z of the node) is debited.
// With bit 0x40: [0x79c664] counts down: at 0 the bit is cleared and (no height given, no [0x785a2c]) CoasterEdit_SwitchToTool1; at 1 under the same two, CoasterEdit_RefreshTools.
// CoasterEdit_SetBounds, the outs (either null) and the answer 1.
u32 __cdecl CoasterEdit_PlaceCar(s32 x, s32 y, u32 height, u8 **outNode, u32 *outCar, u32 kind, u32 claim);
}
