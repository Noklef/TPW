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

// The middle's key the pivot, the two ends walking in, swapped by these when they have not crossed; the left part recursed into, the right part looped on.
template <typename Before, typename Swap, typename Recurse>
void quicksort(s32 first, s32 last, Before before, Swap swap, Recurse recurse) {
    for (;;) {
        s32 left = first, right = last;
        const auto pivot = before.pivot((right + left) >> 1);
        do {
            while (left <= last && before.left(left, pivot)) ++left;
            while (right > first && before.right(right, pivot)) --right;
            if (left > right) break;
            swap(left, right);
            ++left;
            --right;
        } while (left <= right);
        if (first < right) recurse(first, right);
        if (last <= left) return;
        first = left;
    }
}

// The pointers' swap: through the sort's +8 (sic: its temp is overwritten with the pointer).
void swapPointers(RecordSort *sort, u8 **base, s32 left, s32 right) {
    sort->temp = base[left];
    base[left] = base[right];
    base[right] = sort->temp;
}

}  // namespace

extern "C" {

void __fastcall RecordSort_RangeUp(RecordSort *sort, void *, u8 *base, s32 first, s32 last) {
    struct Before {
        RecordSort *sort;
        u8 *base;
        u32 pivot(s32 i) const { return key(sort, base + (u32)i * sort->stride); }
        bool left(s32 i, u32 p) const { return key(sort, base + (u32)i * sort->stride) < p; }
        bool right(s32 i, u32 p) const { return key(sort, base + (u32)i * sort->stride) > p; }
    };
    quicksort(first, last, Before{sort, base}, [&](s32 left, s32 right) {
        u8 *low = base + (u32)left * sort->stride, *high = base + (u32)right * sort->stride;
        copy(sort->temp, low, sort->stride);
        copy(low, high, sort->stride);
        copy(high, sort->temp, sort->stride);
    }, [&](s32 a, s32 b) { RecordSort_RangeUp(sort, nullptr, base, a, b); });
}

void __fastcall RecordSort_RunUp(RecordSort *sort, void *, u8 *base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last) {
    sort->stride = stride;
    sort->keyOffset = keyOffset;
    sort->temp = temp;
    RecordSort_RangeUp(sort, nullptr, base, first, last);
}

void __fastcall RecordSort_PointersUp(RecordSort *sort, void *, u8 **base, s32 first, s32 last) {
    struct Before {
        RecordSort *sort;
        u8 **base;
        u32 pivot(s32 i) const { return key(sort, base[i]); }
        bool left(s32 i, u32 p) const { return key(sort, base[i]) < p; }
        bool right(s32 i, u32 p) const { return key(sort, base[i]) > p; }
    };
    quicksort(first, last, Before{sort, base}, [&](s32 left, s32 right) { swapPointers(sort, base, left, right); }, [&](s32 a, s32 b) { RecordSort_PointersUp(sort, nullptr, base, a, b); });
}

void __fastcall RecordSort_RunPointersUp(RecordSort *sort, void *, u8 **base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last) {
    sort->stride = stride;
    sort->keyOffset = keyOffset;
    sort->temp = temp;
    RecordSort_PointersUp(sort, nullptr, base, first, last);
}

void __fastcall RecordSort_PointersByFloat(RecordSort *sort, void *, u8 **base, s32 first, s32 last) {
    struct Before {
        RecordSort *sort;
        u8 **base;
        float value(s32 i) const { return *reinterpret_cast<const float *>((uintptr_t)base[i] + sort->keyOffset); }
        float pivot(s32 i) const { return value(i); }
        bool left(s32 i, float p) const { return value(i) > p; }
        bool right(s32 i, float p) const { return !(value(i) >= p); }   // (unordered walks on, sic: the left stops at it)
    };
    quicksort(first, last, Before{sort, base}, [&](s32 left, s32 right) { swapPointers(sort, base, left, right); }, [&](s32 a, s32 b) { RecordSort_PointersByFloat(sort, nullptr, base, a, b); });
}

void __fastcall RecordSort_RunPointersByFloat(RecordSort *sort, void *, u8 **base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last) {
    sort->stride = stride;
    sort->keyOffset = keyOffset;
    sort->temp = temp;
    RecordSort_PointersByFloat(sort, nullptr, base, first, last);
}

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
