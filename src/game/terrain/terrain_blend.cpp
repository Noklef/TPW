// Procedural terrain textures: reimplementation of TerrainBlend_Tile (0x55f780) and helpers.
//
// The original blends 16 pixels at a time, partly in MMX. Per run it picks one of six paths
// from the two selectors (all 255 / all 0 / mixed); the paths round differently in byte 0 and
// skip work, so they are kept apart here to reproduce the output byte for byte.
#include "terrain_blend.h"

#include <cstring>
#include <vector>

namespace {

// Pointers into one source texture at the window used for a tile.
struct Window {
    const u8 *base;
    u32 stride;  // bytes per texture row
    u32 bpp;     // 4 for colour, 1 for masks (fixed by the code, not the array's depth)

    Window(const Array3D &tex, u32 bpp_, u32 size, u32 x, u32 y) : stride(tex.width * bpp_), bpp(bpp_) {
        u32 ox = (size * x) % (tex.width - size);
        u32 oy = (size * y) % (tex.height - size);
        base = tex.data + (oy * tex.width + ox) * bpp;
    }
    const u8 *at(u32 r, u32 c) const { return base + r * stride + c * bpp; }
};

inline u8 lerp(int p, int q, int t) { return (u8)(p + ((q - p) * t >> 8)); }

// One colour group: two colours mixed by mask * W1 through a ramp.
struct Group {
    Window c0, c1, mask;
    const u8 *ramp;

    u8 weight(u32 r, u32 c, u8 w1) const { return ramp[*mask.at(r, c) * w1 >> 8]; }
    // The MMX paths work on all four bytes.
    void pure(u32 r, u32 c, u8 w1, u8 *out) const {
        int t = weight(r, c, w1);
        const u8 *a = c1.at(r, c), *b = c0.at(r, c);
        for (int k = 0; k < 4; ++k) out[k] = lerp(a[k], b[k], t);
    }
    // The scalar paths (0x55ee10, 0x55eaa0) skip byte 0.
    void rgb(u32 r, u32 c, u8 w1, u8 *out) const {
        int t = weight(r, c, w1);
        const u8 *a = c1.at(r, c), *b = c0.at(r, c);
        for (int k = 1; k < 4; ++k) out[k] = lerp(a[k], b[k], t);
    }
};

// Selector over a 16-pixel run: max of two masks * weight through a ramp.
enum class Run { Zero, Mixed, Full };

Run selector(const Window &m0, const Window &m1, const u8 *ramp, const u8 *weights, u32 r, u32 c0,
             u8 sel[16]) {
    u8 all = 0xff, any = 0;
    for (u32 i = 0; i < 16; ++i) {
        u8 w = weights[i];
        u8 a = ramp[*m0.at(r, c0 + i) * w >> 8];
        u8 b = ramp[*m1.at(r, c0 + i) * w >> 8];
        sel[i] = a > b ? a : b;
        all &= sel[i];
        any |= sel[i];
    }
    return all == 0xff ? Run::Full : any ? Run::Mixed : Run::Zero;
}

void draw_overlay(const u8 *scratch, u32 size, const TerrainOverlay *ov, u8 *out) {
    const s32 row = (s32)size * 4, total = row * (s32)size;
    s32 start, inner, outer;
    u32 f = ov->flags;
    if (!(f & 0x40)) {
        if (f & 0x08)      { start = total - row; inner = -row; outer = 4; }
        else if (f & 0x10) { start = total - 4;   inner = -4;   outer = -row; }
        else if (f & 0x20) { start = row - 4;     inner = row;  outer = -4; }
        else               { start = 0;           inner = 4;    outer = row; }
    } else {
        if (f & 0x08)      { start = total - row; inner = 4;    outer = -row; }
        else if (f & 0x10) { start = total - 4;   inner = -row; outer = -4; }
        else if (f & 0x20) { start = row - 4;     inner = -4;   outer = row; }
        else               { start = 0;           inner = row;  outer = 4; }
    }
    s32 bpp = ov->bytes_per_pixel;
    if (bpp != 2 && bpp != 4) return;
    s32 step_x = ov->width / (s32)size, step_y = ov->height / (s32)size;
    const u8 *src_row = (const u8 *)ov->pixels;
    for (u32 i = 0; i < size; ++i, start += outer, src_row += step_y * ov->width * bpp) {
        const u8 *src = src_row;
        s32 s = start;
        for (u32 j = 0; j < size; ++j, s += inner, src += step_x * bpp, out += 4) {
            int a, r, g, b;
            if (bpp == 2) {
                u16 w = (u16)(src[0] | src[1] << 8);
                a = (w >> 8) & 0xf0; r = (w >> 4) & 0xf0; g = w & 0xf0; b = (w << 4) & 0xf0;
            } else {
                b = src[0]; g = src[1]; r = src[2]; a = src[3];
            }
            const u8 *p = scratch + s;
            out[0] = 0;
            out[1] = lerp(p[1], r, a);
            out[2] = lerp(p[2], g, a);
            out[3] = lerp(p[3], b, a);
        }
    }
}

}  // namespace

extern "C" void __cdecl Lnd_BuildRamp(u8 *table, s16 centre, s16 width) {
    int half = (width + 1) >> 1;
    int lo = (s16)(centre - half), hi = (s16)(centre + half);  // the edges are words (the original keeps them in 16-bit registers)
    for (int i = 0; i < 256; ++i)
        table[i] = i < lo ? 0 : i > hi ? 0xff : (u8)((i * 255 - lo * 255) / (width + 1));
}

extern "C" bool TerrainBlend_WeightMap(const Array3D *control, u32 x, u32 y, int channel, int size, u8 *out) {
    int n = size >> 2, shift = 0;
    while ((1 << shift) < n) ++shift;
    if ((1 << shift) != n || shift > 8) return false;
    int k = 16 - shift;
    u32 w = control->width;
    for (u32 j = 0; j < 4; ++j) {
        for (u32 i = 0; i < 4; ++i) {
            const u8 *p = control->data + ((4 * y + j) * w + 4 * x + i) * 4 + channel;
            int a = p[0], b = p[4], c = p[w * 4], d = p[w * 4 + 4];
            s32 start = a * 65536, step = (b - a) * (1 << k);
            s32 change = (d - c) * (1 << k) - step;
            u8 *dst = out + (j * size + i) * n;
            for (int r = 0; r < n; ++r, dst += size) {
                s32 v = start;
                for (int col = 0; col < n; ++col, v += step) dst[col] = (u8)((u32)v >> 16);
                start += (c - a) * (1 << k);
                step += change >> shift;
            }
        }
    }
    return true;
}

extern "C" void TerrainBlend_TileInto(const Lnd *lnd, u8 *out, u32 x, u32 y, const TerrainOverlay *overlay,
                                      u8 *scratch) {
    const u32 size = lnd->tile_size;
    const u32 ty = lnd->tiles_y - y - 1;  // tile row from the top of the control map
    const Array3D &ctl = lnd->control;

    const u8 *mid = ctl.data + ((4 * ty + 1) * ctl.width + 4 * x + 1) * 4;
    if ((mid[0] & mid[4] & mid[ctl.width * 4] & mid[ctl.width * 4 + 4]) != 0xff) {
        for (u32 i = 0; i < size * size; ++i) std::memcpy(out + 4 * i, "\x00\x7f\x7f\x7f", 4);
        return;
    }

    std::vector<u8> w1(size * size), w2(size * size), w3(size * size);
    TerrainBlend_WeightMap(&ctl, x, ty, 1, (int)size, w1.data());
    TerrainBlend_WeightMap(&ctl, x, ty, 2, (int)size, w2.data());
    TerrainBlend_WeightMap(&ctl, x, ty, 3, (int)size, w3.data());

    u8 ramps[5][256];
    for (int i = 0; i < 5; ++i) Lnd_BuildRamp(ramps[i], lnd->ramps[i][0], lnd->ramps[i][1]);

    const LndLevel &lv = lnd->levels[lnd->level];
    const Group A = {{lv.colour_a0, 4, size, x, ty}, {lv.colour_a1, 4, size, x, ty}, {lv.mask_a, 1, size, x, ty}, ramps[0]};
    const Group B = {{lv.colour_b0, 4, size, x, ty}, {lv.colour_b1, 4, size, x, ty}, {lv.mask_b, 1, size, x, ty}, ramps[1]};
    const Group C = {{lv.colour_c0, 4, size, x, ty}, {lv.colour_c1, 4, size, x, ty}, {lv.mask_c, 1, size, x, ty}, ramps[3]};
    const Window sb0(lv.select_b0, 1, size, x, ty), sb1(lv.select_b1, 1, size, x, ty);
    const Window sc0(lv.select_c0, 1, size, x, ty), sc1(lv.select_c1, 1, size, x, ty);

    for (u32 r = 0; r < size; ++r) {
        u8 *dst_row = scratch + (size - 1 - r) * size * 4;  // written bottom-up
        for (u32 c0 = 0; c0 + 16 <= size; c0 += 16) {
            const u8 *wt = &w1[r * size + c0];
            u8 *dst = dst_row + c0 * 4;
            u8 sel_c[16], sel_b[16];
            Run run_c = selector(sc0, sc1, ramps[4], &w3[r * size + c0], r, c0, sel_c);
            if (run_c == Run::Full) {
                for (u32 i = 0; i < 16; ++i) C.pure(r, c0 + i, wt[i], dst + 4 * i);
                continue;
            }
            Run run_b = selector(sb0, sb1, ramps[2], &w2[r * size + c0], r, c0, sel_b);
            for (u32 i = 0; i < 16; ++i) {
                u32 c = c0 + i;
                u8 *o = dst + 4 * i;
                if (run_c == Run::Zero) {
                    if (run_b == Run::Full) { B.pure(r, c, wt[i], o); continue; }
                    if (run_b == Run::Zero) { A.pure(r, c, wt[i], o); continue; }
                }
                u8 a[4], b[4], cc[4];
                const Group &lower = run_c == Run::Zero ? A : run_b == Run::Full ? B : A;
                lower.rgb(r, c, wt[i], a);
                if (run_c != Run::Zero && run_b == Run::Mixed) {  // 0x55eaa0: A, B, then C
                    B.rgb(r, c, wt[i], b);
                    for (int k = 1; k < 4; ++k) a[k] = lerp(a[k], b[k], sel_b[i]);
                }
                const Group &upper = run_c == Run::Zero ? B : C;
                upper.rgb(r, c, wt[i], cc);
                u8 s = run_c == Run::Zero ? sel_b[i] : sel_c[i];
                o[0] = 0;
                for (int k = 1; k < 4; ++k) o[k] = lerp(a[k], cc[k], s);
            }
        }
    }

    if (overlay && overlay->pixels)
        draw_overlay(scratch, size, overlay, out);
    else
        std::memcpy(out, scratch, size * size * 4);
}

// The entry the hook patches in: __thiscall on the Lnd with 4 stack arguments, which
// __fastcall matches (ECX = this, EDX unused, callee pops the rest). It blends through the
// game's own scratch buffer, and like the original does nothing before one is allocated.
extern "C" void __fastcall TerrainBlend_Tile(const Lnd *self, void * /*edx*/, u8 *out, u32 x, u32 y,
                                             const TerrainOverlay *overlay) {
    u8 *scratch = *reinterpret_cast<u8 *const *>(0x008798c8);  // gTerrainBlendScratch
    if (scratch) TerrainBlend_TileInto(self, out, x, y, overlay, scratch);
}
