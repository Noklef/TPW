# The tag table's lookup (src/game/world/tagsystem: 0x50af40) against the original: tables whose records mostly hold their own
# index as id, some ids moved elsewhere or missing, the two excluded ids, ids past the table's end and a few small
# negative ones (whose direct record wraps to just before the table); the record bytes otherwise any.
# usage: tagsystem.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x50af40, 0x50afa0), (0x5da3c0, 0x5da3c1)])
TABLE, COUNT = 0x75eca8, 0x17e
BASE = TABLE - 0xa8  # the region: from just before the table (ids -1 to -5) to the records of ids up to 0x1ff
SIZE = 0xa8 + 0x200 * 0x20


def case(r):
    mem = bytearray(r.bytes(SIZE))
    for i in range(0x200):
        put(mem, 0xa8 + 0x20 * i, 'I', r.choice([i, i, i, i, r.randrange(COUNT), r.randrange(0x200), r.getrandbits(32)]))
    for k in range(5):
        put(mem, 0xa8 - 0x20 * (k + 1), 'I', r.choice([0xffffffff - k, r.getrandbits(32)]))
    id = r.choice([r.randrange(COUNT), r.randrange(COUNT), 0x17e, 0x17f, r.randrange(0x180, 0x200), -r.randrange(1, 6)]) & 0xffffffff
    if id < 0x200 and r.random() < 0.5:  # not at its index: elsewhere in the table (more than once at times), or nowhere
        put(mem, 0xa8 + 0x20 * id, 'I', id ^ r.choice([1, 0x100, 0x80000000]))
        for _ in range(r.choice([0, 1, 1, 2])):
            put(mem, 0xa8 + 0x20 * r.choice([r.randrange(COUNT), r.randrange(COUNT), 0, COUNT - 1, COUNT]), 'I', id)  # (COUNT: just past the end)
    if id >= 0x200 and r.random() < 0.5:
        put(mem, 0xa8 + 0x20 * r.randrange(COUNT), 'I', id)
    return Call(0x50af40, [RAM, id], {BASE: mem}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
