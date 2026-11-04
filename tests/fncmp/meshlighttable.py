# The meshes' object made with its falloff table (src/game/render/meshlist: 0x573fd0) against the original: the object there already or not, the block got or not (the error pushed, logged by
# its messages), each of the 128 powers a recorder (the sine's double and the 4.0 logged, past the log's room folded) answering one float for the case, the table stored from it.
# usage: meshlighttable.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, _cdecl_log, _recorder, heap, log_fold, log_region, set_float_return, strings  # noqa: E402

POWK = 12
POW = _recorder(POWK, 4, cdecl=True)


def _pow(e):  # _CIpow: st1 to the power st0, popping both; the recorder sees them as the env's (x, y)
    y, x = e.st.pop(0), e.st.pop(0)
    sp = e.r['ESP']
    e.r['ESP'] = sp - 16
    e.write(sp - 16, struct.pack('<2d', x, y))
    POW(e)
    e.r['ESP'] = sp


B = Bench([(0x573fd0, 0x5740d8), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)], hooks={0x564650: _cdecl_log, 0x67c5d0: _pow})


def case(r):
    log = log_region()
    log_fold(log)
    strings(log, 0x3f, 1)
    set_float_return(log, POWK, r.choice([0.0, 1.0, 0.25, r.uniform(0, 1), r.uniform(-5, 5), 1e-40]))
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    regions = {0x87a248: struct.pack('<I', r.choice([0, 0, 0, 0, 0, 0, 0x1234, r.getrandbits(32)])), 0x879dd8: r.bytes(0x10), HEAP: heap(0x8400, 1 if r.random() < 0.15 else 0),
               0x879ad0: errors, 0x8bd508: struct.pack('<I', flags), LOG: log}
    return Call(0x573fd0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
