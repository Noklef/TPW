# The EAX 2.0 wrappers (src/game/core/soundeax) against the originals: a buffer at BUF whose +4 is an IKsPropertySet at SET (a COM
# recorder vtable; Get answers 0 or a failure now and then, else its hash); the stack block (data and returned size) is logged as 0.
# usage: soundeax.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, set_return, strings, vtable  # noqa: E402

CALLS = {'query': 0x6c4760}
B = Bench([(0x6c4760, 0x6c47a3)])
BUF, OUT, SET, SVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    strings(log, 3, 0, out=1 << 5 | 1 << 7)   # Get's data and returned-size outs: one stack block
    if r.random() < 0.5:
        set_return(log, 3, r.choice([0, 0, 1, 0x80004005, 0x80070057, 0x80000000, 0x10000]))   # (failures with a low word of 0 too)
    regions = {BUF: r.bytes(4) + struct.pack('<I', SET) + r.bytes(8), OUT: r.bytes(4), SET: struct.pack('<I', SVT) + r.bytes(0xc),
               SVT: vtable({}, com={3: 8}), LOG: log}
    return Call(CALLS[name], [BUF, OUT, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
