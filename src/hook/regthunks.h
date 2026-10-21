// Naked thunks for originals that take arguments in registers (hand-written assembly): the hook row of such a function
// points here, and the thunk pushes the registers (then any stack arguments) as the port's ordinary cdecl arguments.
// tools/integrate.py uses RegThunk_NAME for the row of NAME when it is declared here; tools/fncmp.py's register
// conventions (conv='esi', 'esiedi') call them as the game does. The originals clobber the registers their callers
// reload; a port keeps EBX ESI EDI EBP and leaves EAX ECX EDX as the C++ does.
#pragma once

extern "C" {

void RegThunk_DctIdct_Column();     // 0x6780c1: block in ESI
void RegThunk_DctIdct_Row();        // 0x678224: block in ESI
void RegThunk_MovieIdct_Column();   // 0x67673c: block in ESI, out in EDI
void RegThunk_MovieIdct_Row();      // 0x67687b: block in ESI, out on the stack
void RegThunk_MovieIdct_RowShift(); // 0x676965: block in ESI, out in EDI
void RegThunk_TgqVideo_PixelRow16(); // 0x673574: luma in ESI, out in EDI, chroma in EBX
void RegThunk_TgqVideo_PixelRow32(); // 0x6732e9: luma in ESI, out in EDI, chroma in EBX
void RegThunk_TgqVideo_DecodeCoefficients(); // 0x672e60: the counter left in EDI
}
