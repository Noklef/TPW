#include "small17.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __cdecl Names_FirstAfter(const u8 *a, const u8 *b) {
    return Crt().stricmp(at<const char *>(a, 8), at<const char *>(b, 8)) > 0 ? 1 : 0;
}

u32 __cdecl Names_Equal(const u8 *a, const u8 *b) {
    return Crt().stricmp(at<const char *>(a, 8), at<const char *>(b, 8)) == 0 ? 1 : 0;
}

u32 __cdecl Model_FindMouthPart(const u8 *model, u32, s32 part) {
    const u8 *data = at<const u8 *>(at<const u8 *>(model, 8), 4);
    const u32 count = at<u16>(data, 0x42);
    if (count == 0)
        return 0;
    const char *wanted;
    switch (part) {
    case 1: wanted = reinterpret_cast<const char *>(0x0074cf08); break;
    case 2: wanted = reinterpret_cast<const char *>(0x0074cf18); break;
    case 3: wanted = reinterpret_cast<const char *>(0x0074cf24); break;
    case 4: wanted = reinterpret_cast<const char *>(0x0074cefc); break;
    case 5: wanted = reinterpret_cast<const char *>(0x0074cee4); break;
    case -1: wanted = reinterpret_cast<const char *>(0x0074cef0); break;
    default: return 0;
    }
    for (u32 i = 0; i < count; ++i) {
        const char *name = at<const char *>(at<const u8 *>(data, 0x70), 0xa0 * i + 0x54);
        if (Crt().stricmp(name, wanted) == 0)
            return i;
    }
    return 0;
}

}  // extern "C"
