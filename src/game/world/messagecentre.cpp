#include "messagecentre.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/stdtree.h"

extern "C" {

void __fastcall MessageSlot_Dtor(u8 *slot) { StdTree_DtorB(slot + 4); }

void __fastcall MessageCentre_DtorSlots(u8 *centre) { CRT_VectorDtorIterator(centre, 0x14, 0x1d, reinterpret_cast<void(__fastcall *)(void *)>(&MessageSlot_Dtor)); }
}  // extern "C"
