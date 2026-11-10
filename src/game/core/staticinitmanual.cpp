// The two static initialisers tools/gen_static_init.py cannot lift exactly (its MANUAL), by hand; staticinit.cpp's table runs them.
#include "staticinit.h"
#include "staticobjects.h"

#include <cstdint>

namespace {

template <typename T> T &M(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

// 0x412b90: the object at 0x789c18 cleared (+4, +8, +0xc) and its destroyer 0x412bc0 registered. Its first byte (an empty member) is copied
// from an uninitialised byte: the top byte of the ECX the original was entered with (PUSH ECX ; MOV AL,[ESP + 3]), which C++ cannot name;
// 0 here (sic: the original stores whatever byte CRT_initterm's caller left; nothing reads it).
void __cdecl StaticInit_412b90() {
    M<u8>(0x789c18) = 0;
    M<u32>(0x789c1c) = 0;
    M<u32>(0x789c20) = 0;
    M<u32>(0x789c24) = 0;
    StaticInit_Calls().Cd(0x0067a7d0, 0x412bc0u);   // CRT_atexit (0x412bc0: hooked, FontFiles' exit destructor)
}

// 0x583540: the object at 0x8bd530 constructed (0x55e470), the fields at 0x8bd508-0x8bd52c set, the machine's physical memory
// (GlobalMemoryStatus' dwTotalPhys, through the import slot 0x6fd148, into a MEMORYSTATUS on the stack) at 0x8bd520 and 0x55e570's answer for
// 1000 at 0x8bd51c.
void __cdecl StaticInit_583540() {
    const AddrCalls &calls = StaticInit_Calls();
    u32 status[8];   // MEMORYSTATUS (0x20 bytes: the original's SUB ESP,0x20)
    calls.Th(0x0055e470, (u32)(uintptr_t)&gCpuInfo_8bd530);
    M<u32>(0x8bd508) = 0x3000141;
    M<u32>(0x8bd518) = 0;
    M<u32>(0x8bd528) = 1;
    M<u32>(0x8bd52c) = 0;
    calls.St(M<u32>(0x6fd148), (u32)(uintptr_t)status);   // GlobalMemoryStatus
    M<u32>(0x8bd520) = status[2];   // (dwTotalPhys)
    const u32 answer = calls.Th(0x0055e570, (u32)(uintptr_t)&gCpuInfo_8bd530, 0x3e8u);
    M<u32>(0x8bd510) = 0;
    M<u32>(0x8bd514) = 0;
    M<u32>(0x8bd51c) = answer;
    M<u32>(0x8bd524) = 1;
    M<u32>(0x8bd50c) = 0xffffffff;
}

// 0x699cc0 (an exit thunk of the C++ library's 0x699c90): the object at 0xfb1dc8 (none: nothing) released (0x4ef7b0, Counted_Release),
// and the object it answers when unused deleted through its vtable's first slot (the deleting destructor, 1: free it).
void __cdecl StaticInit_699cc0() {
    const AddrCalls &calls = StaticInit_Calls();
    const u32 object = M<u32>(0xfb1dc8);
    if (!object) return;
    const u32 unused = calls.Th(0x004ef7b0, object);
    if (unused) calls.Th(M<u32>(M<u32>(unused)), unused, 1u);
}

}  // extern "C"
