#include "charsets.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/refcounted.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The stream's bytes left: its size (slot 5) less its position (slot 6), asked in that order.
u32 BytesLeft(u8 *stream) {
    const u32 size = vcall<u32>(stream, 0x14);
    return size - vcall<u32>(stream, 0x18);
}

}  // namespace

u8 *__fastcall RangeTable_Ctor(u8 *table, void *, u8 *data) {
    Class70a078_Ctor(table);
    at<u8 *>(table, 8) = data;
    at<u32>(table, 0) = 0x70a0e0;
    return table;
}

void __fastcall RangeTable_Dtor(u8 *table) { at<u32>(table, 0) = 0x70a078; }

u8 *__fastcall RangeTable_DeletingDtor(u8 *table, void *, u32 flags) {
    RangeTable_Dtor(table);
    if (flags & 1) GameHeap_Free(table);
    return table;
}

s32 *__fastcall RangeTable_Load(u8 *, void *, s32 *result, u8 *stream, u8 **object, u32, u8 *) {
    u32 magic = (u32)(uintptr_t)stream;   // (the stream argument's own slot, sic)
    if (vcall<s32>(stream, 8, &magic, 4u) == 4 && magic == 0x4d554642) {   // "BFUM"
        const u32 size = BytesLeft(stream);
        if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size + 0xc))) {
            if ((u32)vcall<s32>(stream, 8, block + 0xc, size) == size) {
                RangeTable_Ctor(block, nullptr, block + 0xc);
                *object = block;
                *result = 0;
                return result;
            }
            GameHeap_Free(block);
        }
    }
    *result = -1;
    return result;
}

u8 *__fastcall CodeTable_Ctor(u8 *table, void *, u32 lead, u32 count, u8 *data) {
    Class70a078_Ctor(table);
    at<u8>(table, 8) = (u8)lead;
    at<u16>(table, 0xa) = (u16)count;
    at<u32>(table, 0) = 0x70a0f8;
    at<u8 *>(table, 0xc) = data;
    return table;
}

void __fastcall CodeTable_Dtor(u8 *table) { at<u32>(table, 0) = 0x70a078; }

u8 *__fastcall CodeTable_DeletingDtor(u8 *table, void *, u32 flags) {
    CodeTable_Dtor(table);
    if (flags & 1) GameHeap_Free(table);
    return table;
}

s32 *__fastcall CodeTable_Load(u8 *, void *, s32 *result, u8 *stream, u8 **object, u32, u8 *) {
    u8 header[8];
    if (vcall<s32>(stream, 8, header, 8u) == 8) {
        const u32 magic = at<u32>(header, 0);
        if (magic == 0x554d4642 || magic == 0x484d4642) {   // "BFMU", "BFMH"
            const u8 lead = (u8)(0xff - header[4]);
            const u16 count = (u16)(0xffu * ((u8)(header[6] - 1) + 0x101u) - 0xfeu * lead + header[7]);
            const u32 size = BytesLeft(stream);
            if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size + 0x10))) {
                if ((u32)vcall<s32>(stream, 8, block + 0x10, size) == size) {
                    CodeTable_Ctor(block, nullptr, lead, count, block + 0x10);
                    *object = block;
                    *result = magic == 0x484d4642 ? -1 : 0;
                    return result;
                }
                GameHeap_Free(block);
            }
        }
    }
    *result = -1;
    return result;
}
