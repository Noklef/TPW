# The game panel's level box's handler (src/game/ui/panel7cc218: Panel7cc218_LevelBoxHandler, 0x4ae430) against the original: every call a recorder numbered as Panel7cc218LevelBoxEnv from 10 (Window7059f0_NthItemData's out filled or not: the slot then stays the window).
# usage: panel7cc218levelbox.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x4ae430, 0x4ae560)],
          hooks={0x65d7d0: _recorder(10, 1), 0x664c71: _recorder(11, 2), 0x4af480: _recorder(12, 2), 0x658d9f: _recorder(13, 4, cdecl=True), 0x4ade40: _recorder(14, 0), 0x48cf70: _recorder(15, 0),
                 0x65e705: _recorder(16, 1), 0x48d860: _recorder(17, 1), 0x65f66e: _recorder(18, 3)})
BOX, LIST, OWNER = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[OWNER] = r.bytes(0x40)
    set_return(log, 16, OWNER)
    set_return(log, 10, r.choice([LIST] * 8 + [0]))
    strings(log, 11, 0, 2)
    if r.random() < 0.85:
        fill(log, 11, 1, struct.pack('<I', r.choice([0xffffe12e, 0xffffcd39, 0, 1, 5, 0xffffffff, r.getrandbits(31), r.getrandbits(32)])))
    set_return(log, 11, r.choice([0, 1]))
    set_return(log, 12, r.choice([0, 1, 1, r.getrandbits(32)]))
    set_return(log, 18, r.getrandbits(32))
    message = r.deal([0x14, 0x100, 0x100, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 5, 0x401, r.getrandbits(32)])
    a = r.choice([0xfffffffe, 0xfffffffe, 0xffffffff, 0x1f4d, r.getrandbits(32)])
    b = r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)])
    return Call(0x4ae430, [BOX, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
