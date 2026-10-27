#include "narrowwriter.h"

#include <cstdint>

#include "methods.h"
#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

void __fastcall NarrowWriter_Finish(u8 *writer) {
    u8 *const cursor = at<u8 *>(writer, 4);
    if (cursor) {
        if (!at<u8>(writer, 0x10) && cursor == at<u8 *>(writer, 8)) at<u8>(writer, 0x10) = 1;
        if (!at<u8>(writer, 0x10))
            cursor[0] = 0;
        else
            cursor[-1] = 0;
        at<u8 *>(writer, 4) = nullptr;
    }
    u8 *const holder = at<u8 *>(writer, 0xc);
    if (holder) {
        at<u8 *>(writer, 0xc) = nullptr;
        vcall<void>(holder, 0);
    }
}

void __fastcall NarrowWriter_PutChar(u8 *writer, void *, u32 character) {
    if (!at<u8>(writer, 0x10) && at<u8 *>(writer, 4) == at<u8 *>(writer, 8)) at<u8>(writer, 0x10) = 1;
    if (at<u8>(writer, 0x10)) return;
    *at<u8 *>(writer, 4) = (u8)character;
    at<u8 *>(writer, 4) += 1;
    if (const u8 high = (u8)(character >> 8)) {
        *at<u8 *>(writer, 4) = high;
        at<u8 *>(writer, 4) += 1;
    }
}

void __fastcall NarrowWriter_PutWideChar(u8 *writer, void *, u32 character) {
    const u8 *const table = at<const u8 *>(writer, 0xc);
    if (!table) return;
    vcall<void>(writer, 8, Ranges_LookupAdapter(table, nullptr, character));
}

void __fastcall NarrowWriter_PutWide(u8 *writer, void *, const u16 *text, s32 count) {
    if (!at<const u8 *>(writer, 0xc)) return;
    if (count == -1) {
        while (*text) {
            const u16 character = *text++;
            vcall<void>(writer, 8, Ranges_LookupAdapter(at<const u8 *>(writer, 0xc), nullptr, character));
        }
        return;
    }
    for (; count != 0; --count) {
        const u16 character = *text++;
        vcall<void>(writer, 8, Ranges_LookupAdapter(at<const u8 *>(writer, 0xc), nullptr, character));
    }
}

void __fastcall NarrowWriter_Dtor(u8 *writer) {
    at<u32>(writer, 0) = 0x70a090;
    NarrowWriter_Finish(writer);
    if (u8 *const table = at<u8 *>(writer, 0xc))
        vcall<void>(table, 0);
    at<u32>(writer, 0) = 0x70a0a8;
}

u8 *__fastcall NarrowWriter_DeletingDtor(u8 *writer, void *, u32 flags) {
    NarrowWriter_Dtor(writer);
    if (flags & 1) GameHeap_Free(writer);
    return writer;
}
