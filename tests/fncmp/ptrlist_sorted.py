# The sorted insertion into a pointer list of src/game/core/ptrlist (0x5e1540 SortedPtrList_Add) against the original: a holder whose list at +8 has none to four nodes whose items carry
# signed keys at +0x24 (equal to the new item's, below or above), and the new item; and PtrList_Append (0x5e42d0) called directly on that list.
# The insertion of a record's copy by its float key (0x6c49b0) into such a list of none to four nodes whose 16-byte records hold float keys at
# +0xc in falling order (equal to the new one's, below or above, now and then a NaN or infinity).
# usage: ptrlist_sorted.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Append,Copy)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, fb, heap, put  # noqa: E402

B = Bench([(0x5e1540, 0x5e15c0), (0x5e2a30, 0x5e2a80), (0x5e2a80, 0x5e2b80), (0x5e42d0, 0x5e4320), (0x67acb0, 0x67acb1),
           (0x6c49b0, 0x6c4b93), (0x6c4fd0, 0x6c4fe3)])
HOLDER, LIST, NODES, ITEMS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400


KEYS = [-5.0, -1.0, 0.0, 0.5, 1.0, 2.0, 100.0, float('inf'), float('-inf')]


def case_copy(r):
    n = r.choice([0, 1, 2, 3, 4])
    keys = sorted((r.choice(KEYS) for _ in range(n)), reverse=True)
    nodes = bytearray(0x20 * 5)
    records = bytearray(r.bytes(0x20 * 6))
    for k in range(n):
        put(nodes, 0x20 * k, 'III', ITEMS + 0x20 * (k + 1), NODES + 0x20 * (k + 1) if k + 1 < n else 0, NODES + 0x20 * (k - 1) if k else 0)
        put(records, 0x20 * (k + 1) + 0xc, 'I', fb(keys[k]) if r.random() < 0.95 else 0x7fc00000)
    put(records, 0xc, 'I', fb(r.choice(keys + [-6.0, 3.0, 1.0, 200.0])) if r.random() < 0.95 else 0xffc00000)
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', NODES if n else 0, NODES + 0x20 * (n - 1) if n else 0, n)
    return Call(0x6c49b0, [LIST, ITEMS], {LIST: lst, NODES: nodes, ITEMS: records, HEAP: heap()}, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Copy':
        return case_copy(r)
    n = r.choice([0, 1, 2, 3, 4])
    keys = sorted(r.choice([-5, -1, 0, 1, 2, 5, 100, 0x7fffffff, -0x80000000]) for _ in range(n))
    nodes = bytearray(0x20 * 5)
    items = bytearray(0x40 * 6)
    for k in range(n):
        put(nodes, 0x20 * k, 'III', ITEMS + 0x40 * (k + 1), NODES + 0x20 * (k + 1) if k + 1 < n else 0, NODES + 0x20 * (k - 1) if k else 0)
        put(items, 0x40 * (k + 1) + 0x24, 'i', keys[k])
    put(items, 0x24, 'i', r.choice(keys + [-6, 3, 200, -0x80000000, 0x7fffffff]))
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', NODES if n else 0, NODES + 0x20 * (n - 1) if n else 0, n)
    holder = bytearray(r.bytes(0x20))
    put(holder, 8, 'I', LIST)
    regions = {HOLDER: holder, LIST: lst, NODES: nodes, ITEMS: items, HEAP: heap()}
    if op == 'Append':
        return Call(0x5e42d0, [LIST, r.choice([ITEMS, 0, r.getrandbits(32)])], regions, conv='t', ret='v')
    return Call(0x5e1540, [HOLDER, ITEMS], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Add,Append,Copy').split(',') if k]


if __name__ == '__main__':
    B.main(case)
