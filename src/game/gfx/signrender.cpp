#include "signrender.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../online/chatlists.h"
#include "array3d.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename F> F Import(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

u32 CreateFontFrom(const void *font) { return Import<u32(__stdcall *)(const void *)>(0x006fd048)(font); }
u32 SelectInto(u32 dc, u32 object) { return Import<u32(__stdcall *)(u32, u32)>(0x006fd044)(dc, object); }
void DeleteGdi(u32 object) { Import<u32(__stdcall *)(u32)>(0x006fd038)(object); }

// ScriptStringAnalyse's SCRIPT_CONTROL and SCRIPT_STATE are stack bytes the game never sets (sic): so are these.
void Analyse(u32 dc, const u16 *text, u32 length, s32 *advances, void **analysis) {
    u32 control;
    u16 state;
    Import<u32(__stdcall *)(u32, const u16 *, u32, u32, s32, u32, u32, void *, void *, s32 *, void *, void *, void **)>(0x006fd3a8)(
        dc, text, length, 0, -1, 0x80, 0, &control, &state, advances, nullptr, nullptr, analysis);
}

void FreeAnalysis(void **analysis) { Import<u32(__stdcall *)(void **)>(0x006fd3a0)(analysis); }

// The text's size with the advances scaled by the spacing. The analysis stays with the caller.
const s32 *Shape(u8 *self, u32 dc, const u16 *text, u32 length, void **analysis) {
    s32 *const advances = reinterpret_cast<s32 *>((uintptr_t)0x00f85f00);
    Analyse(dc, text, length, nullptr, analysis);
    Import<u32(__stdcall *)(void *, s32 *)>(0x006fd3a4)(*analysis, advances);
    for (u32 i = 0; i < length; ++i)
        advances[i] = static_cast<s32>(at<u32>(self, 8) * static_cast<u32>(advances[i])) / 100;
    FreeAnalysis(analysis);
    Analyse(dc, text, length, advances, analysis);
    return Import<const s32 *(__stdcall *)(void *)>(0x006fd3b0)(*analysis);
}

void *MakeInfo(const u8 *image) {
    u8 *info = static_cast<u8 *>(GameHeap_Alloc(0x428));
    at<u32>(info, 0) = 0x28;
    at<s32>(info, 4) = at<s32>(image, 0) * 2;
    at<s32>(info, 8) = -at<s32>(image, 4) * 2;
    at<u16>(info, 0xc) = 1;
    at<u16>(info, 0xe) = 8;
    at<u32>(info, 0x10) = 0;
    at<u32>(info, 0x14) = 0x8000;
    at<u32>(info, 0x18) = 0;
    at<u32>(info, 0x1c) = 0;
    at<u32>(info, 0x20) = 0x100;
    at<u32>(info, 0x24) = 0x100;
    for (u32 i = 0; i < 0x100; ++i) {
        u8 *colour = info + 0x28 + 4 * i;
        colour[0] = colour[1] = colour[2] = static_cast<u8>(i);
    }
    return info;
}

}  // namespace

extern "C" u8 __fastcall SignRender_Text(u8 *self, void *, u32 dc, const u16 *text, u8 *image) {
    const u32 length = CRT_wcslen(text);
    if (!length) {
        at<u32>(self, 0) = 0;
        at<u32>(self, 4) = 0;
        for (s32 i = 0; i < at<s32>(image, 0) * at<s32>(image, 4) * at<s32>(image, 8); ++i)
            at<u8 *>(image, 0xc)[i] = 0;
        return 1;
    }
    void *info = MakeInfo(image);
    u8 *bits;
    const u32 bitmap = Import<u32(__stdcall *)(u32, void *, u32, u8 **, u32, u32)>(0x006fd04c)(dc, info, 0, &bits, 0, 0);
    GameHeap_Free(info);
    if (!bitmap)
        return 0;
    const u32 memory = Import<u32(__stdcall *)(u32)>(0x006fd050)(dc);
    const u32 oldBitmap = SelectInto(memory, bitmap);
    Import<u32(__stdcall *)(u32, u32)>(0x006fd040)(memory, 1);
    const u32 savedWidth = at<u32>(self, 0x14);
    u32 font = CreateFontFrom(self + 0x10);
    u32 oldFont = SelectInto(memory, font);
    void *analysis;
    const s32 *size = Shape(self, memory, text, length, &analysis);
    if (size[0] >= at<s32>(image, 0) * 2) {
        s32 low = 1;
        s32 high = at<s32>(self, 0x14) ? at<s32>(self, 0x14) : -at<s32>(self, 0x10);
        s32 middle = ((high - 1) >> 1) + 1;
        for (;;) {   // The analyses of the sizes found are not freed (sic).
            at<s32>(self, 0x14) = middle;
            font = CreateFontFrom(self + 0x10);
            oldFont = SelectInto(memory, font);
            size = Shape(self, memory, text, length, &analysis);
            if (high - low <= 2 && size[0] < at<s32>(image, 0) * 2)
                break;
            SelectInto(memory, oldFont);
            DeleteGdi(font);
            if (size[0] < at<s32>(image, 0) * 2) {
                const s32 step = (high - middle) >> 1;
                low = middle;
                middle += step;
            } else {
                high = middle;
                middle = ((middle - low) >> 1) + low;
            }
        }
    }
    at<u32>(self, 0x14) = savedWidth;
    Import<u32(__stdcall *)(u32, u32)>(0x006fd034)(memory, 0);
    Import<u32(__stdcall *)(u32, u32)>(0x006fd03c)(memory, 0xffffff);
    Import<u32(__stdcall *)(void *, s32, u32, u32, void *, u32, u32, u32)>(0x006fd3ac)(analysis, 0x100 - size[0] / 2, at<u32>(self, 0xc), 4, nullptr, 0, 0, 0);
    FreeAnalysis(&analysis);
    SelectInto(memory, oldFont);
    DeleteGdi(font);
    Array3D_Realloc(image, nullptr, at<u32>(image, 0), at<u32>(image, 4), 1);
    u8 *to = at<u8 *>(image, 0xc);
    const u8 *row = bits;
    for (s32 y = 0; y < at<s32>(image, 4); ++y) {
        const u8 *upper = row;
        const u8 *lower = row + at<s32>(image, 0) * 2;
        for (s32 x = 0; x < at<s32>(image, 0); ++x) {
            *to++ = static_cast<u8>((upper[0] + upper[1] + lower[0] + lower[1]) >> 2);
            upper += 2;
            lower += 2;
        }
        row += at<s32>(image, 0) * 4;
    }
    SelectInto(memory, oldBitmap);
    DeleteGdi(bitmap);
    Import<u32(__stdcall *)(u32)>(0x006fd05c)(memory);
    at<s32>(self, 0) = -1;
    at<s32>(self, 4) = -1;
    const u8 *pixels = at<u8 *>(image, 0xc);
    for (s32 y = 0; y < at<s32>(image, 4); ++y) {
        const s32 width = at<s32>(image, 0);
        s32 x = 0;
        u8 seen = 0;
        while (x < width && !seen)
            seen = pixels[x++];
        if (x < width) {
            if (at<s32>(self, 0) < 0)
                at<s32>(self, 0) = y;
            else   // A text of one row leaves +4 at -1 (sic).
                at<s32>(self, 4) = y + 1;
        }
        pixels += width;
    }
    return 1;
}
