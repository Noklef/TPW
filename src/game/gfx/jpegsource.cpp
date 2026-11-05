// The game's libjpeg data source: see jpegsource.h.
#include "jpegsource.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

constexpr u32 kInit = 0x006311b0, kFill = 0x006311c0, kSkip = 0x00631230, kResync = 0x0063b8f0, kTerm = 0x00631270;   // (the methods by the game's addresses; jdmarker.c's resync_to_restart, the shared bare RET)

// The memory manager's alloc_small (cinfo->mem, slot 0).
u8 *allocSmall(u8 *cinfo, u32 size) { return reinterpret_cast<u8 *(__cdecl *)(u8 *, u32, u32)>(at<void *>(at<u8 *>(cinfo, 4), 0))(cinfo, 0, size); }

void setCode(u8 *cinfo, u32 code) { at<u32>(at<u8 *>(cinfo, 0), 0x14) = code; }

}  // namespace

extern "C" {

void __cdecl JpegSource_Set(u8 *cinfo, void *stream) {
    if (!at<u8 *>(cinfo, 0x18)) {
        u8 *const source = allocSmall(cinfo, 0x28);
        at<u8 *>(cinfo, 0x18) = source;
        at<u8 *>(source, 0x20) = allocSmall(cinfo, 0x1000);
    }
    u8 *const source = at<u8 *>(cinfo, 0x18);
    at<u32>(source, 8) = kInit;
    at<u32>(source, 0xc) = kFill;
    at<u32>(source, 0x10) = kSkip;
    at<u32>(source, 0x14) = kResync;
    at<u32>(source, 0x18) = kTerm;
    at<void *>(source, 0x1c) = stream;
    at<u32>(source, 4) = 0;
    at<u32>(source, 0) = 0;
}

void __cdecl JpegSource_Init(u8 *cinfo) { at<u8>(at<u8 *>(cinfo, 0x18), 0x24) = 1; }

u8 __cdecl JpegSource_Fill(u8 *cinfo) {
    u8 *const source = at<u8 *>(cinfo, 0x18);
    u32 count = vcall<u32>(at<u8 *>(source, 0x1c), 8, at<u8 *>(source, 0x20), 0x1000u);
    if (!count) {
        if (at<u8>(source, 0x24)) {
            setCode(cinfo, 0x2a);
            reinterpret_cast<void(__cdecl *)(u8 *)>(at<void *>(at<u8 *>(cinfo, 0), 0))(cinfo);
        }
        setCode(cinfo, 0x78);
        reinterpret_cast<void(__cdecl *)(u8 *, s32)>(at<void *>(at<u8 *>(cinfo, 0), 4))(cinfo, -1);
        at<u8 *>(source, 0x20)[0] = 0xff;
        at<u8 *>(source, 0x20)[1] = 0xd9;
        count = 2;
    }
    at<u32>(source, 4) = count;
    at<u8 *>(source, 0) = at<u8 *>(source, 0x20);
    at<u8>(source, 0x24) = 0;
    return 1;
}

void __cdecl JpegSource_Skip(u8 *cinfo, s32 count) {
    u8 *const source = at<u8 *>(cinfo, 0x18);
    if (count <= 0) return;
    while (count > at<s32>(source, 4)) {
        count -= at<s32>(source, 4);
        JpegSource_Fill(cinfo);
    }
    at<u8 *>(source, 0) += count;
    at<u32>(source, 4) -= count;
}

}  // extern "C"
