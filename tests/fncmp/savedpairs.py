# The saved pairs release of src/game/core/vobj2 (0x57b070 Object_ReleaseSavedPairs) against the original: an object with the flag set or clear, a count of none to three and the
# array of pairs whose pointers name words (the elements to destroy are none: their count word is 0, as the destructor they get is the game's). And the block release
# (0x57b5b0 Object_FreeBlock13c): no block, or one tracked or not whose texture (as texturemem.py) is set or not.
# usage: savedpairs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Pairs,Block)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x57b070, 0x57b130), (0x67afa0, 0x67b0d0), (0x57b5b0, 0x57b608), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13),
           (0x58dc50, 0x58dd5e), (0x67acb0, 0x67acb1)])
OBJ, PAIRS, DEST, ELEMS = RAM + 0x5000, RAM + 0x6000, RAM + 0x6400, RAM + 0x6800   # (texturemem.py's regions below RAM + 0x4000)
BLOCK, TEX = RAM + 0x7000, RAM + 0x7100
CALLS = {'Pairs': 0x57b070, 'Block': 0x57b5b0}


def block(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    obj = bytearray(r.bytes(0x140))
    put(obj, 0x13c, 'I', BLOCK if r.random() < 0.85 else 0)
    blk = bytearray(r.bytes(0x20))
    regions[TEX] = texturemem.textures(r, regions, TEX, 1)   # (and the tracked table)
    put(blk, 0x18, 'I', TEX if r.random() < 0.8 else 0)
    if r.random() < 0.7:   # the block tracked (the first pair)
        regions[0x87b1f8] = put(bytearray(regions[0x87b1f8]), 4, 'I', BLOCK)
    regions[BLOCK] = blk
    regions[OBJ] = obj
    return Call(0x57b5b0, [OBJ], regions, conv='t', ret='a')


def case(r):
    if r.choice(OPS) == 'Block':
        return block(r)
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


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
