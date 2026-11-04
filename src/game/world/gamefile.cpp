// A saved game's stream: see gamefile.h.
#include "gamefile.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

const u32 kActions = 0x00785a08, kClock = 0x00785970, kVanillaTime = 0x007859e8, kUi = 0x007b51f0, kCheats = 0x00fb3520, kScoring = 0x00fb3540;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 level() { return global<u32>(0x007cf83c); }

void positions(Stream *stream) {
    Stream_Tell(stream);
    Stream_Tell(stream);
}

// The tag read into the mode's slot and compared (Stream_Read's status is 0 or 1: never the negative the original tests).
bool nextTag(Stream *stream, u32 &slot, u32 tag) {
    positions(stream);
    s32 status;
    Stream_Read(&status, stream, &slot, 4, nullptr);
    return slot == tag;
}

void tagged(Stream *stream, u32 tag) {
    s32 status;
    Stream_Write(&status, stream, reinterpret_cast<const void *>((uintptr_t)tag), 4, nullptr);
    positions(stream);
}

void madeOnce(u32 guard, u32 object, u32 construct, u32 destruct) {
    u8 &made = global<u8>(guard);
    if (made & 1) return;
    made |= 1;
    g_calls.Th(construct, object);
    g_calls.Cd(0x0067a7d0, destruct);
}

void cheatsMade() { madeOnce(0x00fb3519, kCheats, 0x004053b0, 0x005d1750); }
void scoringMade() { madeOnce(0x00fb352c, kScoring, 0x0059bea0, 0x00415e70); }

constexpr u32 fourcc(char a, char b, char c, char d) { return (u32)(u8)a << 24 | (u32)(u8)b << 16 | (u32)(u8)c << 8 | (u8)d; }

}  // namespace

void GameFile_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall GameFile_Load(void *, void *, Stream *stream, u32 mode) {
    Stream_Tell(stream);
    if (g_calls.Th(0x00403780, kActions, stream, 0u, 1u, 0u)) return 0;   // ActionRec_Serialize
    positions(stream);
    if (mode == 1) return 1;
    if (!g_calls.Th(0x005179c0, level(), stream)) return 0;   // World_Load
    if (!nextTag(stream, mode, fourcc('W', 'R', 'L', 'D')) || g_calls.Cd(0x00475730, stream)) return 0;   // SpriteScripts_Load
    if (!nextTag(stream, mode, fourcc('S', 'P', 'S', 'C')) || g_calls.Cd(0x0051f7a0, stream)) return 0;   // Particles_Load
    g_calls.Cd(0x005200b0);   // Particles_EndAll
    if (!nextTag(stream, mode, fourcc('P', 'A', 'R', 'T')) || g_calls.Th(0x0040fd60, g_calls.Cd(0x00415e00), &stream)) return 0;   // MessageCentre_Load
    if (!nextTag(stream, mode, fourcc('M', 'E', 'S', 'S')) || !g_calls.Th(0x00402e30, kClock, stream)) return 0;   // Clock_Load
    if (!nextTag(stream, mode, fourcc('C', 'L', 'O', 'K')) || !g_calls.Th(0x00403260, kVanillaTime, stream)) return 0;   // VanillaTime_Load
    if (!nextTag(stream, mode, fourcc('V', 'A', 'N', 'T')) || !g_calls.Cd(0x005506e0, stream)) return 0;   // GameSystem_Load
    if (!nextTag(stream, mode, fourcc('G', 'S', 'Y', 'S')) || !g_calls.Cd(0x004647a0, stream)) return 0;   // RideSystem_Load
    if (!nextTag(stream, mode, fourcc('R', 'S', 'Y', 'S')) || !g_calls.Cd(0x00543560, stream)) return 0;   // TrackRides_Load
    if (!nextTag(stream, mode, fourcc('T', 'R', 'A', 'K')) || g_calls.Cd(0x0055e020, stream)) return 0;   // FlyingRides_Load
    if (!nextTag(stream, mode, fourcc('F', 'L', 'Y', 'R')) || g_calls.Cd(0x005597a0, stream)) return 0;   // RSSE_Load
    if (!nextTag(stream, mode, fourcc('R', 'S', 'S', 'E')) || !g_calls.Cd(0x0042cec0, stream)) return 0;   // Camera_Load
    if (!nextTag(stream, mode, fourcc('K', 'A', 'M', 'E')) || !g_calls.Cd(0x00437b60, stream)) return 0;   // Coasters_Load
    if (!nextTag(stream, mode, fourcc('C', 'O', 'A', 'S'))) return 0;
    g_calls.Cd(0x00599d00, stream);   // Advisor_Load
    if (!nextTag(stream, mode, fourcc('A', 'D', 'V', 'S'))) return 0;
    g_calls.Cd(0x0051c400, stream);   // Sound_Load
    if (!nextTag(stream, mode, fourcc('S', 'O', 'U', 'N'))) return 0;
    cheatsMade();
    if (!g_calls.Th(0x00405570, kCheats, &stream)) return 0;   // CheatSystem_Load
    if (!nextTag(stream, mode, fourcc('C', 'H', 'T', 'S'))) return 0;
    scoringMade();
    if (!g_calls.Th(0x0059c860, kScoring, &stream)) return 0;   // AdvisorScoring_Load
    if (!nextTag(stream, mode, fourcc('A', 'D', 'S', 'C'))) return 0;
    g_calls.Th(0x00481680, kUi, stream);   // UI_Load
    positions(stream);
    return 1;
}

u32 __fastcall GameFile_Save(void *, void *, Stream *stream, u32 mode) {
    Stream_Tell(stream);
    if (g_calls.Th(0x00403780, kActions, stream, 1u, 1u, 0u)) return 0;   // ActionRec_Serialize
    positions(stream);
    if (mode == 1) return 1;
    if (!g_calls.Th(0x00516c80, level(), stream)) return 0;   // World_Save
    tagged(stream, 0x006fd8b8);
    g_calls.Cd(0x00475650, stream);   // SpriteScripts_Save
    tagged(stream, 0x006fd8bc);
    if (g_calls.Cd(0x0051f680, stream, 0u)) return 0;   // Particles_Save
    tagged(stream, 0x006fd8c0);
    if (g_calls.Th(0x0040fc80, g_calls.Cd(0x00415e00), &stream)) return 0;   // MessageCentre_Save
    tagged(stream, 0x006fd8c4);
    if (!g_calls.Th(0x00402e00, kClock, stream)) return 0;   // Clock_Save
    tagged(stream, 0x006fd8d8);
    if (!g_calls.Th(0x00403220, kVanillaTime, stream)) return 0;   // VanillaTime_Save
    tagged(stream, 0x006fd8dc);
    if (!g_calls.Cd(0x00550520, stream)) return 0;   // GameSystem_Save
    tagged(stream, 0x006fd8e0);
    if (!g_calls.Cd(0x00464140, stream)) return 0;   // RideSystem_Save
    tagged(stream, 0x006fd8e4);
    if (!g_calls.Cd(0x005428e0, stream)) return 0;   // TrackRides_Save
    tagged(stream, 0x006fd8c8);
    if (g_calls.Cd(0x0055de70, stream)) return 0;   // FlyingRides_Save
    tagged(stream, 0x006fd8cc);
    if (g_calls.Cd(0x00559350, stream)) return 0;   // RSSE_Save
    tagged(stream, 0x006fd8d0);
    if (!g_calls.Cd(0x0042cdc0, stream)) return 0;   // Camera_Save
    tagged(stream, 0x006fd8d4);
    if (!g_calls.Cd(0x00437300, stream)) return 0;   // Coasters_Save
    tagged(stream, 0x006fd8e8);
    g_calls.Cd(0x00599c30, stream);   // Advisor_Save
    tagged(stream, 0x006fd8f0);
    g_calls.Cd(0x0051c350, stream);   // Sound_Save
    tagged(stream, 0x006fd8ec);
    cheatsMade();
    if (!g_calls.Th(0x004055d0, kCheats, &stream)) return 0;   // CheatSystem_Save
    tagged(stream, 0x006fd8f4);
    scoringMade();
    if (!g_calls.Th(0x0059c890, kScoring, &stream)) return 0;   // AdvisorScoring_Save
    tagged(stream, 0x006fd8f8);
    g_calls.Th(0x004816a0, kUi, stream);   // Wrap_MsgTags_Save_4816a0
    positions(stream);
    return 1;
}
}
