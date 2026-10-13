"""Theme Park World base.lnd: the source data for the procedural terrain textures.

Each theme's terrain.wad holds base.md2 (the landscape mesh), <Theme>.tct (a text table of
path and queue tile textures) and base.lnd. When procedural terrain textures are on,
Terrain_Load 0x4504c0 hands base.lnd to TerrainTex_Init 0x56e550, and Lnd_Read 0x560ff0 loads
it. The renderer then blends the source textures into terrain tiles on the fly, using
hand-written MMX code (around 0x55f780) and an LRU tile cache (0x572440).

Layout (little-endian):
  u8   version 3
  u8   ramp[10]   five (centre, width) pairs; each builds a 256-entry blend ramp (Lnd_BuildRamp
                  0x55e710): 0 below centre - (width+1)/2, 255 above centre + (width+1)/2,
                  (i - lo) * 255 / (width + 1) between
  arr  control map: 384 x 344 x 4 bytes (the reader also keeps (w-1)/4 x (h-1)/4 = 95 x 85,
       so 4 texels per tile in each direction). byte 0 is 0 or 255 (procedural texturing off/on:
       off under rivers, lakes and the gate area); bytes 1..3 are the weights W1, W2, W3
  3 detail levels, for 64-, 32- and 16-pixel tiles (only the one matching the texture
  quality is kept; the others are skipped):
    u32  tile size (64, 32, 16)
    u8   level number (0, 1, 2)
    arr  x 13 source textures: the first 6 are colour (4 bytes per pixel: 0, R, G, B), the
         last 7 are 1 byte per pixel (grey masks). They are about 3 tiles wide so tiles can be
         cut anywhere.
  arr = u32 width, u32 height, u32 bytes per texel, then width*height*bpp bytes
        (Array3D_Read 0x5e78b0)
In memory (Lnd_ReadLevel 0x55f470; 0xd8 bytes a level from +0x24, after u8 tile size, u8
level, the ramps at +2 and the control map at +0x14) the 13 arrays sit in the order
0, 1, 6, 2, 3, 7, 9, 10, 4, 5, 8, 11, 12: each colour pair next to its masks.

The blend (TerrainBlend_Tile 0x55f780, called by the tile cache 0x572440 with an output buffer,
tile x, tile y and an optional overlay) makes one tile at a time. With Y = 84 - y (the tile row
counted from the top of the control map) and size the tile size:
  - unless byte 0 of all four central control texels (4x+1..4x+2, 4Y+1..4Y+2) is 255, the tile
    is flat grey (0x7f7f7f)
  - Lnd_BuildWeightMap 0x55e780 upsamples W1, W2 and W3 bilinearly (16.16 fixed point) from the
    tile's 4 x 4 control texels (and their right / lower neighbours) to size x size
  - each source texture is cut at ((size * x) % (width - size), (size * Y) % (height - size))
  - three colour groups, each two colours mixed by a mask times W1 through a ramp:
        A = lerp(c1, c0, ramp0[m6 * W1 >> 8])      lerp(p, q, t) = p + ((q - p) * t >> 8)
        B = lerp(c3, c2, ramp1[m7 * W1 >> 8])
        C = lerp(c5, c4, ramp3[m8 * W1 >> 8])
    and two selectors: sB = max(ramp2[m9 * W2 >> 8], ramp2[m10 * W2 >> 8])
                       sC = max(ramp4[m11 * W3 >> 8], ramp4[m12 * W3 >> 8])
    pixel = lerp(lerp(A, B, sB), C, sC), worked in runs of 16 pixels: a selector that is 255
    across the run takes that group as is, one that is 0 skips it (0x55ee10 mixes two groups,
    0x55eaa0 all three; the rest is inline MMX)
  - rows come out bottom-up (the tile is mirrored vertically against the control map), then
    the overlay (path or queue tile: ARGB4444 or ARGB8888, flags 0x08/0x10/0x20/0x40 flip or
    turn it) is mixed in by its alpha; the caller swaps the bytes to B, G, R, 0 for Direct3D.

<Theme>.tct is plain text: '#' comments, then a section name (PathTex, QueueTex) followed by
"index<TAB>file.tga" lines; the textures themselves are the .wct files in the same wad.

Usage:
  python -I tools/lnd.py <terrain.wad | base.lnd> [out_dir]   print, and write PNGs of level 0
  python -I tools/lnd.py <terrain.wad | base.lnd> --blend out.png [--level 0|1|2]
      blend every tile of the map (default level 2, 16-pixel tiles) into one picture, in the
      control map's orientation (rows in stored order, so each tile is the game's mirrored)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402
from wct2png import write_png  # noqa: E402


def parse_lnd(data):
    pos = 0

    def array():
        nonlocal pos
        w, h, bpp = struct.unpack_from('<III', data, pos)
        pos += 12 + w * h * bpp
        return {'width': w, 'height': h, 'bpp': bpp, 'pixels': data[pos - w * h * bpp:pos]}

    if data[0] != 3:
        raise ValueError('unknown .lnd version %d' % data[0])
    ramps = [tuple(data[1 + 2 * i:3 + 2 * i]) for i in range(5)]
    pos = 11
    control = array()
    levels = []
    for _ in range(3):
        tile, number = struct.unpack_from('<IB', data, pos)
        pos += 5
        levels.append({'tile': tile, 'level': number, 'textures': [array() for _ in range(13)]})
    if pos != len(data):
        raise ValueError('%d bytes left over' % (len(data) - pos))
    return {'ramps': ramps, 'control': control, 'levels': levels}


def to_rgba(arr):
    px, bpp = arr['pixels'], arr['bpp']
    if bpp == 1:
        return b''.join(bytes((v, v, v, 255)) for v in px)
    return b''.join(bytes((px[i + 1], px[i + 2], px[i + 3], 255)) for i in range(0, len(px), bpp))


def build_ramp(centre, width):
    """Lnd_BuildRamp 0x55e710."""
    half = (width + 1) >> 1
    lo, hi = centre - half, centre + half
    return bytes(0 if i < lo else 255 if i > hi else ((i - lo) * 255 // (width + 1)) & 0xff
                 for i in range(256))


def weight_map(control, x, y, channel, size):
    """Lnd_BuildWeightMap 0x55e780: bilinear 16.16 upsampling of the tile's 4 x 4 control
    texels (channel 1..3) to size x size bytes. y counts from the top of the control map."""
    w, px = control['width'], control['pixels']
    n = size >> 2
    shift = n.bit_length() - 1
    k = 16 - shift
    out = bytearray(size * size)

    def tex(tx, ty):
        return px[(ty * w + tx) * 4 + channel]

    for j in range(4):
        for i in range(4):
            tx, ty = 4 * x + i, 4 * y + j
            a, b, c, d = tex(tx, ty), tex(tx + 1, ty), tex(tx, ty + 1), tex(tx + 1, ty + 1)
            start, step = a << 16, (b - a) << k
            step_change = ((d - c) << k) - step
            for r in range(n):
                v, o = start, (j * n + r) * size + i * n
                for col in range(n):
                    out[o + col] = (v >> 16) & 0xff
                    v += step
                start += (c - a) << k
                step += step_change >> shift
    return out


def window(tex, x, y, size):
    """The size x size cut of a source texture for tile (x, y): rows of bytes, bpp each."""
    w, h, bpp = tex['width'], tex['height'], tex['bpp']
    ox, oy = (size * x) % (w - size), (size * y) % (h - size)
    return [tex['pixels'][((oy + r) * w + ox) * bpp:((oy + r) * w + ox + size) * bpp] for r in range(size)]


def blend_tile(lnd, level, x, y):
    """TerrainBlend_Tile 0x55f780 without the overlay: size*size RGB triples, rows in control
    map order (the game writes them bottom-up). y counts from the top of the control map."""
    lv = lnd['levels'][level]
    size = lv['tile']
    ctl = lnd['control']
    cw, cp = ctl['width'], ctl['pixels']
    if not all(cp[((4 * y + dy) * cw + 4 * x + dx) * 4] == 0xff for dy in (1, 2) for dx in (1, 2)):
        return [(0x7f, 0x7f, 0x7f)] * (size * size)
    ramp = [build_ramp(*r) for r in lnd['ramps']]
    w1, w2, w3 = (weight_map(ctl, x, y, ch, size) for ch in (1, 2, 3))
    t = [window(tex, x, y, size) for tex in lv['textures']]

    def lerp(p, q, s):
        return tuple(pc + ((qc - pc) * s >> 8) for pc, qc in zip(p, q))

    def group(c0, c1, mask, rmp, r, col, wt):
        s = rmp[t[mask][r][col] * wt >> 8]
        return lerp(t[c1][r][col * 4 + 1:col * 4 + 4], t[c0][r][col * 4 + 1:col * 4 + 4], s)

    out = []
    for r in range(size):
        for run in range(0, size, 16):
            cols = range(run, run + 16)
            sel_b = [max(ramp[2][t[9][r][c] * w2[r * size + c] >> 8],
                         ramp[2][t[10][r][c] * w2[r * size + c] >> 8]) for c in cols]
            sel_c = [max(ramp[4][t[11][r][c] * w3[r * size + c] >> 8],
                         ramp[4][t[12][r][c] * w3[r * size + c] >> 8]) for c in cols]
            for i, c in enumerate(cols):
                wt = w1[r * size + c]
                if min(sel_c) == 255:
                    out.append(group(4, 5, 8, ramp[3], r, c, wt))
                    continue
                if min(sel_b) == 255:
                    base = group(2, 3, 7, ramp[1], r, c, wt)
                elif max(sel_b) == 0:
                    base = group(0, 1, 6, ramp[0], r, c, wt)
                else:
                    base = lerp(group(0, 1, 6, ramp[0], r, c, wt), group(2, 3, 7, ramp[1], r, c, wt), sel_b[i])
                if max(sel_c):
                    base = lerp(base, group(4, 5, 8, ramp[3], r, c, wt), sel_c[i])
                out.append(base)
    return out


def blend_map(lnd, level):
    """Every tile of the map: (width, height, RGBA bytes)."""
    size = lnd['levels'][level]['tile']
    ctl = lnd['control']
    tw, th = (ctl['width'] - 1) >> 2, (ctl['height'] - 1) >> 2
    rgba = bytearray(tw * size * th * size * 4)
    stride = tw * size * 4
    for y in range(th):
        for x in range(tw):
            px = blend_tile(lnd, level, x, y)
            for r in range(size):
                row = b''.join(bytes((*p, 255)) for p in px[r * size:(r + 1) * size])
                o = (y * size + r) * stride + x * size * 4
                rgba[o:o + size * 4] = row
    return tw * size, th * size, bytes(rgba)


def lnd_files(path):
    if path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.lnd'):
                yield '%s:%s' % (path, name), data
    else:
        yield path, open(path, 'rb').read()


def main():
    args = sys.argv[1:]
    blend, level = None, 2
    if '--blend' in args:
        i = args.index('--blend')
        blend = args[i + 1]
        del args[i:i + 2]
    if '--level' in args:
        i = args.index('--level')
        level = int(args[i + 1])
        del args[i:i + 2]
    if not args:
        sys.exit(__doc__)
    out = args[1] if len(args) > 1 else None
    for label, data in lnd_files(args[0]):
        lnd = parse_lnd(data)
        if blend:
            write_png(blend, *blend_map(lnd, level))
            print('%s: wrote %s' % (label, blend))
            continue
        c = lnd['control']
        print('%s: ramps %s, control map %dx%dx%d' % (label, lnd['ramps'], c['width'], c['height'], c['bpp']))
        for lv in lnd['levels']:
            print('  %2d px tiles: %s' % (lv['tile'], ' '.join('%dx%dx%d' % (t['width'], t['height'], t['bpp'])
                                                               for t in lv['textures'])))
        if out:
            os.makedirs(out, exist_ok=True)
            px = c['pixels']
            write_png(os.path.join(out, 'control_mask.png'), c['width'], c['height'],
                      b''.join(bytes((v, v, v, 255)) for v in px[0::4]))
            write_png(os.path.join(out, 'control_weights.png'), c['width'], c['height'],
                      b''.join(bytes((px[i + 1], px[i + 2], px[i + 3], 255)) for i in range(0, len(px), 4)))
            for i, t in enumerate(lnd['levels'][0]['textures']):
                write_png(os.path.join(out, 'source%02d.png' % i), t['width'], t['height'], to_rgba(t))


if __name__ == '__main__':
    main()
