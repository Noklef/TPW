// tpw_tests: checks that need no game process.
//   tpw_tests [path\to\tp.exe]
// With an exe path it also verifies every hook's expected prologue bytes against
// the file on disk (read as data; nothing from the exe is executed).
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

#include "../src/game/rsse/rsse.h"
#include "../src/hook/hooks.h"

static int g_failures = 0;
#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
            ++g_failures;                                                 \
        }                                                                 \
    } while (0)

static void test_rsse_fetch_word() {
    u32 code[3] = {0x80000001u, 0x40000002u, 7};
    RsseContext ctx = {};
    ctx.code = code;
    ctx.code_len = 3;

    CHECK(RSSE_FetchWord(&ctx) == 0x80000001u && ctx.pc == 1);
    CHECK(RSSE_FetchWord(&ctx) == 0x40000002u && ctx.pc == 2);
    CHECK(RSSE_FetchWord(&ctx) == 7 && ctx.pc == 3);
    CHECK(RSSE_FetchWord(&ctx) == 0 && ctx.pc == RSSE_PC_DEAD);  // ran off the end
    CHECK(RSSE_FetchWord(&ctx) == 0 && ctx.pc == RSSE_PC_DEAD);  // stays dead
}

static void test_rsse_operand_value() {
    s32 vars[4] = {10, 20, -30, 40};
    RsseContext ctx = {};
    ctx.vars = vars;

    CHECK(RSSE_OperandValue(&ctx, 0x40000002u) == -30);  // variable
    CHECK(RSSE_OperandValue(&ctx, 500) == 500);          // immediate
    CHECK(RSSE_OperandValue(&ctx, 0xFFFF) == -1);        // sign-extended from 16 bits
    CHECK(RSSE_OperandValue(&ctx, 0x0001FFFF) == -1);    // high bits of an immediate are dropped
    CHECK(RSSE_OperandValue(&ctx, 0x20000005u) == 5);    // a label used as a source: low 16 bits
    CHECK(RSSE_OperandValue(&ctx, 0x41000000u) == 0);    // tag test is on the whole top byte
}

// --- patcher self-test on functions in this process ---------------------------
extern "C" __declspec(noinline) int __cdecl patch_victim(int x) {
    volatile int y = x;  // keep a real prologue
    return y + 1;
}
extern "C" __declspec(noinline) int __cdecl patch_replacement(int x) { return x * 100; }

static void test_patcher() {
    using namespace tpw::hook;
    auto target = reinterpret_cast<std::uint8_t *>(&patch_victim);
    // Incremental linking routes calls through a jmp thunk; follow it to the body.
    if (target[0] == 0xE9) target += 5 + *reinterpret_cast<std::int32_t *>(target + 1);
    int (*victim)(int) = reinterpret_cast<int (*)(int)>(target);

    std::uint8_t expect[5], saved[5];
    std::memcpy(expect, target, 5);
    std::uint8_t wrong[5] = {0xCC, 0xCC, 0xCC, 0xCC, 0xCC};

    CHECK(victim(1) == 2);
    CHECK(patch_jump(target, (void *)&patch_replacement, wrong) == PatchResult::BytesMismatch);
    CHECK(victim(1) == 2);  // untouched
    CHECK(patch_jump(target, (void *)&patch_replacement, expect, saved) == PatchResult::Ok);
    CHECK(victim(3) == 300);
    CHECK(unpatch(target, saved));
    CHECK(victim(3) == 4);
}

// --- hook table vs. the exe on disk ----------------------------------------------
static bool read_va(const std::vector<unsigned char> &pe, std::uint32_t va, std::uint8_t *out, size_t n) {
    auto u16 = [&](size_t o) { return std::uint32_t(pe[o] | pe[o + 1] << 8); };
    auto u32 = [&](size_t o) { return u16(o) | u16(o + 2) << 16; };
    std::uint32_t nt = u32(0x3C), base = u32(nt + 0x34);
    std::uint32_t nsec = u16(nt + 6), sec = nt + 0x18 + u16(nt + 0x14);
    std::uint32_t rva = va - base;
    for (std::uint32_t i = 0; i < nsec; ++i, sec += 40) {
        std::uint32_t vaddr = u32(sec + 12), rsize = u32(sec + 16), raw = u32(sec + 20);
        if (rva >= vaddr && rva + n <= vaddr + rsize) {
            std::memcpy(out, &pe[raw + (rva - vaddr)], n);
            return true;
        }
    }
    return false;
}

static void test_hook_table(const char *exe_path) {
    std::ifstream f(exe_path, std::ios::binary);
    if (!f) { std::printf("cannot open %s\n", exe_path); ++g_failures; return; }
    std::vector<unsigned char> pe((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i) {
        const auto &h = tpw::hook::kHooks[i];
        std::uint8_t actual[5];
        bool ok = read_va(pe, h.address, actual, 5) && std::memcmp(actual, h.expect, 5) == 0;
        std::printf("  %08X %-32s %s\n", h.address, h.name, ok ? "bytes match" : "MISMATCH");
        if (!ok) ++g_failures;
    }
}

int main(int argc, char **argv) {
    test_rsse_fetch_word();
    test_rsse_operand_value();
    test_patcher();
    if (argc > 1) test_hook_table(argv[1]);
    std::printf(g_failures ? "%d FAILED\n" : "all tests passed\n", g_failures);
    return g_failures ? 1 : 0;
}
