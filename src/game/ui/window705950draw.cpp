#include "window705950draw.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../gfx/display.h"
#include "../gfx/palette.h"
#include "../gfx/text.h"
#include "guirect.h"
#include "window705950.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }
const u8 *palette() { return reinterpret_cast<const u8 *>(0x00fa71f8); }

// The GUI's scaling of x and y (signed, toward 0).
s32 scaleX(s32 v) { return v * global<s16>(0x00faa5c4) / 2048; }
s32 scaleY(s32 v) { return v * global<s16>(0x00faa5c0) / 0x600; }

void getEdges(u8 *rect, u16 *left, u16 *top, u16 *right, u16 *bottom) { GuiRect_GetEdges(rect, nullptr, left, top, right, bottom); }

}  // namespace

extern "C" {

void __fastcall Window705950_Draw(u8 *window) {
    u16 left, top, right, bottom;
    Rect clip = {0, 0, 0, 0};
    getEdges(window + 0x2c, &left, &top, &right, &bottom);   // (sic: read over at once)
    getEdges(window + 0x138, &left, &top, &right, &bottom);
    clip.left = scaleX((s16)left);
    clip.top = scaleY((s16)top);
    clip.right = scaleX((s16)right);
    clip.bottom = scaleY((s16)bottom);
    Palette_Nearest(palette(), 0, 0, 0);   // (sic: these three unused)
    Palette_Nearest(palette(), 0xff, 0xff, 0xff);
    Palette_Nearest(palette(), 0x80, 0x80, 0x80);
    DrawColour white;
    white.rgb = 0xffffffff;
    white.index = Palette_Nearest(palette(), 0xff, 0xff, 0xff);
    u8 *node = Window705950_NthNode(window, nullptr, at<s32>(window, 0x150));
    u16 y = at<u16>(window, 0x13e);
    DrawColour red;
    red.rgb = 0xffc80000;
    red.index = Palette_Nearest(palette(), 200, 0, 0);
    if (!Text_SelectFont(1, 0))
        return;
    char buffer[0x20 + 0x80];   // the indent, then the line (as the original's stack has them)
    char *const indent = buffer, *const line = buffer + 0x20;
    Crt().sprintf(indent, text(0x00786de4));
    Display_SetClip(&clip);
    if (node) {
        for (const u8 *p = node; at<const u8 *>(p, 0x4c); p = at<const u8 *>(p, 0x4c))
            std::strcat(indent, text(0x0074f8d0));   // "    "
        for (s32 row = 0; row < at<s16>(window, 0x156) && node; ++row) {
            if ((s32)(row + at<u32>(window, 0x150)) == at<s32>(window, 0x148)) {
                clip.left = scaleX((s16)(at<u16>(window, 0x13c) + 2));
                clip.top = scaleY((s16)y);
                clip.right = scaleX((s16)(at<u16>(window, 0x140) - 2));
                clip.bottom = scaleY((s16)(at<u16>(window, 0x154) + y));
                Display_FillRect(&clip, red);
            }
            Crt().sprintf(line, text(0x0074f8c8), indent, node);   // "%s%S"
            Text_Draw(scaleX(at<s16>(window, 0x13c) + 2), scaleY((s16)y), line, white);
            if (u8 *const child = at<u8 *>(node, 0x48)) {
                node = child;
                std::strcat(indent, text(0x0074f8d0));
            } else if (u8 *const next = at<u8 *>(node, 0x44)) {
                node = next;
            } else {
                while (!at<u8 *>(node, 0x44)) {
                    node = at<u8 *>(node, 0x4c);
                    if (!node) break;
                }
                indent[0] = 0;   // (sic: emptied, not cut back)
                if (node) node = at<u8 *>(node, 0x44);
            }
            y = (u16)(y + at<u16>(window, 0x154));
        }
    }
    Display_SetClip(nullptr);
}
}
