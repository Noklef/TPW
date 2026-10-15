// tpw_fncmp's stand-ins for the game's CRT (see crt.cpp).
#pragma once

#include "../../src/game/core/crt.h"

extern const CrtEnv kDriverCrt;

struct CrtStandIn {
    u32 address;  // the game's function
    void *fn;
};
extern const CrtStandIn kCrtStandIns[];
extern const u32 kCrtStandInCount;
