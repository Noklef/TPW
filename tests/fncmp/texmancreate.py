# The texture manager's creation (src/game/gfx/texman: 0x574a20) against the original, Quickload_Locate and TexManager_Load recorders: one there already, the allocation failing or not.
# usage: texmancreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, log_region, put, set_return, strings  # noqa: E402

LOCATE, LOAD = 1, 2
B = Bench([(0x574a20, 0x574b0d), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x6902c0)], hooks={0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x5755f0: _recorder(LOAD, 1), 0x564650: _cdecl_log})
OPS = ['Made', 'Made', 'Made', 'Failed', 'There']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    strings(log, 0x3f, 1)
    strings(log, LOCATE, 1)
    set_return(log, LOCATE, r.getrandbits(32))
    set_return(log, LOAD, r.choice([0, r.getrandbits(32)]))
    errors = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    regions = {LOG: log, RAM: r.bytes(0x10), 0x87a268: struct.pack('<I', RAM if op == 'There' else 0), HEAP: heap(0x10100, 1 if op == 'Failed' else 0), 0x879ad0: errors,
               0x8bd508: struct.pack('<I', r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)), 0x768a48: r.bytes(8)}
    return Call(0x574a20, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
