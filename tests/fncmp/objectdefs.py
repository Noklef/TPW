# The object definition lookups, walk and schema slots (src/game/world/objectdefs: 0x412e90,
# 0x412ef0, 0x412f50, 0x412e70, 0x412c90, 0x4143f0) against the original: ids among the list
# (repeated, missing), the cache hit (its pointer anything, null included) or missed, counts zero,
# negative or up to the full 0x14a; the walk cursor about the end.
# usage: objectdefs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,At390,At410,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x412e90, 0x412eea), (0x412ef0, 0x412f4c), (0x412f50, 0x412fac), (0x412e70, 0x412e8f),
           (0x412c90, 0x412ca4), (0x4143f0, 0x414404)])
DEFS, ITEMS = RAM, RAM + 0x1000
CALLS = {'Find': 0x412e90, 'At390': 0x412ef0, 'At410': 0x412f50, 'Next': 0x412e70, 'Schema': 0x412c90,
         'TrackSchema': 0x4143f0}


def case(r):
    op = r.choice(OPS)
    n = r.choice([0, 1, r.randrange(2, 20), r.randrange(20, 0x14b), r.randrange(-2, 0)])
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(0x14a * 0x10))
    for i in range(0x14a):
        put(defs, i * 4, 'I', ITEMS + 0x10 * r.randrange(0x14a))
        put(items, i * 0x10, 'H', r.choice([i, r.randrange(16), r.getrandbits(16)]))
    if op in ('Schema', 'TrackSchema'):
        return Call(CALLS[op], [r.getrandbits(32), r.choice([0, 1, r.randrange(64), r.getrandbits(32)])], {}, conv='t', ret='a')
    if op == 'Next':  # the cursor about the end, past it, negative
        put(defs, 0x528, 'i', n)
        put(defs, 0x72c, 'i', max(-1, r.choice([-1, 0, n - 2, n - 1, n, r.randrange(-1, 0x14a)])))
        return Call(0x412e70, [DEFS], {DEFS: defs, ITEMS: items}, conv='t', ret='a')
    id_ = r.choice([r.randrange(16), r.randrange(0x14a), r.getrandbits(16)])
    put(defs, 0x528, 'i', n)
    put(defs, 0x730, 'HxxI', r.choice([id_, id_ ^ 1, r.getrandbits(16)]), r.choice([0, ITEMS, r.getrandbits(32)]))
    return Call(CALLS[op], [DEFS, id_ | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)],
                {DEFS: defs, ITEMS: items}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
