# The dynamic face queue writers and the tag sort of src/game/render/dynfaces (0x53c8d0, 0x53c9c0, 0x53c1f0) against the originals: counts of faces from none to full (the 0x806 limit too), tags
# including 1 with the flag byte either way, the square's arguments and the polygon's four corners / uvs / colours; tag words of few or all 32 tags to sort,
# from none (which the original's unsigned compares send to the partition) to 300 (runs of all sizes). Grid (0x53cab0): up to 4 x 4 quads
# (none and negative counts too) from rows and columns 0 to 17 of the 19-wide position and colour tables (only the rows read laid out),
# float offsets and table values, the queue empty, part full, just full or past it. Wave (0x53d380): up to 5 x 5 vertices (none and negative
# counts too), steps and phases floats, a sine table of 64 or 256 entries, sometimes with a huge scale. Ripple (0x53d570):
# the same with up to 6 rows (the fall's reach 4 rows short: none or negative too) and zero steps. Turned (0x53cda0): up to 4 x 3 quads (none
# and negative counts too) of the tables from their start, float offsets, angles and table values (NaNs now and then), tags 0x12 / 0x13 or
# others, a sine table of 64 or 256 entries (sometimes with a huge scale), the queue empty, part full, just full or past it.
# Frame (0x53c3f0): a frame's faces, see frame().
# usage: dynfaces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Square,Poly,Sort,Grid,Wave,Ripple,Turned,Frame)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put, sine_table  # noqa: E402

CALLS = {'Square': 0x53c8d0, 'Poly': 0x53c9c0, 'Sort': 0x53c1f0, 'Grid': 0x53cab0, 'Wave': 0x53d380, 'Ripple': 0x53d570, 'Turned': 0x53cda0, 'Frame': 0x53c3f0}
B = Bench([(0x53c8d0, 0x53c9c0), (0x53c9c0, 0x53cab0), (0x5da3c0, 0x5da3c1), (0x53c1f0, 0x53c3e4), (0x53cab0, 0x53cd9a), (0x53d380, 0x53d567),
           (0x53d570, 0x53d78e), (0x53cda0, 0x53d377), (0x53c3f0, 0x53c803), (0x53d9e0, 0x53ddc2), (0x53ddd0, 0x53df30), (0x53e530, 0x53e61c),
           (0x519590, 0x5195d0), (0x402df0, 0x402df5), (0x4030c0, 0x4030c4), (0x403390, 0x403394), (0x67a830, 0x67a860)])
BLOCKS, ENTRIES, INDICES, POS, NORMALS, UVS, WREC = RAM + 0x1000, RAM + 0x2000, RAM + 0x4000, RAM + 0x8000, RAM + 0x10000, RAM + 0x18000, RAM + 0x1c000
TABLES = 0x870ec8  # (the offsets, the colour table 0x870ed8, the grid tags 0x871d18 and the globals 0x871ed0-0x871f64, one region)


def frame(r):
    """0x53c3f0 (cdecl): the queue holding 0-8 faces (owned by the blocks or not), or 300 / 518 short of full, or full (then owned by
    none); the ground offsets (0x8186b4 ints, 0x820af8 floats), t (0x790a38) and the sine table (64 or 256, now and then a huge scale
    or NaN entries); the grid offsets, the turned grids' offsets (0x763cf4) and the phases any floats, the grid tags any; three blocks
    (two may be the same) owning faces of user 1, 3 or none, their limits 0-30 or any, their arrays shared and large enough for any
    one's faces; the world's record (0x7cf83c's table entry) its +0x5c any; the clock paused or not, its frame time small, negative,
    huge or NaN; the counter 0x874fc8 and the flag 0x763c98 about their turning points."""
    scale = r.choice([1, 100, 5000])
    n0 = r.choice([0, 0, 0, 3, 8, 0x806 - 300, 0x806 - 518, 0x806])
    total = min(0x806, n0 + 518)
    queue = bytearray(r.bytes(8 + 0xa0 * total))
    put(queue, 0, '2I', r.f(scale), r.f(scale))
    for k in range(n0):
        o = 8 + 0xa0 * k
        put(queue, o + 0x90, '2I', r.choice([1, 3, r.getrandbits(32)]) if n0 <= 8 else r.getrandbits(32), 0)
    tags = [r.getrandbits(32)] + [k << 5 | r.getrandbits(5) for k in range(n0)] + [r.getrandbits(32) for _ in range(total - n0 + 1)]
    tables = bytearray(r.bytes(0x10a0))
    g = lambda a, fmt, *v: put(tables, a - TABLES, fmt, *v)  # noqa: E731
    g(0x870ec8, '3I', *(r.f(scale) for _ in range(3)))
    g(0x871ed0, 'I', r.f(20))
    g(0x871f60, 'I', r.f(20))
    g(0x871ee0, 'I', n0)
    blocks = [BLOCKS + 0x100 * r.randrange(3) if r.random() < 0.1 else BLOCKS + 0x100 * k for k in range(3)]
    g(0x871f64, 'I', blocks[0])
    g(0x871ee4, 'I', blocks[1])
    g(0x871ed8, 'I', blocks[2])
    regions = {0x820af8: queue, 0x872164: struct.pack('<%dI' % len(tags), *tags), TABLES: tables,
               ENTRIES: r.bytes(0x1010), INDICES: r.bytes(0x1a00), POS: r.bytes(0x5000), NORMALS: r.bytes(0x5000), UVS: r.bytes(0x3400)}
    for k in range(3):
        block = bytearray(r.bytes(0xa4))
        put(block, 0, 'I', r.choice([1, 3, 1, 3, r.getrandbits(32)]))
        put(block, 0x50, 'I', ENTRIES)
        put(block, 0x64, '4I', POS, NORMALS, UVS, INDICES)
        put(block, 0xa0, 'I', r.choice([0, 1, 4, 8, 8, 30, 0x7fffffff, r.getrandbits(32)]))
        regions[BLOCKS + 0x100 * k] = block
    sines = sine_table(r.choice([64, 256]))
    if r.random() < 0.3:
        sines[0:4] = struct.pack('<f', r.choice([2.0 ** 20, 2.0 ** 23, 3.0 * 2 ** 22, 2.0 ** 26]))
    if r.random() < 0.1:
        for k in range(12, len(sines), 4):
            if r.random() < 0.3:
                struct.pack_into('<I', sines, k, r.choice([0x7fc00000, 0xff812345, 0x7f800001]))
    regions[0x7b095c] = sines
    regions[0x790a38] = struct.pack('<I', r.choice([fb(r.uniform(0, 1000)), fb(r.uniform(0, 10)), r.f(), 0x7fc00000]))
    regions[0x8186b4] = struct.pack('<2i', *(r.choice([0, r.randrange(-64, 64), r.getrandbits(31), -r.getrandbits(31)]) for _ in range(2)))
    regions[0x763cf4] = struct.pack('<32I', *(r.f(scale) for _ in range(32)))
    regions[0x763c98] = bytes([r.choice([0, 1, 0, 1, 0x80])]) + r.bytes(3)
    world = bytearray(r.bytes(0x80))
    idx = r.choice([0, 1, 2, 3])
    put(world, 0, 'H', idx)
    put(world, 0x10 + 0x5c, 'I', r.choice([r.f(4), r.f(), fb(r.uniform(-7, 7)), 0x7fc00000]))
    regions[WREC] = world
    regions[0x7cf83c] = struct.pack('<I', (WREC - 0x1da724) & 0xffffffff)
    regions[0x7cfb90] = struct.pack('<20I', *(WREC + 0x10 if k % 5 == 0 else r.getrandbits(32) for k in range(20)))
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x20, 'd', r.choice([r.uniform(0, 0.1), r.uniform(0, 3), r.uniform(-3, 3), 1e10, float('nan'), 2.0 ** 31 + 0.5]))
    put(clock, 0x30, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    regions[0x785970] = clock
    pos = bytearray(r.bytes(0xe4c))
    put(pos, 0xe40, 'I', r.f(20))
    put(pos, 0xe48, 'i', r.choice([0, 1, 2, 5, 6, 7, r.getrandbits(32) - (1 << 31)]))
    regions[0x874180] = pos
    return Call(CALLS['Frame'], [], regions, ret='v')


def wave(r, name):
    rows = r.choice([1, 2, 3, 5, 5, 0, -1] if name == 'Wave' else [1, 3, 4, 5, 6, 6, 0, -1])
    cols = r.choice([1, 2, 3, 5, 5, 0, -3])
    span = 0xe4 * max(rows, 1) + 12 * max(cols, 1)
    sines = sine_table(r.choice([64, 256]))
    if r.random() < 0.4:   # (a scale large enough that an ulp of an angle moves the index)
        sines[0:4] = struct.pack('<f', r.choice([2.0 ** 20, 2.0 ** 23, 3.0 * 2 ** 22, 2.0 ** 26]))
    regions = {0x874180: r.bytes(span), 0x870ed8: r.bytes(span), 0x7b095c: sines}
    step = r.choice([r.pos(2), r.f(), r.f(50), 0x3f800000] + ([0, 0x80000000] if name == 'Ripple' else []))
    return Call(CALLS[name], [rows & 0xffffffff, cols & 0xffffffff, step, r.f(r.choice([1, 20]))], regions, ret='v')


def grid(r):
    rows, cols = r.choice([1, 2, 3, 4, 4, 0, -1]), r.choice([1, 2, 3, 4, 4, 0, -2])
    row, col = r.choice([0, 3, 0xa, r.randrange(18)]), r.choice([0, 5, 0x10, 17, r.randrange(18)])
    faces = max(rows, 0) * max(cols, 0)
    n = r.choice([0, 3, 0x806 - faces // 2, 0x806, 0x807])
    span = 12 * 19 * (max(rows, 0) + 2)
    pos, col_bytes = bytearray(span), bytearray(span)
    scale = r.choice([1, 100, 5000])
    nans = r.random() < 0.2   # (NaN offsets and positions now and then: of two NaNs the x87 keeps the larger significand)
    nan = lambda: r.getrandbits(1) << 31 | 0x7f800000 | r.randrange(1, 1 << 23)  # noqa: E731
    for k in range(0, span, 4):
        struct.pack_into('<I', pos, k, nan() if nans and r.random() < 0.5 else r.f(scale))
        struct.pack_into('<I', col_bytes, k, r.f(1))
    regions = {0x874180 + 12 * 19 * row: pos, 0x870ed8 + 12 * 19 * row: col_bytes, 0x871d18: r.bytes(4 * 16 + 4),
               0x871ee0: struct.pack('<I', n)}
    if n < 0x806:
        regions[0x820b00 + 0xa0 * n] = r.bytes(0xa0 * min(faces, 0x806 - n) or 4)
        regions[0x872168 + 4 * n] = r.bytes(4 * min(faces, 0x806 - n) or 4)
    args = [nan() if nans and r.random() < 0.7 else r.f(scale) for _ in range(3)] + [rows & 0xffffffff, cols & 0xffffffff, row, col, 0x10, 0x13]
    return Call(CALLS['Grid'], args, regions, ret='v')


def turned(r):
    rows, cols = r.choice([1, 2, 3, 4, 4, 0, -1]), r.choice([1, 2, 3, 3, 0, -2])
    faces = 2 * max(rows, 0) * max(cols, 0)
    n = r.choice([0, 3, 0x806 - faces // 2, 0x806, 0x807])
    span = 12 * 19 * (max(rows, 0) + 1) + 12 * (max(cols, 0) + 1)
    pos, col_bytes = bytearray(span), bytearray(span)
    scale = r.choice([1, 100, 5000])
    nans = r.random() < 0.2   # (NaNs now and then: of two NaNs the x87 keeps the larger significand)
    nan = lambda: r.getrandbits(1) << 31 | 0x7f800000 | r.randrange(1, 1 << 23)  # noqa: E731
    for k in range(0, span, 4):
        struct.pack_into('<I', pos, k, nan() if nans and r.random() < 0.3 else r.f(scale))
        struct.pack_into('<I', col_bytes, k, nan() if nans and r.random() < 0.3 else r.f(1))
    sines = sine_table(r.choice([64, 256]))
    if r.random() < 0.3:   # (a scale large enough that an ulp of an angle moves the index)
        sines[0:4] = struct.pack('<f', r.choice([2.0 ** 20, 2.0 ** 23, 3.0 * 2 ** 22, 2.0 ** 26]))
    if nans and r.random() < 0.5:   # (NaN sines and cosines: the products' operand order decides between two NaNs)
        for k in range(12, len(sines), 4):
            if r.random() < 0.4:
                struct.pack_into('<I', sines, k, nan())
    regions = {0x874180: pos, 0x870ed8: col_bytes, 0x871ee0: struct.pack('<I', n), 0x7b095c: sines}
    if n < 0x806:
        regions[0x820b00 + 0xa0 * n] = r.bytes(0xa0 * min(faces, 0x806 - n) or 4)
        regions[0x872168 + 4 * n] = r.bytes(4 * min(faces, 0x806 - n) or 4)
    args = [nan() if nans and r.random() < 0.5 else r.f(scale) for _ in range(3)]
    args += [nan() if nans and r.random() < 0.4 else r.choice([0x3f000000, r.f(scale)]), rows & 0xffffffff, cols & 0xffffffff, r.choice([0x12, 0x13, 0x13, 0, 0x1f, r.getrandbits(32)]),
             nan() if nans and r.random() < 0.3 else r.f(r.choice([1, 4, 1000]))]
    return Call(CALLS['Turned'], args, regions, ret='v')


def case(r):
    name = r.deal(OPS)
    if name == 'Frame':
        return frame(r)
    if name == 'Turned':
        return turned(r)
    if name == 'Grid':
        return grid(r)
    if name in ('Wave', 'Ripple'):
        return wave(r, name)
    if name == 'Sort':
        n = r.choice([0, 1, 2, 3, 6, 7, 8, 9, 15, 40, 300]) if r.random() < 0.6 else r.randrange(0, 120)
        kinds = r.choice([1, 2, 3, 5, 32])
        words = [r.getrandbits(27) << 5 | (r.randrange(kinds) if kinds < 32 else r.getrandbits(5)) for _ in range(n + 2)]
        if r.random() < 0.15:  # already sorted or reversed
            words[1:n + 1] = sorted(words[1:n + 1], key=lambda w: w & 0x1f, reverse=r.random() < 0.5)
        return Call(CALLS[name], [RAM + 0x1000, n], {RAM + 0x1000: struct.pack('<%dI' % len(words), *words)}, ret='v')
    n = r.choice([0, 1, 2, 5, 0x805, 0x806, 0x807])
    base = 0x820b00 + 0xa0 * min(n, 6)
    regions = {0x820b00: r.bytes(0xa0 * 9), 0x872168: r.bytes(0x40), 0x871ee0: struct.pack('<I', min(n, 6) if n < 0x806 else n), 0x763c98: bytes([r.choice([0, 1])]) + bytes(3),
               0x874fc4: struct.pack('<I', r.choice([0, 0, 1, 5])), RAM: r.bytes(0x100), RAM + 0x100: r.bytes(0x100), RAM + 0x200: r.bytes(0x100)}
    tag = r.choice([0, 1, 1, 2, 0x1f, 0x20, 0xff, r.getrandbits(32)])
    if name == 'Square':
        w = lambda: r.getrandbits(32)  # noqa: E731
        return Call(CALLS[name], [w(), w(), w(), w(), tag, w(), w(), w(), w(), w(), w()], regions, ret='v')
    return Call(CALLS[name], [RAM, tag, RAM + 0x100, RAM + 0x200, r.getrandbits(32)], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
