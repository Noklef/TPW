# The thing browser's patrol lines (src/game/ui/thingbrowser: 0x4b3640 ThingBrowser_SendPatrolLines) against the original: the thing [0x7cc308] (none now and then) of a kind 4-8 mostly with its
# patrol region's two cells (none set now and then; the same column now and then), the flag 0x1000 of [0x790ab0] mostly set; Camera_CentreOnSprite 0x486860 and Map_SendLine 0x536100 cdecl
# recorders 20 and 21, the line's two pointers (the stack's) logged as what they point to (their own benches have them; driver.cpp's kThingPatrolEnv).
# usage: thingpatrol.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, put, strings  # noqa: E402

B = Bench([(0x4b3640, 0x4b3784), (0x506e70, 0x506e8f), (0x506eb0, 0x506ed8)], hooks={0x486860: _recorder(20, 1, cdecl=True), 0x536100: _recorder(21, 5, cdecl=True)})
THING = RAM + 0x100


def cell(x, z):
    return (z << 7 | x) + 1 & 0xffff


def case(r):
    log = log_region()
    blob(log, 21, 3, 3)   # (a slot has one blob: the line's end x and y are adjacent on both stacks, the y pointer logged as 0)
    strings(log, 21, 0, 0b10000)
    thing = bytearray(r.bytes(0x220))
    thing[2] = r.choice([4, 5, 6, 7, 8] * 4 + [3, 9, 0, r.getrandbits(8)])
    x0, z0, z1 = r.randrange(128), r.randrange(128), r.randrange(128)
    x1 = r.choice([x0, r.randrange(128), r.randrange(128), (x0 + 1) % 128])
    low, high = cell(x0, z0), cell(x1, z1)
    shape = r.random()
    if shape < 0.1:
        low, high = 1, 0x4000   # (no region)
    elif shape < 0.2:
        low, high = r.choice([1, r.getrandbits(16)]), r.choice([0x4000, r.getrandbits(16), 0])
    put(thing, 0x20a, 'HH', low, high)
    regions = {LOG: log, THING: thing, 0x7cc308: struct.pack('<I', r.choice([THING] * 19 + [0])),
               0x790ab0: struct.pack('<I', r.getrandbits(32) & ~0x1000 | r.choice([0x1000] * 7 + [0]))}
    return Call(0x4b3640, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
