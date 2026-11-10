#include "language.h"

#include <cstdint>

#include "methods.h"
#include "../files/resindex.h"
#include "../gfx/textlayout.h"
#include "../gfx/text.h"
#include "crt.h"
#include "gameheap.h"
#include "vcall.h"
#include "initializers.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 *const kWideBuffer = reinterpret_cast<u16 *>(0x007ba058);

AddrCalls g_calls;

// Language_Load's tables: each charset's key and two names (the first the error's), in the original's order; the 13 fonts' keys of each set.
struct CharsetRow { u16 key; u32 name, second; };
const CharsetRow kCharsets[21] = {
    {0x3fc, (u32)(uintptr_t)&gLanguage_74c6b4, (u32)(uintptr_t)&gLanguage_74c6a4}, {0x3e8, (u32)(uintptr_t)&gLanguage_74c668, (u32)(uintptr_t)&gLanguage_74c65c}, {0x3e9, (u32)(uintptr_t)&gLanguage_74c648, (u32)(uintptr_t)&gLanguage_74c638}, {0x3ea, (u32)(uintptr_t)&gLanguage_74c624, (u32)(uintptr_t)&gLanguage_74c618}, {0x3ee, (u32)(uintptr_t)&gLanguage_74c608, (u32)(uintptr_t)&gLanguage_74c600},
    {0x3ef, (u32)(uintptr_t)&gLanguage_74c5ec, (u32)(uintptr_t)&gLanguage_74c5e0}, {0x3eb, (u32)(uintptr_t)&gLanguage_74c5cc, (u32)(uintptr_t)&gLanguage_74c5c0}, {0x3ec, (u32)(uintptr_t)&gLanguage_74c5ac, (u32)(uintptr_t)&gLanguage_74c59c}, {0x3ed, (u32)(uintptr_t)&gLanguage_74c584, (u32)(uintptr_t)&gLanguage_74c574}, {0x3f3, (u32)(uintptr_t)&gLanguage_74c55c, (u32)(uintptr_t)&gLanguage_74c54c},
    {0x3f4, (u32)(uintptr_t)&gLanguage_74c534, (u32)(uintptr_t)&gLanguage_74c524}, {0x3f5, (u32)(uintptr_t)&gLanguage_74c508, (u32)(uintptr_t)&gLanguage_74c4f4}, {0x3f6, (u32)(uintptr_t)&gLanguage_74c4dc, (u32)(uintptr_t)&gLanguage_74c4d0}, {0x3f7, (u32)(uintptr_t)&gLanguage_74c4b4, (u32)(uintptr_t)&gLanguage_74c4a0}, {0x3f8, (u32)(uintptr_t)&gLanguage_74c488, (u32)(uintptr_t)&gLanguage_74c478},
    {0x3fb, (u32)(uintptr_t)&gLanguage_74c460, (u32)(uintptr_t)&gLanguage_74c454}, {0x3fa, (u32)(uintptr_t)&gLanguage_74c440, (u32)(uintptr_t)&gLanguage_74c430}, {0x3f0, (u32)(uintptr_t)&gLanguage_74c418, (u32)(uintptr_t)&gLanguage_74c408}, {0x3f2, (u32)(uintptr_t)&gLanguage_74c3f0, (u32)(uintptr_t)&gLanguage_74c3e0}, {0x3f9, (u32)(uintptr_t)&gLanguage_74c3cc, (u32)(uintptr_t)&gLanguage_74c3b8},
    {0x3f1, (u32)(uintptr_t)&gLanguage_74c3a4, (u32)(uintptr_t)&gLanguage_74c398},
};
const u16 kFontKeys[4][13] = {
    {0x415, 0x40f, 0x410, 0x418, 0x3ff, 0x40c, 0x3fd, 0x3ff, 0x3fd, 0x3fd, 0x40a, 0x3ff, 0x41c},
    {0x416, 0x411, 0x412, 0x419, 0x407, 0x40d, 0x400, 0x402, 0x403, 0x400, 0x40a, 0x400, 0x41c},
    {0x417, 0x413, 0x414, 0x41b, 0x403, 0x40e, 0x402, 0x404, 0x403, 0x402, 0x40a, 0x402, 0x41c},
    {0x417, 0x413, 0x414, 0x41b, 0x409, 0x40e, 0x404, 0x406, 0x408, 0x404, 0x40a, 0x404, 0x41c},
};

bool failed(u32 status) { return *reinterpret_cast<const s32 *>(static_cast<uintptr_t>(status)) < 0; }
s32 load_error(u32 name) {
    g_calls.Th(0x00405710, 0x00785de8u, 3u, 0u, name);
    return 0;
}

// The text widened into the static buffer while the converter is on; status is the caller's argument slot (as the originals use it).
u16 *widen(const u8 *text, u32 *status) {
    if (gLanguage_78ba80 != 0)
        Wide_FromNarrow(gLanguage_78ba7c, nullptr, status, kWideBuffer, text, 0x200);
    return kWideBuffer;
}
// A font entry constructed (0x419680: Obj_CtorValue with 0) and destroyed (0x47f980: the vtable 0x6ff058, then the base's (0x657d6c): the
// vtable 0x7058d8 and its block +0xc freed when set).
void __fastcall EntryCtor(void *entry) { Obj_CtorValue(static_cast<u8 *>(entry), nullptr, 0); }
void __fastcall EntryDtor(void *entry) {
    at<u32>(entry, 0) = (u32)(uintptr_t)Class6ff058_vtable;
    at<u32>(entry, 0) = (u32)(uintptr_t)Class7058d8_vtable;
    if (void *const block = at<void *>(entry, 0xc))
        GameHeap_Free(block);
}
}  // namespace

extern "C" {

void __fastcall Language_SetFontSet(u8 *lang, void *, s32 width) {
    at<u32>(lang, 4) = width <= 0x200 ? 0 : width <= 0x280 ? 1 : width <= 0x320 ? 2 : 3;
}

const char *__fastcall Language_Buffer(const u8 *) { return gLanguage_f7f230; }

u32 __fastcall Language_Id(const u8 *lang) { return at<u32>(lang, 8); }

void __fastcall Language_SetId(u8 *lang, void *, u32 id) { at<u32>(lang, 8) = id; }

u32 __fastcall Language_IsHebrew(const u8 *lang) { return at<u32>(lang, 8) == 0x40d; }

u8 *__cdecl Language_Font(u32 index) {
    if (index >= 13) return nullptr;
    u8 *const lang = reinterpret_cast<u8 *>(0x0078b530);
    return lang + 0x1c + (index + at<u32>(lang, 4) * 13) * 0x18;
}

void __cdecl Language_MirrorAlign(u32 *align) {
    if (!(u8)Language_IsHebrew(reinterpret_cast<const u8 *>(0x0078b530))) return;
    if (*align == 0) {
        *align = 2;
    } else if (*align == 2) {
        *align = 0;
    }
}

u16 *__cdecl Language_Widen(const u8 *text) { return text ? widen(text, reinterpret_cast<u32 *>(&text)) : nullptr; }

u16 *__cdecl Language_WideText(u32 index) {
    const u8 *const text = Records_DataIfTerminated(gLanguage_78ba94, nullptr, index);
    return text ? widen(text, &index) : nullptr;
}

u32 __cdecl Language_SetObjectText(u8 *object, u32 index) {
    const u8 *const text = Records_DataIfTerminated(gLanguage_78baa4, nullptr, index);
    return vcall<u32>(object, 0x24, text ? widen(text, &index) : nullptr);
}

void __fastcall Language_FreeFonts(u8 *lang) {
    at<u32>(lang, 0) = 0;
    for (u32 entry = 0; entry < 13; ++entry) {
        for (u32 set = 0; set < 4; ++set) {
            u8 *&font = at<u8 *>(lang, 0x30 + entry * 0x18 + set * 0x138);
            if (font) {
                vcall<void>(font, 0);
                font = nullptr;
            }
        }
    }
    static const u16 kObjects[23] = {0x54c, 0x550, 0x554, 0x558, 0x55c, 0x560, 0x564, 0x568, 0x56c, 0x570, 0x574, 0x578,
                                     0x57c, 0x580, 0x584, 0x588, 0x590, 0x594, 0x58c, 0x598, 0x59c, 0x5a0, 0x5a4};
    for (const u16 offset : kObjects) {
        u8 *&object = at<u8 *>(lang, offset);
        if (object) {
            vcall<void>(object, 0);
            object = nullptr;
        }
    }
    s32 result;   // (the original's: the counter's slot, 0 by then)
    ResFolder_Close(lang + 0xc, nullptr, &result);
}

u8 *__fastcall Language_Ctor(u8 *lang) {
    at<u32>(lang, 0) = 0;
    at<u32>(lang, 4) = 1;
    ResFolder_Ctor(lang + 0xc);
    CRT_VectorCtorIterator(lang + 0x1c, 0x18, 0x34, EntryCtor, reinterpret_cast<void *>(EntryDtor));   // (the font entries)
    TextStyle_Ctor(lang + 0x4fc);
    static const u16 kCleared[19] = {0x54c, 0x550, 0x554, 0x558, 0x55c, 0x564, 0x568, 0x56c, 0x570, 0x574,
                                     0x578, 0x57c, 0x580, 0x584, 0x588, 0x58c, 0x594, 0x598, 0x59c};   // (not 0x560, 0x590, 0x5a0, 0x5a4)
    for (const u16 offset : kCleared)
        at<u32>(lang, offset) = 0;
    for (u32 entry = 0; entry < 13; ++entry) {   // (each set's entry: its index, its font none)
        for (u32 set = 0; set < 4; ++set) {
            at<u32>(lang, 0x2c + entry * 0x18 + set * 0x138 + 4) = 0;
            at<u32>(lang, 0x2c + entry * 0x18 + set * 0x138) = entry;
        }
    }
    return lang;
}

void __fastcall Language_Dtor(u8 *lang) {
    TextStyle_Dtor(lang + 0x4fc);
    CRT_VectorDtorIterator(lang + 0x1c, 0x18, 0x34, EntryDtor);
    ResFolder_Dtor(lang + 0xc);
}

s32 __fastcall Language_Load(u8 *lang) {
    s32 result;
    const u32 self = static_cast<u32>(reinterpret_cast<uintptr_t>(lang));
    const u32 folder = self + 0xc;
    const u32 out = static_cast<u32>(reinterpret_cast<uintptr_t>(&result));
    if (failed(g_calls.Th(0x006ad1b0, folder, out, (u32)(uintptr_t)&gLanguage_6fd990))) return load_error((u32)(uintptr_t)&gLanguage_6fd990);
    if (failed(g_calls.Th(0x006ad230, folder, out, (u32)(uintptr_t)&gLanguage_f7f230))) return load_error((u32)(uintptr_t)&gLanguage_f7f230);
    if (failed(g_calls.Th(0x006ad4b0, folder, out, self + 0x550, 2u))) return -1;
    if (failed(g_calls.Th(0x006ad460, folder, out, self + 0x54c, 1u))) return -1;
    for (u32 i = 0; i < 21; i++) {
        const CharsetRow &row = kCharsets[i];
        if (failed(g_calls.Th(0x006ad510, folder, out, self + 0x554 + 4 * i, static_cast<u32>(row.key)))) {
            char line[0x100];
            g_calls.Cd(0x0067a650, static_cast<u32>(reinterpret_cast<uintptr_t>(line)), (u32)(uintptr_t)&gLanguage_74c67c, row.second, row.name);
            global<void(__stdcall *)(const char *)>(0x006fd1e0)(line);
            return load_error(row.name);
        }
    }
    const s32 by_setting = global<s32>(0x007858ec);
    const s32 by_screen = global<s32>(0x0078d8dc);
    const u32 set = by_setting > 0 ? (by_setting >= 4 ? 3 : by_setting - 1) : (by_screen == 0 || by_screen == 1 || by_screen == 2 ? by_screen : 3);
    for (u32 i = 0; i < 13; i++)
        if (failed(g_calls.Th(0x006ad570, folder, out, self + 0x30 + 0x138 * set + 0x18 * i, static_cast<u32>(kFontKeys[set][i])))) return load_error((u32)(uintptr_t)&gLanguage_74c390);
    g_calls.Th(0x006ad790, self + 0x4fc, out);
    at<u32>(lang, 0) = 1;
    return 1;
}
}

void Language_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" u32 __fastcall Language_CtorValue0(u8 *self) {
    return g_calls.Th(0x0047f940, (u32)(uintptr_t)self, 0u);   // Obj_CtorValue
}
