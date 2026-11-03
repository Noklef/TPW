# The 0x70a338 object made on its block (Class70a338_Make 0x6be090, src/game/core/fieldops) against the original: the vtable 0x70a338 a recorder's (slot 4's first argument, the
# block's own stack slot, logged as 0), answering a status through it or another status word; a null block now and then.
# usage: class70a338make.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, echo, fill, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6be090, 0x6be0d7), (0x6be0e0, 0x6be0f2), (0x6bb950, 0x6bb990)])
BLOCK, INFO, STATUS = RAM, RAM + 0x100, RAM + 0x200
VALUES = [0, 0, 1, 0x7fffffff, 0xffffffff, 0x80000000, 0x88780078]


def case(r):
    log = log_region()
    strings(log, 4, 0, out=1)
    if r.random() < 0.8:
        echo(log, 4, 0)
        fill(log, 4, 0, struct.pack('<I', r.choice(VALUES)))
    else:
        set_return(log, 4, STATUS)
    regions = {LOG: log, BLOCK: r.bytes(0x60), INFO: r.bytes(0x20), STATUS: struct.pack('<I', r.choice(VALUES)), 0x70a338: vtable({4: 5})}
    return Call(0x6be090, [r.getrandbits(32), 0 if r.random() < 0.1 else BLOCK, r.getrandbits(32), INFO], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
