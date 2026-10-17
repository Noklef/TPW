#include "forecast.h"

#include "../core/savegame.h"

extern "C" {

u32 __fastcall Forecast_Serialize(u8 *forecast, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, forecast, 8, 0x00762418) || !Save_Field(stream, save, forecast + 8, 4, 0x0076239c) ||
        !Save_Field(stream, save, forecast + 0xc, 1, 0x0075e6b8))
        return 1;
    return 0;
}
}
