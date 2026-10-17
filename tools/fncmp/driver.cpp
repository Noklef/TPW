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

#include "../../src/game/core/cpuinfo.h"
#include "../../src/game/core/gameheap.h"
#include "../../src/game/core/ptrlist.h"
#include "../../src/game/core/stdstring.h"
#include "../../src/game/files/bufsource.h"
#include "../../src/game/files/dirsource.h"
#include "../../src/game/files/drives.h"
#include "../../src/game/files/file.h"
#include "../../src/game/files/handles.h"
#include "../../src/game/files/waddir.h"
#include "../../src/game/files/streamopen.h"
#include "../../src/game/files/wadfind.h"
#include "../../src/game/files/wadsource.h"
#include "../../src/game/files/wrapsources.h"
#include "../../src/game/files/zlibcodec.h"
#include "../../src/game/gfx/canvas.h"
#include "../../src/game/render/engineerrors.h"
#include "../../src/game/sprite/script.h"
#include "../../src/game/world/mapline.h"
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
    const u32 countdown = h[0] & 15, p = h[0] & ~15u;  // (fncmp.heap(fail=): the allocation that answers 0)
    h[1] = h[1] * 33 ^ size;
    if (countdown == 1) {
        h[0] = p;
        return nullptr;
    }
    h[0] = p + ((size + 15) & ~15u) + (countdown ? countdown - 1 : 0);
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

// The fake CPU a case maps at kCpu (tools/fncmp.py CPU, cpu_region(); its emulator answers CPUID and EFLAGS.ID from the same bytes): +0 1 when
// EFLAGS.ID can be toggled, then from +0x10 the (EAX, EBX, ECX, EDX) answers for leaf 0, 1, 0x80000001 and any other leaf.
const u32 kCpu = 0x2d000000;

u32 __cdecl stub_has_cpuid() { return *reinterpret_cast<const u32 *>((uintptr_t)kCpu) != 0; }

void __cdecl stub_cpuid(u32 leaf, u32 *regs) {
    const u32 i = leaf == 0 ? 0 : leaf == 1 ? 1 : leaf == 0x80000001 ? 2 : 3;
    std::memcpy(regs, reinterpret_cast<const void *>((uintptr_t)(kCpu + 0x10 + 0x10 * i)), 16);
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

// The same with two arguments (kStubs + 0x80, tools/fncmp.py CDECL_LOG2): the entry k 0x13e.
u32 __cdecl stub_cdecl_log2(u32 a, u32 b) {
    const u32 stack[3] = {0, a, b};
    return record_call(0x13e, 2, 0, stack);
}

// QueryPerformanceFrequency / QueryPerformanceCounter (stdcall) for code that calls through the game's import slots (kStubs + 0x60, tools/fncmp.py COUNTER;
// the case maps the log): writes the 64-bit value at log + 0x7f0, then steps it by the one at log + 0x7f8; returns bit 0 of the dword at log + 0x7e8, which then shifts right.
s32 __stdcall stub_counter(unsigned long long *value) {
    u8 *const log = reinterpret_cast<u8 *>((uintptr_t)(kStubs + 0x100000));
    unsigned long long *const now = reinterpret_cast<unsigned long long *>(log + 0x7f0);
    *value = *now;
    *now += *reinterpret_cast<unsigned long long *>(log + 0x7f8);
    u32 *const results = reinterpret_cast<u32 *>(log + 0x7e8);
    const s32 ok = (s32)(*results & 1);
    *results >>= 1;
    return ok;
}

// A memory stream's seek (kStubs + 0x70, tools/fncmp.py SEEK; memory_stream()), thiscall (offset, origin) over +4 cursor, +8 bytes left, +0xc start,
// +0x10 size: the new position from the start (origin 0), the cursor (1) or the end (else), or -1 past the end, nothing moved.
s32 __fastcall stub_seek(u8 *self, void *, u32 offset, u32 origin) {
    u32 *const s = reinterpret_cast<u32 *>(self + 4);
    const u32 to = (origin == 0 ? 0 : origin == 1 ? s[0] - s[2] : s[3]) + offset;
    if (to > s[3]) return -1;
    s[0] = s[2] + to;
    s[1] = s[3] - to;
    return (s32)to;
}

// What throws in the game (std::_Xlen, _Xran): a case that gets there reports a crash.
void __cdecl heap_throw() { RaiseException(0xe0000001, 0, 0, nullptr); }

const StdStringEnv kStdStringEnv = {&heap_alloc, &heap_free, &heap_throw, &heap_throw};
const MouseModeEnv kMouseModeEnv = {&heap_free};
const PtrListEnv kPtrListEnv = {&heap_alloc};
const GameHeapEnv kGameHeapEnv = {&heap_alloc, &heap_free};
const WadSourceEnv kWadSourceEnv = {&heap_alloc, &heap_free};
const BufferedSourceEnv kBufferedSourceEnv = {&heap_alloc, &heap_free};
// atexit is stub_cdecl_log (kStubs + 0x50; the benches hook 0x67a7d0 with fncmp._cdecl_log); the compressor's destructor stays the game's address, which is only
// handed to atexit, never called here.
const WrapSourcesEnv kWrapSourcesEnv = {&heap_alloc, &heap_free, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)),
                                        reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005f80c0), reinterpret_cast<u8 *>((uintptr_t)0x00fa75f8),
                                        reinterpret_cast<ZlibCodec *>((uintptr_t)0x00fa75fc)};

// The recorders (tools/fncmp.py recorder(k, n)): thunks at kStubs + 0x1000 + 0x400 * n + 0x20 * k, thiscall with n stack arguments (popped by the thunk), that
// log (k, n, this, the arguments) at kLog and return a hash of them. The log is a count, then 12 dwords an entry; room for 80.
const u32 kLog = kStubs + 0x100000;  // (the case maps it)
constexpr u32 recorder(u32 k, u32 n) { return kStubs + 0x1000 + 0x400 * n + 0x20 * k; }

// Optional settings in the log region: at + 0xa00 a mask per slot (k & 63): bits 0-15 the arguments that are C strings, logged and hashed as a hash of the string
// (so that a path in a stack buffer, whose address differs between the sides, compares by its text), bits 16-31 out arguments, logged as 0 (a stack address;
// what is written through them is a fill's, and shows in what the code does with it); at + 0xb00 and + 0xb80 two fills (k | 0x80000000, the argument, the
// length; the bytes from + 0x10, at most 0x70; length bit 31: argument length & 0xff's value, fncmp.fill_arg()) written through that argument before the
// recorder returns (an out structure, FindFirstFile's); at + 0xc00 a mask per slot of BYTE arguments, logged as their low byte (fncmp.LOG_BYTES).
u32 string_hash(const u8 *s) {
    if (!s) return 0;
    u32 h = 0x811c9dc5u;
    for (u32 i = 0; i < 0x400 && s[i]; ++i) h = (h ^ s[i]) * 0x01000193u;
    return h;
}

u32 __cdecl record_call(u32 k, u32 n, u32 self, const u32 *stack) {
    u32 *log = reinterpret_cast<u32 *>((uintptr_t)kLog);
    u32 args[16];
    const u32 mask = log[0x280 + (k & 63)];
    for (u32 i = 0; i < n; ++i) {  // (stack[0] is the thunk's own return address)
        args[i] = stack[1 + i];
        for (u32 f = 0; f < 2; ++f) {
            const u32 *fill = log + 0x2c0 + 0x20 * f;
            if (fill[0] != ((k & 63) | 0x80000000u) || fill[1] != i || !args[i]) continue;
            if (fill[2] & 0x80000000u) {  // (fncmp.fill_arg(): an argument's value, as passed)
                const u32 value = (fill[2] & 0xff) < n ? stack[1 + (fill[2] & 0xff)] : 0;
                std::memcpy(reinterpret_cast<void *>((uintptr_t)args[i]), &value, 4);
            } else {
                std::memcpy(reinterpret_cast<void *>((uintptr_t)args[i]), fill + 4, fill[2] < 0x70 ? fill[2] : 0x70);
            }
        }
        if (mask >> i & 1) args[i] = string_hash(reinterpret_cast<const u8 *>((uintptr_t)args[i]));
        if (mask >> (16 + i) & 1) args[i] = 0;
        if (log[0x300 + (k & 63)] >> i & 1) args[i] &= 0xff;
    }
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
    const u32 *override = log + 0x200 + 2 * (k & 63);  // (LOG_RETURNS: a flag and a value for the slot; flag 2: argument value, as passed)
    return override[0] == 2 && override[1] < n ? stack[1 + override[1]] : override[0] ? override[1] : h;
}

void map_com_recorders(u8 *page) {  // the COM-style ones (this on the stack, ECX undefined): this is logged as 0 and k as k | 0x100
    for (u32 n = 0; n < 12; ++n) {  // (up to 11 stack arguments: to + 0xb000)
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

// The zlib stand-ins for the codec's env (tools/fncmp.py hooks the game's zlib with the same): no compression, a copy of what fits from the input to the
// output, answering 1 (Z_STREAM_END) when the input is used up, 0 (Z_OK) when it moved some, -5 (Z_BUF_ERROR) when it could not; the inits 0. A case can
// override the inits', deflate's and inflate's answers at kLog + 0xf00 (fncmp.zlib_results(): a flag and a value each). Each call is logged as the entry k
// 0x140 + its index in deflateInit2, deflate, deflateEnd, inflateInit2, inflate, inflateEnd with its scalar arguments (the stream is a stack address; deflate / inflate: the flush, then avail_in and avail_out).
s32 zlib_result(u32 which, s32 natural) {
    const u32 *const config = reinterpret_cast<const u32 *>((uintptr_t)(kLog + 0xf00)) + 2 * which;
    return config[0] ? (s32)config[1] : natural;
}

void zlib_log(u32 k, u32 n, u32 a = 0, u32 b = 0, u32 c = 0, u32 d = 0, u32 e = 0, u32 f = 0, u32 g = 0) {
    u32 *const log = reinterpret_cast<u32 *>((uintptr_t)kLog);
    if (log[0] >= 40) return;
    u32 *const entry = log + 1 + 12 * log[0]++;
    const u32 values[12] = {k, n, 0, a, b, c, d, e, f, g, 0, 0};
    std::memcpy(entry, values, sizeof values);
}

s32 zlib_copy(ZStream *z, u32 which, s32 flush) {
    zlib_log(0x140 + 3 * which - 2, 3, (u32)flush, z->availIn, z->availOut);
    const u32 n = z->availIn < z->availOut ? z->availIn : z->availOut;
    std::memmove(z->nextOut, z->nextIn, n);
    z->nextIn += n, z->availIn -= n, z->totalIn += n;
    z->nextOut += n, z->availOut -= n, z->totalOut += n;
    return zlib_result(which, !z->availIn ? 1 : n ? 0 : -5);
}

s32 __cdecl zlib_deflate_init(ZStream *, s32 level, s32 method, s32 windowBits, s32 memLevel, s32 strategy, const char *version, s32 size) {
    zlib_log(0x140, 7, (u32)level, (u32)method, (u32)windowBits, (u32)memLevel, (u32)strategy, (u32)(uintptr_t)version, (u32)size);
    return zlib_result(0, 0);
}
s32 __cdecl zlib_inflate_init(ZStream *, s32 windowBits, const char *version, s32 size) {
    zlib_log(0x143, 3, (u32)windowBits, (u32)(uintptr_t)version, (u32)size);
    return zlib_result(0, 0);
}
s32 __cdecl zlib_deflate(ZStream *z, s32 flush) { return zlib_copy(z, 1, flush); }
s32 __cdecl zlib_inflate(ZStream *z, s32 flush) { return zlib_copy(z, 2, flush); }
s32 __cdecl zlib_deflate_end(ZStream *) {
    zlib_log(0x142, 0);
    return 0;
}
s32 __cdecl zlib_inflate_end(ZStream *) {
    zlib_log(0x145, 0);
    return 0;
}

void *find(u32 address);

void jump(u8 *at, const void *to) {
    at[0] = 0xe9;  // jmp rel32
    const u32 rel = (u32)(uintptr_t)to - ((u32)(uintptr_t)at + 5);
    std::memcpy(at + 1, &rel, 4);
}

// Game vtables whose slots point at the replacements in the driver's copy of the game's data, as the hooked game resolves them (its originals jump to the
// replacements): code that constructs one of these objects itself then calls through the real vtable. (address, slots; a slot not hooked is left as it is,
// and a case that calls it crashes natively)
const u32 kResolvedVtables[][2] = {{0x00703090, 11},  // String_vtable
                                   {0x007030c8, 11},  // WString_vtable
                                   {0x00703e08, 9},   // TextWriter_vtable
                                   {0x00703908, 13},  // WadSource_vtable
                                   {0x006fd930, 12},  // FileStream_vtable
                                   {0x00703ea0, 12},  // SharedSource_vtable
                                   {0x00703ed0, 12},  // BufferSource_vtable (RefPack)
                                   {0x00703f18, 12},  // ZlibBufferSource_vtable
                                   {0x00703f48, 12},  // SliceSource_vtable
                                   {0x007038d0, 14},  // DirSource_vtable
                                   {0x00703e30, 12},  // FileSource_vtable
                                   {0x00703e60, 12},  // MmioSource_vtable
                                   {0x00703128, 8},   // ZlibCodec_vtable
                                   {0x00703180, 12},  // Handle_vtable
                                   {0x006fd9e8, 12},  // FolderHandle_vtable
                                   {0x006fda18, 12},  // WadHandle_vtable
                                   {0x00703898, 13},  // RefObject2_vtable
                                   {0x00703100, 9},   // StreamBase_vtable
                                   {0x00703838, 12},  // RefObject_vtable
                                   {0x00703f78, 12},  // BufferedSource_vtable
                                   {0x00703868, 12},  // MemorySource_vtable
                                   {0x00703160, 3},   // CallbackStream_vtable
                                   {0x006fd6a0, 9},   // the action recorder's streams (FileStream's deleting destructor, StreamBase's)
                                   {0x006fd618, 2},   // the balance files (slot 0 the schema getter, 1 BalanceFile_GetValues): Hoardings,
                                   {0x006fd620, 2},   // Level,
                                   {0x006fd628, 2},   // Online,
                                   {0x006fd630, 2},   // Graphics,
                                   {0x006fd638, 2},   // Resolution,
                                   {0x006fd640, 2},   // Sound,
                                   {0x006fd8a8, 2},   // ObjectDef,
                                   {0x006fd8b0, 2},   // TrackType,
                                   {0x00700398, 2},   // Challenges,
                                   {0x007034d8, 16},  // the 8-bit draw primitives (DrawDriver table; 14, the sprite blitter, is not hooked)
                                   {0x006fd958, 2}};  // AdvisorScoring

void __cdecl debug_printf(const char *, ...) {}  // Debug_Printf 0x5da3c0 (RefObject's teardown slot among others)

void resolve_vtables() {
    for (const auto &v : kResolvedVtables)
        for (Section &s : g_sections)
            if (v[0] >= s.address && v[0] + 4 * v[1] <= s.address + s.raw.size())
                for (u32 i = 0; i < v[1]; ++i) {
                    u32 *const slot = reinterpret_cast<u32 *>(s.raw.data() + (v[0] - s.address) + 4 * i);
                    if (*slot == 0x005da3c0) *slot = (u32)(uintptr_t)&debug_printf;  // (empty in the retail build)
                    else if (void *const fn = find(*slot)) *slot = (u32)(uintptr_t)fn;  // (a slot not hooked stays the game's)
                }
}

void map_stubs() {
    u8 *p = static_cast<u8 *>(VirtualAlloc((void *)(uintptr_t)kStubs, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    if (!p) fail("cannot map the stubs at", kStubs);
    const void *stubs[] = {(const void *)&stub_read, (const void *)&stub_write, (const void *)&stub_global_alloc,
                           (const void *)&stub_global_free, (const void *)&stub_mark, (const void *)&stub_cdecl_log, (const void *)&stub_counter,
                           (const void *)&stub_seek, (const void *)&stub_cdecl_log2};
    for (u32 i = 0; i < sizeof stubs / sizeof stubs[0]; ++i) jump(p + 0x10 * i, stubs[i]);
    map_recorders(p);
}

void *find(u32 address) {
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i)
        if (tpw::hook::kHooks[i].address == address) return tpw::hook::kHooks[i].replacement;
    for (u32 i = 0; i < kCrtStandInCount; ++i)
        if (kCrtStandIns[i].address == address) return kCrtStandIns[i].fn;
    return nullptr;
}

// With FNCMP_CRASHAT set (debugging a bench), where a case faults goes to stderr: the instruction and the address it touched.
int crash_filter(EXCEPTION_POINTERS *x) {
    static const bool report = std::getenv("FNCMP_CRASHAT") != nullptr;
    if (report) {
        const EXCEPTION_RECORD *const r = x->ExceptionRecord;
        std::fprintf(stderr, "crash %08x at %08x touching %08x (esp %08x)\n", (u32)r->ExceptionCode, (u32)(uintptr_t)r->ExceptionAddress,
                     r->NumberParameters > 1 ? (u32)r->ExceptionInformation[1] : 0, (u32)x->ContextRecord->Esp);
    }
    return EXCEPTION_EXECUTE_HANDLER;
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
    } __except (crash_filter(GetExceptionInformation())) {
        __asm fninit
        return (int)GetExceptionCode();
    }
    *eax = ra;
    *edx = rd;
    *st0 = f;
    return 0;
}

void *hooked(u32 address) {
    void *const fn = find(address);
    if (!fn) fail("not hooked", address);
    return fn;
}

// The sprite script list's clock (its env's now): the hooked 0x402d70 on the game clock (0x785970).
s32 __cdecl script_now() {
    static const auto now = reinterpret_cast<s32(__fastcall *)(void *, void *)>(hooked(0x00402d70));
    return now(reinterpret_cast<void *>(0x00785970), nullptr);
}

u8 nibble(char c) { return (u8)(c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10); }

// (by hand: sscanf("%2x") per byte measures the rest of the string each call, which made a 1 MB region cost seconds)
std::vector<u8> unhex(const char *s) {
    std::vector<u8> out(std::strlen(s) / 2);
    for (std::size_t i = 0; i < out.size(); ++i) out[i] = (u8)(nibble(s[2 * i]) << 4 | nibble(s[2 * i + 1]));
    return out;
}

void put_hex(const u8 *p, u32 n) {
    static const char digits[] = "0123456789abcdef";
    char buf[4096];
    for (u32 i = 0; i < n;) {
        u32 k = 0;
        for (; i < n && k < sizeof buf; ++i) {
            buf[k++] = digits[p[i] >> 4];
            buf[k++] = digits[p[i] & 15];
        }
        std::fwrite(buf, 1, k, stdout);
    }
}

}  // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::printf("usage: tpw_fncmp tp.exe < cases\n");
        return 1;
    }
    load_exe(argv[1]);
    map_stubs();
    resolve_vtables();
    StdStrings_SetEnv(&kStdStringEnv);
    MouseModes_SetEnv(&kMouseModeEnv);
    PtrList_SetEnv(&kPtrListEnv);
    GameHeap_SetEnv(&kGameHeapEnv);
    WadSource_SetEnv(&kWadSourceEnv);
    BufferedSource_SetEnv(&kBufferedSourceEnv);
    WrapSources_SetEnv(&kWrapSourcesEnv);
    static FileEnv files = *Files_GetEnv();  // (the import slots the case maps; Mem_Alloc is game code)
    files.alloc = &heap_alloc;
    Files_SetEnv(&files);
    // The WAD tree's and finder's tolower / toupper: the CRT stand-ins (crt.cpp; the C locale's, as the cases map it).
    static const WadDirEnv wadDir = {&heap_alloc, &heap_free, reinterpret_cast<s32(__cdecl *)(s32)>(hooked(0x0067fa60))};
    WadDir_SetEnv(&wadDir);
    static const WadFindEnv wadFind = {&heap_alloc, &heap_free, reinterpret_cast<s32(__cdecl *)(s32)>(hooked(0x0067f1a0))};
    WadFind_SetEnv(&wadFind);
    static const DirSourceEnv dirSource = {reinterpret_cast<RefObject *(__fastcall *)(void *)>(hooked(0x00636e40)),
                                           reinterpret_cast<s32 *(__fastcall *)(RefObject *, void *, s32 *, const char *, u32)>(hooked(0x00636f10)),
                                           reinterpret_cast<RefObject *(__fastcall *)(void *)>(hooked(0x006371e0)),
                                           reinterpret_cast<s32 *(__fastcall *)(RefObject *, void *, s32 *, const char *, u32)>(hooked(0x006372b0))};
    DirSource_SetEnv(&dirSource);
    static const ZlibEnv zlib = {&zlib_deflate_init, &zlib_deflate, &zlib_deflate_end, &zlib_inflate_init, &zlib_inflate, &zlib_inflate_end,
                                 reinterpret_cast<void *(__cdecl *)(void *, u32, u32)>(hooked(0x00619250)),
                                 reinterpret_cast<void(__cdecl *)(void *, void *)>((uintptr_t)0x005da3c0),  // (Debug_Printf: the stand-ins never free)
                                 reinterpret_cast<const char *>((uintptr_t)0x007780f0), reinterpret_cast<const u32 *>((uintptr_t)0x007780ec)};
    ZlibCodec_SetEnv(&zlib);
    // Envs whose game defaults call game code by its address, which the driver cannot (its own code covers the game's .text): that code resolved to its hooked
    // replacement, as the hooked game resolves it; the CPU information constructor (CPUID, the host's) is stub_mark, as tools/fncmp.py's emulator has it.
    static DrivesEnv drives = *Drives_GetEnv();
    drives.cpuCtor = reinterpret_cast<void(__fastcall *)(SystemInfo *)>(&stub_mark);
    drives.stringCtor = reinterpret_cast<String *(__fastcall *)(String *, void *, const char *)>(hooked(0x005f60b0));
    drives.stringCopy = reinterpret_cast<String *(__fastcall *)(String *, void *, const StringView *)>(hooked(0x005f6040));
    drives.stringDtor = reinterpret_cast<void(__fastcall *)(String *)>(hooked(0x005f61e0));
    drives.alloc = &heap_alloc;
    drives.nodeCtor = reinterpret_cast<StringListNode *(__fastcall *)(StringListNode *, void *, String)>(hooked(0x005f8a50));
    drives.stricmp = reinterpret_cast<s32(__cdecl *)(const char *, const char *)>(hooked(0x0067df40));
    Drives_SetEnv(&drives);
    static const CpuInfoEnv cpu = {&stub_has_cpuid, &stub_cpuid};  // the case's fake CPU, as the emulator's
    CpuInfo_SetEnv(&cpu);
    // Game code a port calls directly that is not hooked (yet): a recorder, as the bench's hooks= has it in the emulator.
    static const MapLineEnv mapLine = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, s32, s32)>((uintptr_t)recorder(20, 3))};  // 0x5370e0: tests/fncmp/mapline.py
    MapLine_SetEnv(&mapLine);
    // 0x564650 (the engine debug log, a file): stub_cdecl_log, as tests/fncmp/engineerrors.py hooks it (the text hashed: strings(log, 0x3f, 1)).
    static const EngineErrorsEnv engineErrors = {reinterpret_cast<void(__cdecl *)(const char *)>(&stub_cdecl_log)};
    EngineErrors_SetEnv(&engineErrors);
    static CanvasEnv canvas = *Canvas_GetEnv();
    canvas.infoCtor = reinterpret_cast<SurfaceInfo *(__fastcall *)(SurfaceInfo *)>(hooked(0x005fa680));
    Canvas_SetEnv(&canvas);
    static HandlesEnv handles = *Handles_GetEnv();
    handles.alloc = &heap_alloc;
    handles.free = &heap_free;
    handles.ctor28 = reinterpret_cast<RefObject2 *(__fastcall *)(void *)>(hooked(0x00619950));
    handles.open28 = reinterpret_cast<s32 *(__fastcall *)(RefObject2 *, void *, s32 *, u32)>(hooked(0x00619d70));
    handles.ctorE0 = reinterpret_cast<RefObject2 *(__fastcall *)(void *)>(hooked(0x0061a060));
    handles.openE0 = reinterpret_cast<s32 *(__fastcall *)(RefObject2 *, void *, s32 *, u32, u32)>(hooked(0x0061a250));
    handles.ctorMemory = reinterpret_cast<RefObject *(__fastcall *)(void *, void *, u32)>(hooked(0x00618df0));
    handles.bufferCtor = reinterpret_cast<void *(__fastcall *)(void *, void *, char *, u32, u32)>(hooked(0x00636bc0));
    handles.bufferDtor = reinterpret_cast<void(__fastcall *)(void *)>(hooked(0x00636c70));
    Handles_SetEnv(&handles);
    static const StreamOpenEnv streamOpen = {
        reinterpret_cast<s32 *(__cdecl *)(s32 *, char *, u32, const char *)>(hooked(0x005f5710)),
        reinterpret_cast<u32(__stdcall *const *)(const char *)>((uintptr_t)0x006fd10c),
        reinterpret_cast<Handle *(__fastcall *)(Handle *)>(hooked(0x005f8240)),
        reinterpret_cast<void(__fastcall *)(Handle *)>(hooked(0x005f82a0)),
        reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32)>(hooked(0x005f85d0)),
        reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32)>(hooked(0x005f8510)),
        reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32, u32, u32)>(hooked(0x005f83d0)),
        reinterpret_cast<String *(__fastcall *)(String *, void *, const char *)>(hooked(0x005f60b0)),
        reinterpret_cast<String *(__fastcall *)(String *, void *, const StringView *)>(hooked(0x005f6040)),
        reinterpret_cast<void(__fastcall *)(String *)>(hooked(0x005f61e0)),
        reinterpret_cast<s32(__fastcall *)(const String *, void *, const char *, u32)>(hooked(0x005f7b90)),
        reinterpret_cast<StringView *(__fastcall *)(const String *, void *, StringView *, u32)>(hooked(0x005f7d60)),
        reinterpret_cast<StringView *(__fastcall *)(const String *, void *, StringView *, u32)>(hooked(0x005f7dd0)),
        reinterpret_cast<void *const *>((uintptr_t)0x006fda18),
        reinterpret_cast<void *const *>((uintptr_t)0x006fd9e8),
        reinterpret_cast<RefObject *(__fastcall *)(StreamBase *)>(hooked(0x004217e0)),
        &heap_alloc,
        reinterpret_cast<BufferedSource *(__fastcall *)(BufferedSource *)>(hooked(0x00638ef0)),
        reinterpret_cast<s32 *(__fastcall *)(BufferedSource *, void *, s32 *, u32)>(hooked(0x00638f20)),
        reinterpret_cast<void(__fastcall *)(StreamBase *, void *, RefObject *)>(hooked(0x005f7240))};
    StreamOpen_SetEnv(&streamOpen);
    Crt_SetEnv(&kDriverCrt);
    Libc_SetEnv(&kDriverLibc);
    // The sprite script list's calls: the hooked originals (Mem_GlobalAlloc / Free through the GlobalAlloc import slots the case maps, global_heap()).
    static const SpriteScriptEnv sprites = {reinterpret_cast<void *(__cdecl *)(u32)>(hooked(0x005f5660)), reinterpret_cast<void(__cdecl *)(void *)>(hooked(0x005f5680)),
                                            &script_now, reinterpret_cast<u32(__cdecl *)(u8 *)>(hooked(0x00475010)),
                                            reinterpret_cast<void(__cdecl *)()>(hooked(0x00540ab0)), reinterpret_cast<void(__cdecl *)(u8 *)>(hooked(0x00542010)),
                                            reinterpret_cast<void(__cdecl *)()>(hooked(0x00540b30)),
                                            reinterpret_cast<void(__cdecl *)(u32 *, void *, const void *, u32, u32)>(hooked(0x005f5e50)),
                                            reinterpret_cast<void(__cdecl *)(u32 *, void *, void *, u32, u32)>(hooked(0x005f5e20))};
    SpriteScripts_SetEnv(&sprites);
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
            put_hex(p, s.second);
        }
        std::putchar('\n');
        std::fflush(stdout);
    }
    return 0;
}
