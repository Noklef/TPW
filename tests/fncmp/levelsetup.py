# Bench: Level_SetupAfterTerrain 0x457c30: the map rectangle's four getters into globals, the world size from the grid's cell size, the texture rectangle, and the
# calls in order.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

GET_H, GET_C, GET_B, GET_A = 1, 2, 3, 4
CDECLS = [0x450900, 0x53bfe0, 0x450e60, 0x469550, 0x4628a0, 0x469510, 0x452e10, 0x435500, 0x475290]
ALLOC, RECT, TRACK = 14, 15, 16
hooks = {0x519440: _recorder(GET_H, 0), 0x519430: _recorder(GET_C, 0), 0x519420: _recorder(GET_B, 0), 0x519410: _recorder(GET_A, 0),
         0x462870: _recorder(ALLOC, 1, cdecl=True), 0x56df70: _recorder(RECT, 4, cdecl=True), 0x52f050: _recorder(TRACK, 2, cdecl=True)}
hooks.update({a: _recorder(k + 5, 0, cdecl=True) for k, a in enumerate(CDECLS)})
B = Bench([(0x457c30, 0x457d50)], hooks=hooks)
HOLDER, OBJECT, MODEL, GRID = RAM, RAM + 0x10, RAM + 0x40, RAM + 0xc0


def case(r):
    log = log_region()
    for k in (GET_H, GET_C, GET_B, GET_A):
        set_return(log, k, r.choice([0, r.randrange(0x400), r.getrandbits(32)]))
    model = r.deal([MODEL, MODEL, MODEL, 0])
    grid = r.deal([GRID, GRID, GRID, 0])
    size = struct.pack('<2I', r.f(8), r.f(8))
    regions = {LOG: log, 0x7cf6ec: struct.pack('<I', HOLDER), HOLDER: struct.pack('<I', OBJECT), OBJECT: r.bytes(8), 0x7a0854: struct.pack('<I', model),
               MODEL: r.bytes(0x6c) + struct.pack('<I', grid) + r.bytes(8), GRID: r.bytes(0x10) + size + r.bytes(8), 0x7a1a8c: struct.pack('<I', r.deal([0, 0x2000]) | r.getrandbits(32) & ~0x2000),
               0x7a01c0: r.bytes(4), 0x7a006c: r.bytes(4), 0x7a01bc: r.bytes(4), 0x7a029c: r.bytes(4), 0x7a0314: r.bytes(4), 0x7a0374: r.bytes(4), 0x79fe94: r.bytes(4), 0x7a003c: r.bytes(4)}
    return Call(0x457c30, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
