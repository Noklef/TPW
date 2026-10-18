# The game panel class of the object at 0x7ca228 (src/game/ui/panel7ca228: 0x499b40) against the original. Update: the panel's slots 5, 6 and
# 13 recorders (13's result set), its object +0x20 one with slot 9 a recorder, the thing [0x7c2658] with its byte +0x32 any; the window +0x10's
# children (dialog7cb398.py's lists: decoys about the child 0x1581, whose own children are none or window.py's tree for Window_SetShown).
# usage: panel7ca228.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Update)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from dialog7cb398 import children, window  # noqa: E402
from window import RANGES as WINDOW_RANGES, shown_env  # noqa: E402

CALLS = {'Update': 0x499b40}
B = Bench([(0x499b40, 0x499b8e)] + WINDOW_RANGES)
PANEL, OBJECT, THING, ROOT, CHILD, DECOYS = 0x33100000, 0x33110000, 0x33120000, 0x33130000, 0x33140000, 0x33150000


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    window(r, regions, CHILD, ROOT, r.choice([0, 0, wins[0]]))
    window(r, regions, ROOT, 0, 0)
    put(regions[ROOT], 0x5c, 'I', children(r, regions, [(CHILD, 0x1581)], ROOT, DECOYS, [0, 0x1580, 0x1582, 0x157f]))
    panel = bytearray(r.bytes(0x40))
    put(panel, 0, 'I', PANEL + 0x40)
    put(panel, 0x10, 'I', ROOT)
    put(panel, 0x20, 'I', OBJECT)
    regions[PANEL] = panel
    regions[PANEL + 0x40] = vtable({})
    regions[OBJECT] = struct.pack('<I', OBJECT + 0x40) + r.bytes(0x1c)
    regions[OBJECT + 0x40] = vtable({9: 1})
    thing = bytearray(r.bytes(0x40))
    thing[0x32] = r.choice([0, 0x10, 0xef, 0xff, r.getrandbits(8)])
    regions[THING] = thing
    regions[0x7c2658] = struct.pack('<I', THING)
    set_return(log, 13, r.getrandbits(32))
    return Call(CALLS['Update'], [PANEL], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
