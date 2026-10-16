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
}
