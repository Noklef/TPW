// The C library's small block heap, its allocating side (MSVC6's __sbh_alloc_*), with calloc and _callnewh. Regions of 0x400 groups of pages: a circular list from 0x77eda0, the rover [0x780dc0].
#pragma once

#include "../../../re/types.h"

extern "C" {

// A region: +0 next, +4 previous, +8 the rover of its groups, +0xc the first group with uncommitted pages, +0x10 its memory (-1: none), +0x14 that memory's end,
// +0x18 0x400 groups of (paragraphs free, or -1 for uncommitted; a bound on the free run that may be found, u32).
// A page: +0 where to start, +4 the paragraphs free there, +8 0xf0 bytes (0: free; else the block's paragraphs at its first), +0xf8 0xff, +0x100 the blocks.

// 0x6826b0: _heap_init: HeapCreate(0, 0x1000, 0) to [0xfb3cac] and the first region; without one the heap destroyed. 1, or 0.
s32 __cdecl CRT_heap_init();

// 0x6828a0: __sbh_alloc_new_region: the static region first when its memory is -1; 4 MB reserved, the first 16 pages committed and made free. Fails: 0.
void *__cdecl CRT_sbh_alloc_new_region();

// 0x682e40: __sbh_alloc_block_from_page: para paragraphs from the page, from its start on and then from its first byte up to that start; free counts down from free_paragraphs. None: 0.
void *__cdecl CRT_sbh_alloc_block_from_page(u8 *page, u32 free_paragraphs, u32 para);

// 0x682c00: __sbh_alloc_block: from the rover's group of the rover's region, the groups before, the regions after; then newly committed pages (up to 16), then a new region. None: 0.
void *__cdecl CRT_sbh_alloc_block(u32 para);

// 0x688710: _callnewh: 1 when the handler [0xfb154c] is set and answers nonzero.
s32 __cdecl CRT_callnewh(u32 size);

// 0x67fcb0: calloc: size rounded up to 16 (0: 16); to [0x780dc4] from the small block heap, zeroed, under lock 9; otherwise or when it fails HeapAlloc(HEAP_ZERO_MEMORY); with [0xfb1548], _callnewh and again.
void *__cdecl CRT_calloc(u32 count, u32 size);

// 0x67dd40: _heap_alloc: size rounded up to 16; to [0x780dc4] from the small block heap under lock 9; otherwise or when it fails HeapAlloc([0xfb3cac], 0, rounded).
void *__cdecl CRT_heap_alloc(u32 size);
// 0x67dcf0: _nh_malloc: past 0xffffffe0 0; 0 as 1; _heap_alloc, and while it fails with mode set and _callnewh answers, again.
void *__cdecl CRT_nh_malloc(u32 size, s32 mode);

}
