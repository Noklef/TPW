# The terrain's tile cache made and its file read (src/game/terrain/procedural: 0x56e550) against the original, the cache's constructor included and its other calls recorders: the driver kind, the log flag, a name or none, an old cache or none, the allocation failing, Init and ReadLnd failing.
# usage: terraintexinit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return, strings  # noqa: E402

TEXT, DTOR, INIT, READ, ERROR = 4, 5, 6, 7, 8
B = Bench([(0x56e550, 0x56e6c0), (0x5720d0, 0x572100)], hooks={0x5645e0: _recorder(TEXT, 1, cdecl=True), 0x572100: _recorder(DTOR, 0), 0x572110: _recorder(INIT, 2), 0x5723f0: _recorder(READ, 1), 0x56af20: _recorder(ERROR, 2)})
SELF, NAME = RAM, RAM + 0x400
OPS = ['ok', 'ok', 'init fails', 'read fails', 'no memory', 'no name', 'other driver']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    strings(log, TEXT, 1)
    set_return(log, INIT, 0 if op == 'init fails' else r.choice([1, 2, -1]))
    set_return(log, READ, 0 if op == 'read fails' else r.choice([1, 2, -1]))
    self = bytearray(r.bytes(0x240))
    put(self, 0x230, 'I', r.choice([0, RAM + 0x600]))
    regions = {LOG: log, SELF: self, NAME: b'land.lnd\0', HEAP: heap(0x30000, 1 if op == 'no memory' else 0),
               0x8bd524: struct.pack('<I', r.choice([0, 2, 4]) if op == 'other driver' else 1), 0x8bd508: struct.pack('<I', r.getrandbits(32) & ~0x2000000 | r.choice([0, 0x2000000])), 0x8c162c: r.bytes(4)}
    return Call(0x56e550, [SELF, 0 if op == 'no name' else NAME, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
