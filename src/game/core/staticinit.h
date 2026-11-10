// The C++ static initialisers of tp.exe's CRT table __xc_a..__xc_z (0x727000-0x72e0e8) and the exit thunks they register with atexit:
// compiler-generated, so generated in turn by tools/gen_static_init.py (staticinit.cpp; two by hand in staticinitmanual.cpp). Not hooked:
// the game has run them before the hook DLL loads. The standalone loader runs StaticInit_Run where the CRT ran CRT_initterm (0x67f90d).
#pragma once

#include "addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// The table in its order, read live: a generated initialiser runs its C++, any other entry (a port's or the C library's address) is called
// by its address; a zero is skipped. As 0x67f90d: CRT_initterm(0x727000, 0x72e0e8).
void __cdecl StaticInit_Run();

// Runs the generated initialiser at the original's address; false when the address is not one.
bool __cdecl StaticInit_RunAt(u32 address);
bool __cdecl StaticInit_Has(u32 address);

// The C++ exit thunk of the original's at address (nullptr: not one), and back (0: not one).
void(__cdecl *__cdecl StaticInit_Exit(u32 address))();
u32 __cdecl StaticInit_ExitAddress(void(__cdecl *fn)());

// The calls by address (tools/fncmp's driver: recorders).
void __cdecl StaticInit_SetCalls(const AddrCall *table, u32 count);

// By hand (staticinitmanual.cpp): the initialisers the generator refuses.
void __cdecl StaticInit_412b90();
void __cdecl StaticInit_583540();

}  // extern "C"

const AddrCalls &StaticInit_Calls();   // (the generated module's, for the ones by hand)
