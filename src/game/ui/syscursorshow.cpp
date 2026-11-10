// The system cursor's calls to Windows: see syscursorshow.h.
#include "syscursorshow.h"

#include <cstdint>
#include "../core/imports.h"

namespace {
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }
void setCursor(u32 handle) { (*reinterpret_cast<u32(__stdcall *const *)(u32)>((uintptr_t)IMPORT(User32_SetCursor)))(handle); }
void showCursor(u32 show) { (*reinterpret_cast<u32(__stdcall *const *)(u32)>((uintptr_t)IMPORT(User32_ShowCursor)))(show); }
}  // namespace

extern "C" {

void __fastcall SysCursor_SetHandle(u8 *self, void *, u32 handle, u32 byte) {
    at<u32>(self, 0x14) = handle;
    setCursor(handle);
    at<u8>(self, 7) = static_cast<u8>(byte);
}

void __fastcall SysCursor_Show(u8 *self) {
    showCursor(1);
    at<u8>(self, 4) = 1;
}

void __fastcall SysCursor_Hide(u8 *self) {
    showCursor(0);
    at<u8>(self, 4) = 0;
}

void __fastcall SysCursor_Apply(u8 *self, void *, u32 on) { setCursor(static_cast<u16>(on) ? at<u32>(self, 0x14) : 0); }
}
