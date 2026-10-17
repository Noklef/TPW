#include "funnytime.h"

#include "savegame.h"

extern "C" {

u32 __fastcall GameClock_Serialize(u8 *clock, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, clock, 8, 0x0075c7e0) || !Save_Field(stream, save, clock + 8, 8, 0x0075c7d0) ||
        !Save_Field(stream, save, clock + 0x10, 4, 0x0075c7bc) || !Save_Field(stream, save, clock + 0x14, 4, 0x0075c7a8) ||
        !Save_Field(stream, save, clock + 0x1c, 4, 0x0075c790))
        return 1;
    return 0;
}
}
