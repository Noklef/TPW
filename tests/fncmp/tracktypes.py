# The track type slots of src/game/coaster/tracktypes (0x51fa20 TrackTypeSlot_ReadIntoFree) against the original: a table of 0x140-byte slots with the first byte set or clear (a free one
# within the first six, or none free), read through a stream whose slot at offset 8 is a recorder.
# usage: tracktypes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, vtable  # noqa: E402

B = Bench([(0x51fa20, 0x51fa90), (0x5c4f60, 0x5c4f90), (0x5da3c0, 0x5da3c1)])
STREAM, VT = RAM, RAM + 0x1000


def case(r):
    full = r.random() < 0.05
    free = r.randrange(0, 6)
    size = 0x80c7c8 - 0x804370 if full else 0x140 * (free + 1) + 0x120
    table = bytearray(r.bytes(size))
    for k in range((size - 0x118) // 0x140):
        table[0x118 + 0x140 * k] = 0 if (k == free and not full) else r.choice([1, 2, 0xff])
    regions = {0x804370: table, 0x816d40: struct.pack('<I', r.choice([1, 1, 1, 0])), STREAM: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({2: 2}), LOG: log_region()}
    return Call(0x51fa20, [STREAM], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
