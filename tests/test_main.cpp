// tpw_tests: checks that need no game process.
//   tpw_tests [path\to\tp.exe]
// With an exe path it also verifies every hook's expected prologue bytes against
// the file on disk (read as data; nothing from the exe is executed).
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

#include "../src/game/files/refpack.h"
#include "../src/game/motion/curve.h"
#include "../src/game/rsse/rsse.h"
#include "../src/game/terrain/terrain_blend.h"
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

// --- terrain blend ------------------------------------------------------------
static bool same4(const u8 *p, u8 a, u8 b, u8 c, u8 d) { return p[0] == a && p[1] == b && p[2] == c && p[3] == d; }

static void test_lnd_ramp() {
    u8 t[256];
    Lnd_BuildRamp(t, 48, 40);  // half 20: 0 up to 28, 255 past 68
    CHECK(t[27] == 0 && t[28] == 0);
    CHECK(t[48] == 124);       // (48 - 28) * 255 / 41
    CHECK(t[68] == 248 && t[69] == 255 && t[255] == 255);
    Lnd_BuildRamp(t, 0, 0);    // a step: 0 at 0, 255 above
    CHECK(t[0] == 0 && t[1] == 255);
}

// A 2 x 2 tile map with 16-pixel tiles; every texture 32 x 32 so windows start at 0 or 16.
struct TestLnd {
    Lnd lnd = {};
    std::vector<u8> control = std::vector<u8>(9 * 9 * 4, (u8)0xff);
    std::vector<std::vector<u8>> tex;

    TestLnd() {
        lnd.tile_size = 16;
        lnd.level = 0;
        lnd.tiles_x = lnd.tiles_y = 2;
        lnd.control = {9, 9, 4, control.data()};
        lnd.levels[0].tile_size = 16;
        Array3D *a = &lnd.levels[0].colour_a0;
        static const bool colour[13] = {1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0};
        for (int i = 0; i < 13; ++i) {
            u32 bpp = colour[i] ? 4 : 1;
            tex.emplace_back(32 * 32 * bpp, (u8)0);
            a[i] = {32, 32, bpp, tex.back().data()};
        }
    }
    void fill(Array3D &a, const u8 *value) {
        for (u32 i = 0; i < a.width * a.height; ++i) std::memcpy(a.data + i * a.depth, value, a.depth);
    }
    void channel(int ch, u8 v) { for (size_t i = ch; i < control.size(); i += 4) control[i] = v; }
};

static void test_lnd_weight_map() {
    TestLnd t;
    for (u32 i = 0; i < 81; ++i) t.control[i * 4 + 1] = (u8)((i % 9) * 10);  // W1 = 10 per texel in x
    u8 w[16 * 16];
    CHECK(TerrainBlend_WeightMap(&t.lnd.control, 0, 0, 1, 16, w));
    static const u8 row[16] = {0, 2, 5, 7, 10, 12, 15, 17, 20, 22, 25, 27, 30, 32, 35, 37};
    CHECK(std::memcmp(w, row, 16) == 0 && std::memcmp(w + 15 * 16, row, 16) == 0);
    CHECK(!TerrainBlend_WeightMap(&t.lnd.control, 0, 0, 1, 24, w));
}

static void test_lnd_blend() {
    u8 out[16 * 16 * 4], scratch[16 * 16 * 4];
    {   // procedural texturing off: flat grey
        TestLnd t;
        t.channel(0, 0);
        TerrainBlend_TileInto(&t.lnd, out, 0, 0, nullptr, scratch);
        CHECK(same4(out, 0x00, 0x7f, 0x7f, 0x7f) && same4(out + 1020, 0x00, 0x7f, 0x7f, 0x7f));
    }
    {   // selector C 255 everywhere: group C alone (all four bytes lerped)
        TestLnd t;
        LndLevel &lv = t.lnd.levels[0];
        const u8 c0[4] = {4, 10, 20, 30}, c1[4] = {0, 110, 120, 130}, full = 0xff;
        t.fill(lv.colour_c0, c0); t.fill(lv.colour_c1, c1);
        t.fill(lv.mask_c, &full); t.fill(lv.select_c0, &full);
        TerrainBlend_TileInto(&t.lnd, out, 1, 1, nullptr, scratch);  // ramps (0, 0) are steps
        CHECK(same4(out, 0x03, 0x0a, 0x14, 0x1e));  // byte 0: 0 + 4 * 255 >> 8 = 3
    }
    {   // C off, B on in even columns only: the scalar two-group path, byte 0 cleared
        TestLnd t;
        LndLevel &lv = t.lnd.levels[0];
        t.channel(3, 0);
        const u8 b[4] = {0x55, 200, 200, 200};
        t.fill(lv.colour_b0, b); t.fill(lv.colour_b1, b);
        for (u32 i = 0; i < 32 * 32; i += 2) lv.select_b0.data[i] = 0xff;
        TerrainBlend_TileInto(&t.lnd, out, 0, 0, nullptr, scratch);
        CHECK(same4(out, 0x00, 0xc7, 0xc7, 0xc7));  // 200 * 255 >> 8 = 199
        CHECK(same4(out + 4, 0, 0, 0, 0));
    }
}

static void test_lnd_overlay() {
    TestLnd t;
    LndLevel &lv = t.lnd.levels[0];
    for (u32 i = 0; i < 81; ++i) t.control[i * 4 + 1] = (u8)((i % 9) * 30 + (i / 9) * 2);
    const u8 white[4] = {0, 255, 255, 255}, full = 0xff;
    t.fill(lv.colour_a0, white); t.fill(lv.mask_a, &full);
    t.lnd.ramps[0][0] = 128; t.lnd.ramps[0][1] = 255;  // a smooth ramp
    t.channel(2, 0); t.channel(3, 0);
    u8 plain[16 * 16 * 4], out[16 * 16 * 4], scratch[16 * 16 * 4];
    TerrainBlend_TileInto(&t.lnd, plain, 0, 0, nullptr, scratch);
    CHECK(plain[4 * 15 + 1] != plain[1]);  // a gradient across the tile

    std::vector<u8> clear(32 * 32 * 4, 0);  // alpha 0: shows the tile, re-oriented
    TerrainOverlay ov = {clear.data(), 32, 32, 4, 0x10};
    TerrainBlend_TileInto(&t.lnd, out, 0, 0, &ov, scratch);
    bool turned = true;
    for (int i = 0; i < 256; ++i) turned &= std::memcmp(out + 4 * i, plain + 4 * (255 - i), 4) == 0;
    CHECK(turned);
    ov.flags = 0x40 | 0x08;  // mirrored top to bottom
    TerrainBlend_TileInto(&t.lnd, out, 0, 0, &ov, scratch);
    bool flipped = true;
    for (int r = 0; r < 16; ++r) flipped &= std::memcmp(out + 64 * r, plain + 64 * (15 - r), 64) == 0;
    CHECK(flipped);

    std::vector<u8> solid(32 * 32 * 2);  // ARGB4444 opaque red: 0xff00 little-endian
    for (size_t i = 0; i < solid.size(); i += 2) { solid[i] = 0x00; solid[i + 1] = 0xff; }
    ov = {solid.data(), 32, 32, 2, 0};
    TerrainBlend_TileInto(&t.lnd, out, 0, 0, &ov, scratch);
    CHECK(out[0] == 0 && out[1] == (u8)(plain[1] + ((0xf0 - plain[1]) * 0xf0 >> 8)) && out[2] == (u8)(plain[2] + ((0 - plain[2]) * 0xf0 >> 8)));
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
static void test_refpack() {
    // 4 literals, then 2 literals + a 4-byte copy from 2 back (overlapping), then the end.
    const u8 short_form[] = {0x10, 0xfb, 0x00, 0x00, 0x0a, 0xe0, 'a', 'b', 'c', 'd',
                             0x06, 0x01, 'e', 'f', 0xfc};
    u8 out[32];
    std::memset(out, 0xee, sizeof out);
    CHECK(RefPack_DecodedSize(short_form) == 10);
    CHECK(RefPack_Decode(out, short_form, 0) == 10);
    CHECK(std::memcmp(out, "abcdefefef", 10) == 0 && out[10] == 0xee);

    // Flag 0x01: a compressed size comes first. Then 4 literals; the 3-byte form (1 literal,
    // 5 bytes from 5 back); the 4-byte form (6 bytes from 10 back); 1 last literal.
    const u8 long_form[] = {0x11, 0xfb, 0x00, 0x00, 0x17, 0x00, 0x00, 0x11, 0xe0, 'w', 'x', 'y', 'z',
                            0x81, 0x40, 0x04, 'q', 0xc0, 0x00, 0x09, 0x01, 0xfd, '!'};
    std::memset(out, 0xee, sizeof out);
    CHECK(RefPack_DecodedSize(long_form) == 0x11);
    CHECK(RefPack_Decode(out, long_form, 0) == 0x11);
    CHECK(std::memcmp(out, "wxyzqwxyzqwxyzqw!", 0x11) == 0 && out[0x11] == 0xee);

    CHECK(RefPack_Decode(out, nullptr, 0) == 0);
}

static bool same3(const Vec3 &v, float x, float y, float z) { return v.x == x && v.y == y && v.z == z; }

static void test_curves() {
    const Vec3 p[5] = {{0, 0, 0}, {1, 2, 3}, {3, 2, 1}, {4, 4, 4}, {8, 0, -8}};
    Vec3 o;
    // Bezier segment i starts at point base + 3i - 1 (wrapping); at fraction 0 it is that point,
    // with tangent 3 * (next - first).
    Curve_BezierAt(p, 5, 1, 0.0f, &o);
    CHECK(same3(o, 0, 0, 0));
    Curve_BezierAt(p, 5, 0, 0.0f, &o);  // wraps to point 4
    CHECK(same3(o, 8, 0, -8));
    Curve_BezierAt(p, 5, 1, 0.5f, &o);  // (a + 3b + 3c + d) / 8 = (0+3+9+4, 0+6+6+4, 0+9+3+4) / 8
    CHECK(same3(o, 2.0f, 2.0f, 2.0f));
    Curve_BezierTangentAt(p, 5, 1, 0.0f, &o);
    CHECK(same3(o, 3, 6, 9));
    // Linear: segment i between points base + i and base + i + 1; no wrapping.
    Curve_LinearAt(p, 1, 1.25f, &o);
    CHECK(same3(o, 3.25f, 2.5f, 1.75f));
    Curve_LinearTangentAt(p, 1, 1.25f, &o);
    CHECK(same3(o, 1, 2, 3));
    // Catmull-Rom passes through b at fraction 0.
    Curve_CatmullRomAt(p, 5, 2, 0.0f, &o);
    CHECK(same3(o, 3, 2, 1));
}

static void test_refpack_encode() {
    u8 empty[16];
    CHECK(RefPack_Encode(empty, nullptr, 0) == 6);
    CHECK(std::memcmp(empty, "\x10\xfb\x00\x00\x00\xfc", 6) == 0);

    // One part per command form: noise (literal runs), a short near repeat (2 bytes), 30
    // bytes from 1.5 KB back (3 bytes), a long run and a repeat from 20 KB back (4 bytes).
    std::vector<u8> plain;
    u32 x = 12345;
    auto noise = [&](int n) { while (n--) plain.push_back((u8)((x = x * 1103515245 + 12345) >> 24)); };
    noise(2000);
    for (const char *w = "hello, hello, world"; *w; ++w) plain.push_back((u8)*w);
    plain.insert(plain.end(), plain.begin() + 500, plain.begin() + 530);
    plain.insert(plain.end(), 3000, (u8)'z');
    noise(20000);
    plain.insert(plain.end(), plain.begin() + 100, plain.begin() + 400);

    std::vector<u8> enc(RefPack_EncodeBound((u32)plain.size())), dec(plain.size() + 16, 0xee);
    u32 n = RefPack_Encode(enc.data(), plain.data(), (u32)plain.size());
    CHECK(n < plain.size());
    bool forms[4] = {};
    for (u32 i = 5; i < n;) {  // walk the commands
        u8 b = enc[i];
        if (b < 0x80) forms[0] = true, i += 2 + (b & 3);
        else if (b < 0xc0) forms[1] = true, i += 3 + (enc[i + 1] >> 6);
        else if (b < 0xe0) forms[2] = true, i += 4 + (b & 3);
        else if (b < 0xfc) forms[3] = true, i += 1 + (b & 0x1f) * 4 + 4;
        else { CHECK(i + 1 + (b & 3) == n); break; }
    }
    CHECK(forms[0] && forms[1] && forms[2] && forms[3]);
    CHECK(RefPack_Decode(dec.data(), enc.data(), 0) == (s32)plain.size());
    CHECK(std::memcmp(dec.data(), plain.data(), plain.size()) == 0 && dec[plain.size()] == 0xee);
}

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
    test_lnd_ramp();
    test_lnd_weight_map();
    test_lnd_blend();
    test_lnd_overlay();
    test_refpack();
    test_refpack_encode();
    test_curves();
    test_patcher();
    if (argc > 1) test_hook_table(argv[1]);
    std::printf(g_failures ? "%d FAILED\n" : "all tests passed\n", g_failures);
    return g_failures ? 1 : 0;
}
