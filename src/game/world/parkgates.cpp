#include "parkgates.h"

#include "../core/savegame.h"
#include "../core/timesig.h"

namespace {

const SaveSpec kRate[] = {{0x0, 4, 0x00759b98}};
const SaveSpec kRest[] = {{0x8, 4, 0x00759b80}, {0xc, 4, 0x00759b70}, {0x10, 1, 0x00759b64}, {0x11, 1, 0x00759b58}};
}

extern "C" {

u32 __fastcall ParkGates_Serialize(u8 *gates, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (version < 0 || version > 1) return 0;
    if (!Save_Fields(stream, save, gates, kRate, 1)) return 1;
    if (TimeSig_Serialize(gates + 4, nullptr, stream, save, version, unused) != 0) return 1;
    if (!Save_Fields(stream, save, gates, kRest, 4)) return 1;
    return 0;
}
}
