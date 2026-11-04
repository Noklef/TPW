# A child of the window at [0xf82884]+4 shown by the user slots (src/game/ui/dialogf82884child: 0x4bbbd0) against the original. Every call a recorder (their own benches have them).
# usage: dialogf82884child.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

SLOTS_CALL, FIND, SHOWN = range(10, 13)
SLOTS, HOLDER = RAM + 0x1000, RAM + 0x2000
B = Bench([(0x4bbbd0, 0x4bbc17)], hooks={0x48bcd0: _recorder(SLOTS_CALL, 0, cdecl=True), 0x65d7d0: _recorder(FIND, 1), 0x65d9dd: _recorder(SHOWN, 1)})


def case(r):
    log = log_region()
    set_return(log, SLOTS_CALL, SLOTS)
    set_return(log, FIND, r.getrandbits(32))
    set_return(log, SHOWN, r.getrandbits(32))
    regions = {LOG: log, SLOTS: put(bytearray(r.bytes(0x100)), 0xc4, 'I', r.deal([0, 1, 2, r.getrandbits(32) | 1])), HOLDER: r.bytes(8), 0xf82884: struct.pack('<I', HOLDER)}
    return Call(0x4bbbd0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
