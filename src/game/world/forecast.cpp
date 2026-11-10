#include "forecast.h"

#include "../core/savegame.h"
#include "advisorscore.h"
#include "forecast.h"

extern "C" {

u32 __fastcall Forecast_Serialize(u8 *forecast, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, forecast, 8, (u32)&gText_Forecast_Serialize_762418) || !Save_Field(stream, save, forecast + 8, 4, 0x0076239c) ||
        !Save_Field(stream, save, forecast + 0xc, 1, (u32)&gText_Advisorscore_75e6b8))
        return 1;
    return 0;
}
}
