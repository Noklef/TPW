# The array holder clear and the eleven dword read of src/game/core/small52 (0x5eb090, 0x5eb1a0) against the originals: four array headers with or without blocks
# (Array3D_Free frees them first, so the second free is normally a no-op), and a stream whose slot 2 is a recorder.
# usage: small52.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Read)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

CALLS = {'Clear': 0x5eb090, 'Read': 0x5eb1a0}
B = Bench([(0x5eb090, 0x5eb180), (0x5eb1a0, 0x5eb280), (0x5d3860, 0x5d38b0), (0x67acb0, 0x67acb1), (0x5c4f60, 0x5c4f90)])
VT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    obj = bytearray(r.bytes(0x80))
    if name == 'Clear':
        for off in (0x2c, 0x3c, 0x4c, 0x5c):
            put(obj, off, 'III', r.choice([0, 4, 8]), r.choice([0, 3]), r.choice([0, 4]))
            put(obj, off + 0xc, 'I', r.choice([0, HEAP + 0x100 + off, HEAP + 0x200]))
        return Call(CALLS[name], [RAM], {RAM: obj, HEAP: heap()}, conv='t', ret='v')
    stream = bytearray(r.bytes(0x20))
    put(stream, 0, 'I', VT)
    return Call(CALLS[name], [RAM, RAM + 0x400], {RAM: obj, RAM + 0x400: stream, VT: vtable({2: 2}), LOG: log_region()}, conv='t', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
