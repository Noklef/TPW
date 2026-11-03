# The classic locale's start (std::locale::_Init 0x699da0, src/game/core/iosbase) against the original: made once (its _Locimp constructor a recorder, atexit too; no block: the fields written at 0,
# in both), its name "C", the use it gets for the held copies, or already there. The locks do nothing on both sides.
# usage: stdlocale.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

LOCIMP, ATEXIT = 19, 20
B = Bench([(0x699da0, 0x699ee0), (0x45b340, 0x45b9a0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x699f70: _recorder(LOCIMP, 1), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
OBJ = RAM


def case(r):
    log = log_region()
    h = heap(0x400, fail=r.choice([0, 0, 0, 0, 0, 1]))
    set_return(log, LOCIMP, OBJ)
    obj = bytearray(r.bytes(0x40))
    struct.pack_into('<I', obj, 4, r.choice([0, 1, 0xfffffffe, 0xffffffff]))
    struct.pack_into('<III', obj, 0x1c, 0, 0, 0)
    made = r.random() < 0.3
    regions = {LOG: log, HEAP: h, 0xfb1dc8: struct.pack('<3I', r.getrandbits(32), r.getrandbits(32), OBJ if made else 0), OBJ: obj}
    return Call(0x699da0, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
