#include "globalsave.h"

#include <cstdint>

#include "savegame.h"
#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// One field written or read whole (Stream_Write / Stream_Read; a negative result or fewer bytes: false).
bool Field(Stream *stream, u8 *at, s32 count, bool save) {
    s32 result, done = 0;
    const s32 got = *(save ? Stream_Write(&result, stream, at, count, &done) : Stream_Read(&result, stream, at, count, &done));
    return got >= 0 && done == count;   // sic: the result is 0 or 1, never negative
}

// The current theme's sign name at +at given (the CRT's wcsncpy, 0x67b0d0, of 0x21 units: up to and with the NUL, then zeros) and
// mNameChanged set; nothing without a current theme.
void SetSignName(u8 *info, u32 at, const u16 *name) {
    u8 *const theme = ::at<u8 *>(info, 0x48);
    if (!theme)
        return;
    u16 *to = reinterpret_cast<u16 *>(theme + at);
    u32 count = 0x21;
    while (count && (*to++ = *name++) != 0)
        --count;
    if (count)
        while (--count)
            *to++ = 0;
    theme[0xb8] = 1;
}

}  // namespace

extern "C" {

u32 __fastcall ThemeSaveRecord_Serialize(u8 *record, void *, Stream *stream, u32 save, u32) {
    for (u32 i = 0; i < 6; ++i)
        if (!Field(stream, record + i, 1, save != 0))
            return 1;   // (Debug_Printf "File write failed" / "File read failed")
    for (u32 k = 0; k < 4; ++k) {
        if (!Field(stream, record + 6 + k, 1, save != 0))
            return 1;
        if (!Field(stream, record + 0xc + 4 * k, 4, save != 0))
            return 1;
    }
    if (save) {
        for (u32 k = 0; k < 0x21; ++k) {
            if (!Field(stream, record + 0x34 + 2 * k, 2, true))
                return 1;
            if (!Field(stream, record + 0x76 + 2 * k, 2, true))
                return 1;
        }
        if (!Field(stream, record + 0xb8, 1, true) || !Field(stream, record + 0xb9, 1, true))
            return 1;
        return 0;
    }
    for (u32 k = 0; k < 0x21; ++k) {
        if (Save_ReadU16(stream, record + 0x34 + 2 * k, "mSignNameA[i]", 1))
            return 1;
        if (Save_ReadU16(stream, record + 0x76 + 2 * k, "mSignNameB[i]", 1))
            return 1;
    }
    if (Save_ReadU8(stream, record + 0xb8, "mNameChanged", 1))
        return 1;
    if (Save_ReadU8(stream, record + 0xb9, "mAllResearchCompleted", 1))
        return 1;
    return 0;
}

u8 __fastcall GlobalSave_ThemeByte(const u8 *info, void *, u32 i) { return at<const u8 *>(info, 0x48)[i]; }

u8 __fastcall GlobalSave_Byte26(const u8 *info, void *, u32 i) { return info[0x26 + i]; }

void __fastcall GlobalSave_Inc20(u8 *info) { ++at<u32>(info, 0x20); }

u8 __fastcall GlobalSave_Byte24(const u8 *info) { return info[0x24]; }

void __fastcall GlobalSave_SetByte24(u8 *info, void *, u32 value) { info[0x24] = (u8)value; }

u8 __fastcall GlobalSave_Byte25(const u8 *info) { return info[0x25]; }

void __fastcall GlobalSave_SetByte25(u8 *info, void *, u32 value) { info[0x25] = (u8)value; }

void __fastcall GlobalSave_SetSignNameA(u8 *info, void *, const u16 *name) { SetSignName(info, 0x34, name); }

void __fastcall GlobalSave_SetSignNameB(u8 *info, void *, const u16 *name) { SetSignName(info, 0x76, name); }

void __fastcall GlobalSave_SetAllResearchCompleted(u8 *info) {
    u8 *const theme = at<u8 *>(info, 0x48);
    if (theme) theme[0xb9] = 1;
}

u8 __fastcall GlobalSave_Byte4c(const u8 *info) { return info[0x4c]; }

void __fastcall GlobalSave_ClearByte4c(u8 *info) { info[0x4c] = 0; }
}
