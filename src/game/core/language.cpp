#include "language.h"

#include <cstdint>

#include "methods.h"
#include "../files/resindex.h"
#include "../gfx/textlayout.h"
#include "../gfx/text.h"
#include "crt.h"
#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 *const kWideBuffer = reinterpret_cast<u16 *>(0x007ba058);

// The text widened into the static buffer while the converter is on; status is the caller's argument slot (as the originals use it).
u16 *widen(const u8 *text, u32 *status) {
    if (global<u32>(0x0078ba80) != 0)
        Wide_FromNarrow(global<const u8 *>(0x0078ba7c), nullptr, status, kWideBuffer, text, 0x200);
    return kWideBuffer;
}
// A font entry constructed (0x419680: Obj_CtorValue with 0) and destroyed (0x47f980: the vtable 0x6ff058, then the base's (0x657d6c): the
// vtable 0x7058d8 and its block +0xc freed when set).
void __fastcall EntryCtor(void *entry) { Obj_CtorValue(static_cast<u8 *>(entry), nullptr, 0); }
void __fastcall EntryDtor(void *entry) {
    at<u32>(entry, 0) = 0x006ff058;
    at<u32>(entry, 0) = 0x007058d8;
    if (void *const block = at<void *>(entry, 0xc))
        GameHeap_Free(block);
}
}  // namespace

extern "C" {

void __fastcall Language_SetFontSet(u8 *lang, void *, s32 width) {
    at<u32>(lang, 4) = width <= 0x200 ? 0 : width <= 0x280 ? 1 : width <= 0x320 ? 2 : 3;
}

const char *__fastcall Language_Buffer(const u8 *) { return reinterpret_cast<const char *>(0x00f7f230); }

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
    const u8 *const text = Records_DataIfTerminated(global<const u8 *>(0x0078ba94), nullptr, index);
    return text ? widen(text, &index) : nullptr;
}

u32 __cdecl Language_SetObjectText(u8 *object, u32 index) {
    const u8 *const text = Records_DataIfTerminated(global<const u8 *>(0x0078baa4), nullptr, index);
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
}
