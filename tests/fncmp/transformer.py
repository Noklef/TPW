# The vertex transformer's slot 1 forwarder (src/game/render/transformer: 0x570d20) against the original: the transformer at 0x87b1d8 an
# object whose vtable's slot 1 is a cdecl recorder logging the eight arguments (its result now and then set); the object and the six
# arguments any dwords.
# usage: transformer.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x570d20, 0x570d52)])
XF, VT = RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    if r.random() < 0.6:
        set_return(log, 1, r.getrandbits(32))
    regions = {LOG: log, 0x87b1d8: struct.pack('<I', XF), XF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 4, cdecl={1: 8})}
    args = [r.choice([0, 0x10, 1, r.getrandbits(32)]) for _ in range(6)]
    return Call(0x570d20, [r.getrandbits(32)] + args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
