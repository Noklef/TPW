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
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "../../src/hook/hooks.h"

namespace {

using u8 = std::uint8_t;
using u32 = std::uint32_t;

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

void *find(u32 address) {
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i)
        if (tpw::hook::kHooks[i].address == address) return tpw::hook::kHooks[i].replacement;
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
        const int code = invoke(fn, ecx, edx, args.data() + skip, count - skip, ret[0] == 'd', &eax, &edxOut, &st0);
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
