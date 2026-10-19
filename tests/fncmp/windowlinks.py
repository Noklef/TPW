# A GUI window's links (src/game/ui/window: 0x65f538, 0x65e526) against the originals.
# Unlink: window.py's tree for Window_SetShown (shown_env: parents +0x60, children from +0x5c by +0x54, the hover's root, point and
# queues), each window's previous sibling +0x58 set to match (now and then 0 for one that has one, and the window's own parent now and then
# 0 too); the window one of the tree's. SetHandler: any window bytes, the handler 0 or any.
# usage: windowlinks.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unlink,SetHandler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, log_region, put  # noqa: E402
from window import RANGES as WINDOW_RANGES, shown_env  # noqa: E402

CALLS = {'Unlink': 0x65f538, 'SetHandler': 0x65e526}
B = Bench([(0x65f538, 0x65f5b7), (0x65e526, 0x65e549)] + WINDOW_RANGES)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    if op == 'SetHandler':
        regions[RAM] = r.bytes(0x120)
        return Call(CALLS[op], [RAM, r.choice([0, 0, r.getrandbits(32), 0x401000])], regions, conv='t', ret='w')
    wins = shown_env(r, regions, log)
    for w in wins:
        put(regions[w], 0x58, 'I', 0)
    for w in wins:   # (each child list's previous links)
        prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
        while c:
            put(regions[c], 0x58, 'I', prev if r.random() < 0.95 else 0)
            prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
    window = r.choice(wins)
    if r.random() < 0.1:
        put(regions[window], 0x60, 'I', 0)
    return Call(CALLS[op], [window], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
