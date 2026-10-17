# The saved pairs release of src/game/core/vobj2 (0x57b070 Object_ReleaseSavedPairs) against the original: an object with the flag set or clear, a count of none to three and the
# array of pairs whose pointers name words (the elements to destroy are none: their count word is 0, as the destructor they get is the game's).
# usage: savedpairs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x57b070, 0x57b130), (0x67afa0, 0x67b0d0)])
OBJ, PAIRS, DEST, ELEMS = RAM + 0x2000, RAM + 0x3000, RAM + 0x3400, RAM + 0x3800


def case(r):
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


if __name__ == '__main__':
    B.main(case)
