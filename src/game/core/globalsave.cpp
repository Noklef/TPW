#include "globalsave.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 __fastcall GlobalSave_ThemeByte(const u8 *info, void *, u32 i) { return at<const u8 *>(info, 0x48)[i]; }

u8 __fastcall GlobalSave_Byte26(const u8 *info, void *, u32 i) { return info[0x26 + i]; }

void __fastcall GlobalSave_Inc20(u8 *info) { ++at<u32>(info, 0x20); }

u8 __fastcall GlobalSave_Byte24(const u8 *info) { return info[0x24]; }

void __fastcall GlobalSave_SetByte24(u8 *info, void *, u32 value) { info[0x24] = (u8)value; }

u8 __fastcall GlobalSave_Byte25(const u8 *info) { return info[0x25]; }

void __fastcall GlobalSave_SetByte25(u8 *info, void *, u32 value) { info[0x25] = (u8)value; }

void __fastcall GlobalSave_SetAllResearchCompleted(u8 *info) {
    u8 *const theme = at<u8 *>(info, 0x48);
    if (theme) theme[0xb9] = 1;
}

u8 __fastcall GlobalSave_Byte4c(const u8 *info) { return info[0x4c]; }

void __fastcall GlobalSave_ClearByte4c(u8 *info) { info[0x4c] = 0; }
}
