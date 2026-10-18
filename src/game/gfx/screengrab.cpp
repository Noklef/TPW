#include "screengrab.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../render/engineerrors.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall ScreenGrab_ToRgb24(u8 *grab, void *) {
    const s32 depth = global<s32>(0x008bcbe4);
    if (depth == 0x10) {
        const u32 out = TrackedGlobal_Alloc(at<u32>(grab, 8) * 3 + 1);
        at<u32>(grab, 4) = out;
        if (!out) {
            TrackedGlobal_Free(at<u32>(grab, 0));
            at<u32>(grab, 0) = 0;
            EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
            return 0;
        }
        const u32 maskA = global<u32>(0x008bcc54), maskB = global<u32>(0x008bcc6c), maskC = global<u32>(0x008bcc60);
        const u32 shiftA = (0x18 - global<u32>(0x008bcc58) - global<u32>(0x008bcc5c)) & 31;
        const u32 shiftC = (0x10 - global<u32>(0x008bcc64) - global<u32>(0x008bcc68)) & 31;
        const u32 shiftB = (8 - global<u32>(0x008bcc70) - global<u32>(0x008bcc74)) & 31;
        const u16 *pixels = at<const u16 *>(grab, 0);
        for (s32 i = 0; i < at<s32>(grab, 8); ++i) {
            const u32 p = pixels[i];
            *reinterpret_cast<u32 *>((uintptr_t)at<u32>(grab, 4) + 3 * i) = (p & maskB) << shiftB | (p & maskA) << shiftA | (p & maskC) << shiftC;
        }
        TrackedGlobal_Free(at<u32>(grab, 0));
        at<u32>(grab, 0) = 0;
        return 1;
    }
    if (depth == 0x20) {
        for (s32 i = 0; i < at<s32>(grab, 8); ++i) {
            at<u8 *>(grab, 0)[3 * i] = at<u8 *>(grab, 0)[4 * i];
            at<u8 *>(grab, 0)[3 * i + 1] = at<u8 *>(grab, 0)[4 * i + 1];
            at<u8 *>(grab, 0)[3 * i + 2] = at<u8 *>(grab, 0)[4 * i + 2];
        }
    } else if (depth != 0x18) {
        return 1;
    }
    const u32 pixels = at<u32>(grab, 0);
    at<u32>(grab, 0) = 0;
    at<u32>(grab, 4) = pixels;
    return 1;
}

u32 __fastcall ScreenGrab_SetName(u8 *grab, void *, const char *name) {
    if (name) {
        u32 size = 0;  // repne scasb, then rep movsd / movsb of the name and its 0
        while (name[size++] != 0) {
        }
        u8 *const out = grab + 0xc;
        for (u32 i = 0; i < size; i++) out[i] = (u8)name[i];
        at<u8 *>(grab, 0x8c) = out;
    }
    return 1;
}
}
