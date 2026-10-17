#include "gameclock.h"

#include "../files/file.h"

extern "C" {

u32 __fastcall GameClock_Save24(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Write(&result, stream, clock + 0x24, 4, nullptr) >= 0;
}

u32 __fastcall GameClock_Load24(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Read(&result, stream, clock + 0x24, 4, nullptr) >= 0;
}

u32 __fastcall GameClock_Save4c(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Write(&result, stream, clock + 0x4c, 4, nullptr) >= 0;
}

u32 __fastcall GameClock_Load4c(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Read(&result, stream, clock + 0x4c, 4, nullptr) >= 0;
}

u32 __fastcall Clock_Save(u8 *clock, void *, Stream *stream) {
    const u32 ok = GameClock_Save4c(clock, nullptr, stream);
    return ok ? GameClock_Save24(clock + 0x50, nullptr, stream) : ok;
}

u32 __fastcall Clock_Load(u8 *clock, void *, Stream *stream) {
    const u32 ok = GameClock_Load4c(clock, nullptr, stream);
    return ok ? GameClock_Load24(clock + 0x50, nullptr, stream) : ok;
}

double __fastcall GameClock_Double20(const u8 *clock) { return *reinterpret_cast<const double *>(clock + 0x20); }
}
