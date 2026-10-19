# The GUI manager's front ends (src/game/ui/guimanager) against the originals: slots 4 and 3 of the manager [0x77c48c] (a recorder vtable, its
# result now and then set) and its word +4 tested (0x659631); slot 0 of the clock [0x77c490] (the same vtable) (0x65968e). MousePos
# (0x65958a): the words 0xfaa540 / 0xfaa544 any, the two out words apart, adjacent or the same.
# usage: guimanager.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=CallSlot4,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'CallSlot4': 0x659607, 'CallSlot3': 0x65961c, 'Word4Set': 0x659631, 'ClockNow': 0x65968e, 'MousePos': 0x65958a}
B = Bench([(0x659607, 0x65964a), (0x659790, 0x6597c0), (0x65968e, 0x6596a2), (0x65958a, 0x6595a8)])
MANAGER, VTABLE, OUT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    op = r.deal(OPS)
    if op == 'MousePos':
        regions = {0xfaa540: bytearray(r.bytes(8)), OUT: bytearray(r.bytes(0x10))}
        x = OUT + r.choice([0, 2, 4])
        return Call(CALLS[op], [x, r.choice([OUT + 8, x + 2, x])], regions, ret='v')
    log = log_region()
    for k in (0, 3, 4):
        if r.random() < 0.5:
            set_return(log, k, r.choice([0, 1, -1, r.getrandbits(32)]))
    manager = bytearray(r.bytes(0x10))
    put(manager, 0, 'I', VTABLE)
    put(manager, 4, 'H', r.choice([0, 0, 1, 0x8000, 0xffff, r.getrandbits(16)]))
    regions = {LOG: log, VTABLE: vtable({}), MANAGER: manager, 0x77c48c: struct.pack('<II', MANAGER, r.choice([MANAGER, MANAGER, 0]))}
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
