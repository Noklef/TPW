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

void __cdecl Advisor_Save(Stream *stream) {
    s32 result;
    u32 value = 0x5f564441;
    Stream_Write(&result, stream, &value, 4, nullptr);
    value = 0x168;
    Stream_Write(&result, stream, &value, 4, nullptr);
    Stream_Write(&result, stream, reinterpret_cast<const void *>(0x00f79660), (s32)value, nullptr);
    const u32 pointer = *reinterpret_cast<const u32 *>(0x00f797a0);
    if (pointer) {
        Stream_Write(&result, stream, reinterpret_cast<const void *>(0x00f79794), 4, nullptr);
        Stream_Write(&result, stream, reinterpret_cast<const void *>(pointer), (s32) * reinterpret_cast<const u32 *>(0x00f79794), nullptr);
    } else {
        value = 0;
        Stream_Write(&result, stream, &value, 4, nullptr);
    }
}

u32 __fastcall AdvisorScoring_Load(u8 *scoring, void *, const u32 *chunk) {
    s32 result;
    return *Stream_Read(&result, reinterpret_cast<Stream *>(chunk[0]), scoring, 4, nullptr) >= 0 ? 1 : 0;
}

u32 __fastcall AdvisorScoring_Save(u8 *scoring, void *, const u32 *chunk) {
    s32 result;
    return *Stream_Write(&result, reinterpret_cast<Stream *>(chunk[0]), scoring, 4, nullptr) >= 0 ? 1 : 0;
}

u32 __fastcall CheatSystem_Save(u8 *cheats, void *, const u32 *chunk) {
    Stream *stream = reinterpret_cast<Stream *>(chunk[0]);
    s32 result;
    if (*Stream_Write(&result, stream, cheats + 0xa, 1, nullptr) < 0)
        return 0;
    return *Stream_Write(&result, stream, cheats + 0xb, 1, nullptr) >= 0 ? 1 : 0;
}
}
