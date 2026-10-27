#include "charsets.h"

#include <cstdarg>
#include <cstdint>

#include "../core/gameheap.h"
#include "../core/methods.h"
#include "../core/narrowwriter.h"
#include "../core/refcounted.h"
#include "../core/vcall.h"
#include "../core/widewriter.h"
#include "resindex.h"

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

u8 *__fastcall Charset_Ctor(u8 *charset, void *, u32 count, u32 *table, u8 *index) {
    Class70a078_Ctor(charset);
    at<u32>(charset, 0) = 0x70a060;
    at<u32>(charset, 8) = count;
    at<u32 *>(charset, 0xc) = table;
    for (u32 left = count; left; --left, ++table) *table += at<u32>(charset, 0xc);
    s32 slot = (s32)count;
    u8 *record = ResIndex_Find(index, nullptr, 2);
    if (Record_Word4OrSix(record) == 1) {
        ResIndex_GetEntry(index, nullptr, &slot, record, &at<u8 *>(charset, 0x14));
    } else {
        at<u32>(charset, 0x14) = 0;
    }
    record = ResIndex_Find(index, nullptr, 1);
    if (Record_Word4OrSix(record) == 0) {
        ResIndex_GetEntry(index, nullptr, &slot, record, &at<u8 *>(charset, 0x10));
    } else {
        at<u32>(charset, 0x10) = 0;
    }
    return charset;
}

void __fastcall Charset_Dtor(u8 *charset) {
    at<u32>(charset, 0) = 0x70a060;
    if (u8 *const range = at<u8 *>(charset, 0x14)) {
        vcall<void>(range, 0);
        at<u32>(charset, 0x14) = 0;
    }
    if (u8 *const code = at<u8 *>(charset, 0x10)) {
        vcall<void>(code, 0);
        at<u32>(charset, 0x10) = 0;
    }
    at<u32>(charset, 0) = 0x70a078;
}

u8 *__fastcall Charset_DeletingDtor(u8 *charset, void *, u32 flags) {
    Charset_Dtor(charset);
    if (flags & 1) GameHeap_Free(charset);
    return charset;
}

s32 *__fastcall Charset_Load(u8 *, void *, s32 *result, u8 *stream, u8 **object, u32 key, u8 *index) {
    u32 header[3];
    if (vcall<s32>(stream, 8, header, 12u) == 12 && header[0] == 0x54534642 && header[1] == key) {   // "BFST"
        const u32 size = BytesLeft(stream);
        if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size + 0x18))) {
            if ((u32)vcall<s32>(stream, 8, block + 0x18, size) == size) {
                Charset_Ctor(block, nullptr, header[2], reinterpret_cast<u32 *>(block + 0x18), index);
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

u8 *__stdcall TextArgs_Find(u32 key, u8 ***cursor) {
    u8 *found = nullptr;
    for (;;) {
        u8 *const arg = *(*cursor)++;
        if (!arg) return found;
        if (at<u32>(arg, 4) == 0x5641524d && at<u32>(arg, 8) == key) found = arg;   // 'MRAV'
        if (at<u32>(arg, 4) != 0x5641524d || found) return found;
    }
}

namespace {

// The state an argument is told (12 bytes): +0 a pointer into the program, +4 and +6 two words, +8 a dword.
struct FormatState {
    u32 data;
    u16 low;
    u16 high;
    u32 extra;
};

}  // namespace

s32 __fastcall Charset_Format(u8 *charset, void *, u8 *writer, u32 entry, u8 **args) {
    FormatState state = {0, 0xffff, 0, 0};
    const u32 *op = at<const u32 **>(charset, 0xc)[entry];
    for (;;) {
        const u32 word = *op++;
        const u32 operand = word >> 8;
        switch (word & 0xff) {
        case 0:
            vcall<void>(writer, 4);
            return 0;
        case 1:
            vcall<void>(writer, 0xc, op, operand);
            op += (operand + 3) >> 2;
            break;
        case 2: {
            u8 **cursor = args;
            u8 *const arg = TextArgs_Find(operand, &cursor);
            if (!arg) return -1;
            const s32 written = vcall<s32>(arg, 0, writer, &state);
            state = {0, 0xffff, 0, 0};
            if (written < 0) return -1;
            break;
        }
        case 3:
            state.data = (u32)(uintptr_t)op;
            op += operand;
            break;
        case 4:
            state.extra = operand;
            break;
        case 5:
            state.high = (u16)(word >> 16);
            break;
        case 6:
            state.low = (u16)(word >> 16);
            break;
        default:
            return -1;
        }
    }
}

s32 __cdecl Charset_FormatNarrow(u8 *charset, u8 *dest, u32 size, u32 entry, ...) {
    u8 writer[0x14];
    at<u32>(writer, 0) = 0x70a0a8;
    at<u8 *>(writer, 4) = dest;
    at<u8 *>(writer, 8) = dest + size;
    u8 *const table = at<u8 *>(charset, 0x14);
    at<u8 *>(writer, 0xc) = table;
    if (table) vcall<void>(table, 4);
    at<u8>(writer, 0x10) = 0;
    at<u32>(writer, 0) = 0x70a090;
    va_list args;
    va_start(args, entry);
    const s32 result = Charset_Format(charset, nullptr, writer, entry, reinterpret_cast<u8 **>(args));
    va_end(args);
    at<u32>(writer, 0) = 0x70a090;
    NarrowWriter_Finish(writer);
    if (u8 *const held = at<u8 *>(writer, 0xc)) vcall<void>(held, 0);
    return result;
}

s32 __cdecl Charset_FormatWide(u8 *charset, u16 *dest, u32 size, u32 entry, ...) {
    u8 writer[0x14];
    at<u32>(writer, 0) = 0x70a0a8;
    at<u16 *>(writer, 4) = dest;
    at<u16 *>(writer, 8) = dest + size;
    u8 *const table = at<u8 *>(charset, 0x10);
    at<u8 *>(writer, 0xc) = table;
    if (table) vcall<void>(table, 4);
    at<u8>(writer, 0x10) = 0;
    at<u32>(writer, 0) = 0x70a0c0;
    va_list args;
    va_start(args, entry);
    const s32 result = Charset_Format(charset, nullptr, writer, entry, reinterpret_cast<u8 **>(args));
    va_end(args);
    at<u32>(writer, 0) = 0x70a0c0;
    WideWriter_Finish(writer);
    if (u8 *const held = at<u8 *>(writer, 0xc)) vcall<void>(held, 0);
    return result;
}
