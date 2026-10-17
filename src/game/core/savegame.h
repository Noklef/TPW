// The save game's field announcer: reimplementation of 0x416e10. Every module's serializer announces each field it saves or loads (its size and
// name) before the stream call; the shipped build compiled the announcer to an empty debug stub.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x416e10 (stdcall; size, name, isLoad): 0, nothing else.
u32 __stdcall Save_FieldName(u32 size, const char *name, u32 isLoad);
}
