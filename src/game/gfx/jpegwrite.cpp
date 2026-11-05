// The game's JPEG writer: see jpegwrite.h.
#include "jpegwrite.h"

#include <cstring>

#include "../core/addrcalls.h"

namespace {

AddrCalls g_calls;

constexpr u32 kAlloc = 0x005d4ba0, kFree = 0x005d4c20, kFileWrite = 0x005d5350;   // (stdcall: Th with no object)
constexpr u32 kStdError = 0x0064dd60, kCreate = 0x0064c5f0, kSetDefaults = 0x0064cf20, kSetQuality = 0x0064cef8, kStart = 0x0064ca90, kWriteMarker = 0x0064c982, kWriteScanlines = 0x0064cb3c,
              kFinish = 0x0064c7fd, kDestroy = 0x0064c740;

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> const T &at(const u8 *base, u32 offset) { return *reinterpret_cast<const T *>(base + offset); }

u32 word(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

u8 *destOf(u8 *cinfo) { return at<u8 *>(cinfo, 0x14); }

u32 allocSmall(u8 *cinfo, u32 pool, u32 size) {
    return reinterpret_cast<u32(__cdecl *)(u8 *, u32, u32)>(at<void *>(at<u8 *>(cinfo, 4), 0))(cinfo, pool, size);
}

// ERREXIT(cinfo, 0x24): err->msg_code, then err->error_exit.
void writeFailed(u8 *cinfo) {
    u8 *const err = at<u8 *>(cinfo, 0);
    at<u32>(err, 0x14) = 0x24;
    reinterpret_cast<void(__cdecl *)(u8 *)>(at<void *>(err, 0))(cinfo);
}

}  // namespace

void JpegWrite_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __stdcall JpegWrite_Open(u8 **out, u32 file, u32, u32) {
    u8 *const block = reinterpret_cast<u8 *>((uintptr_t)g_calls.Th(kAlloc, 0u, 0x18u));
    if (!block) return 0;
    std::memset(block, 0, 0x18);
    at<u32>(block, 0x10) = file;
    at<u32>(block, 0xc) = 0;
    *out = block;
    return 1;
}

void __cdecl JpegWrite_ConvertRow(const u8 *from, u8 *to, s32 count, s32 mode, const u8 *info) {
    const bool direct = at<u32>(info, 0x18) == 0x20;
    switch (mode) {
    case 2:
        for (s32 i = 0; i < count; ++i, to += 3) {
            if (direct) {
                to[0] = from[2];
                to[1] = from[1];
                to[2] = from[0];
                from += 4;
            } else {
                to[0] = info[*from * 4u + 0x2a];
                to[1] = info[*from * 4u + 0x29];
                to[2] = info[*from * 4u + 0x28];
                from += 1;
            }
        }
        break;
    case 1:
        for (s32 i = 0; i < count; ++i, to += 1) {
            if (direct) {
                to[0] = from[1];
                from += 4;
            } else {
                to[0] = info[*from * 4u + 0x29];
                from += 1;
            }
        }
        break;
    case 0:
        for (s32 i = 0; i < count; ++i, to += 4) {
            if (direct) {
                to[0] = from[3];
                to[1] = from[2];
                to[2] = from[1];
                to[3] = from[0];
                from += 4;
            } else {
                to[0] = info[*from * 4u + 0x2b];
                to[1] = info[*from * 4u + 0x2a];
                to[2] = info[*from * 4u + 0x29];
                to[3] = info[*from * 4u + 0x28];
                from += 1;
            }
        }
        break;
    default:
        break;
    }
}

u32 __cdecl JpegWrite_IsGrey(const u8 *entries, s32 count) {
    for (s32 i = 0; i < count; ++i)
        if (entries[i * 4 + 2] != entries[i * 4 + 1] || entries[i * 4 + 2] != entries[i * 4]) return 0;
    return 1;
}

u32 __stdcall JpegWrite_Image(const u8 *handle, const u8 *info, const u8 *pixels, s32 pitch) {
    const u32 quality = at<u32>(info, 0x430);
    const s32 width = at<s32>(info, 0x10);
    const u32 height = at<u32>(info, 0x14);
    u32 result = 0;
    s32 components = 3, colourSpace = 2;
    const bool withMarker = at<u32>(info, 0x428) & 1;
    if (withMarker) {
        components = 4;
        colourSpace = 0;
    } else if (at<u32>(info, 0x18) == 8) {
        if (JpegWrite_IsGrey(info + 0x28, at<s32>(info, 0x24))) {
            components = 1;
            colourSpace = 1;
        }
    } else {
        bool grey = true;
        for (s32 y = 0; y < at<s32>(info, 0x14); ++y)
            if (!JpegWrite_IsGrey(pixels + y * pitch, width)) {
                grey = false;
                break;
            }
        if (grey) {
            components = 1;
            colourSpace = 1;
        }
    }
    alignas(4) u8 errorManager[0xa0];   // (as the original's frame: 0x208 below EBP)
    alignas(4) u8 cinfo[0x168];
    at<u32>(cinfo, 0) = g_calls.Cd(kStdError, errorManager);
    g_calls.Cd(kCreate, cinfo, 0x3du, 0x168u);
    JpegWrite_SetDest(cinfo, at<u32>(handle, 0x10));
    at<s32>(cinfo, 0x18) = width;
    at<u32>(cinfo, 0x1c) = height;
    at<s32>(cinfo, 0x20) = components;
    at<s32>(cinfo, 0x24) = colourSpace;
    g_calls.Cd(kSetDefaults, cinfo);
    g_calls.Cd(kSetQuality, cinfo, quality, 1u);
    g_calls.Cd(kStart, cinfo, 1u);
    if (at<u32>(info, 0x428) & 1) g_calls.Cd(kWriteMarker, cinfo, 0xedu, 0x0077b2d4u, 9u);
    u8 *const row = reinterpret_cast<u8 *>((uintptr_t)g_calls.Th(kAlloc, 0u, static_cast<u32>(width * components)));
    if (row) {
        for (s32 y = 0; y < at<s32>(cinfo, 0x1c); ++y) {
            JpegWrite_ConvertRow(pixels + y * pitch, row, width, colourSpace, info);
            u8 *rowPointer = row;
            g_calls.Cd(kWriteScanlines, cinfo, &rowPointer, 1u);
        }
        g_calls.Th(kFree, 0u, word(row));
        result = 1;
    }
    g_calls.Cd(kFinish, cinfo);
    g_calls.Cd(kDestroy, cinfo);
    return result;
}

u32 __stdcall JpegWrite_Close(u8 *handle) {
    if (!handle) return 0;
    return g_calls.Th(kFree, 0u, word(handle));
}

void __cdecl JpegWrite_SetDest(u8 *cinfo, u32 file) {
    if (!destOf(cinfo)) at<u32>(cinfo, 0x14) = allocSmall(cinfo, 0, 0x1c);
    u8 *const dest = destOf(cinfo);
    at<u32>(dest, 8) = 0x0064dc5a;
    at<u32>(dest, 0xc) = 0x0064dc9c;
    at<u32>(dest, 0x10) = 0x0064dd00;
    at<u32>(dest, 0x14) = file;
}

void __cdecl JpegDest_Init(u8 *cinfo) {
    u8 *const dest = destOf(cinfo);
    at<u32>(dest, 0x18) = allocSmall(cinfo, 1, 0x1000);
    at<u32>(dest, 0) = at<u32>(dest, 0x18);
    at<u32>(dest, 4) = 0x1000;
}

u32 __cdecl JpegDest_Empty(u8 *cinfo) {
    u8 *const dest = destOf(cinfo);
    if (g_calls.Th(kFileWrite, 0u, at<u32>(dest, 0x14), at<u32>(dest, 0x18), 0x1000u) != 0x1000) writeFailed(cinfo);
    at<u32>(dest, 0) = at<u32>(dest, 0x18);
    at<u32>(dest, 4) = 0x1000;
    return 1;
}

void __cdecl JpegDest_Term(u8 *cinfo) {
    u8 *const dest = destOf(cinfo);
    const u32 left = 0x1000 - at<u32>(dest, 4);
    if (left != 0 && g_calls.Th(kFileWrite, 0u, at<u32>(dest, 0x14), at<u32>(dest, 0x18), left) != left) writeFailed(cinfo);
}
}
