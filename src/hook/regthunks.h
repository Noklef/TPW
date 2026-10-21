// Naked thunks for originals that take arguments in registers (hand-written assembly): the hook row of such a function
// points here, and the thunk pushes the registers (then any stack arguments) as the port's ordinary cdecl arguments.
// tools/integrate.py uses RegThunk_NAME for the row of NAME when it is declared here; tools/fncmp.py's register
// conventions (conv='esi', 'esiedi') call them as the game does. The originals clobber the registers their callers
// reload; a port keeps EBX ESI EDI EBP and leaves EAX ECX EDX as the C++ does.
#pragma once
extern "C" {
}
