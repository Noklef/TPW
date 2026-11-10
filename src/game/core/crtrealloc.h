// The C library's small block heap, its freeing and resizing side (MSVC6's __sbh_find_block, __sbh_free_block, __sbh_resize_block and what frees pages), with realloc. The regions and pages are as in crtsbh.h.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x682b40: __sbh_find_block: the block's entry in its page, with its region and page; 0 when no region holds it, or it is not aligned to 16 or lies in the page's header. The test on the region's memory is > and < (sic).
void *__cdecl CRT_sbh_find_block(void *block, void **region, void **page);

// 0x682a10: __sbh_release_region: VirtualFree of its memory; unlinked and HeapFree'd, except the static region [0x77eda0], which only gets memory -1.
void __cdecl CRT_sbh_release_region(void *region);

// 0x682a70: __sbh_decommit_pages: up to count wholly free pages of the regions, backwards from [0x77eda4]; a region left all uncommitted is released. A region other than the first with memory -1 would spin forever (sic).
void __cdecl CRT_sbh_decommit_pages(s32 count);

// 0x682ba0: __sbh_free_block: the entry cleared and its paragraphs counted free; the 0x20th wholly free page decommits 0x10.
void __cdecl CRT_sbh_free_block(void *region, void *page, void *entry);

// 0x682fc0: __sbh_resize_block: to para paragraphs in place. 1: done. 0: no room, or the same size (sic).
s32 __cdecl CRT_sbh_resize_block(void *region, void *page, u8 *entry, u32 para);

// 0x681740: realloc: null: malloc; size 0: free. Under lock 9 a small block stays or moves to a small or heap one; otherwise HeapReAlloc. With the new mode [0xfb1548] a failure retries through _callnewh.
void *__cdecl CRT_realloc(void *block, u32 size);

// 0x6818f0: _msize: under lock 9 a small block's paragraphs * 16; otherwise HeapSize.
u32 __cdecl CRT_msize(void *block);
}
