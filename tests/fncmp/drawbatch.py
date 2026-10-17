# The draw batch opening of src/game/render/drawbuffer (0x565310 DrawBatch_Open) against the original: drawbuffer.py's stream and batch table (the kind values 0xc4, 0x1c4, 0x2c4,
# 0x3c4 and others; the buffer resumed or not).
# usage: drawbatch.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import drawbuffer  # noqa: E402
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x565310, 0x5653e0), (0x582640, 0x582804)])
OBJ = RAM + 0x2000


def case(r):
    drawbuffer.OPS = ['Resume']
    c = drawbuffer.case(r)
    regions = dict(c.regions)
    regions[OBJ] = r.bytes(0x40)
    regions[0x8bcbc4] = struct.pack('<I', drawbuffer.BUF)
    kind = r.choice([0xc4, 0x1c4, 0x2c4, 0x3c4, 0, 0x1c5, 0xc3, r.getrandbits(32)])
    return Call(0x565310, [OBJ, c.args[1], r.getrandbits(32), kind, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
