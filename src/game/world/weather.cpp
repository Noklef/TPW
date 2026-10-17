#include "weather.h"

#include "../core/savegame.h"
#include "forecast.h"
#include "things.h"

extern "C" {

u32 __fastcall Weather_Serialize(u8 *weather, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kHead[] = {{0xc, 4, 0x0076239c}, {0x10, 8, 0x0076238c}, {0x18, 8, 0x0076237c}};
    static const SaveSpec kTail[] = {{0x30, 4, 0x0076236c}, {0x34, 4, 0x0076235c}, {0x38, 1, 0x00762350}, {0x3c, 4, 0x0076233c}, {0x40, 4, 0x00762324},
                                     {0x44, 4, 0x00762310}, {0x48, 12, 0x007622f8}, {0x54, 4, 0x007622e4}, {0x58, 4, 0x007622dc}, {0x5c, 4, 0x007622d0},
                                     {0x60, 4, 0x007622c4}, {0x64, 1, 0x007622b8}};
    if (Thing_Serialize(weather, nullptr, stream, save, version, unused))
        return 1;
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Fields(stream, save, weather, kHead, 3) || Forecast_Serialize(weather + 0x20, nullptr, stream, save, version, unused))
        return 1;
    return Save_Fields(stream, save, weather, kTail, 12) ? 0 : 1;
}
}
