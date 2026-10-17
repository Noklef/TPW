#include "timesig.h"

#include "savegame.h"

extern "C" {

u32 __fastcall TimeSig_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1) return 0;
    const char *name = reinterpret_cast<const char *>(0x0074c76c);
    s32 result, done;
    if (save) {
        Save_FieldName(4, name, 0);
        done = 0;
        if (*Stream_Write(&result, stream, self, 4, &done) >= 0 && done == 4) return 0;
    } else {
        Save_FieldName(4, name, 1);
        done = 0;
        if (*Stream_Read(&result, stream, self, 4, &done) >= 0 && done == 4) return 0;
    }
    return 1;
}
}
