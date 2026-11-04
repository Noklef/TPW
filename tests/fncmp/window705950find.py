# The list window's row found by its text (src/game/ui/window705950find: 0x65b864) against the original: 0-6 nodes of random bytes, the compare a recorder (driver.cpp's kWindow705950FindEnv) with its answers from a table.
# usage: window705950find.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put  # noqa: E402

COMPARE = 10
WINDOW, NODES, TABLE, TEXT = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x900
B = Bench([(0x65b864, 0x65b8dc)], hooks={0x67c290: _recorder(COMPARE, 2, cdecl=True)})


def case(r):
    log = log_region()
    answers(log, COMPARE, TABLE)
    window = bytearray(r.bytes(0x180))
    count = r.deal([0, 1, 2, 3, 4, 5, 6])
    put(window, 0x144, 'I', NODES if count else 0)
    nodes = bytearray(r.bytes(0x60 * 6))
    for k in range(count):
        put(nodes, 0x60 * k + 0x44, 'I', NODES + 0x60 * (k + 1) if k + 1 < count else 0)
    table = [r.deal([1, 1, 1, 0, -1 & 0xffffffff, 2]) for _ in range(40)]
    regions = {LOG: log, WINDOW: window, NODES: nodes, TABLE: struct.pack('<40I', *table)}
    return Call(0x65b864, [WINDOW, r.deal([TEXT, TEXT, TEXT, 0])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
