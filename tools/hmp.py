"""Theme Park World .hmp height maps: the collision/placement height grid of an object model.

Every placeable thing (ride, shop, scenery, queue piece, hoarding) has one next to its .md2
model inside its .wad. The game builds it from the model and caches it on disk:
HeightMap_LoadOrBuild 0x451640 reads <model>.hmp, and if it is missing, has a bad header, or
was built for a different footprint, HeightMap_Build 0x451880 rasterises the model
(HeightMap_Rasterize 0x451e20) and writes the file back.

Layout (little-endian, 0x30-byte header; offsets are from the start of the file):
  0x00 u16 version 3
  0x02 u16 magic 0xab1e
  0x04 u16 cells per tile side (5)
  0x06 u16 height range (100 world units)
  0x08 u16 tiles wide, 0x0a u16 tiles deep  (the object's footprint)
  0x0c u32 offset of the cell grid      (always 0x30)
  0x10 u32 offset of the tile heights
  0x14 u32 offset of the tile flags
  0x18 f32 bounds min x, y, z; 0x24 f32 bounds max x, y, z   (y is up; a tile is 10 units)
  cell grid    (w*5) x (h*5) bytes, row by row: highest point of the model over each 2x2-unit
               cell, as byte = y * 2.55 with y clamped to [0, 100)
  tile heights w x h bytes: the highest cell of each tile, or a fixed height when the model
               loader passes flag 0x800 (the park gates; their cell grid is left empty)
  tile flags   w x h bytes: 1 if the footprint marks the tile as used, else 0
For a model with extra animations the builder steps through their frames and writes
FLT_MAX / FLT_MIN bounds instead of the model's; no shipped file has those.

Usage:
  python -I tools/hmp.py <file.hmp | archive.wad | folder> [out_dir]   print, and write PNGs
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402
from wct2png import write_png  # noqa: E402

MAGIC = 0xab1e
SCALE = 2.55   # byte per world unit of height (0x6fe3e8)


def parse_hmp(data):
    version, magic, sub, top, w, h, og, ot, of = struct.unpack_from('<HHHHHHIII', data)
    if version != 3 or magic != MAGIC:
        raise ValueError('not a v3 .hmp (version %d, magic %#x)' % (version, magic))
    gw, gh = w * sub, h * sub
    if og + gw * gh > len(data) or ot + w * h > len(data) or of + w * h > len(data):
        raise ValueError('truncated')
    return {
        'cells_per_tile': sub, 'height_range': top, 'tiles': (w, h),
        'bounds': (struct.unpack_from('<3f', data, 0x18), struct.unpack_from('<3f', data, 0x24)),
        'grid': [list(data[og + y * gw:og + (y + 1) * gw]) for y in range(gh)],
        'tile_height': [list(data[ot + y * w:ot + (y + 1) * w]) for y in range(h)],
        'tile_used': [list(data[of + y * w:of + (y + 1) * w]) for y in range(h)],
    }


def hmp_files(path):
    if os.path.isdir(path):
        for root, _, files in os.walk(path):
            for f in sorted(files):
                yield from hmp_files(os.path.join(root, f)) if f.lower().endswith(('.wad', '.hmp')) else ()
    elif path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.hmp'):
                yield '%s:%s' % (path, name), name, data
    else:
        yield path, os.path.basename(path), open(path, 'rb').read()


def to_png(path, hm, zoom=8):
    """Grey levels = height; tiles the footprint does not use are tinted red."""
    sub = hm['cells_per_tile']
    grid = hm['grid']
    gh, gw = len(grid), len(grid[0])
    rows = []
    for y in range(gh * zoom):
        row = bytearray()
        for x in range(gw * zoom):
            v = grid[y // zoom][x // zoom]
            used = hm['tile_used'][y // zoom // sub][x // zoom // sub]
            row += bytes((v, v if used else v // 2, v if used else v // 2, 255))
        rows.append(bytes(row))
    write_png(path, gw * zoom, gh * zoom, b''.join(rows))


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else None
    if out:
        os.makedirs(out, exist_ok=True)
    count = 0
    for label, name, data in hmp_files(sys.argv[1]):
        hm = parse_hmp(data)
        count += 1
        (x0, y0, z0), (x1, y1, z1) = hm['bounds']
        peak = max(max(r) for r in hm['grid'])
        print('%s: %dx%d tiles, peak %.1f units, bounds y %.1f..%.1f' %
              (label, hm['tiles'][0], hm['tiles'][1], peak / SCALE, y0, y1))
        if out:
            to_png(os.path.join(out, os.path.splitext(name)[0] + '.png'), hm)
    print('%d height maps' % count)


if __name__ == '__main__':
    main()
