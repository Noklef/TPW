# A mouse mode's cancel (src/game/world/mousemode: 0x46c890 MouseMode_CancelStaff) against the original: its sprite script at +8 or none, the byte at +0x18,
# SpriteScript_Kill (k 9), World_ClearStaffSlots (k 6), the world's table (k 7) and Advisor_Cancel (k 10) recorders.
# usage: mousemodecancel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

KILL, CLEAR, TABLE, CANCEL = 9, 6, 7, 10
B = Bench([(0x46c890, 0x46c8db)], hooks={0x475550: _recorder(KILL, 1, cdecl=True), 0x5083b0: _recorder(CLEAR, 1), 0x5194d0: _recorder(TABLE, 0), 0x59aa70: _recorder(CANCEL, 1)})
SELF, WORLD, ADVISOR = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    set_return(log, TABLE, ADVISOR)
    me = bytearray(r.bytes(0x20))
    put(me, 8, 'I', r.choice([0, r.getrandbits(32)]))
    me[0x18] = r.choice([0, 0, 1, r.getrandbits(8)])
    regions = {LOG: log, SELF: me, WORLD: r.bytes(0x20), ADVISOR: r.bytes(0x20), 0x7cf83c: struct.pack('<I', WORLD)}
    return Call(0x46c890, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
