# The chat rooms' text windows' handler (src/game/online/roomtexthandler: 0x4c56e0) against the original. Every call a recorder (their own benches have them; 0x5b7910 is agent B's).
# usage: roomtexthandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

ONLINE, DWORD, ENTER, UPDATE, DEFAULT = range(10, 15)
SESSION, PARTS = RAM + 0x1000, RAM + 0x2000
B = Bench([(0x4c56e0, 0x4c5772)], hooks={0x48b390: _recorder(ONLINE, 0, cdecl=True), 0x65e705: _recorder(DWORD, 1), 0x5b7910: _recorder(ENTER, 1), 0x4c4bb0: _recorder(UPDATE, 0, cdecl=True), 0x65f66e: _recorder(DEFAULT, 3)})


def case(r):
    log = log_region()
    set_return(log, ONLINE, SESSION)
    set_return(log, DWORD, r.deal([0, 1, 2, r.getrandbits(32)]))
    set_return(log, DEFAULT, r.getrandbits(32))
    session = bytearray(r.bytes(0x100))
    parts = bytearray(r.bytes(0x100))
    for k, offset in enumerate((0xe0, 0xf8, 0xfc, 0xf4)):
        put(session, offset, 'I', PARTS + 0x40 * k)
        put(parts, 0x40 * k + 0x28, 'I', r.deal([0, 1, 1, 1, 1, 1, r.getrandbits(32) | 1]))
    regions = {LOG: log, SESSION: session, PARTS: parts}
    return Call(0x4c56e0, [r.getrandbits(32), r.deal([0x1e, 0x10006, 0x10006, 0x10006, 0x1f, 0x10007, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
