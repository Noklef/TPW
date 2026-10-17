// tpw_fncmp's stand-ins for the game's CRT (see crt.cpp).
#pragma once

#include "../../src/game/core/crt.h"
#include "../../src/game/core/libc.h"

extern const CrtEnv kDriverCrt;
extern const LibcEnv kDriverLibc;  // the libc wrappers' (core/libc)

struct CrtStandIn {
    u32 address;  // the game's function
    void *fn;
};
extern const CrtStandIn kCrtStandIns[];
extern const u32 kCrtStandInCount;
