# The vertex transformer's slot 1 forwarder (src/game/render/transformer: 0x570d20) against the original: the transformer at 0x87b1d8 an
# object whose vtable's slot 1 is a cdecl recorder logging the eight arguments (its result now and then set); the object and the six
# arguments any dwords.
# Its making (0x5781d0, op Create): the CPU flags and the 3DNow flag any, the allocation failing now and then, the globals it sets random
# before (the SSE probe 0x579a80, whose __try the emulator does not run, answers 1 as the port's does).
# usage: transformer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Transform,Create)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x570d20, 0x570d52), (0x5781d0, 0x578381)], hooks={0x579a80: lambda e: e.r.__setitem__('EAX', 1)})
XF, VT = RAM + 0x100, RAM + 0x200
OPS = [k for k in os.environ.get('OPS', 'Transform,Create').split(',') if k]


def create(r):
    """0x5781d0 (cdecl): the flags [0x8bd534] with 0x2000000 or not, [0x8bd53c] 0 or not, [0x8bcbcc] any float; the first allocation
    failing now and then."""
    flags = bytearray(r.bytes(0x20))   # 0x8bd520..: +8 the kind, +0x14 the CPU flags, +0x1c the 3DNow flag
    put(flags, 0x14, 'I', r.getrandbits(32) & ~0x2000000 | r.choice([0, 0x2000000]))
    put(flags, 0x1c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {0x8bd520: flags, 0x8bcbcc: struct.pack('<I', r.f()), 0x87af50: r.bytes(0x290), HEAP: heap(fail=r.choice([0, 0, 0, 1]))}
    return Call(0x5781d0, [], regions, conv='c', ret='a')


def case(r):
    if r.deal(OPS) == 'Create':
        return create(r)
    log = log_region()
    if r.random() < 0.6:
        set_return(log, 1, r.getrandbits(32))
    regions = {LOG: log, 0x87b1d8: struct.pack('<I', XF), XF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 4, cdecl={1: 8})}
    args = [r.choice([0, 0x10, 1, r.getrandbits(32)]) for _ in range(6)]
    return Call(0x570d20, [r.getrandbits(32)] + args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
