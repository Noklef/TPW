#include "signrender.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../online/chatlists.h"
#include "array3d.h"
#include "bumplight.h"
#include "imageops.h"

namespace {

const SignRenderEnv kGameEnv = {&SignRender_Text, &Bump_FilterInto, &Bump_Build};
const SignRenderEnv *g_env = &kGameEnv;

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

void MakeMask(u8 *image, u32 width, u32 height) {
    at<u32>(image, 0) = width;
    at<u32>(image, 4) = height;
    at<u32>(image, 8) = 1;
    at<void *>(image, 0xc) = GameHeap_Alloc(width * height);
}

void Tint(u8 *image, u8 *lit, const u8 *colour) { Mask_Tint(image, nullptr, lit, colour[3], colour[0], colour[1], colour[2]); }

void Stamp(u8 *picture, u8 *lit) { Pixels_Stamp(picture, nullptr, lit, 0, 0); }

u16 Pack4444(const u8 *p) {
    return static_cast<u16>((p[0] & 0xf0) << 8 | (p[1] & 0xf0) << 4 | (p[2] & 0xf0) | p[3] >> 4);
}

// Where the game has its two copies of the mode's checks; the masks stay until the end.
void DrawLayers(u8 *self, u8 *lit, u8 *a, u8 *b) {
    Array3D_Fill(lit, nullptr, 0, 0, 0, 0);
    if (self[8])
        Stamp(self + 0x18, lit);
    if (at<u32>(self, 0xc) == 2 && at<u32>(self, 0x10) == 2) {
        u8 both[0x10];
        MakeMask(both, at<u32>(a, 0), at<u32>(a, 4));
        Array3D_Fill(both, nullptr, 0, 0, 0, 0);
        Array3D_CopyTo(a, nullptr, both);
        Array3D_MaxInto(b, nullptr, both);
        Tint(both, lit, self + 0x430);
        if (at<void *>(both, 0xc))
            GameHeap_Free(at<void *>(both, 0xc));
    } else if (at<u32>(self, 0xc) == 2) {
        Tint(a, lit, self + 0x430);
    } else if (at<u32>(self, 0x10) == 2) {
        Tint(b, lit, self + 0x444);
    }
    if (at<s32>(self, 4) == 0) {
        if (at<u32>(self, 0xc) == 1)
            Tint(a, lit, self + 0x430);
        Stamp(self + 0x3b4, lit);
        if (at<u32>(self, 0x10) == 1)
            Tint(b, lit, self + 0x444);
        Stamp(self + 0x420, lit);
    } else {
        if (at<u32>(self, 0x10) == 1)
            Tint(b, lit, self + 0x444);
        Stamp(self + 0x420, lit);
        if (at<u32>(self, 0xc) == 1)
            Tint(a, lit, self + 0x430);
        Stamp(self + 0x3b4, lit);
    }
}

void Write32(u8 *self, u8 *lit, u8 *sign) {
    const u32 skip = (at<s32>(sign, 0x14) * 4 - 4) * at<u32>(sign, 0x10);
    u8 *to1 = at<u8 *>(sign, 8) + skip;
    u8 *to2 = at<u8 *>(sign, 0xc) + skip;
    const u32 *from = at<const u32 *>(self, 0x34);
    Array3D_ReverseBytes(lit);
    for (s32 y = 0; y < at<s32>(sign, 0x14); ++y) {
        u32 *to = reinterpret_cast<u32 *>(to1);
        for (s32 x = 0; x < at<s32>(sign, 0x10); ++x)
            *to++ = *from++;
        to = reinterpret_cast<u32 *>(to2);
        for (s32 x = 0; x < at<s32>(sign, 0x10); ++x)
            *to++ = *from++;
        to1 -= at<s32>(sign, 0x10) * 4;
        to2 -= at<s32>(sign, 0x10) * 4;
    }
}

void Write16(u8 *self, u8 *sign) {
    const u32 skip16 = (at<s32>(sign, 0x14) * 2 - 2) * at<u32>(sign, 0x10);
    u8 *row1 = at<u8 *>(sign, 8) + skip16;
    u8 *row2 = at<u8 *>(sign, 0xc) + skip16;
    const u8 *from = at<const u8 *>(self, 0x34);
    for (s32 y = 0; y < at<s32>(sign, 0x14); ++y) {
        u16 *to = reinterpret_cast<u16 *>(row1);
        for (s32 x = 0; x < at<s32>(sign, 0x10); ++x, from += 4)
            *to++ = Pack4444(from);
        to = reinterpret_cast<u16 *>(row2);
        for (s32 x = 0; x < at<s32>(sign, 0x10); ++x, from += 4)
            *to++ = Pack4444(from);
        row1 -= at<s32>(sign, 0x10) * 2;
        row2 -= at<s32>(sign, 0x10) * 2;
    }
}

}  // namespace

void SignRender_SetEnv(const SignRenderEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" u8 __fastcall SignRender_Draw(u8 *self, void *, u32 dc, u8 *sign) {
    if (!at<u32>(sign, 0) || !at<u32>(sign, 4) || !at<u32>(sign, 8) || !at<u32>(sign, 0xc))
        return 0;
    const u32 width = at<u32>(sign, 0x10) * 2;
    const u32 height = at<u32>(sign, 0x14) * 2;
    u8 *const lit = self + 0x28;
    u8 *const first = self + 0x384;
    u8 *const second = self + 0x3f0;
    Array3D_Realloc(lit, nullptr, width, height, 4);
    Array3D_Realloc(first, nullptr, width, height, 1);
    Array3D_Realloc(second, nullptr, width, height, 1);
    u8 a[0x10], b[0x10];
    MakeMask(a, width, height);
    MakeMask(b, width, height);
    g_env->text(self + 0x38, nullptr, dc, at<const u16 *>(sign, 0), first);
    g_env->text(self + 0x1c8, nullptr, dc, at<const u16 *>(sign, 4), second);
    if (at<u32>(self, 0xc))
        g_env->filter(self + 0x430, nullptr, reinterpret_cast<u32 *>(first), a);
    else
        Array3D_Fill(a, nullptr, 0, 0, 0, 0);
    if (at<u32>(self, 0x10))
        g_env->filter(self + 0x444, nullptr, reinterpret_cast<u32 *>(second), b);
    else
        Array3D_Fill(b, nullptr, 0, 0, 0, 0);
    at<u32>(self, 0x380) = at<u32>(self, 0x38);
    at<u32>(self, 0x37c) = at<u32>(self, 0x3c) - at<u32>(self, 0x38);
    g_env->build(self + 0x358);
    at<u32>(self, 0x3e8) = at<u32>(self, 0x1cc) - at<u32>(self, 0x1c8);
    at<u32>(self, 0x3ec) = at<u32>(self, 0x1c8);
    g_env->build(self + 0x3c4);
    DrawLayers(self, lit, a, b);
    if (at<u32>(sign, 0x18) == 1)
        Write32(self, lit, sign);
    else if (at<u32>(sign, 0x18) == 2)
        Write16(self, sign);
    if (at<void *>(b, 0xc))
        GameHeap_Free(at<void *>(b, 0xc));
    if (at<void *>(a, 0xc))
        GameHeap_Free(at<void *>(a, 0xc));
    return 1;
}

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
