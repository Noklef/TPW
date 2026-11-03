# A frame set read from a stream (src/game/gfx/spriteframe: SpriteFrames_Load 0x587db0) against the original: a record stream holding the kind (3 run-length, 2 WCT, others), the count (0-3) and the images, now and then short
# (spritetextures.py's loaders' fields; the WCT header's as there), the stream's slots 0 and 1 recorders; the palette's format table of any bits; the heap failing an array, the palette or the last image's allocation
# (an earlier image's failure is a crash in both, a few); the loading screen's step (bit 0 of 0x8bd508) slot 20's recorder.
# usage: spritesload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put, record_stream, recorder  # noqa: E402

TICK = 20
COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
B = Bench([(0x587db0, 0x588221), (0x5839e0, 0x5839f0), (0x67acf0, 0x67ad80), (0x5c4f60, 0x5c4f7c), (0x5c4f80, 0x5c4fa0), (0x588320, 0x588350), (0x588ba0, 0x588d6e), (0x588230, 0x5882d4), (0x588350, 0x58850d),
           (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x590a00, 0x590a70), (0x590d40, 0x590f7e), (0x67d830, 0x67d8f0),
           (0x56af20, 0x56af9c), (0x588510, 0x58865e), (0x588660, 0x588813), (0x564ba0, 0x564bf0), (0x564d00, 0x564e12), (0x564ff0, 0x565070), (0x67a830, 0x67a860), (0x5646c0, 0x564708), (0x564730, 0x564741),
           (0x564750, 0x564790), (0x564a80, 0x564af0), (0x41a840, 0x41a84a)], hooks={0x587c80: _recorder(TICK, 0)})
SET, STREAM, DATA = RAM, RAM + 0x100, RAM + 0x1000


def rle(r):
    small = r.choice([0, 1, 4, 0x10, 0x40])
    return struct.pack('<I4H2I', small, *r.sample(range(0x10000), 4), r.getrandbits(32), r.getrandbits(32)) + r.bytes(small)


def wct(r):
    header = bytearray(r.bytes(0x2c))
    for at in range(0x14, 0x24, 4):
        put(header, at, 'f', r.choice([r.uniform(0, 100), float(r.randrange(0, 40)), 0.5, 1.0, -3.0, 1e20]))
    sizes = [r.choice([0, 1, 0x10, 0x40]) for _ in range(2)]
    put(header, 0x24, '2I', *sizes)
    put(header, 4, '2H', *(r.choice([0, 1, 0x10, 0x40, r.getrandbits(16)]) for _ in range(2)))
    return bytes(header) + r.bytes(sizes[0]) + r.bytes(sizes[1])


def case(r):
    kind = r.choice([3, 3, 3, 2, 2, 2, 0, 1, 4, 0x103, r.getrandbits(16)])
    count = r.choice([0, 1, 1, 2, 2, 3])
    data = struct.pack('<HHI', kind, r.getrandbits(16), count)
    if kind == 3:
        data += r.bytes(0x400)
    for _ in range(count):
        data += rle(r) if kind == 3 else wct(r) if kind == 2 else b''
    per, first = (2, 4) if kind == 3 else (3, 3)
    fail = r.choice([0] * 14 + [1, 2] + ([3] if kind == 3 else []) + ([first + per * (count - 1)] * 3 if kind in (2, 3) and count else []) + ([first] if count > 1 else []))
    stream = record_stream(STREAM, DATA, len(data) - r.choice([0] * 12 + [1]))
    put(stream, 0x10, '2I', recorder(0, 1), recorder(1, 0))   # (its slots 0 and 1: the finishing and the deleting)
    settings = bytearray(r.bytes(0x4088))
    put(settings, 4, 'I', r.choice([2, 2, 2, 0, 1, 3]))
    regions = {SET: bytearray(r.bytes(0x10)), STREAM: stream, DATA: data, HEAP: heap(0x20000, fail=fail), **global_heap(), LOG: log_region(), 0x8bd550: settings, 0x8bd508: r.bytes(4),
               0x879ad0: put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40])), 0x8798c8: bytes(4), 0x879a38: bytearray(r.bytes(0x2c)),
               TABLE: bytearray(8 * 8), COUNT: bytes(4)}
    return Call(0x587db0, [SET, STREAM, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
