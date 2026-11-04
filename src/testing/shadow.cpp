#include "shadow.h"

#include <windows.h>
#include <xmmintrin.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../hook/hooks.h"
#include "../hook/patch.h"
#include "log.h"

namespace tpw::testing {

namespace {

const unsigned kCalls = 600, kLinesPerCall = 24, kLinesInAll = 4000, kCallsAlwaysLogged = 20;
const std::size_t kObjectWords = 16;
using u32 = std::uint32_t;

void *g_target;
void(__cdecl *g_port)();
const std::uint8_t *g_expect;
u32 *g_data, *g_before, *g_port_result;
std::size_t g_words;
unsigned g_calls, g_lines;

struct Fpu {
    WORD control, status;
};

Fpu read_fpu() {
    Fpu f;
    WORD control, status;
    __asm {
        fnstcw control
        fnstsw status
    }
    f.control = control;
    f.status = status;
    return f;
}

unsigned top(const Fpu &f) { return (f.status >> 11) & 7; }

double as_float(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}

void log_difference(const void *address, u32 was, u32 port, u32 orig) {
    if (g_lines >= kLinesInAll) return;
    ++g_lines;
    log_line("  %p was %08X (%g) port %08X (%g) orig %08X (%g)", address, was, as_float(was), port, as_float(port), orig, as_float(orig));
}

void __cdecl shadow() {
    if (g_calls >= kCalls) {
        g_port();
        return;
    }
    const unsigned call = ++g_calls;
    u32 *const object = *reinterpret_cast<u32 **>(0x0087afe8);
    u32 object_before[kObjectWords] = {}, object_port[kObjectWords] = {};
    std::memcpy(g_before, g_data, g_words * 4);
    if (object) std::memcpy(object_before, object, sizeof object_before);
    const Fpu at_entry = read_fpu();
    const unsigned csr_entry = _mm_getcsr();

    g_port();
    const Fpu after_port = read_fpu();
    const unsigned csr_port = _mm_getcsr();
    std::memcpy(g_port_result, g_data, g_words * 4);
    if (object) std::memcpy(object_port, object, sizeof object_port);
    for (std::size_t i = 0; i < g_words; ++i)
        if (g_port_result[i] != g_before[i]) g_data[i] = g_before[i];
    if (object) std::memcpy(object, object_before, sizeof object_before);

    tpw::hook::unpatch(g_target, g_expect);
    reinterpret_cast<void(__cdecl *)()>(g_target)();
    const Fpu after_orig = read_fpu();
    const unsigned csr_orig = _mm_getcsr();
    tpw::hook::patch_jump(g_target, reinterpret_cast<const void *>(&shadow), g_expect);

    unsigned differ = 0;
    for (std::size_t i = 0; i < g_words; ++i)
        differ += g_data[i] != g_port_result[i];
    if (object)
        for (std::size_t i = 0; i < kObjectWords; ++i) differ += object[i] != object_port[i];
    if (differ || call <= kCallsAlwaysLogged)
        log_line("shadow %u: fpu cw %04X, top %u/%u/%u, mxcsr %04X/%04X/%04X, %u dwords differ", call, at_entry.control, top(at_entry), top(after_port),
                 top(after_orig), csr_entry, csr_port, csr_orig, differ);
    unsigned lines = 0;
    for (std::size_t i = 0; i < g_words; ++i) {
        if (g_data[i] == g_port_result[i]) continue;
        if (lines++ < kLinesPerCall) log_difference(g_data + i, g_before[i], g_port_result[i], g_data[i]);
        g_data[i] = g_port_result[i];
    }
    if (object) {
        for (std::size_t i = 0; i < kObjectWords; ++i) {
            if (object[i] == object_port[i]) continue;
            if (lines++ < kLinesPerCall) log_difference(object + i, object_before[i], object_port[i], object[i]);
        }
        std::memcpy(object, object_port, sizeof object_port);
    }
}

}  // namespace

void start_shadow(const char *ini) {
    char text[32];
    GetPrivateProfileStringA("tpw", "shadow", "", text, sizeof text, ini);
    const u32 address = std::strtoul(text, nullptr, 16);
    if (!address) return;
    const tpw::hook::Hook *hook = nullptr;
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i)
        if (tpw::hook::kHooks[i].address == address) hook = &tpw::hook::kHooks[i];
    auto *const target = reinterpret_cast<std::uint8_t *>(static_cast<uintptr_t>(address));
    if (!hook || target[0] != 0xe9) {
        log_line("shadow: %08X is not a hook that is on", address);
        return;
    }
    const auto *self = reinterpret_cast<const BYTE *>(GetModuleHandleA(nullptr));
    const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(self + reinterpret_cast<const IMAGE_DOS_HEADER *>(self)->e_lfanew);
    const IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION(nt), *largest = nullptr;
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
        if ((section->Characteristics & IMAGE_SCN_MEM_WRITE) && (!largest || section->Misc.VirtualSize > largest->Misc.VirtualSize)) largest = section;
    if (!largest) return;
    g_data = reinterpret_cast<u32 *>(const_cast<BYTE *>(self) + largest->VirtualAddress);
    g_words = largest->Misc.VirtualSize / 4;
    g_before = static_cast<u32 *>(VirtualAlloc(nullptr, g_words * 8, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!g_before) return;
    g_port_result = g_before + g_words;
    g_target = target;
    g_expect = hook->expect;
    g_port = reinterpret_cast<void(__cdecl *)()>(hook->replacement);
    tpw::hook::unpatch(target, g_expect);
    tpw::hook::patch_jump(target, reinterpret_cast<const void *>(&shadow), g_expect);
    log_line("shadow: %s at %08X, data %p, %u dwords", hook->name, address, g_data, (unsigned)g_words);
}

}  // namespace tpw::testing
