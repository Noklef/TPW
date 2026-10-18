// The GUI library's text pools: two blocks of fixed-size rows of wide characters, a free row marked by a dword -1 at its start. The long
// pool: the block 0xfaa644, its row length 0xfaa648 (characters) and row count 0xfaa64c; the short pool: 0xfaa63c, 0xfaa640 and
// 0xfaa650. Reimplementation of 0x669440, 0x669586, 0x6695b9 and 0x66969e.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x669440 (cdecl (long length, long rows, short length, short rows); name unconfirmed): 0 when any is 0, the short length is 4 or less
// or the long one not above it (signed). Otherwise the four stored, both blocks allocated (Mem_Alloc, rows * length * 2 bytes); 0 when
// either failed. Each row of the long pool marked free, and as many rows (sic: the long pool's count) of the short one; 1.
u32 __cdecl TextPool_Init(s32 longLength, s32 longRows, s32 shortLength, s32 shortRows);

// 0x669586 (cdecl; name unconfirmed): both blocks freed (Mem_Free); the globals are left as they are.
void __cdecl TextPool_Free();

// 0x6695b9 (cdecl (length); name unconfirmed): 0 without both blocks. A length below the short pool's (signed) takes its first free
// row; otherwise, or when it has none, a length below the long pool's takes the long pool's last free one. The row's first character is
// set to 0 and the row returned; 0 when none.
u16 *__cdecl TextPool_Take(s32 length);

// 0x66969e (cdecl (row); name unconfirmed): the row (when not null) marked free again.
void __cdecl TextPool_Release(u8 *row);

// 0x6696b4 (cdecl (text); name unconfirmed): a row for the text's length (TextPool_Take(wcslen)) and, when there is one, the text
// copied into it (wcscpy; Take's rows are longer than the length, so the terminator fits); returns the row or 0.
u16 *__cdecl TextPool_Dup(const u16 *text);
}
