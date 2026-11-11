// The preview of the dialog [0x7cc1f8]: see dialog7cc1f8preview.h.
#include "dialog7cc1f8preview.h"

#include <cstdint>

#include "../core/globalsave.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../core/vcall.h"
#include "../gfx/display.h"
#include "../gfx/surface.h"
#include "../world/mapcell.h"
#include "../world/objectdefs.h"
#include "guirect.h"
#include "signrect.h"
#include "window.h"
#include "dialog7cc1f8.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc1f8PreviewEnv kGameEnv = {&ObjectDefs_Find, &GlobalSave_HasKey, &Sign_PlaceOverRect, &Window_ImplSlot7, &Window_ImplSlot6, &Format_FillInfo, &Display_SetSurfaceInfo, &Display_BoxFill, &MapGrid_ClassifyTypes, &Records_DataIfTerminated, &Language_Widen, &Language_WideText};
const Dialog7cc1f8PreviewEnv *g_env = &kGameEnv;

u8 *defs() { return reinterpret_cast<u8 *>((uintptr_t)0x007890a0); }

// The image's pixels, its surface made the display's and cleared; false when it has none.
bool clearImage(u8 *image, SurfaceInfo *info, s32 *width, s32 *height) {
    const u32 pixels = g_env->pixels(image, nullptr, (u32)(uintptr_t)width, (u32)(uintptr_t)height);
    if (!pixels) return false;
    g_env->fillInfo(pixels, reinterpret_cast<u32 *>(info), (u32)*width, (u32)*height);
    g_env->setInfo(info);
    return true;
}

void drawGrid(const u8 *def, s32 width, s32 height) {
    u32 *const types = g_env->classify(def);
    DrawColour colour;
    colour.rgb = 0;
    colour.index = 0;
    Rect rect = {0, 0, width - 1, height - 1};
    g_env->boxFill(&rect, colour);
    const s32 cellWidth = width / 8, cellHeight = height / 8;
    for (s32 column = 0; column < at<s32>(types, 0x400); ++column) {
        const u32 *cell = reinterpret_cast<const u32 *>(reinterpret_cast<const u8 *>(types) + column * 0x40);
        s32 y = 0;
        for (s32 row = 0; row < at<s32>(types, 0x404); ++row, ++cell, y += cellHeight) {
            const u32 kind = *cell;
            if (!kind) continue;
            if (kind == 1 || kind == 4) colour.rgb = 0x801eaaff;
            else if (kind == 2) colour.rgb = 0x800fdc32;
            else if (kind == 3) colour.rgb = 0x80dc640f;
            rect.left = cellWidth * column;
            rect.top = height - cellHeight - y - 1;
            rect.right = (column + 1) * cellWidth;
            rect.bottom = height - y - 1;
            g_env->boxFill(&rect, colour);
        }
    }
}

void missing(u8 *panel, const u8 *rect, s16 word) {
    u16 defId, text;
    if (word > 0) defId = 0x64, text = 0x89;
    else if (word == -1) defId = 0x65, text = 0x86;
    else defId = 0x66, text = 0x87;
    const u8 *const def = g_env->find(defs(), nullptr, defId);
    g_env->sign(rect, at<u32>(def, 0x490), at<s16>(panel, 0xe0) + 10);
    u8 *const label = Window_FindChild(panel, nullptr, 0x1ec);
    vcall<void>(label, 0x24, (u32)(uintptr_t)g_env->wideText(text));
}

}  // namespace

void Dialog7cc1f8_SetPreviewEnv(const Dialog7cc1f8PreviewEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc1f8_ShowPreview(const u16 *id) {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007cc1f8), nullptr, 0x1ea);
    if (!panel) return;
    const u8 *const first = g_env->find(defs(), nullptr, *id);
    u8 *const image = Window_FindChild(panel, nullptr, 0x1eb);
    SurfaceInfo info;
    SurfaceInfo_Ctor(&info);
    u8 unused[12], rect[12];
    GuiRect_Ctor(unused);
    GuiRect_Ctor(rect);
    const s16 word = static_cast<s16>(*id);
    Window_GetSavedRect(panel, nullptr, rect);
    s32 width, height;
    if (word >= 0 && (at<s32>(first, 0xc4) <= 0 || static_cast<u8>(g_env->hasKey(at<u16>(first, 0))))) {
        const u8 *const def = g_env->find(defs(), nullptr, *id);
        g_env->sign(rect, at<u32>(def, 0x490), at<s16>(panel, 0xe0) + 10);
        if (clearImage(image, &info, &width, &height)) {
            drawGrid(first, width, height);
            g_env->unlock(image);
        }
        u8 *const label = Window_FindChild(panel, nullptr, 0x1ec);
        vcall<void>(label, 0x24, (u32)(uintptr_t)g_env->widen(g_env->record(at<u8 *>(gLanguage_78b530, 0x5a0), nullptr, at<u32>(def, 0x90))));
    } else {
        if (clearImage(image, &info, &width, &height)) {
            DrawColour colour;
            colour.rgb = 0;
            colour.index = 0;
            Rect all = {0, 0, width - 1, height - 1};
            g_env->boxFill(&all, colour);
            g_env->unlock(image);
        }
        missing(panel, rect, word);
    }
    GuiRect_Dtor(rect);
    GuiRect_Dtor(unused);
}
