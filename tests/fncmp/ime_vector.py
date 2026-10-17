# The u32 vector inserts of src/game/core/small39 (0x47c740, 0x47cf20) against the originals: vectors of 0 to 6 elements with 0 to 6 spare (a null block when empty), positions
# over the elements, the end, or anywhere in the block, counts 0 to 8, and values that may point into the vector itself.
# usage: small39.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=One,Many)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'One': 0x47c740, 'Many': 0x47cf20}
B = Bench([(0x47c740, 0x47c910), (0x47cf20, 0x47d140), (0x47c910, 0x47c980), (0x47db00, 0x47db20), (0x47c720, 0x47c740), (0x6b70d0, 0x6b70e0), (0x67acb0, 0x67acb1), (0x67a400, 0x690000)])
VEC, BUF, VAL = RAM, RAM + 0x400, RAM + 0x200


def case(r):
    name = r.choice(OPS)
    n = r.choice([0, 1, 2, 3, 4, 6])
    spare = r.choice([0, 0, 1, 2, 3, 6])
    vec = bytearray(r.bytes(0x10))
    if n == 0 and r.random() < 0.6:
        put(vec, 4, '3I', 0, 0, 0)
    else:
        put(vec, 4, '3I', BUF, BUF + 4 * n, BUF + 4 * (n + spare))
    at = r.randrange(n + 1)
    pos = BUF + 4 * at
    if r.random() < 0.1:
        pos = BUF + 4 * r.randrange(n + spare + 1)
    value = RAM + 0x200
    if r.random() < 0.15:
        value = BUF + 4 * r.randrange(n + 1)
    regions = {VEC: vec, BUF: r.bytes(0x100), VAL: r.bytes(0x10), HEAP: heap()}
    if name == 'One':
        return Call(CALLS[name], [VEC, pos, value], regions, conv='t', ret='a')
    return Call(CALLS[name], [VEC, pos, r.choice([0, 1, 1, 2, 3, 4, 8]), value], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
