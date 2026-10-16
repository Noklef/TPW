#include "cheatmodule.h"

#include <cstdint>

#include "../files/file.h"

extern "C" {

u32 __fastcall CheatSystem_Load(u8 *cheats, void *, const u32 *chunk) {
    Stream *stream = reinterpret_cast<Stream *>(chunk[0]);
    s32 result;
    if (*Stream_Read(&result, stream, cheats + 0xa, 1, nullptr) < 0)
        return 0;
    return *Stream_Read(&result, stream, cheats + 0xb, 1, nullptr) >= 0 ? 1 : 0;
}

u32 __fastcall CheatSystem_Save(u8 *cheats, void *, const u32 *chunk) {
    Stream *stream = reinterpret_cast<Stream *>(chunk[0]);
    s32 result;
    if (*Stream_Write(&result, stream, cheats + 0xa, 1, nullptr) < 0)
        return 0;
    return *Stream_Write(&result, stream, cheats + 0xb, 1, nullptr) >= 0 ? 1 : 0;
}
}
