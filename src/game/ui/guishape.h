// The GUI library's other shapes (over the base 0x705938, beside the rectangle of guirect.h; the windows' hit areas, as the window scripts build
// them): the circle (vtable 0x705b50: centre x +4, y +6, radius +8), the ellipse (0x705b68: centre x +4, y +6, radii +8 across and +0xa down)
// and the polygon (0x705b80: the point count, a word, at +4 and the points, pairs of words x, y in a block (Mem_Alloc) at +8). Their vtables
// follow the rectangle's: slot 0 contains, 1 the centre (the base's, which does nothing), 2 offset, 3 adjust (nothing), 4 the scalar deleting
// destructor, 5 the base's 0. Reimplementation of 0x65a9f0 and 0x66c01a-0x66c7b0. Names unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65a9f0 (thiscall (flags), pops 4; the base's slot 4): the base destructor (Fn_66bff7), then the shape freed (Mem_Free) when flags bit 0 is set;
// returns the shape.
u8 *__fastcall GuiShape_DeletingDtor(u8 *shape, void *, u32 flags);

// 0x66c01a / 0x66c054 (thiscall; the second (x, y, radius: words), pops 12): the base constructor (Fn_65a9d0), the vtable 0x705b50, the three
// words 0 / as given; returns the circle.
u8 *__fastcall GuiCircle_Ctor(u8 *circle);
u8 *__fastcall GuiCircle_CtorWith(u8 *circle, void *, u32 x, u32 y, u32 radius);

// 0x66c0c4 (thiscall (x, y: words), pops 8; slot 0): 1 in AX when dx * dx + dy * dy < radius * radius (signed, from the point to the centre), else 0.
// Only AX is set.
u32 __fastcall GuiCircle_Contains(u8 *circle, void *, u32 x, u32 y);

// 0x66c125 (thiscall (dx, dy: words), pops 8; slot 2): the centre moved. The ellipse's is the same code (0x66c2a1).
void __fastcall GuiCircle_Offset(u8 *circle, void *, u32 dx, u32 dy);
void __fastcall GuiEllipse_Offset(u8 *ellipse, void *, u32 dx, u32 dy);

// 0x66c163 / 0x66c1a6 (thiscall; the second (x, y, radius across, radius down: words), pops 16): as the circle's with the vtable 0x705b68 and four
// words.
u8 *__fastcall GuiEllipse_Ctor(u8 *ellipse);
u8 *__fastcall GuiEllipse_CtorWith(u8 *ellipse, void *, u32 x, u32 y, u32 rx, u32 ry);

// 0x66c22c (thiscall (x, y: words), pops 8; slot 0): dx scaled to a circle, (x - centre x) * ry / rx (a signed division, which faults on rx 0),
// then 1 (in EAX) when dx * dx + dy * dy < ry * ry, else 0.
u32 __fastcall GuiEllipse_Contains(u8 *ellipse, void *, u32 x, u32 y);

// 0x66c760 (thiscall (flags), pops 4; the circle's and the ellipse's slot 4): GuiCircle_Dtor, then the shape freed (Mem_Free) when flags bit 0 is set;
// returns the shape.
u8 *__fastcall GuiCircle_DeletingDtor(u8 *circle, void *, u32 flags);

// 0x66c790 (thiscall): the base destructor (Fn_66bff7), whose EAX is left; the vtable is not set first (sic).
u32 __fastcall GuiCircle_Dtor(u8 *circle);

// 0x66c2df (thiscall): the base constructor, the vtable 0x705b80, no points (+8 null, count 0); returns the polygon.
u8 *__fastcall GuiPolygon_Ctor(u8 *polygon);

// 0x66c311 / 0x66c3cb (thiscall (three / four points: x, y words), pops 24 / 32): the polygon of 3 / 4 points, its block (Mem_Alloc 0xc / 0x10, not
// checked) filled from the words; returns the polygon.
u8 *__fastcall GuiPolygon_CtorTriangle(u8 *polygon, void *, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2);
u8 *__fastcall GuiPolygon_CtorQuad(u8 *polygon, void *, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3);

// 0x66c4a1 (thiscall (count: a word, points: pairs of words), pops 8): the polygon of count points, its block (Mem_Alloc of the count, signed, times 4;
// not checked) filled from the array (none for a count below 1); returns the polygon.
u8 *__fastcall GuiPolygon_CtorPoints(u8 *polygon, void *, u32 count, const u16 *points);

// 0x66c568 (thiscall): the vtable 0x705b80, the block freed (Mem_Free) when there is one, then the base destructor (Fn_66bff7), whose EAX is left.
u32 __fastcall GuiPolygon_Dtor(u8 *polygon);

// 0x66c5a4 (thiscall (x, y: words), pops 8; slot 0): with points (count not 0, block not null) the crossing test: for each edge from the previous
// point (the last first) whose ends lie on either side of y (y_end >= y differs), the inside flag flips when ((y_cur - y) * (x_prev - x_cur) >=
// (x_cur - x) * (y_prev - y_cur)) equals the current end's side; 1 / 0 in AX. Only AX is set. (A negative count runs on through memory, sic.)
u32 __fastcall GuiPolygon_Contains(u8 *polygon, void *, u32 x, u32 y);

// 0x66c6c3 (thiscall (dx, dy: words), pops 8; slot 2): with points, each point moved.
void __fastcall GuiPolygon_Offset(u8 *polygon, void *, u32 dx, u32 dy);

// 0x66c7b0 (thiscall (flags), pops 4; slot 4): GuiPolygon_Dtor, then the polygon freed (Mem_Free) when flags bit 0 is set; returns the polygon.
u8 *__fastcall GuiPolygon_DeletingDtor(u8 *polygon, void *, u32 flags);
}
