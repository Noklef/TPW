# The system cursor's calls to Windows (src/game/ui/syscursorshow: 0x66197a, 0x6619a6, 0x6619c0, 0x6619da) against the originals: the imports recorders; a cursor of random bytes.
# usage: syscursorshow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, import_table, log_region  # noqa: E402

SET, SHOW = 10, 11
B = Bench([(0x66197a, 0x6619a6), (0x6619a6, 0x6619c0), (0x6619c0, 0x6619da), (0x6619da, 0x661a06)])


def case(r):
    log = log_region()
    imports = import_table(log, {'SetCursor': (SET, 0x6fd334, 1, 0, 0), 'ShowCursor': (SHOW, 0x6fd30c, 1, 0, 0)})
    regions = {LOG: log, RAM: r.bytes(0x40), IMPORT_TABLE: imports}
    make = r.deal([0, 1, 2, 3])
    if make == 0:
        return Call(0x66197a, [RAM, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')
    if make == 3:
        return Call(0x6619da, [RAM, r.deal([0, 1, 0x10000, 0xffff0000, r.getrandbits(32)])], regions, conv='t', ret='v')
    return Call(0x6619a6 if make == 1 else 0x6619c0, [RAM], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
