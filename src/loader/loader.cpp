// tpw.exe: the game without tp.exe's code (.docs/08-loader.md). The user's decrypted tp.exe is read as a FILE and its headers and sections
// are copied to their own addresses, 0x400000-0xfc6000, where the ports read and write the game's data as they do inside tp.exe. Nothing
// copied there runs: the code is data, read-only, and DEP makes a call into it fault; the fault handler takes it to the port.
//
// The image is fixed at 0x10000 with no relocations (CMakeLists.txt): its code ends below 0x400000 and its section .game (g_game, zero
// on load, after the code) spans the game's whole image. So the range is tpw.exe's own from the moment Windows maps the image, before any
// code of the process runs: no DLL, heap, stack or file mapping can be placed there. The entry point LoaderEntry runs before tpw.exe's own
// C runtime and C++ initialisers:
//   1. copies the exe in (CopyExe) and checks that every hooked function's first bytes are this exe's;
//   2. fills the exe's import table with the real DLLs' addresses (a missing DLL or function: Missing);
//   3. points each code slot of the data at its port (codeslots.h, generated at build time by tools/gen_loader_slots.py), and each pointer
//      into a global tpw.exe has as an object of its own at that object (kGameDataFixups, tools/gamedata.py); zeroes those globals' ranges;
//   4. installs the handler for calls into the copy (OnFault): the port's address, or a trap that names the original address and stops;
// then it starts the C runtime, whose WinMain runs the original's start-up (mainCRTStartup 0x67ee10) by the game's addresses.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <cstdarg>
#include <cstring>

#include "../game/core/addrcalls.h"
#include "../game/core/gamedata.h"
#include "../game/core/staticinit.h"
#include "../hook/hooks.h"
#include "codeslots.h"
#include "../game/core/imports.h"

// gamedata_object.cpp (tools/gamedata.py): the original's address and size of each global tpw.exe has as an object of its own, and the
// fixups for the pointers into them and for their code addresses.
extern "C" const u32 kGameDataRanges[][2];
extern "C" const u32 kGameDataRangeCount;
extern "C" const GameDataFixup kGameDataFixups[];
extern "C" const u32 kGameDataFixupCount;

namespace {

const u32 kBase = 0x00400000, kCode = 0x00401000, kCodeEnd = 0x006fc800;   // the game's image base; .text and cseg
const u32 kImageEnd = 0x00fc6000;                                            // its SizeOfImage 0xbc6000 past the base
const u32 kDebugPrintf = 0x005da3c0;                                         // Debug_Printf: a RET (in many vtables)

#pragma bss_seg(".game")
// (sized for code of no size, so it reaches past kImageEnd wherever the code ends.) shortcut: the linker puts the code first, so tpw.exe's
// code must stay under 0x3ef000 (0x35a000 in 2026-11, with /OPT:REF); past that LoaderEntry stops with a message, and the game's code then
// moves to a DLL the loader loads after the copy, tpw.exe keeping only the copy and g_game.
u8 g_game[kImageEnd - 0x00011000];
#pragma bss_seg()

template <typename T> T &G(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

[[noreturn]] void Fail(const char *format, ...) {
    char text[512];
    va_list args;
    va_start(args, format);
    wvsprintfA(text, format, args);
    va_end(args);
    OutputDebugStringA(text);
    MessageBoxA(nullptr, text, "tpw", MB_ICONERROR | MB_SETFOREGROUND);
    ExitProcess(3);
}

void Protect(u32 address, u32 size, DWORD protection) {
    DWORD was;
    if (size && !VirtualProtect((void *)(uintptr_t)address, size, protection, &was)) Fail("tpw: cannot protect %08X (%u bytes)", address, size);
}

// --- the exe ---------------------------------------------------------------------------------------------------------------------

char g_exe[MAX_PATH];   // tp.exe's path; its folder is where the game's own DLLs are looked for first

// --tp-exe=PATH (in quotes when it has spaces) on the command line, which is cut from the line before anything reads it (the game's
// parser would take it for a park file); otherwise tp.exe next to tpw.exe.
void FindExe() {
    char *const line = GetCommandLineA();
    if (char *const option = strstr(line, "--tp-exe=")) {
        char *value = option + 9, *end;
        if (*value == '"') {
            end = strchr(++value, '"');
            if (!end) end = value + strlen(value);
        } else {
            end = value + strcspn(value, " \t");
        }
        lstrcpynA(g_exe, value, (int)std::min<size_t>(end - value + 1, MAX_PATH));
        if (*end == '"') ++end;
        end += strspn(end, " \t");
        memmove(option, end, strlen(end) + 1);
        return;
    }
    GetModuleFileNameA(nullptr, g_exe, MAX_PATH);
    char *const slash = strrchr(g_exe, '\\');
    lstrcpyA(slash ? slash + 1 : g_exe, "tp.exe");
}

// The headers and sections at their addresses (each read from the file, its bounds checked); the rest of the image stays zero.
const IMAGE_NT_HEADERS32 &CopyExe() {
    const HANDLE file = CreateFileA(g_exe, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) Fail("tpw: cannot open %s (error %u). Give its path as --tp-exe=PATH, or put tpw.exe next to it.", g_exe, GetLastError());
    const DWORD size = GetFileSize(file, nullptr);
    u8 *const d = size && size != INVALID_FILE_SIZE ? static_cast<u8 *>(VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) : nullptr;
    DWORD got = 0;
    if (!d || !ReadFile(file, d, size, &got, nullptr) || got != size) Fail("tpw: cannot read %s", g_exe);
    CloseHandle(file);
    const auto *const dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(d);
    const u32 lfanew = size >= 0x400 && (u32)dos->e_lfanew <= 0x400 - sizeof(IMAGE_NT_HEADERS32) ? (u32)dos->e_lfanew : 0;
    const auto *const nt = reinterpret_cast<const IMAGE_NT_HEADERS32 *>(d + lfanew);
    const IMAGE_OPTIONAL_HEADER32 &opt = nt->OptionalHeader;
    if (size < 0x400 || dos->e_magic != IMAGE_DOS_SIGNATURE || lfanew == 0 || nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 || opt.ImageBase != kBase || opt.SizeOfImage > kImageEnd - kBase ||
        opt.SizeOfHeaders > 0x1000 || opt.SizeOfHeaders > size ||
        (u32)((const u8 *)(IMAGE_FIRST_SECTION(nt) + nt->FileHeader.NumberOfSections) - d) > opt.SizeOfHeaders)
        Fail("tpw: %s is not the game's exe (a 32-bit image at 0x400000 of at most 0xbc6000 bytes)", g_exe);
    std::memcpy((void *)(uintptr_t)kBase, d, opt.SizeOfHeaders);
    const IMAGE_SECTION_HEADER *const sections = IMAGE_FIRST_SECTION(nt);
    for (u32 i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        const IMAGE_SECTION_HEADER &s = sections[i];
        const u32 va = kBase + s.VirtualAddress, n = s.SizeOfRawData;
        if (s.PointerToRawData > size || n > size - s.PointerToRawData || s.VirtualAddress < kCode - kBase || s.VirtualAddress >= kImageEnd - kBase ||
            n > kImageEnd - va)
            Fail("tpw: %s's section %u lies outside the file or the image", g_exe, i);
        std::memcpy((void *)(uintptr_t)va, d + s.PointerToRawData, n);
    }
    VirtualFree(d, 0, MEM_RELEASE);
    return G<const IMAGE_NT_HEADERS32>(kBase + lfanew);   // (the copy's headers)
}

// --- the ports ---------------------------------------------------------------------------------------------------------------------

struct Port {
    u32 address, target;
};
Port *g_ports;   // kHooks by address
u32 g_portCount;

void __cdecl EmptyDebugPrintf() {}   // Debug_Printf 0x5da3c0 (empty in the retail build)

// Adjustor thunks (SUB ECX,[ECX-4] / SUB ECX,n / ADD ECX,n ; JMP f) that a code slot holds: made again at load, the same first
// instruction, then a jump to f's port. Executable, outside the copy.
struct Thunk {
    u32 address, code;
};
Thunk g_thunks[64];
u32 g_thunkCount;
u8 *g_thunkCode;

// The address to run for a call to the original's address, 0 when there is none: a hooked function's port, the empty Debug_Printf, a
// generated exit thunk (StaticInit_Exit), a remade adjustor thunk, or what a plain JMP there leads to.
u32 Resolve(u32 address) {
    for (int hop = 0; hop < 4; ++hop) {
        const Port *const p = std::lower_bound(g_ports, g_ports + g_portCount, address, [](const Port &e, u32 a) { return e.address < a; });
        if (p != g_ports + g_portCount && p->address == address) return p->target;
        if (address == kDebugPrintf) return (u32)(uintptr_t)&EmptyDebugPrintf;
        if (void(__cdecl *const exit)() = StaticInit_Exit(address)) return (u32)(uintptr_t)exit;
        for (u32 i = 0; i < g_thunkCount; ++i)
            if (g_thunks[i].address == address) return g_thunks[i].code;
        if (address < kCode || address > kCodeEnd - 5 || G<u8>(address) != 0xe9) return 0;
        address += 5 + G<u32>(address + 1);   // JMP rel32: its target
    }
    return 0;
}

// Test only: with TPW_TRACE=path set, each call by an original address is written there (one hex address a line, unbuffered so an
// ExitProcess keeps it), to find where a run stops.
HANDLE g_trace = INVALID_HANDLE_VALUE;

void Trace(u32 address) {
    char line[12];
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 8; ++i) line[i] = hex[address >> (28 - 4 * i) & 0xf];
    line[8] = '\r', line[9] = '\n';
    DWORD written;
    WriteFile(g_trace, line, 10, &written, nullptr);
}

u32 __cdecl ResolveOrSelf(u32 address) {   // AddrCalls' g_addressTarget: the address itself when there is no port (the call then traps)
    if (g_trace != INVALID_HANDLE_VALUE) Trace(address);
    const u32 target = Resolve(address);
    return target ? target : address;
}

u32 MakeThunk(u32 address) {
    if (address < kCode || address > kCodeEnd - 8) return 0;
    const u8 *const b = reinterpret_cast<const u8 *>((uintptr_t)address);
    const bool adjust = (b[0] == 0x2b && b[1] == 0x49 && b[2] == 0xfc) || (b[0] == 0x83 && (b[1] == 0xe9 || b[1] == 0xc1));
    const u32 target = adjust && b[3] == 0xe9 ? Resolve(address + 8 + G<u32>(address + 4)) : 0;
    if (!target || g_thunkCount == sizeof g_thunks / sizeof g_thunks[0]) return 0;
    u8 *const code = g_thunkCode + 8 * g_thunkCount;
    std::memcpy(code, b, 3);
    code[3] = 0xe9;
    const u32 rel = target - ((u32)(uintptr_t)code + 8);
    std::memcpy(code + 4, &rel, 4);
    g_thunks[g_thunkCount++] = {address, (u32)(uintptr_t)code};
    return (u32)(uintptr_t)code;
}

void MapPorts() {
    using tpw::hook::kHooks;
    u32 differ = 0;
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i)
        if (std::memcmp((const void *)(uintptr_t)kHooks[i].address, kHooks[i].expect, 5) != 0) ++differ;
    if (differ) Fail("tpw: %u hooked functions' first bytes differ from %s's: not the exe the ports were made for", differ, g_exe);
    g_portCount = (u32)tpw::hook::kHookCount;
    g_ports = static_cast<Port *>(HeapAlloc(GetProcessHeap(), 0, g_portCount * sizeof(Port)));
    if (!g_ports) Fail("tpw: out of memory");
    for (u32 i = 0; i < g_portCount; ++i) g_ports[i] = {kHooks[i].address, (u32)(uintptr_t)kHooks[i].replacement};
    std::sort(g_ports, g_ports + g_portCount, [](const Port &a, const Port &b) { return a.address < b.address; });

    g_thunkCode = static_cast<u8 *>(VirtualAlloc(nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!g_thunkCode) Fail("tpw: out of memory");
    auto toPort = [](u32 slot) {
        u32 &value = G<u32>(slot);
        u32 target = Resolve(value);
        if (!target) target = MakeThunk(value);
        if (!target) Fail("tpw: the code-slot table is stale (rebuild tpw): %08X holds %08X, which has no port", slot, value);
        value = target;
    };
    for (const u32 slot : kCodeSlots) toPort(slot);
    for (u32 i = 0; i < kGameDataFixupCount; ++i) {   // pointers into tpw.exe's own globals, and the code slots in them (gamedata.h)
        const GameDataFixup &f = kGameDataFixups[i];
        const u32 slot = f.object ? (u32)(uintptr_t)f.object + f.offset : f.offset;
        if (f.target)
            G<u32>(slot) = (u32)(uintptr_t)f.target;
        else
            toPort(slot);
    }
    Protect((u32)(uintptr_t)g_thunkCode, 0x1000, PAGE_EXECUTE_READ);
}

// --- imports -----------------------------------------------------------------------------------------------------------------------

// An import whose DLL or function is missing: its slot holds an address of a reserved, inaccessible block, so a call through it faults and
// OnFault answers for it. A member function (the EA Online wea*.dll's are all thiscall) fails as the DLL's own code reports failure:
// 0 (FALSE, no request), a constructor its object, the arguments its undecorated name gives popped. Anything else (a C name, an
// ordinal) stops with its name.
struct Missing {
    const char *dll, *name;
    u32 pops;
    bool known, constructor;
};
Missing g_missing[512];
u32 g_missingCount, g_missingBase;

u32 AddMissing(const char *dll, const char *name) {
    if (!g_missingBase) g_missingBase = (u32)(uintptr_t)VirtualAlloc(nullptr, 0x1000, MEM_RESERVE, PAGE_NOACCESS);
    if (!g_missingBase || g_missingCount == sizeof g_missing / sizeof g_missing[0]) Fail("tpw: cannot stand in for %s", dll);
    Missing &m = g_missing[g_missingCount];
    m = {dll, name, 0, false, false};
    using Undecorate = DWORD(WINAPI *)(PCSTR, PSTR, DWORD, DWORD);
    const HMODULE dbghelp = IS_INTRESOURCE(name) || name[0] != '?' ? nullptr : LoadLibraryA("dbghelp.dll");
    const auto undecorate = dbghelp ? reinterpret_cast<Undecorate>(GetProcAddress(dbghelp, "UnDecorateSymbolName")) : nullptr;
    char text[1024];
    if (undecorate && undecorate(name, text, sizeof text, 0)) {   // e.g. "public: int __thiscall Auth::Login(unsigned short,unsigned short *,unsigned short *,void *)"
        const char *const call = strstr(text, "__thiscall ");
        const char *const open = call ? strchr(call, '(') : nullptr;
        if (open) {
            u32 commas = 0, depth = 0;
            const char *p = open + 1;
            for (; *p && !(*p == ')' && depth == 0); ++p) {
                if (*p == '(' || *p == '<') ++depth;
                if (*p == ')' || *p == '>') --depth;
                if (*p == ',' && depth == 0) ++commas;
            }
            const bool none = std::strncmp(open + 1, "void)", 5) == 0 || p == open + 1;
            m.pops = none ? 0 : 4 * (commas + 1);   // (every argument here is 4 bytes: pointers, references, int, unsigned short, bool)
            m.known = *p == ')';
            m.constructor = std::strncmp(name, "??0", 3) == 0;
        }
    }
    return g_missingBase + g_missingCount++;
}

HMODULE Load(const char *dll) {   // as tp.exe from its folder: that folder first (QMixer, the wea DLLs, a DirectDraw wrapper), then the system
    char path[MAX_PATH];
    lstrcpynA(path, g_exe, MAX_PATH);
    char *const slash = strrchr(path, '\\');
    if (slash && slash + 1 - path + lstrlenA(dll) < MAX_PATH) {
        lstrcpyA(slash + 1, dll);
        if (const HMODULE m = LoadLibraryExA(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH)) return m;
    }
    return LoadLibraryA(dll);
}

// Each slot twice: in the copy (its code calls through it) and in gImports (the ports' IMPORT(member)), at the same offset into the table.
void FillImports(const IMAGE_NT_HEADERS32 &nt) {
    const IMAGE_DATA_DIRECTORY &dir = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    const IMAGE_DATA_DIRECTORY &iat = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT];
    if (iat.Size != sizeof(ImportTable)) Fail("tpw: tp.exe's import address table is %X bytes; gImports is %X", iat.Size, (u32)sizeof(ImportTable));
    u8 *const own = reinterpret_cast<u8 *>(&gImports);
    const u32 first = kBase + iat.VirtualAddress;
    for (auto *desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>((uintptr_t)kBase + dir.VirtualAddress); dir.VirtualAddress && desc->Name; ++desc) {
        const char *const dll = reinterpret_cast<const char *>((uintptr_t)kBase + desc->Name);
        const HMODULE module = Load(dll);
        const u32 *names = &G<u32>(kBase + (desc->OriginalFirstThunk ? desc->OriginalFirstThunk : desc->FirstThunk));
        for (u32 *slot = &G<u32>(kBase + desc->FirstThunk); *names; ++names, ++slot) {
            const char *const name = IMAGE_SNAP_BY_ORDINAL32(*names) ? MAKEINTRESOURCEA(IMAGE_ORDINAL32(*names))
                                                                     : reinterpret_cast<const char *>((uintptr_t)kBase + *names + 2);
            const FARPROC fn = module ? GetProcAddress(module, name) : nullptr;
            *slot = fn ? (u32)(uintptr_t)fn : AddMissing(dll, name);
            *reinterpret_cast<u32 *>(own + ((u32)(uintptr_t)slot - first)) = *slot;
        }
    }
}

// --- calls into the copy -----------------------------------------------------------------------------------------------------------

LONG CALLBACK OnFault(EXCEPTION_POINTERS *e) {
    const EXCEPTION_RECORD &r = *e->ExceptionRecord;
    CONTEXT &c = *e->ContextRecord;
    if (r.ExceptionCode != EXCEPTION_ACCESS_VIOLATION || r.NumberParameters < 2 || r.ExceptionInformation[0] != EXCEPTION_EXECUTE_FAULT)
        return EXCEPTION_CONTINUE_SEARCH;
    const u32 at = c.Eip, from = G<u32>(c.Esp);   // (from: the return address a CALL left)
    if (g_missingBase && at - g_missingBase < g_missingCount) {
        const Missing &m = g_missing[at - g_missingBase];
        if (!m.known) {
            if (IS_INTRESOURCE(m.name)) Fail("tpw: %s is missing: its ordinal %u was called (from %08X)", m.dll, (u32)(uintptr_t)m.name, from);
            Fail("tpw: %s is missing: %s was called (from %08X)", m.dll, m.name, from);
        }
        c.Eax = m.constructor ? c.Ecx : 0;   // RET pops
        c.Eip = from;
        c.Esp += 4 + m.pops;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (at < kBase || at >= kImageEnd) return EXCEPTION_CONTINUE_SEARCH;
    if (const u32 target = Resolve(at)) {
        c.Eip = target;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    Fail("tpw: no port for the original's %08X (called from %08X; build\\tpw.map names tpw.exe's addresses)", at, from);
}

}  // namespace

extern "C" int __cdecl WinMainCRTStartup(void *);

// The entry point (/ENTRY): before tpw.exe's C runtime, so that nothing of tpw.exe (C++ initialisers among it) sees the image before it is
// in place. (No /GS checks here: the C runtime sets the security cookie inside the call below, which does not return.)
extern "C" __declspec(safebuffers) int __cdecl LoaderEntry(void *peb) {
    const u32 lo = (u32)(uintptr_t)g_game, end = lo + (u32)sizeof g_game;   // (the section starts on a page)
    if (lo > kBase || end < kImageEnd) Fail("tpw: g_game does not cover 0x400000-0xfc6000; it starts at %08X", lo);
    DWORD dep = 0;
    BOOL permanent = FALSE;
    if (!GetProcessDEPPolicy(GetCurrentProcess(), &dep, &permanent) || !(dep & PROCESS_DEP_ENABLE))
        if (!SetProcessDEPPolicy(PROCESS_DEP_ENABLE)) Fail("tpw: DEP is off for this process, so the copy's code could run (error %u)", GetLastError());
    FindExe();
    const IMAGE_NT_HEADERS32 &nt = CopyExe();
    MapPorts();
    for (u32 i = 0; i < kGameDataRangeCount; ++i)   // the globals tpw.exe has as objects stay zero here: no port uses these addresses
        std::memset((void *)(uintptr_t)kGameDataRanges[i][0], 0, kGameDataRanges[i][1]);   // (after MapPorts: code slots in them are not stale)
    FillImports(nt);
    // The copy as tp.exe's sections are (read-only or read-write; never executable); no access around it, so a pointer past the game's
    // image faults as in tp.exe.
    Protect(lo, kBase - lo, PAGE_NOACCESS);
    Protect(kBase, kCode - kBase, PAGE_READONLY);
    const IMAGE_SECTION_HEADER *const sections = IMAGE_FIRST_SECTION(&nt);
    for (u32 i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
        const IMAGE_SECTION_HEADER &s = sections[i];
        const u32 size = (std::max<u32>(s.Misc.VirtualSize, s.SizeOfRawData) + 0xfff) & ~0xfffu;
        Protect(kBase + s.VirtualAddress, size, s.Characteristics & IMAGE_SCN_MEM_WRITE ? PAGE_READWRITE : PAGE_READONLY);
    }
    Protect(kImageEnd, end - kImageEnd, PAGE_NOACCESS);
    if (!AddVectoredExceptionHandler(1, &OnFault)) Fail("tpw: cannot install the fault handler");
    char tracePath[MAX_PATH];
    if (GetEnvironmentVariableA("TPW_TRACE", tracePath, MAX_PATH) - 1u < MAX_PATH - 1u)
        g_trace = CreateFileA(tracePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    g_addressTarget = &ResolveOrSelf;
    return WinMainCRTStartup(peb);
}

// The original's start-up, mainCRTStartup 0x67ee10, step by step: the C library's calls by the game's addresses (resolved to their ports;
// one with no port yet traps), _cinit 0x67f8f0 with StaticInit_Run for the C++ initialiser table, then WinMain and exit. Left out: its SEH
// frame (_except_handler3 0x67dbf4 with the C library's exception filter).
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    const AddrCalls calls;   // (no table: the loader's resolver)
    u16 cw = 0x27f;          // the control word a new process starts with, which the C library's _FPinit keeps (PC53); Direct3D sets
    __asm fldcw cw           // the game's 0x7f later
    const u32 version = G<u32(__stdcall *)()>(IMPORT(Kernel32_GetVersion))();   // GetVersion
    G<u32>(0x00fb1370) = version >> 8 & 0xff;                  // _osminor
    G<u32>(0x00fb136c) = version & 0xff;                       // _osmajor
    G<u32>(0x00fb1368) = ((version & 0xff) << 8) + (version >> 8 & 0xff);   // _winver
    G<u32>(0x00fb1364) = version >> 16;                        // _osver
    if (!calls.Cd(0x006826b0)) calls.Cd(0x0067eff0, 0x1c);     // _heap_init, else fast_error_exit(_RT_HEAPINIT)
    if (!calls.Cd(0x006823e0)) calls.Cd(0x0067eff0, 0x10);     // _mtinit, else fast_error_exit(_RT_THREAD)
    calls.Cd(0x00687230);                                      // _ioinit
    calls.Cd(0x0068a160);
    G<u32>(0x00fb3cb8) = G<u32(__stdcall *)()>(IMPORT(Kernel32_GetCommandLineA))();   // _acmdln = GetCommandLineA()
    G<u32>(0x00fb1354) = calls.Cd(0x00689b00);                  // _aenvptr = __crtGetEnvironmentStrings()
    if (!G<u32>(0x00fb1354) || !G<u32>(0x00fb3cb8)) calls.Cd(0x0067f920, -1);   // exit(-1)
    calls.Cd(0x00689850);                                      // _setargv
    calls.Cd(0x00689760);                                      // _setenvp
    if (const u32 fpinit = G<u32>(0x0077e718)) calls.Cd(fpinit);   // _cinit: _FPinit,
    calls.Cd(0x0067fa40, 0x0072e0ec, 0x0072e0fc);                  // the C initialisers (CRT_initterm),
    StaticInit_Run();                                              // the C++ ones (0x727000-0x72e0e8)
    const u8 *p = G<const u8 *>(0x00fb3cb8);   // the command line past the program's name
    if (*p == '"') {
        for (;;) {
            const u8 ch = *++p;
            if (ch == '"' || ch == 0) break;
            if (calls.Cd(0x00689700, (u32)ch)) ++p;   // _ismbblead
        }
        if (*p == '"') ++p;
    } else {
        while (*p > ' ') ++p;
    }
    while (*p != 0 && *p <= ' ') ++p;
    STARTUPINFOA startup;
    startup.dwFlags = 0;
    G<void(__stdcall *)(STARTUPINFOA *)>(IMPORT(Kernel32_GetStartupInfoA))(&startup);   // GetStartupInfoA
    const u32 show = startup.dwFlags & STARTF_USESHOWWINDOW ? startup.wShowWindow : SW_SHOWDEFAULT;
    // GetModuleHandleA(nullptr): tpw.exe's, a loaded module, as DirectInputCreateA wants; the game loads no resources through it.
    const u32 instance = G<u32(__stdcall *)(const char *)>(IMPORT(Kernel32_GetModuleHandleA))(nullptr);
    const u32 result = calls.St(0x0045a960, instance, 0u, (u32)(uintptr_t)p, show);   // Game_WinMain
    calls.Cd(0x0067f920, result);                                                    // exit
    return (int)result;
}
