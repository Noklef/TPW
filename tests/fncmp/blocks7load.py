# The seven blocks' object loaded (src/game/core/buffers: 0x5d47a0) against the original: five images by name, located and loaded through recorders (Quickload_Locate 0x44a280,
# Image_LoadFile 0x5d3890, each with a bench of its own), the fields before any, the blocks +0x5c and +0x6c set or null (a pointer into the case, freed), the big block failing or not.
# usage: blocks7load.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

LOCATE, LOAD = 1, 2
B = Bench([(0x5d47a0, 0x5d4895), (0x67a400, 0x690000)], hooks={0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x5d3890: _recorder(LOAD, 2)})
OBJECT, OLD = RAM + 0x100, RAM + 0x400


def case(r):
    log = log_region()
    set_return(log, LOCATE, r.getrandbits(32))
    set_return(log, LOAD, r.getrandbits(32))
    block = bytearray(r.bytes(0x80))
    struct.pack_into('<I', block, 0x5c, r.choice([0, OLD, OLD]))
    struct.pack_into('<I', block, 0x6c, r.choice([0, OLD + 0x40, OLD + 0x40]))
    regions = {LOG: log, OBJECT: block, OLD: r.bytes(0x80), HEAP: heap(size=0x98000, fail=r.choice([0, 0, 0, 1]))}
    return Call(0x5d47a0, [OBJECT], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
