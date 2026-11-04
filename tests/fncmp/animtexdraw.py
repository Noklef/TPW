# The animated textures' sheet drawn (src/game/gfx/animtex: 0x590750) against the original: the flag off or on; six animations of 0-256 frames from a pool of records (the first animation always over one frame: under it the height is a stack slot not yet written, sic); textures of small sizes and some past 24 bits; the clock, the running count, the screen's sizes, the mode (4 clips) any. The renderer's slot 10 a recorder.
# usage: animtexdraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_fold, log_region, vtable  # noqa: E402

B = Bench([(0x590750, 0x5909b7)])
RENDERER, RENDERER_VT, RECORDS, TEXTURES = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x200
SIDES = [0, 1, 8, 16, 32, 64, 128, 256]


def case(r):
    log = log_region()
    log_fold(log)
    level = r.randrange(4) == 0   # (one height, the rows landing exactly on the screen's)
    tall = r.choice(SIDES)
    textures = b''.join(r.bytes(8) + struct.pack('<2i', r.choice(SIDES + [0x1000001]), tall if level else r.choice(SIDES)) + r.bytes(0x10) for _ in range(8))
    records = b''.join(struct.pack('<I', TEXTURES + 0x20 * r.randrange(8)) + r.bytes(4) for _ in range(0x10))
    counts = [r.choice([2, 3, 8, 0x20, 0x100])] + [r.choice([0, 1, 2, 3, 8, 0x20, 0x100]) for _ in range(5)]
    wide, high = r.choice([0, 0x20, 0x80, 0x140, r.randrange(0x200)]), r.choice([0, 0x20, 0x80, 0x140, r.randrange(0x200)])
    if level:
        high = (tall + 2) * r.randrange(1, 6)
    regions = {LOG: log, RENDERER: struct.pack('<I', RENDERER_VT), RENDERER_VT: vtable({10: 6}, 12), RECORDS: records, TEXTURES: textures,
               0x8c4b80: struct.pack('<6I', *counts),
               0x8c4b98: b''.join(struct.pack('<I', RECORDS + 8 * r.randrange(0x10)) for _ in range(0x600)),
               0x8c639c: struct.pack('<B', r.choice([1, 1, 1, 1, 1, 1, 1, 0x80, 0])),
               0x8c63a0: struct.pack('<I', r.choice([0, 1, 2, r.randrange(0x100), r.getrandbits(32)])),
               0x790acc: struct.pack('<f', 0.0 if level else r.choice([0.0, 0.25, 1.0, -1.0, 20.0, r.uniform(-50, 50)])),
               0x8bd524: struct.pack('<I', r.choice([4, 4, 4, 1, 0])),
               0x8bcbd0: struct.pack('<2i', r.choice([wide, wide, 0x40, -1]), r.choice([high, high, 0x40, -1])),
               0x879ab0: struct.pack('<I', RENDERER), 0x7a1968: struct.pack('<I', wide), 0x7a1930: struct.pack('<I', high)}
    return Call(0x590750, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
