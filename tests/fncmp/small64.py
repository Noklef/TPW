# The batch opening and saved-pairs release of src/game/core/small64 (0x565310, 0x57b070) against the originals: drawbuffer.py's stream and batch table (the kind
# values 0xc4, 0x1c4, 0x2c4, 0x3c4 and others; the buffer resumed or not) (Open); an object with the flag set or clear, a count of none to three and the array of pairs whose
# pointers name words (the elements to destroy are none: their count word is 0, as the destructor they get is the game's) (Release).
# usage: small64.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,Release)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import drawbuffer  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x565310, 0x5653e0), (0x57b070, 0x57b130), (0x582640, 0x582804), (0x67afa0, 0x67b0d0)])
OBJ, PAIRS, DEST, ELEMS = RAM + 0x2000, RAM + 0x3000, RAM + 0x3400, RAM + 0x3800


def case(r):
    name = r.choice(OPS)
    if name == 'Open':
        drawbuffer.OPS = ['Resume']
        c = drawbuffer.case(r)
        regions = dict(c.regions)
        regions[OBJ] = r.bytes(0x40)
        regions[0x8bcbc4] = struct.pack('<I', drawbuffer.BUF)
        kind = r.choice([0xc4, 0x1c4, 0x2c4, 0x3c4, 0, 0x1c5, 0xc3, r.getrandbits(32)])
        return Call(0x565310, [OBJ, c.args[1], r.getrandbits(32), kind, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    obj = bytearray(r.bytes(0x140))
    count = r.choice([0, 1, 2, 3, 3, 0xffffffff])
    put(obj, 4, 'I', r.getrandbits(32) | r.choice([0x20000000, 0x20000000, 0]))
    put(obj, 0x12c, 'I', count)
    put(obj, 0x124, 'I', PAIRS if 0 < count < 4 else r.choice([PAIRS, 0]))
    put(obj, 0x128, 'I', r.choice([ELEMS, 0]))
    pairs = bytearray(r.bytes(0x40))
    for k in range(4):
        put(pairs, 8 * k + 4, 'I', DEST + 4 * r.randrange(8))
    regions = {OBJ: obj, PAIRS: pairs, DEST: r.bytes(0x40), ELEMS - 4: struct.pack('<I', 0) + r.bytes(0x40), HEAP: heap()}
    return Call(0x57b070, [OBJ], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Open,Release').split(',') if k]

if __name__ == '__main__':
    B.main(case)
