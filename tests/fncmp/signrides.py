# A ride's sign applied and all the marked ones again (src/game/model/signapply: 0x468420 Ride_ApplySign, 0x468500 Rides_ReapplySigns) against
# the originals: the folder's text with or without its last backslash through the real sprintf, Sign_Apply (k 1) with the path hashed, the
# things' list walked, their names (k 2, k 3) filled and handed to Sign_ApplyObject (k 4).
# usage: signrides.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402
from texdirs import merged  # noqa: E402

APPLY, NAMEA, NAMEB, OBJECT = 1, 2, 3, 4
B = Bench(merged([(0x468420, 0x4684f1), (0x468500, 0x4685be), (0x5c5520, 0x5c5530), (0x5da3c0, 0x5da3c1), (0x67a400, 0x690000)]),
          hooks={0x467a80: _recorder(APPLY, 6, cdecl=True), 0x4e2860: _recorder(NAMEA, 2), 0x4e2920: _recorder(NAMEB, 2), 0x4682b0: _recorder(OBJECT, 3, cdecl=True)})
TEXT, RIDES, WORLD = RAM, RAM + 0x100, RAM + 0x1000
THINGS = 0x7cfb90
OPS = [k for k in os.environ.get('OPS', 'one,all').split(',') if k]


def rides(r, regions):
    table = bytearray(0x20)
    for i in range(1, 8):
        at = RIDES + 0x10 * i
        if r.random() < 0.8:
            put(table, 4 * i, 'I', at)
        regions[at] = struct.pack('<I', r.getrandbits(32)) + struct.pack('<I', r.getrandbits(32) & ~0x8000001 | r.choice([0, 1, 0x8000000, 0x8000001])) + r.bytes(8)
    regions[0x7a4610] = bytes(table)


def one(r, regions):
    log = regions[LOG]
    text = bytes(r.choice(b'abcXYZ') for _ in range(r.randrange(1, 40))) + r.choice([b'\\', b'', b'/'])
    regions[TEXT] = text + bytes(4)
    regions[0xf816c0] = r.bytes(8) + struct.pack('<I', r.choice([TEXT, TEXT, TEXT, TEXT, 0])) + r.bytes(4)
    regions[0x78d8d8] = struct.pack('<I', r.choice([0, 1, 1, r.getrandbits(32)]))
    strings(log, APPLY, 1 << 3)
    set_return(log, APPLY, r.getrandbits(32))
    return Call(0x468420, [r.choice([0, 1, 2, 3, 4, 5, 6, 7]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


def everything(r, regions):
    log = regions[LOG]
    count = r.randrange(0, 7)
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    thing_at = RAM + 0x2000
    table = bytearray(0x14 * (count + 2))
    for i in range(count):
        put(table, 0x14 * (i + 1), 'I', thing_at + 0x40 * i)
        m = bytearray(r.bytes(0x40))
        put(m, 0x20, 'I', r.randrange(0, 8))
        m[0xc:0xe] = struct.pack('<H', i + 2 if i + 1 < count else 0)
        regions[thing_at + 0x40 * i] = bytes(m)
    regions[THINGS] = bytes(table)
    regions[WORLD + 0x1da746] = struct.pack('<H', 1 if count else 0)
    fill(log, NAMEA, 0, bytes(r.choice(b'abcd') for _ in range(8)) + b'\0')
    fill(log, NAMEB, 0, bytes(r.choice(b'wxyz') for _ in range(8)) + b'\0', which=1)
    strings(log, NAMEA, 0, out=1)
    strings(log, NAMEB, 0, out=1)
    strings(log, OBJECT, 1 << 1 | 1 << 2)
    return Call(0x468500, [], regions)


def case(r):
    regions = {LOG: log_region()}
    rides(r, regions)
    return (one if r.deal(OPS) == 'one' else everything)(r, regions)


if __name__ == '__main__':
    B.main(case)
