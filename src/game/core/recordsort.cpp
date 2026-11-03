// The records' quicksort: see recordsort.h.
#include "recordsort.h"

#include <cstdint>

namespace {

u32 key(const RecordSort *sort, const u8 *record) { return *reinterpret_cast<const u32 *>((uintptr_t)record + sort->keyOffset); }

// rep movsd, then rep movsb.
void copy(u8 *to, const u8 *from, u32 size) {
    for (u32 i = 0; i < size >> 2; ++i, to += 4, from += 4) *reinterpret_cast<u32 *>(to) = *reinterpret_cast<const u32 *>(from);
    for (u32 i = 0; i < (size & 3); ++i) *to++ = *from++;
}

}  // namespace

extern "C" {

void __fastcall RecordSort_Range(RecordSort *sort, void *, u8 *base, s32 first, s32 last) {
    for (;;) {
        s32 left = first, right = last;
        u8 *low = base + (u32)left * sort->stride, *high = base + (u32)right * sort->stride;
        const u32 pivot = key(sort, base + (u32)((right + left) >> 1) * sort->stride);
        do {
            while (left <= last && key(sort, low) > pivot) {
                ++left;
                low += sort->stride;
            }
            while (right > first && key(sort, high) < pivot) {
                --right;
                high -= sort->stride;
            }
            if (left > right) break;
            copy(sort->temp, low, sort->stride);
            copy(low, high, sort->stride);
            copy(high, sort->temp, sort->stride);
            ++left;
            --right;
            low += sort->stride;
            high -= sort->stride;
        } while (left <= right);
        if (first < right) RecordSort_Range(sort, nullptr, base, first, right);
        if (last <= left) return;
        first = left;
    }
}

void __fastcall RecordSort_Run(RecordSort *sort, void *, u8 *base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last) {
    sort->stride = stride;
    sort->keyOffset = keyOffset;
    sort->temp = temp;
    RecordSort_Range(sort, nullptr, base, first, last);
}

}  // extern "C"
