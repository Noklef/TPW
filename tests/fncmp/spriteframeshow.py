# A sprite frame queued for drawing (src/game/gfx/spriteframe: 0x588ef0) against the original: flag 1, no image; an image of frames (+4 set: its size compared, its textures freed and made, recorders; each touched) or a plain one (flag 0x40000, or the frame's point through the matrix recorder, written back by it, against its size near and off each edge of the view).
# usage: spriteframeshow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_fold, log_region, put  # noqa: E402

FREE, MAKE, TOUCH, ADD, ADD2, TRANSFORM = 1, 2, 3, 4, 5, 6
B = Bench([(0x588ef0, 0x5890db)], hooks={0x588ba0: _recorder(FREE, 0), 0x5889b0: _recorder(MAKE, 2), 0x58c9f0: _recorder(TOUCH, 2), 0x58a6b0: _recorder(ADD, 1), 0x58a6e0: _recorder(ADD2, 1), 0x578d20: _recorder(TRANSFORM, 3)})
FRAME, IMAGE = RAM, RAM + 0x100


def view_value(r, depth):
    """A coordinate near the edge the bench tests."""
    return r.choice([0.0, depth, -depth, depth * 0.5, depth + 1.0, -depth - 1.0, depth * 1.5, -depth * 1.5, depth + 10.0, -depth - 10.0])


def case(r):
    log = log_region()
    log_fold(log)
    frame = bytearray(r.bytes(0x40))
    flags = r.getrandbits(32) & ~0x40001 | r.choice([0, 0, 0, 0, 0, 0, 1]) | r.choice([0, 0, 0, 0x40000])
    put(frame, 0x10, 'I', flags)
    put(frame, 0x28, 'I', r.choice([IMAGE, IMAGE, IMAGE, IMAGE, IMAGE, IMAGE, IMAGE, 0]))
    wide, high = r.choice([0, 1, 0x20, 0xffff, r.getrandbits(16)]), r.choice([0, 1, 0x20, 0xffff, r.getrandbits(16)])
    put(frame, 0x30, '2H', wide, high)
    image = bytearray(r.bytes(0x20))
    put(image, 0, 'I', r.choice([0, 0x20000000 + r.randrange(0x100), RAM + 0x200]))
    put(image, 4, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(image, 0x10, '2H', *r.choice([(wide, high), (wide, high), (wide, r.getrandbits(16)), (r.getrandbits(16), high), (r.getrandbits(16), r.getrandbits(16))]))
    put(image, 0x16, 'H', r.choice([0, 1, 2, 5]))
    depth = r.choice([1.0, 8.0, 100.0, -1.0, 0.0])
    put(frame, 0x2c, 'f', r.choice([0.0, 0.05, 0.1, 1.0, -0.05]))
    fill(log, TRANSFORM, 0, struct.pack('<3f', view_value(r, depth), view_value(r, depth), r.choice([1.0, 0.0, -1.0, 8.0, 100.0, 200.0])) + struct.pack('<f', depth), which=0)
    regions = {LOG: log, FRAME: frame, IMAGE: image, 0x8c4b44: r.bytes(4), 0x8c47a0: r.bytes(4), 0x87b088: r.bytes(4)}
    return Call(0x588ef0, [FRAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
