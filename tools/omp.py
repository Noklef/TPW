"""Theme Park World level side files: Scape.omp ambient emitters and the unused test.tpt.

data/levels/<theme>/Scape.omp places the theme's fixed sound and particle emitters (the sea,
the castle, jungle waterfalls). Scape_LoadObjects 0x550e00 reads it when a park loads:

  0x00 "OBJ_"
  0x04 u32 record count
  0x08 u32 record size (0x3c; shorter records are zero-filled, longer ones skipped)
  records, i32 each; positions are world units * 1024 (a tile is 10 units, y is up):
    [0] type    1 sound at a point, 2 particle effect, 3 sound over an area
    [1] type 1/3: 0 = the shared cat_ambient bank, 1 = the theme's own cat_ambient bank
        type 2: particle effect number (0..0x68)
    [2] sound number in that bank (types 1 and 3)
    [3] [4] [5] x, y, z
    [6] type 1: range (Sound_PlayAtRange gets 2 * [6])
    [7] [8] type 3: area size in x and z, from (x, z); may be negative
    [9..14] unused
  then an "INCL" chunk the game never reads: u32 count, then count x (u32 length, path) of
  the editor's header files (par_lib.h, SfxEvent.h, <Theme>Event.h), which named the numbers

test.tpt (jungle and space, identical) is chunked weather test data; tp.exe has none of its
tags. u32 chunk count, then chunks of tag[4], u32 size, data. TEMP, WNDS, WNDD and CLCV
(temperature, wind speed, wind direction, cloud cover) hold u32 n and n (day, value) keys;
WTHR and ADSR are 4 zero bytes.

Usage:
  python -I tools/omp.py <file.omp | file.tpt>...
"""
import os
import struct
import sys

KINDS = {1: 'sound', 2: 'particles', 3: 'sound area'}


def parse_omp(data):
    magic, count, size = struct.unpack_from('<4sII', data)
    if magic != b'OBJ_':
        raise ValueError('not an OBJ_ file')
    out = []
    for i in range(count):
        rec = data[12 + i * size:12 + (i + 1) * size][:0x3c].ljust(0x3c, b'\0')
        out.append(struct.unpack('<15i', rec))
    rest = data[12 + count * size:]
    includes = []
    if rest[:4] == b'INCL':
        pos, n = 8, struct.unpack_from('<I', rest, 4)[0]
        for _ in range(n):
            length = struct.unpack_from('<I', rest, pos)[0]
            includes.append(rest[pos + 4:pos + 4 + length].split(b'\0')[0].decode('latin-1'))
            pos += 4 + length
    return out, includes


def describe(r):
    kind, a, b, x, y, z, rng, w, d = r[:9]
    pos = '(%.1f, %.1f, %.1f)' % (x / 1024, y / 1024, z / 1024)
    if kind == 2:
        return 'particles %d at %s' % (a, pos)
    bank = 'theme' if a else 'shared'
    if kind == 1:
        return 'sound %s/%d at %s range %.1f' % (bank, b, pos, 2 * rng / 1024)
    if kind == 3:
        return 'sound %s/%d over %.1f x %.1f from %s' % (bank, b, w / 1024, d / 1024, pos)
    return 'type %d %r' % (kind, r)


def parse_tpt(data):
    count, pos, out = struct.unpack_from('<I', data)[0], 4, []
    for _ in range(count):
        tag, size = struct.unpack_from('<4sI', data, pos)
        body = data[pos + 8:pos + 8 + size]
        keys = []
        if size >= 4 and size == 4 + 8 * struct.unpack_from('<I', body)[0]:
            keys = [struct.unpack_from('<II', body, 4 + 8 * i) for i in range((size - 4) // 8)]
        out.append((tag.decode('latin-1'), keys, body))
        pos += 8 + size
    return out


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for path in sys.argv[1:]:
        data = open(path, 'rb').read()
        print(path)
        if data[:4] == b'OBJ_':
            records, includes = parse_omp(data)
            for r in records:
                print('  ' + describe(r))
            for inc in includes:
                print('  include ' + inc)
        else:
            for tag, keys, body in parse_tpt(data):
                print('  %s %s' % (tag, ' '.join('%d:%d' % k for k in keys) if keys else body.hex()))


if __name__ == '__main__':
    main()
