#include "vanillatime.h"

#include <cstdint>

#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall VanillaTime_Save(u8 *, void *, Stream *stream) {
    const u32 now = Time_GetMs();
    s32 result;
    return *Stream_Write(&result, stream, &now, 4, nullptr) >= 0;
}

u32 __fastcall VanillaTime_Load(u8 *time, void *, Stream *stream) {
    u32 saved = (u32)(uintptr_t)stream;   // (sic: read into the stream argument's own slot, so a short read leaves its bytes)
    s32 result;
    if (*Stream_Read(&result, stream, &saved, 4, nullptr) < 0) return 0;
    at<u32>(time, 0) = saved - Time_GetMs();
    return 1;
}
}  // extern "C"
