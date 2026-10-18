#include "textpool.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *&LongBlock() { return global<u8 *>(0x00faa644); }
s32 &LongLength() { return global<s32>(0x00faa648); }
s32 &LongRows() { return global<s32>(0x00faa64c); }
u8 *&ShortBlock() { return global<u8 *>(0x00faa63c); }
s32 &ShortLength() { return global<s32>(0x00faa640); }
s32 &ShortRows() { return global<s32>(0x00faa650); }

// the first (or with `last` the last) free row among `rows` rows of `length` characters from `row`, or 0
u8 *FreeRow(u8 *row, s32 rows, s32 length, bool last) {
    u8 *found = nullptr;
    for (s32 i = 0; i < rows && (last || !found); ++i, row += length * 2)
        if (*reinterpret_cast<s32 *>(row) == -1) found = row;
    return found;
}
}

extern "C" {

u32 __cdecl TextPool_Init(s32 longLength, s32 longRows, s32 shortLength, s32 shortRows) {
    if (!longLength || !longRows || !shortLength || !shortRows || shortLength <= 4 || longLength <= shortLength) return 0;
    LongLength() = longLength;
    ShortLength() = shortLength;
    LongRows() = longRows;
    ShortRows() = shortRows;
    LongBlock() = static_cast<u8 *>(GameHeap_Alloc((u32)(LongRows() * LongLength()) << 1));
    ShortBlock() = static_cast<u8 *>(GameHeap_Alloc((u32)(ShortRows() * ShortLength()) << 1));
    if (!LongBlock() || !ShortBlock()) return 0;
    u8 *row = LongBlock();
    for (s32 i = 0; i < LongRows(); ++i, row += LongLength() * 2) *reinterpret_cast<u32 *>(row) = 0xffffffff;
    row = ShortBlock();
    for (s32 i = 0; i < LongRows(); ++i, row += ShortLength() * 2) *reinterpret_cast<u32 *>(row) = 0xffffffff;  // sic: the long count
    return 1;
}

void __cdecl TextPool_Free() {
    GameHeap_Free(LongBlock());
    GameHeap_Free(ShortBlock());
}

u16 *__cdecl TextPool_Take(s32 length) {
    if (!LongBlock() || !ShortBlock()) return nullptr;
    u8 *found = nullptr;
    if (length < ShortLength()) found = FreeRow(ShortBlock(), ShortRows(), ShortLength(), false);
    if (length < LongLength() && !found) found = FreeRow(LongBlock(), LongRows(), LongLength(), true);  // (the last)
    if (found) *reinterpret_cast<u16 *>(found) = 0;
    return reinterpret_cast<u16 *>(found);
}

void __cdecl TextPool_Release(u8 *row) {
    if (row) *reinterpret_cast<u32 *>(row) = 0xffffffff;
}
}
