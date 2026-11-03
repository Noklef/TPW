# The sky and its sprites (Landscape_DrawSprites 0x5860d0, Landscape_Draw 0x5863c0, src/game/terrain/landscape) against the originals: the transformer's slots 1, 3 and 6 are cdecl recorders (k 1, 3, 6); Batch_Begin (k 11),
# DrawBatch_AddPolygon (k 12), DrawBuffer_End (k 13), Batch_Flush (k 14), DrawBatch_AddClippedTriangles (k 15), Batch_Close (k 16) and atexit (k 17) are recorders; Batch_DrawQuad, the clip codes, the fog, Mat4_Transform,
# Texture_FormatBits and Quad_TurnXY run as they are. The first texture always has an id: without one the original's coordinates are unset stack. A polygon's hash leaves out the vertices' unset dwords 5, 13, 21, 29.
# usage: landscapesky.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_float_return, set_return, strings, vtable  # noqa: E402

B = Bench([(0x5860d0, 0x5863bd), (0x5863c0, 0x586cc8), (0x565f90, 0x566218), (0x561bc0, 0x561c40), (0x5747d0, 0x5749db), (0x590c40, 0x590cb0), (0x578d20, 0x579004), (0x591430, 0x591510)],
          hooks={0x565240: _recorder(11, 5), 0x565bb0: _recorder(12, 5), 0x582780: _recorder(13, 2), 0x565590: _recorder(14, 0), 0x565960: _recorder(15, 4), 0x5658c0: _recorder(16, 1),
                 0x67a7d0: _recorder(17, 1, cdecl=True)})
LS = RAM
OBJECT, BUF, TEX, TEX2, MASK, XF, XFT, TABLES, BATCHES, MATRIX = (RAM + 0x4000 + 0x200 * k for k in range(10))
VERTS = RAM + 0x8000
QUAD_VERTS = RAM + 0x7000
VERTICES = 0xffffffff & ~(1 << 5 | 1 << 13 | 1 << 21 | 1 << 29)
NANS = [0x7fc12345, 0x7f812345, 0xffc00000, 0x7f800000, 0xff812345]
SINES = struct.pack('<256f', *[math.sin(i * math.pi / 128) for i in range(256)])


def fnum(r, lo, hi, nan=0.03):
    return struct.pack('<I', r.choice(NANS)) if r.random() < nan else struct.pack('<f', r.uniform(lo, hi))


def floats(r, n, lo, hi, nan=0.03):
    return b''.join(fnum(r, lo, hi, nan) for _ in range(n))


def texture(r, id_kept):
    t = bytearray(r.bytes(0x48))
    flags = r.getrandbits(32)
    put(t, 8, 'I', flags | 0x1000 if id_kept else flags)
    if id_kept:
        put(t, 0x1c, 'I', r.getrandbits(32) | 1)
    t[0x20:0x30] = floats(r, 4, -2, 2)
    return t


def sprites(r, landscape):
    """0-3 sprites: the texture none, the first or the second; a format or none; mask bits of 1, 2, both or neither."""
    count = r.deal([0, 1, 1, 2, 3])
    put(landscape, 0x21dc, 'i', count)
    for i in range(count):
        tex = r.deal([0, TEX, TEX, TEX2])
        entry = struct.pack('<III', tex, r.choice([0, 0, r.getrandbits(32)]) if tex else r.getrandbits(32) | 1, r.deal([1, 2, 3, 0, r.getrandbits(32)])) + floats(r, 3, -50, 50) + fnum(r, 0, 20)
        landscape[0x1fdc + i * 0x20:0x1fdc + i * 0x20 + len(entry)] = entry
    return count


def world(r, log, render, landscape, last):
    """What Batch_DrawQuad and the calls about it read, for a case with a begun batch `last`."""
    if r.random() < 0.5:
        strings(log, 12, 0, out=1 << 3)
        blob(log, 12, 0, VERTICES)
    else:
        strings(log, 12, 0, out=1 << 0)
        blob(log, 12, 3, 1)
    set_return(log, 12, r.deal([1, 1, 1, 1, 1, 0]))
    set_return(log, 14, r.deal([1, 1, 1, r.getrandbits(32) | 1, 0]))
    set_float_return(log, 3, r.choice([0.0, 1.5, r.uniform(-1e4, 1e4)]))
    start, end = r.uniform(-10, 10), r.uniform(-10, 10)
    fog = struct.pack('<2I3f3fI', r.getrandbits(32), r.getrandbits(32), r.uniform(0, 255), r.uniform(0, 255), r.uniform(0, 255), start, end, 1 / ((end - start) or 1.0), r.getrandbits(32))
    buf = bytearray(r.bytes(0x38))
    put(buf, 0x20, 'II', r.deal([1, 2, 4, 0]), r.deal([3, 6, 0]))
    put(buf, 0x28, 'I', QUAD_VERTS)
    obj = bytearray(r.bytes(0x30))
    put(obj, 0x1c, 'I', 0x20)
    put(obj, 0x20, 'I', BUF)
    clip = bytes(128) if r.random() < 0.8 else bytes(r.choice([0, 0, 1, 2, 4]) for _ in range(128))
    return {LOG: log, LS + 0x1fdc: bytes(landscape[0x1fdc:0x2204]), LS + 0x2ecc: bytes(landscape[0x2ecc:0x2f20]), BUF: buf, OBJECT: obj, TEX: bytes(texture(r, True)), TEX2: bytes(texture(r, False)), MASK: r.bytes(0x30),
            XF: struct.pack('<I', XFT), XFT: vtable({}, 8, cdecl={1: 8, 3: 5, 6: 4}), 0x87b1d8: struct.pack('<I', XF), 0x87a248: struct.pack('<I', MASK), 0x879ab0: struct.pack('<I', OBJECT),
            0x879a20: struct.pack('<I', r.getrandbits(32)), 0x879970: clip, 0x8bcbc4: struct.pack('<II', r.getrandbits(32), render) + fnum(r, -2, 2), 0x8bcc1c: r.bytes(0x24), 0x8bd47c: fog, 0x87af60: fnum(r, -3, 3),
            0x8c63d8: SINES, QUAD_VERTS: r.bytes(0x80)}


def sprites_case(r):
    log = log_region()
    landscape = bytearray(r.bytes(0x2f20))
    sprites(r, landscape)
    landscape[0x2ecc:0x2f0c] = floats(r, 16, -3, 3, 0.01)
    render = r.getrandbits(32) & ~0x400008 | r.deal([0x400000, 0x400000, 0x400000, 0x400000, 0x400000, 0x400000, 0x400008, 8, 0])
    set_return(log, 11, BATCHES)
    batch = bytearray(r.bytes(0x38))
    put(batch, 0x20, 'I', r.deal([0, 1, 4]))
    put(batch, 0x28, 'I', QUAD_VERTS)
    regions = world(r, log, render, landscape, BATCHES)
    regions[BATCHES] = batch
    return Call(0x5860d0, [LS, r.deal([1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


def draw_case(r):
    log = log_region()
    landscape = bytearray(r.bytes(0x2f20))
    count = sprites(r, landscape) if r.random() < 0.3 else 0
    if not count:
        put(landscape, 0x21dc, 'i', 0)
    layers = r.deal([1, 1, 2, 2, 2, 4])
    put(landscape, 0x2ec8, 'i', r.deal([layers] * 12 + [0]))
    put(landscape, 0x1fd0, 'I', r.deal([TEX] * 20 + [0]))
    put(landscape, 0x1fd8, 'I', r.deal([TEX2] * 15 + [0]))
    landscape[0x360:0x360 + 0xc00] = floats(r, 0x300, -100, 100, 0.01)
    landscape[0x1fa0:0x1fd0] = floats(r, 12, -50, 50)
    landscape[0x21ec:0x21fc] = floats(r, 4, -5, 5)
    render = r.getrandbits(32) & ~0x2400008 | r.deal([0, 0, 8]) | r.deal([0, 0x2000000]) | r.deal([0, 0x400000])
    regions = world(r, log, render, landscape, BATCHES)
    batches = {}
    table = []
    for i in range(10):                                                   # (five with room for their vertices, in turn; a batch not begun now and then)
        at = BATCHES + 0x40 * (i % 5)
        if i < 5:
            b = bytearray(r.bytes(0x38))
            put(b, 0x20, 'I', r.deal([0, 1, 2, 4]))
            put(b, 0x28, 'I', VERTS + 0x2000 * i)
            batches[at] = bytes(b)
            batches[VERTS + 0x2000 * i] = r.bytes(0x2000)
        table.append(0 if r.random() < 0.03 else at)
    regions.update(batches)
    regions[TABLES] = struct.pack('<40I', *(table + table + table + table))
    answers(log, 11, TABLES)
    regions[TABLES + 0xa0] = struct.pack('<40I', *[0 if r.random() < 0.04 else r.getrandbits(32) | 1 for _ in range(40)])
    answers(log, 16, TABLES + 0xa0)
    strings(log, 15, 0, out=1 << 0)
    fill(log, 15, 0, struct.pack('<4I', *[r.choice([0x100, 0x48, r.randrange(0x200)]) for _ in range(4)]), records=4)
    set_return(log, 15, r.randrange(0x400))
    blob(log, 1, 3, 0xffffffff)
    blob(log, 6, 1, 0xffff)
    fill(log, 6, 3, floats(r, 28, -3, 3, 0.01), records=2)
    buf = bytearray(regions[BUF])
    room = r.deal(['open', 'open', 'open', 'open', 'open', 'exact', 'vertex', 'index'])
    used, indices, verts, end = struct.unpack_from('<IIII', buf, 0x20)
    vertex_end, index_end = (verts + 0x20 * (used + 0x100)) & 0xffffffff, (end + indices * 2) & 0xffffffff
    obj = bytearray(regions[OBJECT])
    limits = {'open': (0xffffffff, 0xffffffff), 'exact': (vertex_end, index_end), 'vertex': (vertex_end - 1, 0xffffffff), 'index': (0xffffffff, index_end - 1)}[room]
    put(obj, 0x28, 'II', *[x & 0xffffffff for x in limits])
    regions[OBJECT] = bytes(obj)
    regions.update({LS: bytes(landscape[:0x2f20]), MATRIX: floats(r, 16, -3, 3, 0.01), 0x87b154: struct.pack('<I', MATRIX), 0xfb3440: struct.pack('<I', r.getrandbits(32) | 1 if r.random() < 0.7 else r.getrandbits(32) & ~1), 0xfb3460: r.bytes(0x40),
                    0x8bd524: struct.pack('<I', r.deal([0, 1, 2, 3, 5, 0, 1, 2, 3, 5, 0, 1, 2, 3, 5, 4]))})
    del regions[LS + 0x1fdc], regions[LS + 0x2ecc]
    return Call(0x5863c0, [LS], regions, conv='t', ret='a')


def case(r):
    return sprites_case(r) if r.deal(['s', 'd']) == 's' else draw_case(r)


if __name__ == '__main__':
    B.main(case)
