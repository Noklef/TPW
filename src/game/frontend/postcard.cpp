#include "postcard.h"

#include "../core/dequeiter.h"

extern "C" {

void __cdecl Postcard_OnSent(u32 done, u32 result) {
    static const u32 kCommands[5] = {0x34, 0x39, 0x3a, 0x3b, 0x3b};   // (the jump table 0x4a7c3c)
    if (!done || result > 4) return;
    CommandQueue_Post(kCommands[result], 0);
}
}
