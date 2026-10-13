"""Theme Park World attribute maps: Terrain\\Base.map in data/levels/<theme>/terrain.wad.

Each theme's terrain.wad holds a 128x128 byte grid that gives every map cell of a new park its
starting state. TerrainAttrMap_Load 0x450900 formats "%s\\Terrain\\Base.map" and hands it to
TP2M_ReadChunks 0x5192d0, a small chunk reader (0x4d3850 / 0x4d3a60) with one handler for the
"MAP " chunk (vtable 0x700aa8: 0x519390 matches the tag, AttrMap_ReadMapChunk 0x4d8cf0 reads it).
(jungle also carries an older terrain.map, which nothing loads.)

Layout (little-endian):
  0x00 char[4]  "TP2M"
  0x04 char[32] "Theme Park 2 Attribute Map File\\0"
  then chunks: char[4] tag, u32 size, size bytes
    "MAP "  u32 width (128), u32 height (128), u32 x 5 (all 8; not read), width*height bytes
    "END "  size 0
The reader stores byte (x, y) in map cell (y - mHeightfieldYStart) * 128 + (x - XStart) + 1
(gBalanceLevel offsets, 0 in the shipped levels), at MapCell.mStatusFlags (+0x26). Cell ids
are 1-based: the cell array is the world's at +0x2d8, 0x44 bytes a cell.

MapCell_Reset 0x536490 turns the byte into the cell's default type (MapCell +0x08) and flags
(+0x0e) whenever a cell is cleared (new park, demolition). Terrain_IsParkLand 0x450420 asks the
base.md2 heightfield whether the cell is park land:
  outside the park:  type 2 if bit 0x02, else 0x1e if bit 0x80, else 7;  flags |= 0x40
  inside the park:   type 1 (path, laid by 0x5348d0, flags = 0x20) if bit 0x08, else type 0;
                     the same 0x01 / 0x02 / 0x80 tests run first, but their results are then
                     overwritten (a missing "else" in the original), so they have no effect here
  bit 0x10           flags |= 0x80
  bit 0x04           flags |= 0x400 (tested by 0x5363b0)
Bit 0x08 also exempts a path cell from the dead-end check in 0x527ee0. Bit 0x01 is not read by
the reset; it outlines the park's border and the land features. Map_Serialize 0x4d7ea0 resets
each cell before saving it and stores only what differs, so a save is a delta against this map.

What the shipped maps hold (the same layout in all four themes, plus jungle's land features):
  0x01  the park border (an 85 x 96 rectangle), a C-shaped outline inside it in every theme,
        and in jungle the river banks and a round patch
  0x11  the entrance plaza west of the park (0x01 | 0x10)
  0x90 / 0x94  the walkway across the plaza to the gate (type 0x1e outside the park);
        0x94 adds 0x04 on 14 of its cells
  0x08  the 5 x 2 path cells the park starts with, just inside the gate
  0x03  jungle's river (0x01 | 0x02: type 2 where the land is not park land), and 0x80 its
        four bridge cells (type 0x1e, like the walkway)
The ASCII view prints . 0, # 0x01, ~ 0x03, p 0x08, e 0x10/0x11, = 0x80, w 0x90, W 0x94.

Usage:
  python -I tools/tp2m.py <file.map | terrain.wad | folder>... [-o out_dir]
      print the chunks, a histogram and an ASCII view; with -o write a PNG per map
"""
import collections
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402
from wct2png import write_png  # noqa: E402

MAGIC = b'TP2M'
TITLE = b'Theme Park 2 Attribute Map File'

# One colour per attribute value seen in the shipped maps (RGB); anything else is magenta.
COLOURS = {
    0x00: (40, 110, 40), 0x01: (200, 200, 200), 0x03: (40, 90, 200), 0x08: (230, 200, 60),
    0x10: (120, 80, 40), 0x11: (170, 130, 90), 0x80: (140, 90, 30), 0x90: (230, 150, 60),
    0x94: (250, 220, 120),
}


def parse_chunks(data):
    if data[:4] != MAGIC or not data[4:0x24].startswith(TITLE):
        raise ValueError('not a TP2M attribute map')
    pos, chunks = 0x24, []
    while pos + 8 <= len(data):
        tag = data[pos:pos + 4].decode('latin-1')
        size = struct.unpack_from('<I', data, pos + 4)[0]
        chunks.append((tag, data[pos + 8:pos + 8 + size]))
        pos += 8 + size
        if tag == 'END ':
            break
    if pos != len(data):
        raise ValueError('%d bytes after the END chunk' % (len(data) - pos))
    return chunks


def parse_map(data):
    """Returns (width, height, the five unread dwords, rows of bytes)."""
    body = dict(parse_chunks(data)).get('MAP ')
    if body is None:
        raise ValueError('no MAP chunk')
    w, h = struct.unpack_from('<II', body)
    extra = struct.unpack_from('<5I', body, 8)
    grid = body[0x1c:]
    if len(grid) != w * h:
        raise ValueError('MAP chunk holds %d bytes for %dx%d' % (len(grid), w, h))
    return w, h, extra, [grid[y * w:(y + 1) * w] for y in range(h)]


SYMBOLS = {0x00: '.', 0x01: '#', 0x03: '~', 0x08: 'p', 0x10: 'e', 0x11: 'e', 0x80: '=',
           0x90: 'w', 0x94: 'W'}


def ascii_view(rows, step=2):
    """One character per cell (SYMBOLS, '?' for other values), every step-th row."""
    return [''.join(SYMBOLS.get(v, '?') for v in row) for row in rows[::step]]


def inputs(path):
    if os.path.isdir(path):
        for root, _, files in os.walk(path):
            for f in sorted(files):
                if f.lower().endswith(('.wad', '.map')):
                    yield from inputs(os.path.join(root, f))
    elif path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if data[:4] == MAGIC:
                yield '%s:%s' % (path, name), data
    else:
        data = open(path, 'rb').read()
        if data[:4] == MAGIC:
            yield path, data


def main():
    args = sys.argv[1:]
    out = None
    if '-o' in args:
        i = args.index('-o')
        out = args[i + 1]
        del args[i:i + 2]
    if not args:
        sys.exit(__doc__)
    for arg in args:
        for label, data in inputs(arg):
            w, h, extra, rows = parse_map(data)
            hist = collections.Counter(b for row in rows for b in row)
            print('%s: %dx%d, extra %s' % (label, w, h, extra))
            print('  ' + ', '.join('%#04x x%d' % kv for kv in sorted(hist.items())))
            for line in ascii_view(rows):
                print('  ' + line.rstrip('.'))
            if out:
                os.makedirs(out, exist_ok=True)
                rgba = bytearray()
                for row in rows:
                    for v in row:
                        rgba += bytes(COLOURS.get(v, (255, 0, 255))) + b'\xff'
                name = label.replace(':', '_').replace('\\', '_').replace('/', '_').replace('.', '_')
                write_png(os.path.join(out, name + '.png'), w, h, bytes(rgba))


if __name__ == '__main__':
    main()
