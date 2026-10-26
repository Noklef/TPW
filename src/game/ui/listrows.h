// The row windows the GUI library's list controls (0x6639cb) are given factories and initialisers for: reimplementation of 0x480d30,
// 0x480dd0, 0x4a8380, 0x4a8420, 0x4b4c70, 0x4bacc0, 0x4bad70 and of the row factories 0x4938c0, 0x4944d0, 0x49bbb0, 0x49f0b0, 0x4a02b0,
// 0x4ad240, 0x4ae700, 0x496c10 and 0x4bcbc0 (named after the dword their height is cached in, or 0x4bcbc0 after its list's window), the
// jump 0x49c870 and the initialisers 0x49c880, 0x49f170, 0x4ae7d0, 0x4bcca0, 0x494690, 0x4939d0 and 0x496d20 of the same lists.
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x480d30 (cdecl, no arguments; the row factory of the "hilight" list built at 0x480b00): a window of 0x138 bytes (GameHeap_Alloc;
// 0 when that fails) built (Window_Ctor: no parent, flags 0, id 0, the rectangle (0, 0, 100, height)) with the height font 7's
// (Language_Font; its slot 1, a signed word) * 0x600 / the signed word [0xfaa5c0] + 6. The height is worked out before the allocation.
u8 *__cdecl HilightList_NewRow();

// 0x4a8380 (cdecl, no arguments; the row factory of the list of the window [0x7cc188]): the same, the height worked out only once the
// allocation succeeded.
u8 *__cdecl List7cc188_NewRow();

// 0x480dd0 / 0x4a8420 (cdecl (row); the row initialisers of the same two lists, the same code): the row's impl a new text part (0x1a4
// bytes, GameHeap_Alloc; TextPart_Ctor(0), or none when the allocation fails; Window_SetImpl), font 7 (Language_Font; Window_Set4c) and
// the four bytes 0xff (Window_Notify40000).
void __cdecl HilightList_InitRow(u8 *row);
void __cdecl List7cc188_InitRow(u8 *row);

// 0x4b4c70 (cdecl, no arguments; the row factory of the list built by 0x4b4a50, the child 0x1e7b of the window [0x7cc338]): the height
// worked out once (the dword [0x7cc35c] while 0: the font [0x756f38]'s height as above), then a window of 0x138 bytes built as above
// with the height's low word, and when there is one a child of it (Window_CreateChild: kind 9, flags 0x11, id 1, the rectangle
// (100, 0, 200, 50)). Returns the window or 0.
u8 *__cdecl List7cc338_NewRow();

// 0x4bacc0 (cdecl, no arguments; the row factory of the list child 0xc of the window [0x7cc530], built at 0x4ba980): as 0x480d30, then
// a child of the window (kind 6, flags 1, id 2, the same rectangle) whether the window was made or not (sic: 0 faults).
u8 *__cdecl List7cc530_NewRow();

// 0x4bad70 (cdecl (row); that list's row initialiser): as 0x480dd0, then the row's first child (+0x5c) given the object of the GUI
// window registered as "i_boxtick" (GuiWindows_FindByName; Window_SetObject98) and its slot 7 called with 0.
void __cdecl List7cc530_InitRow(u8 *row);

// The row factories below (cdecl, no arguments) all build a window of 0x138 bytes (GameHeap_Alloc; 0 when that fails; Window_Ctor: no
// parent, id 0, the rectangle (0, 0, right, height)) and give it children (Window_CreateChild (kind, flags, id, rectangle); the rectangle
// (0, 0, 0, 0) where not given). Their height, as 0x480d30's, is cached in a dword (worked out while it is 0, with the font index a
// global holds) and passed as its low word. Each returns the window or 0.

// 0x4938c0: the cache [0x7ca140] set to 0x38 whatever it held (sic: while 0 the font [0x7508b4]'s slot 1, its height, is called and the answer dropped);
// flags 0, right 0, height 0x38; when made, the children (1, 1, 3, (0, 0x2c, 0, 0x2c)), (1, 1, 0x3e7), (1, 0x11, 4), (6, 1, 4), (9, 0x11, 5).
u8 *__cdecl List7ca140_NewRow();

// 0x4944d0: the cache [0x7ca158] (font [0x750d54]); flags 0, right 0; the handler 0x4959c0 (Window_SetHandler; sic: the window not
// checked); when made, by the dword [0x7ca154]: 0: (1, 1, 4), (9, 0x11, 5), (9, 0x11, 6), (9, 0x11, 3); 1: (1, 1, 3), (1, 1, 4),
// (1, 1, 5), (9, 0x11, 6); 2: (1, 1, 2), (9, 0x11, 3), (1, 1, 4), (1, 1, 5), (9, 0x11, 6); 3: (1, 1, 4); none past 3.
u8 *__cdecl List7ca158_NewRow();

// 0x496c10 (with the initialiser 0x496d20, the list built at 0x496680): the cache [0x7ca170] (font [0x751184]); flags 0, right 0;
// when made, the children (1, 1, 0x3e7) twice (sic), (9, 0x11, 4), (9, 0x11, 5).
u8 *__cdecl List7ca170_NewRow();

// 0x49bbb0 (reached through the jump 0x49c870): the cache [0x7ca314] (font [0x7523dc]); flags 0, right 100; when made, the child
// (1, 1, 1, (100, 0, 200, 50)).
u8 *__cdecl List7ca314_NewRow();

// 0x49f0b0 (with the initialiser 0x49f170, the lists built at 0x49f05e and 0x49f3c1): the cache [0x7cb24c] (font 7); flags 0, right
// 100; the child (1, 1, 1, (0, 0, 100, height)) whether the window was made or not (sic).
u8 *__cdecl List7cb24c_NewRow();

// 0x4a02b0: the cache [0x7cb264] (font [0x752934]); flags 0, right 100; the children (1, 1, 2) to (1, 1, 6) whether the window was made
// or not (sic).
u8 *__cdecl List7cb264_NewRow();

// 0x4ad240: the cache [0x7cc200] (font [0x755134]); flags 0, right 100; when made, the handler 0x4acb70 (Window_SetHandler) for it and
// for its child (1, 1, 1, (100, 0, 200, 50)) (sic: that child not checked), then the child (6, 1, 2, (100, 0, 200, 50)).
u8 *__cdecl List7cc200_NewRow();

// 0x4ae700 (with the initialiser 0x4ae7d0): the cache [0x7cc250] (font [0x75583c]); flags 1, right 100; when made, the child
// (1, 1, 1, (100, 0, 200, height)).
u8 *__cdecl List7cc250_NewRow();

// 0x4bcbc0 (the list child 0xee8 of the window [0x7cc654], with the initialiser 0x4bcca0): the height of font 7 as 0x480d30's, worked
// out first, not cached; flags 0, right 100; when made, the children (1, 1, 1, (100, 0, 200, 50)) and (6, 1, 2, (100, 0, 200, 50)) twice
// (sic).
u8 *__cdecl List7cc654_NewRow();

// 0x49c870 (a jump to 0x49bbb0: the factory the list built at 0x49c0aa is given).
u8 *__cdecl List7ca314_NewRowJump();

// The initialisers below (cdecl (row)) give the row and its first child (+0x5c) new text parts (0x1a4 bytes, GameHeap_Alloc; none when
// the allocation fails; Window_SetImpl) and fonts (Language_Font; Window_Set4c); "as 0x480dd0" is 0x480dd0's TextPart_Ctor, font 7 and
// the four bytes 0xff (Window_Notify40000).

// 0x49c880 (with 0x49c870): the row a TextPart_Ctor part and the font [0x7523dc]; its first child a TextPart6ff3b0_Ctor part, the font
// [0x7523dc] (read again), the bytes 2, 1 as 0xc00 (Window_NotifyC00) and its slot 7 called with 0.
void __cdecl List7ca314_InitRow(u8 *row);

// 0x49f170 (with 0x49f0b0): the row and its first child as 0x480dd0, then the child's bytes 2, 1 as 0xc00.
void __cdecl List7cb24c_InitRow(u8 *row);

// 0x4ae7d0 (with 0x4ae700): the row a TextPart_Ctor part, the font [0x75583c] and the bytes 0xff; its first child a TextPart6ff410_Ctor
// part, the font [0x75583c] (read again), the bytes 2, 1 as 0xc00 and its slot 7 called with 0.
void __cdecl List7cc250_InitRow(u8 *row);

// 0x4bcca0 (with 0x4bcbc0): the row a TextPart_Ctor part, the bytes 0xff, font 7 and the text 0 (Window_NotifyImpl100); its first child
// a TextPart_Ctor part, the bytes 1, 1 as 0xc00 and as 0x3000 (Window_Notify3000), font 7, the bytes 0xff and the text 0; that child's
// next two siblings (+0x54, then theirs) given the object of the GUI window registered as "i_boxtick" (Window_SetObject98).
void __cdecl List7cc654_InitRow(u8 *row);

// 0x4c02d0 (the list child 0xe05b of the window [0x7cc944], with the initialiser 0x4c03e0): the cache [0x7cc94c] (font 7); flags 0,
// right 0; the children (1, 1, 2) to (1, 1, 6) whether the window was made or not (sic).
u8 *__cdecl List7cc94c_NewRow();

// 0x4c1b30 (the list child 0x14beb of the window [0x7cca70], with the initialiser 0x4c1c30): the cache [0x7cca90] (font 7); flags 0,
// right 0; the children (1, 1, 2) to (1, 1, 5) whether the window was made or not (sic).
u8 *__cdecl List7cca90_NewRow();

// 0x4c03e0 (with 0x4c02d0): the row a TextPart_Ctor part, font 7 and the text 0 (Window_NotifyImpl100); its first child and that
// child's next sibling (+0x54) the same and the bytes 1, 1 as 0xc00; the next a TextPart6ff500_Ctor part as its object +0x98
// (Window_SetObject98), font 7 and the bytes 1, 1; the next two each a TextPart6ff350_Ctor part, font 7, slot 7 called with 0 and the
// bytes 1, 1.
void __cdecl List7cc94c_InitRow(u8 *row);

// 0x4c1c30 (with 0x4c1b30): as 0x4c03e0 with one TextPart_Ctor child fewer, and that one (the first) not told the bytes 1, 1.
void __cdecl List7cca90_InitRow(u8 *row);

// 0x4ad340 (with 0x4ad240): the row a TextPart_Ctor part, the font [0x755134] and the bytes 0xff; its first child a TextPart6ff3e0_Ctor
// part, the font [0x755134] (read again), the bytes 2, 1 as 0xc00 and its slot 7 called with 0; that child's next sibling given the
// object of the GUI window registered as "i_boxtick" (Window_SetObject98) and its slot 7 called with 0.
void __cdecl List7cc200_InitRow(u8 *row);

// 0x4a03d0 (with 0x4a02b0): the row a TextPart_Ctor part, the font [0x752934] and the handler 0x4a0710 (Window_SetHandler); then its
// first child and the next four siblings, each given the handler 0x4a0710 first, then a part (the kinds 0x6ff3b0, 0x6ff350, 0x6ff440,
// 0x6ff3b0, 0x6ff3b0), the font [0x752934] (read each time), its slot 7 called with 0 and the bytes as 0xc00: 2, 1 for the 0x6ff3b0
// parts (told before the font), 1, 1 for the others (told last).
void __cdecl List7cb264_InitRow(u8 *row);

// The initialisers below read their font index (Language_Font) from their dword each time; a bar (ui/panelparts) is a child whose
// rectangle is inset (Window_SaveInsetRect, by 0 and 10 unless said) and whose impl is a MeshOverlay_Construct mesh of 0x124 bytes
// with the .wct name (none when the allocation fails).
// 0x494690 (with 0x4944d0): the row a TextPart_Ctor part, the font [0x750d54] and the text 0 (Window_NotifyImpl100); its first child
// (read before the state) then, with the dword [0x7ca154] 0-3 (nothing more past 3), a TextPart6ff350_Ctor part, the font, its slot 7
// called with 0 and the bytes 1, 1 as 0xc00; then its next siblings by the state, each with its slot 7 called with 0 last: 0: three
// ridestatbar bars; 1: two 0x6ff3b0 parts (the bytes 2, 1 as 0xc00, then the font) and a bar; 2: a bar, two such parts, a bar; 3: none.
void __cdecl List7ca158_InitRow(u8 *row);

// 0x4b4d40 (with List7cc338_NewRow; name unconfirmed): the row a TextPart_Ctor part, the font [0x756f38] and the text 0; its first
// child a ridestatbar bar with its slot 7 called with 0.
void __cdecl List7cc338_InitRow(u8 *row);

// 0x4939d0 (with 0x4938c0): the row a TextPart6ff470_Ctor part, the font [0x7508b4] and the bytes 1, 1 as 0xc00; its first child a
// 0x6ff3b0 part, the font, slot 7 with 0 and the bytes 2, 1; the next a 0x6ff4a0 part, the font and the bytes 1, 1; the next a 0x6ff350
// part, the font and slot 7 with 0; the next inset by 4 and 0 and given the GUI window registered as "ti_hungry" (GuiWindows_FindByName,
// or 0) as its impl (Window_SetImpl, sic); the next a happygrad bar inset by 0 and 8.
void __cdecl List7ca140_InitRow(u8 *row);

// 0x496d20 (with 0x496c10): the row a TextPart_Ctor part, the font [0x751184] and the text 0; its first child the same; the next a
// 0x6ff3b0 part, the bytes 2, 1 as 0xc00, the font and slot 7 with 0; the next a ridestatbar bar inset by 0 and 8 with slot 7 given
// 0x1f4; the next a happygrad bar inset by 0 and 8.
void __cdecl List7ca170_InitRow(u8 *row);
}
