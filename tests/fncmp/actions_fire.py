# The action table firing of src/game/core/actions_fire against the originals: tables of 0 to 4 entries (enabled or not), searched for words of their entries or others, handlers
# that are recorders (or none); the log records each call and the ECX it was made with.
# usage: actions_fire.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Down,Up)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, recorder  # noqa: E402

CALLS = {'Down': 0x40c900, 'Up': 0x40c990}
B = Bench([(a, a + 0x90) for a in CALLS.values()])
ENTRIES = RAM + 0x200


def case(r):
    name = r.choice(OPS)
    n = r.choice([0, 1, 2, 3, 4, 4])
    table = bytearray(r.bytes(0x10))
    put(table, 0, 'I', ENTRIES)
    put(table, 4, 'hh', n, r.choice([0, 1, 1, 1, 5]))
    entries = bytearray(r.bytes(0x14 * 4))
    for k in range(n):
        put(entries, 0x14 * k + 2, 'HH', r.choice([1, 2, 3, 0x41, 0xffff]), r.choice([0, 1, 2, 3]))
        put(entries, 0x14 * k + 0xc, 'I', recorder(2 * k, 0) if r.random() < 0.7 else 0)
        put(entries, 0x14 * k + 0x10, 'I', recorder(2 * k + 1, 0) if r.random() < 0.7 else 0)
    if n and r.random() < 0.7:
        k = r.randrange(n)
        a, b = struct.unpack_from('<HH', entries, 0x14 * k + 2)
    else:
        a, b = r.choice([1, 2, 3, 0x41, 0xffff, 7]), r.choice([0, 1, 2, 3, 9])
    a |= r.getrandbits(16) << 16 if r.random() < 0.4 else 0
    b |= r.getrandbits(16) << 16 if r.random() < 0.4 else 0
    return Call(CALLS[name], [RAM, a, b], {RAM: table, ENTRIES: entries, LOG: log_region()}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
