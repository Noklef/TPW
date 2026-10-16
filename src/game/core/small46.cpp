#include "small46.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall StdListString_Unique(u8 *list) {
    u8 *const head = at<u8 *>(list, 4);
    u8 *previous = at<u8 *>(head, 0);
    if (previous == head) return;
    u8 *current = at<u8 *>(previous, 0);
    if (current == head) return;
    do {
        const u8 *characters = at<const u8 *>(current, 0xc);
        const u32 length = at<u32>(current, 0x10);
        if (!characters) characters = reinterpret_cast<const u8 *>(0x006fe5b0);
        const u32 previousLength = at<u32>(previous, 0x10);
        const u8 *other = at<const u8 *>(previous, 0xc);
        bool equal = previousLength == length;
        if (equal) {
            for (u32 i = 0; i < length; ++i) {
                if (characters[i] != other[i]) { equal = false; break; }
            }
        }
        if (equal) {
            at<u8 *>(at<u8 *>(current, 4), 0) = at<u8 *>(current, 0);
            at<u8 *>(at<u8 *>(current, 0), 4) = at<u8 *>(current, 4);
            u8 *const text = at<u8 *>(current, 0xc);
            if (text) {
                const u8 count = text[-1];
                if (count == 0 || count == 0xff) GameHeap_Free(text - 1);
                else text[-1] = (u8)(count - 1);
            }
            at<u32>(current, 0xc) = 0;
            at<u32>(current, 0x10) = 0;
            at<u32>(current, 0x14) = 0;
            GameHeap_Free(current);
            at<u32>(list, 8) = at<u32>(list, 8) - 1;
        } else {
            previous = current;
        }
        current = at<u8 *>(previous, 0);
    } while (current != head);
}

void __fastcall StdListDword_Merge(u8 *list, void *, u8 *other) {
    if (other == list) return;
    u8 *const head1 = at<u8 *>(list, 4);
    u8 *first1 = at<u8 *>(head1, 0);
    u8 *const head2 = at<u8 *>(other, 4);
    u8 *first2 = at<u8 *>(head2, 0);
    while (first1 != head1 && first2 != head2) {
        if (at<u32>(first2, 8) < at<u32>(first1, 8)) {
            u8 *const next2 = at<u8 *>(first2, 0);
            at<u8 *>(at<u8 *>(next2, 4), 0) = first1;
            at<u8 *>(at<u8 *>(first2, 4), 0) = next2;
            at<u8 *>(at<u8 *>(first1, 4), 0) = first2;
            u8 *const saved = at<u8 *>(first1, 4);
            at<u8 *>(first1, 4) = at<u8 *>(next2, 4);
            at<u8 *>(next2, 4) = at<u8 *>(first2, 4);
            at<u8 *>(first2, 4) = saved;
            first2 = next2;
        } else {
            first1 = at<u8 *>(first1, 0);
        }
    }
    if (first2 != head2) {
        at<u8 *>(at<u8 *>(head2, 4), 0) = head1;
        at<u8 *>(at<u8 *>(first2, 4), 0) = head2;
        at<u8 *>(at<u8 *>(head1, 4), 0) = first2;
        u8 *const saved = at<u8 *>(head1, 4);
        at<u8 *>(head1, 4) = at<u8 *>(head2, 4);
        at<u8 *>(head2, 4) = at<u8 *>(first2, 4);
        at<u8 *>(first2, 4) = saved;
    }
    at<u32>(list, 8) = at<u32>(list, 8) + at<u32>(other, 8);
    at<u32>(other, 8) = 0;
}

}  // extern "C"
