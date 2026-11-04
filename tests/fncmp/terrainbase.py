# Bench: Terrain_Load 0x4504c0: the path strings, the Base / TestBase fallback, the crease bit from the height grid (Vec3_Cross, Normalize and Dot
# run as they are), and the texture and holder calls by the option and holder flags.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402
from texdirs import merged  # noqa: E402

BASE, SHARE, RIDE, MODEL, LEVEL, CELLS, CLONE, DIRS, INIT, PROC, DISABLE, AT20, CREATE, ADD, WAKE = range(1, 16)
B = Bench(merged([(0x4504c0, 0x4508b0), (0x4508b0, 0x4508f1), (0x43a970, 0x43a995), (0x43a9a0, 0x43aa20), (0x5c55b0, 0x5c55b4), (0x67a400, 0x690000)]),
          hooks={0x40ef40: _recorder(BASE, 0, cdecl=True), 0x46e650: _recorder(SHARE, 1, cdecl=True), 0x461a60: _recorder(RIDE, 3, cdecl=True),
                 0x461f10: _recorder(MODEL, 5, cdecl=True), 0x427ad0: _recorder(LEVEL, 1, cdecl=True), 0x46f0c0: _recorder(CELLS, 1, cdecl=True),
                 0x450950: _recorder(CLONE, 2, cdecl=True), 0x46e610: _recorder(DIRS, 0, cdecl=True), 0x56dfe0: _recorder(INIT, 2, cdecl=True),
                 0x46ded0: _recorder(PROC, 1, cdecl=True), 0x56dfd0: _recorder(DISABLE, 0, cdecl=True), 0x46dd50: _recorder(AT20, 1, cdecl=True),
                 0x45bbc0: _recorder(CREATE, 1, cdecl=True), 0x45bcb0: _recorder(ADD, 3, cdecl=True), 0x45bd00: _recorder(WAKE, 1, cdecl=True)})
H, X, MODEL_AT, FIRST_GRID, GRID, HEIGHTS, ATTRS = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x1c0, RAM + 0x200, RAM + 0x240, RAM + 0x300
BASE_TEXT, NAME, DIR_TEXT = RAM + 0x400, RAM + 0x440, RAM + 0x480
NAN = float('nan')


def word(r):
    return bytes(r.choice(b'abcXYZ') for _ in range(r.randrange(1, 20))) + bytes(1)


def heights(r, count, w1):
    style = r.deal(['plane', 'bump', 'nudge', 'nan'])
    a, b, c = r.uniform(-20, 20), r.uniform(-5, 5), r.uniform(-5, 5)
    plane = [a + b * (i % w1) + c * (i // w1) for i in range(count)]
    if style == 'bump':
        plane = [r.uniform(-30, 30) for _ in range(count)]
    elif style == 'nudge':
        plane[r.randrange(count)] += r.choice([0.001, 0.05, 0.3, 2.0])
    elif style == 'nan':
        plane[r.randrange(count)] = r.choice([NAN, float('inf')])
    return struct.pack('<%df' % count, *plane)


def case(r):
    log = log_region()
    set_return(log, BASE, BASE_TEXT)
    strings(log, SHARE, 0, out=1)
    fill(log, SHARE, 0, word(r), which=1)
    strings(log, RIDE, 7)
    strings(log, MODEL, 1)
    strings(log, INIT, 1)
    set_return(log, DIRS, r.choice([DIR_TEXT, DIR_TEXT, 0]))
    found = r.deal(['preset', 'preset', 'preset', 'first', 'second', 'none'])
    if found in ('first', 'second'):
        fill(log, MODEL, 2, struct.pack('<2I', H if found == 'first' else 0, H), records=2)
    w, h = r.randrange(4), r.randrange(4)
    other = r.getrandbits(1)
    grid = GRID if other else FIRST_GRID
    if other:
        fill(log, CELLS, 0, struct.pack('<I', GRID), which=2, offset=0x6c)
    grid_bytes = r.bytes(0x18) + struct.pack('<2I', w, h) + r.bytes(8) + struct.pack('<2I', HEIGHTS, ATTRS)
    regions = {LOG: log, 0x79fcb4: struct.pack('<I', H if found == 'preset' else 0), 0x7a0854: r.bytes(4), 0x7a0884: r.bytes(4), 0x79fc54: r.bytes(4),
               0x7a1a8c: struct.pack('<I', r.deal([0, 0x2000, 0x4000, 0x6000]) | r.getrandbits(32) & ~0x6000), 0xf81704: struct.pack('<I', NAME),
               H: r.bytes(4) + struct.pack('<I', r.deal([0, 0x4000]) | r.getrandbits(32) & ~0x4000) + struct.pack('<I', X) + r.bytes(0xbc),
               X: r.bytes(4) + struct.pack('<I', 0 if r.getrandbits(5) == 0 else MODEL_AT) + r.bytes(8),
               MODEL_AT: r.bytes(0x6c) + struct.pack('<I', FIRST_GRID) + r.bytes(8),
               FIRST_GRID: grid_bytes if grid == FIRST_GRID else r.bytes(0x30), HEIGHTS: heights(r, (w + 1) * (h + 1), w + 1), ATTRS: r.bytes(max(w * h, 1) * 4),
               BASE_TEXT: word(r), NAME: word(r), DIR_TEXT: word(r)}
    if other:
        regions[GRID] = grid_bytes
    return Call(0x4504c0, [r.choice([0, 0, 1, r.getrandbits(32)]), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
