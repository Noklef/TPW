# An image read from a file (src/game/gfx/imageops: 0x5d3890) against the original: the two names joined, the codec's two calls and the bits' allocation recorders (each failing now and then; the description written first by the one, then by the other); one-byte images copied and four-byte ones with their bytes reversed; an old block or none; the new block's allocation failing.
# usage: imageload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, echo, fill, heap, log_region, mask_this, put, strings  # noqa: E402

INFO, LOAD, ALLOC, FREE = 1, 2, 3, 4
B = Bench([(0x5d3890, 0x5d3aba), (0x5fa680, 0x5fa6a0)],
          hooks={0x5fa8a0: _recorder(INFO, 4), 0x5faa00: _recorder(LOAD, 5), 0x5fa6f0: _recorder(ALLOC, 1), 0x5fa740: _recorder(FREE, 0)})
IMAGE, NAMES, BITS = RAM, RAM + 0x40, RAM + 0x200
NAME = 0x40


def description(r, bits, wide, high, paletted):
    return struct.pack('<4I', *[r.getrandbits(32) for _ in range(4)]) + struct.pack('<I', r.getrandbits(32)) + struct.pack('<I', paletted) + struct.pack('<4I', bits, wide, high, r.getrandbits(32))


def case(r):
    log = log_region()
    image = bytearray(r.bytes(0x10))
    if r.randrange(3) == 0:
        put(image, 0xc, 'I', 0)
    wide, high = r.choice([0, 1, 2, 3, 8, r.randrange(0x10)]), r.choice([0, 1, 2, 3, 8, r.randrange(0x10)])
    paletted = r.choice([1, 1, 0, 0, 2, 0x100, r.getrandbits(32)])
    first = ''.join(r.choice('abcdef/') for _ in range(r.randrange(0x20))).encode() + b'\0'
    second = ''.join(r.choice('xyz.0') for _ in range(r.randrange(0x18))).encode() + b'\0'
    for k in INFO, LOAD, ALLOC:
        echo(log, k, 0)
    strings(log, INFO, 2, 1 | 8)
    strings(log, LOAD, 4, 1 | 2)
    strings(log, ALLOC, 0, 1)
    for k in INFO, LOAD, ALLOC, FREE:
        mask_this(log, k)
    fill(log, INFO, 3, description(r, r.getrandbits(32), r.randrange(0x100), r.randrange(0x100), r.choice([0, 1])), which=0)
    fill(log, INFO, 0, struct.pack('<i', r.choice([0, 0, 0, 0, 1, -1, -0x80000000])), which=1)
    fill(log, LOAD, 1, description(r, BITS, wide, high, paletted), which=2)
    fill(log, LOAD, 0, struct.pack('<i', r.choice([0, 0, 0, 0, 1, -1, -0x80000000])), which=3)
    fill(log, ALLOC, 0, struct.pack('<i', r.getrandbits(31)), which=4)
    regions = {LOG: log, IMAGE: image, NAMES: first.ljust(NAME, b'\0') + second, BITS: r.bytes(0x400), HEAP: heap(0x600, r.choice([0, 0, 0, 1]))}
    return Call(0x5d3890, [IMAGE, NAMES, NAMES + NAME], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
