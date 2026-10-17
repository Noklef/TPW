// tpw_fncmp: calls any hooked reimplementation by its original address, for tools/fncmp.py
// to compare against the original code under tools/x87emu.py.
//
// usage: tpw_fncmp.exe path\to\tp.exe   then one case per stdin line:
//   ADDRESS CONV RET NARGS ARG... NREGIONS (REGION_ADDRESS HEX)...
// (all hex; CONV c = cdecl, s = stdcall, t = thiscall (first arg in ECX), f = fastcall
// (ECX, EDX); RET d = also fstp the double the function leaves in st0). Prints per case
//   EAX EDX ST0 REGION_HEX...     or "crash CODE" / "nohook".
//
// Memory is laid out as the emulator has it: the game's sections (all but .text) sit at their
// own addresses, reloaded from tp.exe before each case, and any other region is mapped on
// demand and zeroed before each case. The driver is fixed at 0x400000 like the game, so its
// own code covers the game's .text; g_game, a .bss array, is placed to cover the rest.
//
// Each case runs with the x87 at single precision, as the game runs it (Direct3D sets it),
// so that float code the compiler leaves on the x87 rounds there as it will in the game.
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "../../src/game/core/gameheap.h"
#include "../../src/game/core/ptrlist.h"
#include "../../src/game/core/stdstring.h"
#include "../../src/game/world/mousemode.h"
#include "crt.h"
#include "../../src/hook/hooks.h"

namespace {

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;

const u32 kGameLow = 0x006e7000, kGameHigh = 0x00fc5200;  // cseg .. .rsrc
u8 g_game[0xb00000];

struct Section {
    u32 address;
    std::vector<u8> raw;
    u32 size;  // virtual size; past raw is zero
};
std::vector<Section> g_sections;
std::set<u32> g_chunks;

void fail(const char *what, u32 value) {
    std::printf("fncmp: %s %08x\n", what, value);
    std::exit(3);
}

void load_exe(const char *path) {
    FILE *f = std::fopen(path, "rb");
    if (!f) fail("cannot open exe", 0);
    std::vector<u8> d;
    u8 buf[65536];
    for (size_t n; (n = std::fread(buf, 1, sizeof buf, f)) > 0;) d.insert(d.end(), buf, buf + n);
    std::fclose(f);
    auto rd16 = [&](u32 o) { return (u32)(d[o] | d[o + 1] << 8); };
    auto rd32 = [&](u32 o) { return rd16(o) | rd16(o + 2) << 16; };
    const u32 pe = rd32(0x3c), count = rd16(pe + 6), opt = rd16(pe + 20), base = rd32(pe + 52);
    for (u32 i = 0; i < count; ++i) {
        const u32 s = pe + 24 + opt + 40 * i;
        const u32 vsize = rd32(s + 8), va = base + rd32(s + 12), rsize = rd32(s + 16), raw = rd32(s + 20);
        if (va < kGameLow) continue;  // .text: the driver's own code lives there
        Section sec{va, {}, vsize};
        sec.raw.assign(d.begin() + raw, d.begin() + raw + (rsize < vsize ? rsize : vsize));
        g_sections.push_back(sec);
    }
    const u32 lo = (u32)(uintptr_t)g_game;
    if (lo > kGameLow || lo + sizeof g_game < kGameHigh) fail("g_game does not cover the game's data; it starts at", lo);
}

void reset_memory() {
    for (const Section &s : g_sections) {
        u8 *p = reinterpret_cast<u8 *>((uintptr_t)s.address);
        std::memcpy(p, s.raw.data(), s.raw.size());
        std::memset(p + s.raw.size(), 0, s.size - s.raw.size());
    }
    for (u32 c : g_chunks) std::memset(reinterpret_cast<void *>((uintptr_t)c), 0, 0x10000);
}

void map(u32 addr, u32 size) {
    const u32 lo = (u32)(uintptr_t)g_game;
    if (addr >= lo && addr + size <= lo + sizeof g_game) return;
    if (addr < kGameLow) fail("region inside the driver's image", addr);
    for (u32 p = addr & ~0xffffu; p < addr + size; p += 0x10000) {  // allocation granularity
        if (g_chunks.count(p)) continue;
        if (!VirtualAlloc((void *)(uintptr_t)p, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) fail("cannot map", p);
        g_chunks.insert(p);
    }
}

// Stubs for what a case hands the code under test, at fixed addresses that tools/fncmp.py's
// emulator hooks too: a record stream's read (vtable +8) and write (+0xc) at kStubs and
// kStubs + 0x10. The stream is +4 cursor, +8 bytes left; each copies at most what is left
// from or to the cursor, moves it on and returns the count.
const u32 kStubs = 0x2f000000;

s32 __fastcall stub_read(u8 *self, void *, u8 *buffer, u32 count) {
    u32 &cursor = *reinterpret_cast<u32 *>(self + 4), &left = *reinterpret_cast<u32 *>(self + 8);
    const u32 n = count < left ? count : left;
    std::memmove(buffer, reinterpret_cast<const void *>((uintptr_t)cursor), n);
    cursor += n;
    left -= n;
    return (s32)n;
}

s32 __fastcall stub_write(u8 *self, void *, const u8 *buffer, u32 count) {
    u32 &cursor = *reinterpret_cast<u32 *>(self + 4), &left = *reinterpret_cast<u32 *>(self + 8);
    const u32 n = count < left ? count : left;
    std::memmove(reinterpret_cast<void *>((uintptr_t)cursor), buffer, n);
    cursor += n;
    left -= n;
    return (s32)n;
}

// The heap a case hands the code (tools/fncmp.py heap(), emulated the same way there): a bump
// allocator whose state is the head of its region at kHeap, +0 next block, +4 a hash of the
// sizes asked for, +8 frees, +0xc a hash of the freed pointers. Blocks are 16-byte aligned;
// nothing is reused.
const u32 kHeap = 0x2e000000;

void *__cdecl heap_alloc(u32 size) {
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    const u32 p = h[0];
    h[0] = p + ((size + 15) & ~15u);
    h[1] = h[1] * 33 ^ size;
    return reinterpret_cast<void *>((uintptr_t)p);
}

void __cdecl heap_free(void *block) {
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    ++h[2];
    h[3] = h[3] * 33 ^ (u32)(uintptr_t)block;
}

// GlobalAlloc / GlobalFree (stdcall) for code that calls through the game's import slots
// (tools/fncmp.py global_heap() points them here, at kStubs + 0x20 and + 0x30): the same heap;
// GlobalAlloc also hashes the flags in and zeroes the block for GMEM_ZEROINIT (0x40).
void *__stdcall stub_global_alloc(u32 flags, u32 size) {
    u8 *p = static_cast<u8 *>(heap_alloc(size));
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    h[1] = h[1] * 33 ^ flags;
    if (flags & 0x40) std::memset(p, 0, size);
    return p;
}

void *__stdcall stub_global_free(void *block) {
    heap_free(block);
    return nullptr;
}

// A constructor / destructor for code that calls one through a pointer (kStubs + 0x40, tools/fncmp.py STUBS + 0x40): thiscall, no other arguments;
// puts a vtable pointer (0x6fd800) in the first dword of the object.
void __fastcall stub_mark(u32 *self) { *self = 0x006fd800; }

u32 __cdecl record_call(u32 k, u32 n, u32 self, const u32 *stack);

// A callback the code under test calls through a function pointer, cdecl with one argument (kStubs + 0x50, tools/fncmp.py CDECL_LOG): logged as the COM-style entry k 0x13f.
u32 __cdecl stub_cdecl_log(u32 arg) {
    const u32 stack[2] = {0, arg};
    return record_call(0x13f, 1, 0, stack);
}

// What throws in the game (std::_Xlen, _Xran): a case that gets there reports a crash.
void __cdecl heap_throw() { RaiseException(0xe0000001, 0, 0, nullptr); }

const StdStringEnv kStdStringEnv = {&heap_alloc, &heap_free, &heap_throw, &heap_throw};
const MouseModeEnv kMouseModeEnv = {&heap_free};
const PtrListEnv kPtrListEnv = {&heap_alloc};
const GameHeapEnv kGameHeapEnv = {&heap_alloc, &heap_free};

// The recorders (tools/fncmp.py recorder(k, n)): thunks at kStubs + 0x1000 + 0x400 * n + 0x20 * k, thiscall with n stack arguments (popped by the thunk), that
// log (k, n, this, the arguments) at kLog and return a hash of them. The log is a count, then 12 dwords an entry; room for 80.
const u32 kLog = kStubs + 0x100000;  // (the case maps it)

u32 __cdecl record_call(u32 k, u32 n, u32 self, const u32 *stack) {
    u32 *log = reinterpret_cast<u32 *>((uintptr_t)kLog);
    const u32 *args = stack + 1;  // (stack[0] is the thunk's own return address)
    if (log[0] < 40) {
        u32 *entry = log + 1 + 12 * log[0];
        entry[0] = k;
        entry[1] = n;
        entry[2] = self;
        for (u32 i = 0; i < 7; ++i) entry[3 + i] = i < n ? args[i] : 0;
        entry[10] = entry[11] = 0;
        ++log[0];
    }
    u32 h = self * 0x9e3779b1u + (k + 1) * 0x10001u + n;
    for (u32 i = 0; i < n; ++i) h = h * 33 ^ args[i];
    const u32 *override = log + 0x200 + 2 * (k & 63);  // (LOG_RETURNS: a flag and a value for the slot)
    return override[0] ? override[1] : h;
}

void map_com_recorders(u8 *page) {  // the COM-style ones (this on the stack, ECX undefined): this is logged as 0 and k as k | 0x100
    for (u32 n = 0; n < 8; ++n) {
        for (u32 k = 0; k < 64; ++k) {
            u8 *at = page + 0x5000 + 0x800 * n + 0x20 * k;
            const u8 head[] = {0x89, 0xe0, 0x50, 0x6a, 0x00, 0x6a, (u8)n, 0x68, (u8)k, 0x01, 0x00, 0x00, 0xe8};  // mov eax,esp; push eax; push 0; push n; push k|0x100; call rel32
            std::memcpy(at, head, sizeof head);
            const u32 rel = (u32)(uintptr_t)&record_call - ((u32)(uintptr_t)at + sizeof head + 4);
            std::memcpy(at + sizeof head, &rel, 4);
            u8 *tail = at + sizeof head + 4;
            tail[0] = 0x83, tail[1] = 0xc4, tail[2] = 0x10;  // add esp,16
            if (n) {
                tail[3] = 0xc2, tail[4] = (u8)(4 * n), tail[5] = 0;  // ret 4n
            } else {
                tail[3] = 0xc3;
            }
        }
    }
}

void map_recorders(u8 *page) {
    map_com_recorders(page);
    for (u32 n = 0; n < 16; ++n) {  // (up to 15 arguments: the layout ends where the COM recorders begin)
        for (u32 k = 0; k < 32; ++k) {
            u8 *at = page + 0x1000 + 0x400 * n + 0x20 * k;
            const u8 head[] = {0x89, 0xe0, 0x50, 0x51, 0x6a, (u8)n, 0x6a, (u8)k, 0xe8};  // mov eax,esp; push eax; push ecx; push n; push k; call rel32
            std::memcpy(at, head, sizeof head);
            const u32 rel = (u32)(uintptr_t)&record_call - ((u32)(uintptr_t)at + sizeof head + 4);
            std::memcpy(at + sizeof head, &rel, 4);
            u8 *tail = at + sizeof head + 4;
            tail[0] = 0x83, tail[1] = 0xc4, tail[2] = 0x10;  // add esp,16
            if (n) {
                tail[3] = 0xc2, tail[4] = (u8)(4 * n), tail[5] = 0;  // ret 4n
            } else {
                tail[3] = 0xc3;
            }
        }
    }
}

void map_stubs() {
    u8 *p = static_cast<u8 *>(VirtualAlloc((void *)(uintptr_t)kStubs, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    if (!p) fail("cannot map the stubs at", kStubs);
    const void *stubs[] = {(const void *)&stub_read, (const void *)&stub_write, (const void *)&stub_global_alloc,
                           (const void *)&stub_global_free, (const void *)&stub_mark, (const void *)&stub_cdecl_log};
    for (u32 i = 0; i < 6; ++i) {
        u8 *at = p + 0x10 * i;
        at[0] = 0xe9;  // jmp rel32
        const u32 rel = (u32)(uintptr_t)stubs[i] - ((u32)(uintptr_t)at + 5);
        std::memcpy(at + 1, &rel, 4);
    }
    map_recorders(p);
}

void *find(u32 address) {
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i)
        if (tpw::hook::kHooks[i].address == address) return tpw::hook::kHooks[i].replacement;
    for (u32 i = 0; i < kCrtStandInCount; ++i)
        if (kCrtStandIns[i].address == address) return kCrtStandIns[i].fn;
    return nullptr;
}

// Pushes the stack arguments right to left, sets ECX/EDX, calls, and restores ESP whatever
// the convention popped.
int invoke(void *fn, u32 ecxIn, u32 edxIn, const u32 *stack, u32 count, int wantSt0, u32 *eax, u32 *edx,
           double *st0) {
    u32 ra = 0, rd = 0, saved = 0;
    double f = 0;
    __try {
        __asm {
            mov saved, esp
            mov esi, stack
            mov ecx, count
        next:
            test ecx, ecx
            jz pushed
            dec ecx
            push dword ptr [esi + ecx * 4]
            jmp next
        pushed:
            mov ecx, ecxIn
            mov edx, edxIn
            call fn
            mov ra, eax
            mov rd, edx
            mov esp, saved
            cmp wantSt0, 0
            je done
            fstp f
        done:
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        __asm fninit
        return (int)GetExceptionCode();
    }
    *eax = ra;
    *edx = rd;
    *st0 = f;
    return 0;
}

std::vector<u8> unhex(const char *s) {
    std::vector<u8> out(std::strlen(s) / 2);
    for (std::size_t i = 0; i < out.size(); ++i) {
        unsigned v;
        std::sscanf(s + 2 * i, "%2x", &v);
        out[i] = (u8)v;
    }
    return out;
}

}  // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::printf("usage: tpw_fncmp tp.exe < cases\n");
        return 1;
    }
    load_exe(argv[1]);
    map_stubs();
    StdStrings_SetEnv(&kStdStringEnv);
    MouseModes_SetEnv(&kMouseModeEnv);
    PtrList_SetEnv(&kPtrListEnv);
    GameHeap_SetEnv(&kGameHeapEnv);
    Crt_SetEnv(&kDriverCrt);
    static char token[1 << 22];
    for (;;) {
        u32 address, count;
        char conv[8], ret[8];
        if (std::scanf("%x %7s %7s %x", &address, conv, ret, &count) != 4) break;
        std::vector<u32> args(count);
        for (u32 &a : args) std::scanf("%x", &a);
        u32 regions;
        std::scanf("%x", &regions);
        reset_memory();
        std::vector<std::pair<u32, u32>> spans;
        for (u32 i = 0; i < regions; ++i) {
            u32 at;
            std::scanf("%x %4194303s", &at, token);
            std::vector<u8> bytes = unhex(token);
            map(at, (u32)bytes.size());
            std::memcpy(reinterpret_cast<void *>((uintptr_t)at), bytes.data(), bytes.size());
            spans.push_back({at, (u32)bytes.size()});
        }
        void *fn = find(address);
        if (!fn) {
            std::printf("nohook\n");
            std::fflush(stdout);
            continue;
        }
        u32 ecx = 0, edx = 0, skip = 0;
        if (conv[0] == 't' && count > 0) ecx = args[0], skip = 1;
        if (conv[0] == 'f') {
            if (count > 0) ecx = args[0], skip = 1;
            if (count > 1) edx = args[1], skip = 2;
        }
        u32 eax = 0, edxOut = 0;
        double st0 = 0;
        _control87(_PC_24 | _RC_NEAR, _MCW_PC | _MCW_RC);
        if (std::strchr(ret, 'c')) {  // the case's control word (ret 'c:CW'), with nothing flagged it could unmask
            u16 cw = (u16)std::strtoul(std::strchr(ret, ':') + 1, nullptr, 16);
            __asm fnclex
            __asm fldcw cw
        }
        const int code = invoke(fn, ecx, edx, args.data() + skip, count - skip, ret[0] == 'd', &eax, &edxOut, &st0);
        if (std::strchr(ret, 'c')) {  // the control word the call left, then the driver's back
            u16 cw, ours = 0x7f;
            __asm fnstcw cw
            __asm fldcw ours
            edxOut = cw;
        }
        if (code) {
            std::printf("crash %08x\n", (u32)code);
            std::fflush(stdout);
            continue;
        }
        unsigned long long bits;
        std::memcpy(&bits, &st0, 8);
        std::printf("%08x %08x %016llx", eax, edxOut, bits);
        for (auto &s : spans) {
            std::putchar(' ');
            const u8 *p = reinterpret_cast<const u8 *>((uintptr_t)s.first);
            for (u32 i = 0; i < s.second; ++i) std::printf("%02x", p[i]);
        }
        std::putchar('\n');
        std::fflush(stdout);
    }
    return 0;
}
