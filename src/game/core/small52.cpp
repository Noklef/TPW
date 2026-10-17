#include "small52.h"

#include <cstdint>

#include "gameheap.h"
#include "small10.h"
#include "small5.h"
#include "small7.h"
#include "vcall.h"
#include "vwrap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall ArrayHolder_Clear(u8 *object) {
    Array3D_Free(object + 0x2c);
    Array3D_Free(object + 0x3c);
    Array3D_Free(object + 0x4c);
    Array3D_Free(object + 0x5c);
    for (u32 offset = 0x5c; offset >= 0x2c; offset -= 0x10) {
        u8 *const array = object + offset;
        if (at<void *>(array, 0xc)) GameHeap_Free(at<void *>(array, 0xc));
        at<u32>(array, 0) = 0;
        at<u32>(array, 4) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 0xc) = 0;
    }
}

u8 Image_CropInto(u8 *destination, u8 *source, s32 x, s32 y) {
    const s32 height = at<s32>(source, 4), width = at<s32>(source, 0);
    const s32 lastRow = height - 1, lastColumn = width - 1;
    auto start = [](s32 offset, s32 last) { return -offset <= last ? (-offset < 0 ? 0 : -offset) : last; };
    auto end = [](s32 size, s32 offset, s32 last) { const s32 e = size - offset - 1; return e > last ? last : (e < 0 ? 0 : e); };
    const s32 firstRow = start(y, lastRow), endRow = end(height, y, lastRow);
    const s32 firstColumn = start(x, lastColumn), endColumn = end(width, x, lastColumn);
    Array3D_Realloc(destination, nullptr, (u32)width, (u32)height, 1);
    const s32 stride = at<s32>(destination, 0) * at<s32>(destination, 8);
    u8 *row = at<u8 *>(destination, 0xc) + stride * (y < 0 ? 0 : y) + (x < 0 ? 0 : x);
    for (s32 line = firstRow; line <= endRow; ++line) {
        const u8 *from = at<u8 *>(source, 0xc) + (at<s32>(source, 0) * line + firstColumn) * at<s32>(source, 8);
        u8 *to = row;
        if (firstColumn <= endColumn) {
            for (s32 n = endColumn - firstColumn + 1; n != 0; --n) {
                *to++ = *from;
                from += at<s32>(source, 8);
            }
        }
        row += stride;
    }
    return 1;
}

void __fastcall TileSet_Clear(u8 *object) {
    at<u8>(object, 4) = 0;
    at<u32>(object, 0) = 0;
    for (u32 offset = 8; offset <= 0xc8; offset += 0x10)
        Array3D_Free(object + offset);
    for (u32 offset = 0xc8; offset >= 0x28; offset -= 0x10) {
        u8 *const array = object + offset;
        if (at<void *>(array, 0xc)) GameHeap_Free(at<void *>(array, 0xc));
        at<u32>(array, 0) = 0;
        at<u32>(array, 4) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 0xc) = 0;
    }
    BufferC_FreeAndClear(object + 0x18);
    BufferC_FreeAndClear(object + 8);
}

namespace {
// Four arrays from base + 0x2c in steps of 0x10: Array3D_Free on each, the block of the last freed and cleared, then BufferC_FreeAndClear on the first three from the third back.
void ClearGroup(u8 *base) {
    Array3D_Free(base + 0x2c);
    Array3D_Free(base + 0x3c);
    Array3D_Free(base + 0x4c);
    Array3D_Free(base + 0x5c);
    u8 *const last = base + 0x5c;
    if (at<void *>(last, 0xc)) GameHeap_Free(at<void *>(last, 0xc));
    at<u32>(last, 0) = 0;
    at<u32>(last, 4) = 0;
    at<u32>(last, 8) = 0;
    at<u32>(last, 0xc) = 0;
    BufferC_FreeAndClear(base + 0x4c);
    BufferC_FreeAndClear(base + 0x3c);
    BufferC_FreeAndClear(base + 0x2c);
}
}  // namespace

void __fastcall Renderer_Teardown(u8 *object) {
    vcall<u32>(at<u8 *>(object, 0x14), 0x18);
    ClearGroup(object + 0x3c4);
    ClearGroup(object + 0x358);
    static const u32 offsets[2] = {0x28u, 0x18u};
    for (u32 offset : offsets) {
        u8 *const block = object + offset;
        if (at<void *>(block, 0xc)) GameHeap_Free(at<void *>(block, 0xc));
        at<u32>(block, 0) = 0;
        at<u32>(block, 4) = 0;
        at<u32>(block, 8) = 0;
        at<u32>(block, 0xc) = 0;
    }
}

u8 __fastcall Eleven_Read(u8 *object, void *, u8 *stream) {
    for (u32 offset = 0; offset < 0x2c; offset += 4)
        Object_CallSlot8Product((u32)(uintptr_t)(object + offset), 4, 1, stream);
    return 1;
}

}  // extern "C"
