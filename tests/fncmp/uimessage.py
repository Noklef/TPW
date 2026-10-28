# The interface object's message handler (src/game/ui/uimessage: 0x50ca50) against the original. Message: a message whose vtable's slot 0
# (a recorder, k 0) answers its type (7-0x1c mostly, or others), its +8 5 or 6 now and then (else any), its +0xc any; what 0x7b51f0 is told
# thiscall recorders (driver.cpp's UiMessageEnv): CheckOnline 1 of 1 (logged as its low word), RefreshResearch 2 of 1, PassThing 3 of 1,
# GuestRemoved 4 of 1, GuestAdded 5 of 1, Staff 6 of 2 (the handle logged as its low word: the original's high half is another
# register's); Stub_Ret4b (0x4f7c50) the game's.
# usage: uimessage.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Message)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x50ca50, 0x50cb5c), (0x4f7c50, 0x4f7c53)],
          hooks={0x481af0: _recorder(1, 1), 0x481520: _recorder(2, 1), 0x481530: _recorder(3, 1), 0x4818c0: _recorder(4, 1),
                 0x481920: _recorder(5, 1), 0x481960: _recorder(6, 2)})
CALLS = {'Message': 0x50ca50}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
MSG, VT = RAM + 0x100, RAM


def case(r):
    r.deal(OPS)
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 1, 0x10000)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 6, 0x10000)
    set_return(log, 0, r.choice(list(range(7, 0x1d)) + [7, 8, 0xa, 0xf, 0x12, 0x13, 0x13, 0x1b, 0x1c, 0, 6, 0x1d, r.getrandbits(32)]))
    msg = put(bytearray(r.bytes(0x10)), 0, 'I', VT)
    put(msg, 8, 'I', r.choice([5, 6, 5, 6, r.getrandbits(16), r.getrandbits(32)]))
    return Call(CALLS['Message'], [MSG], {LOG: log, VT: vtable({0: 0}), MSG: msg}, conv='s')


if __name__ == '__main__':
    B.main(case)
