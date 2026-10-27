#include "widewriter.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The table's code for a character (the bytes after a lead byte read through next), 0 past its count.
u16 code_of(const u8 *table, u32 first, const u8 *&next) {
    u16 code = (u16)(first - 1);
    const u8 lead = at<u8>(table, 8);
    if (code >= lead)
        code = (u16)((first + 0x100) * 0xff - lead * 0xfe + *next++);
    return code < at<u16>(table, 0xa) ? at<const u16 *>(table, 0xc)[code] : 0;
}

}  // namespace

void __fastcall WideWriter_Finish(u8 *writer) {
    u16 *const cursor = at<u16 *>(writer, 4);
    if (cursor) {
        if (!at<u8>(writer, 0x10) && cursor == at<u16 *>(writer, 8)) at<u8>(writer, 0x10) = 1;
        if (!at<u8>(writer, 0x10))
            cursor[0] = 0;
        else
            cursor[-1] = 0;
        at<u16 *>(writer, 4) = nullptr;
    }
    u8 *const holder = at<u8 *>(writer, 0xc);
    if (holder) {
        at<u8 *>(writer, 0xc) = nullptr;
        vcall<void>(holder, 0);
    }
}

void __fastcall WideWriter_PutChar(u8 *writer, void *, u32 character) {
    const u8 *const table = at<const u8 *>(writer, 0xc);
    if (!table) return;
    const u8 high = (u8)(character >> 8);
    const u8 *next = &high;
    vcall<void>(writer, 0x10, (u32)code_of(table, character & 0xff, next));
}

void __fastcall WideWriter_PutNarrow(u8 *writer, void *, const u8 *text, s32 count) {
    if (!at<const u8 *>(writer, 0xc)) return;
    if (count == -1) {
        if (!*text) return;
        do {
            const u8 first = *text++;
            vcall<void>(writer, 0x10, (u32)code_of(at<const u8 *>(writer, 0xc), first, text));
        } while (*text);
        return;
    }
    const u8 *const end = text + count;
    while (text != end) {
        const u8 first = *text++;
        vcall<void>(writer, 0x10, (u32)code_of(at<const u8 *>(writer, 0xc), first, text));
    }
}

void __fastcall WideWriter_PutWide(u8 *writer, void *, const u16 *text, s32 count) {
    const auto put = [&]() {
        if (!at<u8>(writer, 0x10) && at<u16 *>(writer, 4) == at<u16 *>(writer, 8)) at<u8>(writer, 0x10) = 1;
        if (at<u8>(writer, 0x10)) return false;
        *at<u16 *>(writer, 4) = *text++;
        at<u16 *>(writer, 4) += 1;
        return true;
    };
    if (count == -1) {
        if (!*text) return;
        do {
            if (!put()) return;
        } while (*text);
        return;
    }
    while (count != 0) {
        --count;
        if (!put()) return;
    }
}

void __fastcall WideWriter_Dtor(u8 *writer) {
    at<u32>(writer, 0) = 0x70a0c0;
    WideWriter_Finish(writer);
    if (u8 *const table = at<u8 *>(writer, 0xc))
        vcall<void>(table, 0);
    at<u32>(writer, 0) = 0x70a0a8;
}

u8 *__fastcall WideWriter_DeletingDtor(u8 *writer, void *, u32 flags) {
    WideWriter_Dtor(writer);
    if (flags & 1) GameHeap_Free(writer);
    return writer;
}
