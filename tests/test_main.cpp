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
#include "../src/game/math/matrix.h"
#include "../src/game/math/vector.h"
#include "../src/game/motion/curve.h"
#include "../src/game/motion/keys.h"
#include "../src/game/motion/quat.h"
#include "../src/game/motion/track.h"
#include "../src/game/motion/uvanim.h"
#include "../src/game/motion/vertexanim.h"
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

    const float pct[4] = {10, 30, 90, 5};
    float r;
    Path_PercentAt(pct, 4, 0.5f, &r, 0);
    CHECK(r == 20);
    Path_PercentAt(pct, 4, 4.0f, &r, 0);  // t == count: the end of the last segment
    CHECK(r == 5);
    Path_PercentAt(pct, 4, 2.5f, &r, 0);  // 90 -> 5 crosses 100: lerp 90 -> 105, modulo 100
    CHECK(r == 97.5f);
    Path_PercentAt(pct, 4, 3.5f, &r, 0);  // open: the last segment holds its value
    CHECK(r == 5);
    Path_PercentAt(pct, 4, 3.5f, &r, 1);  // closed: wraps to the first entry
    CHECK(r == 7.5f);
}

static void test_quat() {
    float m[9];
    const float id[4] = {0, 0, 0, 2};  // not unit length: scaled by 2 / |q|^2
    Quat_ToMatrix(id, m);
    CHECK(m[0] == 1 && m[1] == 0 && m[2] == 0 && m[3] == 0 && m[4] == 1 && m[5] == 0 && m[8] == 1);
    const float half = 0.70710677f;  // 90 degrees about z
    const float rz[4] = {0, 0, half, half};
    Quat_ToMatrix(rz, m);
    auto near = [](float a, float b) { return a - b < 1e-6f && b - a < 1e-6f; };
    CHECK(near(m[0], 0) && near(m[1], 1) && near(m[3], -1) && near(m[4], 0) && near(m[8], 1));
}

static void test_slerp() {
    static float values[4096];
    SinTable sines{values, 0, 0};
    SinTable_Build(values, 4096, &sines.mask, &sines.scale);
    CHECK(sines.mask == 0xfff && sines.scale == 4096 * 0.159154937f);
    CHECK(values[0] == 0 && values[1024] == 1 && values[3072] == -1);

    auto near = [](float a, float b) { return a - b < 2e-3f && b - a < 2e-3f; };  // table steps
    const float id[4] = {0, 0, 0, 1}, rz[4] = {0, 0, 0.70710677f, 0.70710677f};
    float q[4];
    Quat_SlerpWith(sines, id, rz, 0, q);  // t = 0 is a exactly
    CHECK(q[0] == 0 && q[1] == 0 && q[2] == 0 && q[3] == 1);
    Quat_SlerpWith(sines, id, rz, 0.5f, q);  // 45 degrees about z
    CHECK(near(q[2], 0.38268343f) && near(q[3], 0.92387953f));
    Quat_SlerpWith(sines, rz, rz, 0.25f, q);  // equal keys lerp
    CHECK(near(q[2], 0.70710677f) && near(q[3], 0.70710677f));
    const float flip[4] = {0, 0, 0, -1};
    Quat_SlerpWith(sines, id, flip, 1, q);  // opposite: through (-y, x, -w, z); w is never blended
    CHECK(q[0] == 0 && q[1] == 0 && q[2] == -1 && q[3] == 0);

    const RotationKey keys[2] = {{0, -1, {0, 0, 0, 1}}, {10, -1, {0, 0, 1, 0}}};
    float m[9];
    Motion_RotationAtWith(sines, keys, 2, 2, 1, 0.5f, m);  // the last key holds: 180 degrees about z
    CHECK(m[0] == -1 && m[4] == -1 && m[8] == 1);
    Motion_RotationAtWith(sines, keys, 2, 2, 0, 0.5f, m);  // flag 0x2 lerps: (0, 0, .5, .5), 90 degrees
    CHECK(m[0] == 0 && m[1] == 1 && m[3] == -1 && m[8] == 1);
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

static void test_matrix() {
    float m[16];
    Matrix_Identity(m);
    CHECK(m[0] == 1 && m[5] == 1 && m[10] == 1 && m[15] == 1 && m[1] == 0 && m[12] == 0);
    m[12] = 1, m[13] = 2, m[14] = 3, m[2] = 7;
    Matrix_ResetRotation(m);  // the translation stays
    CHECK(m[2] == 0 && m[12] == 1 && m[13] == 2 && m[14] == 3);
    float v[3] = {4, 5, 6};
    Matrix_TransformPoint(m, v);
    CHECK(v[0] == 5 && v[1] == 7 && v[2] == 9);
    Matrix_TransformVector(m, v);
    CHECK(v[0] == 5 && v[1] == 7 && v[2] == 9);

    Matrix_RotateZ(m, 0, 1);  // 90 degrees about z: x axis -> y, translation untouched
    CHECK(m[0] == 0 && m[1] == 1 && m[4] == -1 && m[5] == 0 && m[12] == 1 && m[13] == 2);
    Matrix_RotateZ4(m, 0, 1);  // the 4 form turns the translation too
    CHECK(m[0] == -1 && m[1] == 0 && m[12] == -2 && m[13] == 1);
    Matrix_Identity(m);
    Matrix_RotateY(m, 0, 1);  // z axis -> x
    CHECK(m[8] == 1 && m[10] == 0 && m[0] == 0 && m[2] == -1);
    auto near = [](double a, double b) { return a - b < 1e-6 && b - a < 1e-6; };
    CHECK(near(Matrix_Heading(m), 1.5707963267948966));
    const float up[3] = {1, 0, 0};
    CHECK(near(Vec3_AngleY(up), 1.5707963267948966) && Vec3_AngleZ(up) < -1.57 && Vec3_AngleX(up) == 0);

    static float values[4096];
    SinTable sines{values, 0, 0};
    SinTable_Build(values, 4096, &sines.mask, &sines.scale);
    const float angles[3] = {0, 1.5707964f, 0};
    Matrix_Identity(m);
    Matrix_RotateEulerWith(sines, m, 0, angles);  // "FUK": nothing
    Matrix_RotateEulerWith(sines, m, 10, angles);  // "XYZ": 90 degrees about y
    CHECK(m[0] == values[2048] && m[2] == -1 && m[8] == 1 && m[5] == 1);  // cos(pi/2) is sin(pi_f): -8.7e-8
    Matrix_Identity(m);
    Matrix_RotateEuler(m, 2, angles + 1);  // "Y" through fsin/fcos
    CHECK(near(m[8], 1) && near(m[2], -1) && near(m[0], 0));

    Matrix_Identity(m);
    m[0] = 3, m[1] = 4;
    const float two = 2, one = 1, half = 0.5f;
    Matrix_SetAxisLengths(m, &two, &one, &half);
    CHECK(m[0] == 1.2f && m[1] == 1.6f && m[5] == 1 && m[10] == 0.5f);
    Matrix_ScaleAxes(m, 10, 1, 2);
    CHECK(m[0] == 12 && m[10] == 1);
    const float x[3] = {1, 2, 3};
    Matrix_SetAxes(m, nullptr, x, nullptr);
    CHECK(m[0] == 12 && m[4] == 1 && m[5] == 2 && m[6] == 3);
    Matrix_FromPitchYaw(m, 0, 0);
    CHECK(m[0] == 1 && m[5] == 1 && m[10] == 1 && m[6] == 0 && m[9] == 0);
}

static void test_matrix_products() {
    float t[16], r[16], m[16];
    Matrix_Identity(t);
    t[12] = 1, t[13] = 2, t[14] = 3;
    Matrix_Identity(r);
    Matrix_RotateZ(r, 0, 1);  // (x, y) -> (-y, x)
    std::memcpy(m, t, sizeof m);
    Matrix_Mul(m, r);  // row vectors: translate, then rotate
    float v[3] = {1, 0, 0};
    Matrix_TransformPoint(m, v);
    CHECK(v[0] == -2 && v[1] == 2 && v[2] == 3);
    float p[16];
    std::memcpy(p, r, sizeof p);
    Matrix_PreMul(t, p);  // the same product, into the second argument
    CHECK(std::memcmp(p, m, sizeof p) == 0);
    std::memcpy(p, r, sizeof p);
    Matrix_PreMul3(t, p);  // rotation parts only: the translation of r (none) stays
    CHECK(p[1] == 1 && p[4] == -1 && p[12] == 0 && p[13] == 0);

    Matrix_Invert(m, t);
    CHECK(m[12] == -1 && m[13] == -2 && m[14] == -3 && m[0] == 1 && m[15] == 1 && m[1] == 0);
    Matrix_Invert(t, t);  // in place
    CHECK(std::memcmp(t, m, sizeof t) == 0);
    float lu[16] = {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, parity;
    s32 index[4];
    Matrix_LUDecompose(lu, index, &parity);  // rows 0 and 1 swap
    CHECK(index[0] == 1 && index[1] == 1 && index[2] == 2 && index[3] == 3 && parity == -1);
    float sing[16] = {1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    Matrix_LUDecompose(sing, index, &parity);  // singular: zero pivots become 1e-20
    CHECK(index[0] == 1 && index[1] == 3 && index[2] == 2 && index[3] == 3 && parity == 1);
    CHECK(sing[5] == 1e-20f && sing[15] == 1e-20f && sing[7] == 1);  // ties pivot on the last row

    Matrix_Identity(m);
    const float dir[3] = {1, 0, 0};
    Matrix_LookAlong(m, dir);  // x = y cross dir, y = dir cross x
    CHECK(m[0] == 0 && m[1] == 0 && m[2] == -1 && m[4] == 0 && m[5] == 1 && m[6] == 0 && m[8] == 1 && m[10] == 0);

    const float a[3] = {1, 1, 1}, b[3] = {4, 5, 1};
    CHECK(Vec3_Distance(a, b) == 5);

    const float keys[5] = {3, 1, 2, 1, 0};
    s32 in[5] = {0, 4, 8, 12, 16}, out[5];
    Sort_MergeByKey(5, in, out, 0, reinterpret_cast<const u8 *>(keys));
    CHECK(out[0] == 16 && out[1] == 4 && out[2] == 12 && out[3] == 8 && out[4] == 0);  // stable
}

static void test_vector() {
    float v[3] = {1, 0, 0};
    Vec3_RotateYCS(v, 0, 1);  // 90 degrees about y: x -> -z
    CHECK(v[0] == 0 && v[1] == 0 && v[2] == -1);
    float whole = 0;
    CHECK(Float_Modf(2.75f, &whole) == 0.75 && whole == 2);
    CHECK(Float_Modf(-1.5f, nullptr) == -0.5);

    static float values[4096];
    SinTable sines{values, 0, 0};
    SinTable_Build(values, 4096, &sines.mask, &sines.scale);
    float y[3] = {0, 1, 0};
    Vec3_RotateXWith(sines, y, 0);  // cos 0 = values[1024] = 1, sin 0 = values[0] = 0
    CHECK(y[0] == 0 && y[1] == 1 && y[2] == 0);
    Vec3_RotateXWith(sines, y, 1.5707964f);  // y -> z, with the table's cos(pi/2)
    CHECK(y[1] == values[2048] && y[2] == 1);
    float x[3] = {1, 0, 0}, m[16];
    Vec3_RotateYWith(sines, x, 1.5707964f);
    Matrix_Identity(m);
    Matrix_RotateYAngleWith(sines, m, 1.5707964f);  // the same turn on the x axis row
    CHECK(x[0] == m[0] && x[2] == m[2] && x[2] == -1);
}

static void test_uv_animation() {
    const UvRange ranges[2] = {{0, 2}, {2, 3}};
    const u16 frames[5] = {0, 10, 0, 4, 8};
    const float uvs[10] = {0, 0, 1, 2, /* UV 1: */ 0, 1, 4, 5, 8, 9};
    const UvTrack track{2, ranges, 0, frames, uvs};
    float blocks[8] = {9, 9, 9, 9, 1, 1, 1, 1};
    Motion_UVsAt(&track, 5, blocks, 0);  // UV 0 half way, UV 1 past its middle key
    CHECK(blocks[0] == 0.5f && blocks[4] == 1 && blocks[1] == 5 && blocks[5] == 6 && blocks[2] == 9);
    Motion_UVsAt(&track, 20, blocks, 1);  // past the last keys: t = 1 on the last pair, added
    CHECK(blocks[0] == 1.5f && blocks[4] == 3 && blocks[1] == 13 && blocks[5] == 15);
}

static void test_motion_keys() {
    const u16 keys[6] = {0, 0xaaaa, 10, 0xaaaa, 20, 0xaaaa};  // 4-byte keys (kind 1)
    float state[16] = {};
    u32 i = 99, j = 99;
    float t = 0;
    auto at = [&](float frame, s32 loop) {
        state[8] = frame;  // +0x20
        return Motion_FindKeys(state, keys, 3, 1, loop, &i, &j, &t);
    };
    CHECK(at(15, 0) == 1 && i == 1 && j == 2 && t == 0.5f);
    CHECK(at(25, 0) == 1 && i == 2 && j == 2 && t == 5);       // last key: frame + 1
    CHECK(at(25, 1) == 1 && i == 2 && j == 0 && t == -0.25f);  // looping: the first key's frame
    CHECK(at(-1, 0) == 1 && i == 2);  // (u32)-1: past every key
    CHECK(at(5, 0) == 1 && i == 0 && j == 1 && t == 0.5f);
    CHECK(Motion_FindKeys(state, keys, 3, 3, 0, &i, &j, &t) == 0);
    CHECK(Motion_FindKeys(state, keys, 0, 1, 0, &i, &j, &t) == 0);

    const u32 mkeys[3] = {0 | 7u << 16, 10 | 8u << 16, 20 | 9u << 16};
    const MaterialAnim anim{1, 3, mkeys};
    u8 mats[16] = {};
    u8 table[0x40] = {}, model[0x60] = {};
    const MaterialAnim *ap = &anim;
    u8 *mp = mats;
    const u16 one = 1;
    std::memcpy(table + 0x18, &one, 2);
    std::memcpy(table + 0x30, &ap, 4);
    std::memcpy(model + 0x50, &mp, 4);
    state[8] = 12;
    Motion_ApplyMaterial(model, table, state);
    CHECK(mats[8 + 2] == 8 && mats[8 + 1] == 4);
    mats[9] = 0;
    Motion_ApplyMaterial(model, table, state);  // unchanged: not flagged again
    CHECK(mats[9] == 0 && mats[10] == 8);
}

static void test_packed_vertices() {
    float m[16];
    for (int i = 0; i < 16; ++i) m[i] = (float)(i + 1);
    const float twice[9] = {2, 0, 0, 0, 2, 0, 0, 0, 2};
    Matrix_PreMul33(twice, m);  // rotation part doubled, column 3 and the translation kept
    CHECK(m[0] == 2 && m[6] == 14 && m[10] == 22 && m[3] == 4 && m[11] == 12 && m[12] == 13);
    const float swap[9] = {0, 1, 0, 1, 0, 0, 0, 0, 1};  // swaps the x and y axes
    Matrix_PreMul33(swap, m);
    CHECK(m[0] == 10 && m[4] == 2 && m[8] == 18);

    auto pack = [](s32 x, s32 y, s32 z) { return (u32)(x & 0x3ff) | (u32)(y & 0x3ff) << 10 | (u32)(z & 0x3ff) << 20; };
    float block[11] = {};
    block[5] = 1, block[6] = 2, block[7] = 3;           // offsets (+0x14)
    block[8] = block[9] = block[10] = 0.5f;             // scales (+0x20)
    const u32 keys[4] = {pack(2, -4, 0), pack(0, 0, 0), pack(4, 4, 10), pack(0, 0, 0)};  // key 0, then key 1
    const u16 slots[2] = {5, 0};
    const PackedGroup group{0, 2, slots, nullptr, keys, 0};
    float pos[24] = {};
    pos[13] = 10;
    Motion_LerpPackedVertices(block, pos, &group, 0.5f, 0);  // slot 5: x at 12 * 1 + 1, y and z 4 apart
    CHECK(pos[13] == 2.5f && pos[17] == 2 && pos[21] == 5.5f && pos[0] == 1 && pos[4] == 2 && pos[8] == 3);
    Motion_LerpPackedVertices(block, pos, &group, 0.5f, 1);
    CHECK(pos[13] == 5 && pos[0] == 2);

    float box[6] = {};
    Motion_LerpPackedBox(block, box, &group, 0.5f, 0);  // min from entry 0, max from entry 1, out by s + 0.25
    CHECK(box[0] == 1.75f && box[1] == 1.25f && box[2] == 4.75f && box[3] == 1.75f && box[4] == 2.75f &&
          box[5] == 3.75f);
    Motion_LerpPackedBox(block, box, &group, 0.5f, 1);
    CHECK(box[0] == 3.5f && box[5] == 7.5f);

    // Unpack: box group, raw group, vertex group; frames 0, 10, 20.
    const u16 frames[3] = {0, 10, 20};
    const u32 boxkeys[6] = {pack(0, 0, 0), pack(2, 2, 2), pack(2, 0, 0), pack(4, 2, 2), pack(4, 0, 0), pack(6, 2, 2)};
    const u32 rawkeys[1] = {pack(8, 0, 0)};
    const u32 vtxkeys[3] = {pack(0, 0, 0), pack(4, 0, 0), pack(8, 0, 0)};
    const u16 rawslot[1] = {1}, vtxslot[1] = {2};
    PackedGroup groups[3] = {{0, 2, nullptr, frames, boxkeys, 1}, {0, 1, rawslot, frames, rawkeys, 0},
                             {0, 1, vtxslot, frames, vtxkeys, 1}};
    u8 vblock[0x2c] = {2, 0, 3, 0};
    PackedGroup *gp = groups;
    std::memcpy(vblock + 0xc, &gp, 4);
    std::memcpy(vblock + 0x14, block + 5, 24);  // the offsets and scales above
    float vpos[12] = {};
    u32 node[0x24] = {};
    float *vp = vpos;
    std::memcpy(&node[0x18], &vp, 4);
    Motion_UnpackVertices(node, vblock, 15, 0, 0);  // first unpack: keys restart, then advance to 1
    CHECK(node[0] == 0x810000 && groups[0].key == 1 && groups[2].key == 1);
    CHECK(vpos[1] == 5 && vpos[2] == 4);  // raw 8 * 0.5 + 1; lerp of 2 and 4 by 0.5 at slot 2
    float nbox[6];
    std::memcpy(nbox, &node[0x1e], 24);
    CHECK(nbox[0] == 1.75f && nbox[3] == 4.25f);  // x of 2.5 / 3.5 out by 0.75
    vpos[1] = 0;
    Motion_UnpackVertices(node, vblock, 5, 0, 0);  // flagged: no restart (key stays 1), raw skipped
    CHECK(groups[2].key == 1 && vpos[1] == 0 && vpos[2] == 2);  // t = -1: 3 - 1
}

static void test_track_apply() {
    // Visibility, a scale lerp and a linear path, at frame 5 of 0 .. 10.
    float state[16] = {};
    state[7] = 100, state[8] = 5;  // end (+0x1c), frame (+0x20)
    const s16 visible[2] = {0, -3};  // from frame 3: hidden
    struct ScaleKey { u16 frame, pad; float s[3]; } scales[2] = {{0, 0, {1, 1, 1}}, {10, 0, {3, 3, 3}}};
    const u16 pathkeys[4] = {0, 0, 10, 0};
    const Vec3 points[2] = {{0, 0, 0}, {10, 20, 30}};
    u8 path[16] = {8, 0, 0, 0, 2, 0, 2, 0};  // linear, 2 points, 2 keys
    const Vec3 *pp = points;
    const u16 *kp = pathkeys;
    std::memcpy(path + 8, &pp, 4);
    std::memcpy(path + 12, &kp, 4);
    u8 track[0x40] = {}, model[0xb0] = {};
    const u32 flags = 0x20000 | 0x80 | 0x1;
    const u16 nvis = 2, nscale = 2;
    const void *ptrs[3] = {path, scales, visible};
    std::memcpy(track + 4, &flags, 4);
    std::memcpy(track + 0x12, &nscale, 2);
    std::memcpy(track + 0x16, &nvis, 2);
    std::memcpy(track + 0x18, &ptrs[0], 4);
    std::memcpy(track + 0x20, &ptrs[1], 4);
    std::memcpy(track + 0x30, &ptrs[2], 4);
    u32 node[0x24] = {};
    Matrix_Identity(reinterpret_cast<float *>(node + 4));
    u32 rotations = 0, points_count = 0, motion = 0;
    float values[4096];
    SinTable sines{values, 0, 0};
    SinTable_Build(values, 4096, &sines.mask, &sines.scale);
    const MotionEnv env{&rotations, &points_count, &motion, sines};
    CHECK(Track_ApplyWith(env, state, model, node, track) == 1);
    const float *m = reinterpret_cast<const float *>(node + 4);
    CHECK(node[0] == 0x10 && points_count == 2);
    CHECK(m[0] == 2 && m[5] == 2 && m[10] == 2 && m[1] == 0);   // axes scaled to 2
    CHECK(m[12] == 5 && m[13] == 10 && m[14] == 15 && m[15] == 1);  // half way along the path
    state[8] = 101;  // past the end: nothing happens
    CHECK(Track_ApplyWith(env, state, model, node, track) == 0);

    // The same track twice in a block: the second scales the scaled axes again.
    u8 tracks[0x80];
    std::memcpy(tracks, track, 0x40);
    std::memcpy(tracks + 0x40, track, 0x40);
    u32 *np = node;
    std::memcpy(tracks + 0x3c, &np, 4);
    std::memcpy(tracks + 0x7c, &np, 4);
    u8 block[0x30] = {};
    const u16 two = 2;
    u8 *tp = tracks, *mp = model;
    std::memcpy(block + 0x12, &two, 2);
    std::memcpy(block + 0x2c, &tp, 4);
    u8 holder[8] = {};
    std::memcpy(holder + 4, &mp, 4);
    state[8] = 5;
    Matrix_Identity(reinterpret_cast<float *>(node + 4));
    CHECK(Motion_ApplyWith(env, state, holder, block) == 1);
    CHECK(m[0] == 2 && m[12] == 5);  // set, not doubled: model flag 4 is clear
    model[0x30] = 4;  // adding: the path position adds up
    CHECK(Motion_ApplyWith(env, state, holder, block) == 1 && m[12] == 15 && m[14] == 45);
    CHECK(Motion_Apply(state, holder, nullptr) == 1);

    // Copying a node's vertices and box: only flagged nodes unless all.
    float from_pos[24], to_pos[24] = {};
    for (int i = 0; i < 24; ++i) from_pos[i] = (float)i;
    u8 from_nodes[0xa0] = {}, to_nodes[0xa0] = {}, from_model[0x80] = {}, to_model[0x80] = {};
    const u16 five = 5, one = 1;
    float *fp = from_pos, *tpp = to_pos;
    std::memcpy(from_nodes + 0x58, &five, 2);
    std::memcpy(from_nodes + 0x60, &fp, 4);
    from_nodes[0x78] = 7;
    std::memcpy(to_nodes + 0x58, &one, 2);
    std::memcpy(to_nodes + 0x60, &tpp, 4);
    u8 *fn = from_nodes, *tn = to_nodes, *fm = from_model, *tm = to_model;
    std::memcpy(from_model + 0x70, &fn, 4);
    std::memcpy(to_model + 0x70, &tn, 4);
    std::memcpy(to_model + 0x44, &one, 2);
    u8 from_holder[8] = {}, to_holder[8] = {};
    std::memcpy(from_holder + 4, &fm, 4);
    std::memcpy(to_holder + 4, &tm, 4);
    Motion_CopyVertices(from_holder, to_holder, 0);  // to's node lacks 0x200000
    CHECK(to_pos[23] == 0 && to_nodes[0x78] == 0);
    Motion_CopyVertices(from_holder, to_holder, 1);  // the source's 5 vertices: 2 blocks
    CHECK(to_pos[23] == 23 && to_nodes[0x78] == 7);
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
    test_quat();
    test_slerp();
    test_matrix();
    test_matrix_products();
    test_vector();
    test_uv_animation();
    test_motion_keys();
    test_packed_vertices();
    test_track_apply();
    test_patcher();
    if (argc > 1) test_hook_table(argv[1]);
    std::printf(g_failures ? "%d FAILED\n" : "all tests passed\n", g_failures);
    return g_failures ? 1 : 0;
}
