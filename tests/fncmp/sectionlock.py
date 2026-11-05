# The critical section wrappers (src/game/core/sectionlock) against the originals: the four imports are recorders, the block from the real heap (or none).
# usage: sectionlock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region, put  # noqa: E402

ADDRESS = {'Ctor': 0x633900, 'Dtor': 0x633920, 'GuardDtor': 0x633940, 'Enter': 0x633950, 'Leave': 0x633970}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
B = Bench([(0x633900, 0x633994), (0x67acb0, 0x67acb1), (0x67a400, 0x690000)])
IMPORTS = {'Init': (1, 0x6fd144, 1, 0, 0), 'Delete': (2, 0x6fd0d0, 1, 0, 0), 'Enter': (3, 0x6fd140, 1, 0, 0), 'Leave': (4, 0x6fd130, 1, 0, 0)}
OBJ, BLOCK = RAM, RAM + 0x10


def case(r):
    op = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x10))
    regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), OBJ: obj, BLOCK: r.bytes(0x18)}
    if op == 'Ctor':
        regions[HEAP] = heap(0x40, fail=1 if r.random() < 0.15 else 0)
        return Call(ADDRESS[op], [OBJ], regions, conv='t', ret='a')
    if op == 'Dtor':
        regions[HEAP] = heap(0x40)
        put(obj, 0, 'I', r.choice([HEAP + 0x10, 0]))
        return Call(ADDRESS[op], [OBJ], regions, conv='t', ret='v')
    put(obj, 0, 'I', BLOCK)
    put(obj, 4, 'II', r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)]))
    return Call(ADDRESS[op], [OBJ], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
