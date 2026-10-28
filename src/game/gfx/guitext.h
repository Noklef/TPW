// The GUI's text drawing and measuring (0x492750, 0x4929f0, 0x492c40):a text layout (gfx/textlayout) built on the stack for one call, the global text
// style (0x78ba2c) given a font of the current language's table (0x78b54c: 13 entries of 24 bytes a language (0x78b534), the font at +0x14),
// the wide text narrowed into the global buffer (Text_NarrowToGlobalBuffer) and laid out in a rectangle.
//
// The originals' quirks are kept: an alignment (0 left, 1 centre, 2 right; mirrored in Hebrew) without `fit` widens the rectangle to 0x4000,
// and for centred or right-aligned text its left becomes half the width less 0x2000 or the width less 0x4000, the rectangle's own left
// dropped; a font index past 12 reads the font from address 0x14.
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492750 (cdecl (text, rect, colour, font, hAlign, vAlign, fit, bounds, keepSurface); the three alignment / font arguments signed
// bytes): unless there is neither a surface (0x7ca120) nor keepSurface, or no text: the layout's kinds set from the alignments
// (Obj_SetKindA4: 0 left, 2 centre, 1 right; Obj_SetKindA0: vAlign 0 top 4, 1 centre 2, 2 bottom 5; another alignment leaves the kind
// unset, as TextLayout_Ctor leaves it: the stack's garbage in the original), the global style's font and colour
// (TextStyle_SetFont, TextStyle_SetColour), without keepSurface the GUI's SurfaceInfo (0x7ca0e8) drawn on (Display_SetSurfaceInfo), the
// rectangle the clip (Display_SetClip), the text drawn (TextLayout_Draw), the clip reset; bounds (unless 0) the drawn glyphs' bounding box
// ({0} when nothing was drawn).
void __cdecl GuiText_Draw(const u16 *text, const s32 *rect, const u8 *colour, s8 font, s8 hAlign, s8 vAlign, u32 fit, s32 *bounds,
                          u32 keepSurface);

// 0x4929f0 (cdecl (text, rect, font, hAlign, vAlign, fit, box)): with a text, as GuiText_Draw without the colour, the surface and the clip:
// the text measured (TextLayout_Measure) and its box copied to box (unless 0).
void __cdecl GuiText_Measure(const u16 *text, const s32 *rect, s8 font, s8 hAlign, s8 vAlign, u32 fit, s32 *box);

// 0x492c40 (cdecl (text, a, c, d, left, top, right, bottom, rgb); name invented): the text drawn onto a surface of its own: the colour
// rgb's bytes (blue low) with alpha 0xff and the game palette's (0xfa71f8) nearest entry (Palette_Nearest), the GUI's SurfaceInfo
// (0x7ca0e8) filled as kind 3 from a, c, d (Format_FillInfoByKind), the surface flag 0x7ca120 set around it and the global style's +0x10
// on (Obj_SetOn10 / Obj_SetOff10), GuiText_Draw with font 0xc, left / top aligned, fit 1, no bounds, keepSurface 0. Its one caller
// is 0x4a8585.
void __cdecl GuiText_DrawRgb(const u16 *text, u32 a, u32 c, u32 d, s32 left, s32 top, s32 right, s32 bottom, u32 rgb);
}
