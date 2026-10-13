"""Theme Park World base.lnd: the source data for the procedural terrain textures.

Each theme's terrain.wad holds base.md2 (the landscape mesh), <Theme>.tct (a text table of
path and queue tile textures) and base.lnd. When procedural terrain textures are on,
Terrain_Load 0x4504c0 hands base.lnd to TerrainTex_Init 0x56e550, and Lnd_Read 0x560ff0 loads
it. The renderer then blends the source textures into terrain tiles on the fly, using
hand-written MMX code (around 0x55f780) and an LRU tile cache (0x572440).

Layout (little-endian):
  u8   version 3
  u8   ramp[10]   five (centre, width) pairs; each builds a 256-entry blend ramp
                  (0 below centre - width/2, 255 above centre + width/2, linear between)
  arr  control map: 384 x 344 x 4 bytes (the reader also keeps (w-1)/4 x (h-1)/4 = 95 x 85,
       so 4 texels per tile in each direction). byte 0 is 0 or 255 (procedural texturing off/on: off under rivers, lakes and the
       gate area); bytes 1..3 are blend weights between the base textures
  3 detail levels, for 64-, 32- and 16-pixel tiles (only the one matching the texture
  quality is kept; the others are skipped):
    u32  tile size (64, 32, 16)
    u8   level number (0, 1, 2)
    arr  x 13 source textures: the first 6 are colour (4 bytes per pixel: B, G, R and a
         per-texel value the blender uses), the last 7 are 1 byte per pixel (grey
         detail/blend masks). They are about 3 tiles wide so tiles can be cut anywhere.
  arr = u32 width, u32 height, u32 bytes per texel, then width*height*bpp bytes
        (Array3D_Read 0x5e78b0)

<Theme>.tct is plain text: '#' comments, then a section name (PathTex, QueueTex) followed by
"index<TAB>file.tga" lines; the textures themselves are the .wct files in the same wad.

Usage:
  python -I tools/lnd.py <terrain.wad | base.lnd> [out_dir]   print, and write PNGs of level 0
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
    return b''.join(bytes((px[i + 2], px[i + 1], px[i], 255)) for i in range(0, len(px), bpp))


def lnd_files(path):
    if path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.lnd'):
                yield '%s:%s' % (path, name), data
    else:
        yield path, open(path, 'rb').read()


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else None
    for label, data in lnd_files(sys.argv[1]):
        lnd = parse_lnd(data)
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
