# Bench: TexCache_Create 0x58cff0, TexCache_Setup 0x58da40 and TexCache_ChangeFormat 0x58db90 against the original, with the CRT's sprintf running and the rest recorded.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, log_region, set_return, strings  # noqa: E402
from texdirs import merged  # noqa: E402

BUDGET, CREATE, LEVELS, RELEASE, FREE, POOL, FILL, DEFAULTS, ERROR = range(2, 11)
B = Bench(merged([(0x58cff0, 0x58d08a), (0x58da40, 0x58dc10), (0x67a400, 0x690000)]),
          hooks={0x56b630: _recorder(FILL, 0, cdecl=True), 0x58d0f0: _recorder(DEFAULTS, 0), 0x56af20: _recorder(ERROR, 2), 0x564650: _cdecl_log, 0x58d400: _recorder(BUDGET, 0), 0x58d670: _recorder(CREATE, 0), 0x58b220: _recorder(LEVELS, 3),
                 0x58dd60: _recorder(RELEASE, 1), 0x58d380: _recorder(FREE, 0), 0x58b280: _recorder(POOL, 0)})
SELF = RAM


def case(r):
    log = log_region()
    strings(log, 0x3f, 1)
    for k in (BUDGET, CREATE, LEVELS):
        set_return(log, k, r.deal([0, 1, 1, 1, 7]))
    regions = {LOG: log, SELF: r.bytes(0x1804) + struct.pack('<2I', r.randrange(4), r.deal([0, 2])) + r.bytes(0xc0) +
               struct.pack('<I', r.deal([0, 0x20000000]) | r.getrandbits(32) & ~0x20000000) + r.bytes(0x18) + struct.pack('<I', r.randrange(6)) + r.bytes(0x20),
               0x8bd508: struct.pack('<I', r.deal([0, 0x1000000]) | r.getrandbits(32) & ~0x1000000), 0x8bd550: struct.pack('<I', r.deal([0, 2]) | r.getrandbits(32) & ~2),
               0x8bd524: struct.pack('<I', r.deal([1, 4, 2])), 0x8bd554: r.bytes(4), 0x7022e8: r.bytes(4), 0x8c4b44: r.bytes(4)}
    op = r.deal([0, 1, 2])
    if op == 2:
        set_return(log, DEFAULTS, r.deal([SELF, SELF, 0]))
        regions[0x8c4b44] = struct.pack('<I', r.deal([0, 0, 0, 0x12345678]))
        regions[HEAP] = heap(0x2000, fail=r.deal([0, 0, 0, 1]))
        return Call(0x58cff0, [r.randrange(8)], regions, ret='a')
    if op:
        return Call(0x58da40, [SELF, r.randrange(8)], regions, conv='t', ret='a')
    return Call(0x58db90, [SELF, r.deal([0xffffffff, 3, 5])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
