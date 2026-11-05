# The action replay started (src/game/coaster/arreplay: 0x404140 ArReplay_Begin) against the original: already running or not, the mode, the streams a
# vtable's recorder (k 4), ArReplay_Update (k 7) and the game's object (k 2) recorders.
# usage: arreplaybegin.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

OBJECT, CLOSE, UPDATE = 2, 4, 7
B = Bench([(0x404140, 0x4041c5), (0x5da3c0, 0x5da3c1)], hooks={0x5195d0: _recorder(OBJECT, 0), 0x4041d0: _recorder(UPDATE, 0)})
SELF, VT, RESULT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    me = bytearray(r.bytes(0x40))
    for at in (0xc, 0x14, 0x1c):
        put(me, at, 'I', VT)
    put(me, 4, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(me, 0x24, 'I', r.choice([0] * 9 + [1, r.getrandbits(32)]))
    set_return(log, OBJECT, RESULT)
    regions = {LOG: log, SELF: me, VT: vtable({CLOSE: 2}), RESULT: r.bytes(0x118), 0x7cf83c: r.bytes(4)}
    return Call(0x404140, [SELF, r.choice([0, 1, 1, 2, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
