// The tag system's CTagTable: reimplementation of 0x50af40.
#include "tagsystem.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/textarg.h"
#include "../core/thunks.h"
#include "../core/vcall.h"
#include "messagecentre.h"
#include "objectdefs.h"
#include "things.h"
#include "../ui/window7cc8c0handler.h"
#include "tagsystem.h"
#include "../core/imports.h"

namespace {

u32 table() { return (u32)(uintptr_t)&gTagTable_75eca8; }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u8 *defs() { return reinterpret_cast<u8 *>((uintptr_t)0x007890a0); }
const u16 *text(u32 address) { return reinterpret_cast<const u16 *>((uintptr_t)address); }
void Debug_Line(u32 address) { (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)IMPORT(Kernel32_OutputDebugStringA)))(address); }  // (OutputDebugStringA, an import)

// (The formatter's arguments: a number is 0x14 bytes, signed; a string 0x10, kept in a number's room here.)
struct Arg {
    u8 bytes[0x14];
};
u8 *number(Arg &arg, u32 n, s32 value) { return TextArgInt_Ctor(arg.bytes, nullptr, n, value, 1); }
u8 *string(Arg &arg, u32 n, const u16 *value) { return TextArgString_Ctor(arg.bytes, nullptr, n, (u32)(uintptr_t)value); }

template <typename... A> void format(u8 *self, const u8 *record, A... args) {
    TextFormat().format(global<void *>(0x0078ba90), &at<u16>(self, 0xc), 0x400, at<u32>(record, 0xc), args...);
}

// The kind's text (the table [0x78bac0]: 1, 3, 5, 7, 9). Past kind 4 the buffer stays as it was (sic: the original's uninitialised stack).
void kindText(u32 kind, u16 *out) {
    if (kind <= 4) TextFormat().format(global<void *>(0x0078bac0), out, 0x40, kind * 2 + 1);
}

void fullName(const u8 *c, u32 offset, u16 *out) { ObjectDefs_GetFullName(defs(), nullptr, at<u16>(c, offset), out, 0x40); }

// The message for 0x481580 from the record (0x28 bytes: id, +4, the text, +0xc, word +0x10, +0x14, +0x18, word +0x1c, +0x20, byte +0x24 0).
void post(u8 *self, const u32 *record, u32 id, u32 value, u16 word, u32 last) {
    u8 message[0x28] = {};
    at<u32>(message, 0) = id;
    at<u32>(message, 4) = at<u32>(record, 4);
    at<u16 *>(message, 8) = &at<u16>(self, 0xc);
    at<u32>(message, 0xc) = value;
    at<u16>(message, 0x10) = at<u16>(record, 8);
    at<u32>(message, 0x14) = at<u32>(record, 0x10);
    at<u32>(message, 0x18) = at<u32>(record, 0x14);
    at<u16>(message, 0x1c) = word;
    at<u32>(message, 0x20) = last;
    Wrap_MsgTags_Post_481580(message);
}

u32 *Record(u32 address) { return reinterpret_cast<u32 *>((uintptr_t)address); }

}  // namespace

extern "C" u32 *__fastcall TagTable_Find(void *, void *, u32 id) {
    if (id == 0x17f || id == 0x17e) return nullptr;
    u32 *direct = Record(table() + (id << 5));
    if (*direct == id) return direct;
    for (u32 address = table(); (s32)address < (s32)(table() + sizeof(gTagTable_75eca8)); address += 0x20)
        if (*Record(address) == id) return Record(address);
    return nullptr;
}

extern "C" u8 *__fastcall TagSystem_Ctor(u8 *tags) {
    Thing_Construct(tags, nullptr, 1, 0xc);
    static const s32 kSlots[] = {7, 0xc, 0x14, 0x18, 0x19, 0x13};
    for (s32 slot : kSlots)
        MessageCentre_AddThing(MessageCentre_Instance(), nullptr, tags, slot);
    return tags;
}

extern "C" u8 __fastcall TagSystem_FormatTag(u8 *self, void *, const u8 *record, u32 a, u32 b, const u8 *c) {
    u16 *const dest = &at<u16>(self, 0xc);
    if (!record) {
        Crt().swprintf(dest, text(reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374)));
        return 0;
    }
    Arg x, y, z, w, s, t;
    u16 first[0x40], second[0x40];
    switch (at<u32>(record, 0x1c)) {
    case 0:
        format(self, record);
        return 1;
    case 1:
        if ((u16)a == 0 || at<u32>(reinterpret_cast<void *>((uintptr_t)0x007cfb90), 20 * (u16)a) == 0) return 0;
        Crt().swprintf(dest, gText_TagSystem_HandleMessage_75ec64);
        return 1;
    case 2:
        if ((u16)b == 0) return 0;
        format(self, record, string(x, 0, reinterpret_cast<const u16 *>(ObjectDefs_FindAt410(defs(), nullptr, (u16)b))));
        return 1;
    case 3:
        format(self, record, number(x, 1, (s32)b));
        return 1;
    case 4:
        Crt().swprintf(dest, gText_TagSystem_HandleMessage_75ebe0);
        return 1;
    case 5:
        format(self, record, number(x, 1, global<s32>(0x00785790)));
        return 1;
    case 6:
        format(self, record, number(x, 1, global<s32>(0x00785794)));
        return 1;
    case 7:
        format(self, record, number(x, 1, global<s32>(0x007857a0)));
        return 1;
    case 8:
        format(self, record, number(x, 1, global<s32>(0x007857a4)), number(y, 2, global<s32>(0x007857a8)));
        return 1;
    case 9:
    case 0xb:
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 1, at<s32>(c, 0x1c)), number(z, 2, at<s32>(c, 0x18)));
        return 1;
    case 0xa:
    case 0xe:
        fullName(c, 0x28, first);
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 1, at<s32>(c, 0x1c)), number(z, 2, at<s32>(c, 0x18)), string(s, 6, first));
        return 1;
    case 0xc:
    case 0xd:
        kindText(at<u32>(c, 0x2c), first);
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 1, at<s32>(c, 0x1c)), number(z, 2, at<s32>(c, 0x18)), string(s, 6, first));
        return 1;
    case 0xf:
        fullName(c, 0x28, first);
        fullName(c, 0x2a, second);
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 2, at<s32>(c, 0x18)), string(s, 6, first), string(t, 7, second));
        return 1;
    case 0x10:
    case 0x12:
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 1, at<s32>(c, 0x1c)), number(z, 4, at<s32>(c, 0x24)), number(w, 5, at<s32>(c, 0x20)));
        return 1;
    case 0x11:
    case 0x15:
        fullName(c, 0x28, first);
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 1, at<s32>(c, 0x1c)), number(z, 4, at<s32>(c, 0x24)), number(w, 5, at<s32>(c, 0x20)),
               string(s, 6, first));
        return 1;
    case 0x13:
    case 0x14:
        kindText(at<u32>(c, 0x2c), first);
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 1, at<s32>(c, 0x1c)), number(z, 4, at<s32>(c, 0x24)), number(w, 5, at<s32>(c, 0x20)),
               string(s, 6, first));
        return 1;
    case 0x16:
        fullName(c, 0x28, first);
        fullName(c, 0x2a, second);
        format(self, record, number(x, 3, at<s32>(c, 0x14)), number(y, 5, at<s32>(c, 0x20)), string(s, 6, first), string(t, 7, second));
        return 1;
    case 0x17: case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d:
        format(self, record, number(x, 3, at<s32>(c, 0x14)));
        return 1;
    case 0x1e: case 0x1f: case 0x20: case 0x21: case 0x22: case 0x23: case 0x24:
        format(self, record, number(x, 1, at<s32>(c, 0x1c)), number(y, 4, at<s32>(c, 0x24)));
        return 1;
    default:
        return 1;
    }
}

extern "C" void __fastcall TagSystem_HandleMessage(u8 *self, void *, u8 *msg) {
    switch (vcall<u32>(msg, 0)) {
    case 0x13: {
        u32 id;
        switch (at<u32>(msg, 8)) {
        case 7: Debug_Line((u32)&gText_TagSystem_HandleMessage_75eb20); id = 0x17b; break;
        case 8: Debug_Line((u32)&gText_TagSystem_HandleMessage_75eb40); id = 0x17c; break;
        case 9: Debug_Line((u32)&gText_TagSystem_HandleMessage_75eb68); id = 0x17d; break;
        default: return;
        }
        const u32 *const record = TagTable_Find(self + 0x80c, nullptr, id);
        if (!record) return;
        TagSystem_FormatTag(self, nullptr, reinterpret_cast<const u8 *>(record), 0, 0, nullptr);
        post(self, record, id, at<u32>(record, 0x18), 0, 0);
        return;
    }
    case 0x18: {
        const u32 id = at<u32>(msg, 8);
        const u32 *const record = TagTable_Find(self + 0x80c, nullptr, id);
        if (!record) return;
        const u16 word = at<u16>(msg, 0xc);
        const s32 value = at<s32>(msg, 0x10);
        TagSystem_FormatTag(self, nullptr, reinterpret_cast<const u8 *>(record), word, at<u32>(msg, 0x14), nullptr);
        post(self, record, id, value >= 0 ? (u32)value : at<u32>(record, 0x18), word, at<u32>(msg, 0x14));
        return;
    }
    case 0x19: {
        const u32 id = at<u32>(msg, 8);
        const u32 *const record = TagTable_Find(self + 0x80c, nullptr, id);
        if (!record) return;
        TagSystem_FormatTag(self, nullptr, reinterpret_cast<const u8 *>(record), 0, 0, msg);
        post(self, record, id, at<u32>(record, 0x18), 0, 0);
        return;
    }
    default:
        vcall<u32>(msg, 0);  // (the original asks again for its debug print, not reproduced)
        return;
    }
}
