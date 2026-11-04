# The online session's frame poll (src/game/online/session: 0x5b5bf0) against the original. Its calls recorders, each with a bench of its own.
# usage: onlinepoll.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

LOGIN, NEWS, CITY, UPLOAD, DOWNLOAD, MESSAGE, SHOW, SET = range(8, 16)
B = Bench([(0x5b5bf0, 0x5b5ca4)],
          hooks={0x5b6510: _recorder(LOGIN, 1), 0x5b6a30: _recorder(NEWS, 0), 0x5b75d0: _recorder(CITY, 0), 0x5b8490: _recorder(UPLOAD, 0), 0x5b90e0: _recorder(DOWNLOAD, 1),
                 0x4c25e0: _recorder(MESSAGE, 0, cdecl=True), 0x4c25b0: _recorder(SHOW, 1, cdecl=True), 0x4c2600: _recorder(SET, 1, cdecl=True)})
SESSION, TRANSFER = RAM, RAM + 0x800


def case(r):
    session = bytearray(r.bytes(0x400))
    for at, zero in ((0, 0.3), (0x64, 0.3), (0x39c, 0.1), (0x3a0, 0.1), (0x38c, 0.1), (0x390, 0.5), (0x394, 0.5)):
        put(session, at, 'I', 0 if r.random() < zero else r.choice([1, 0x80000000, r.getrandbits(32)]))
    if session[0x3a0:0x3a4] != bytes(4):
        put(session, 0x3a0, 'I', TRANSFER)
    transfer = bytearray(r.bytes(0x40))
    put(transfer, 0x18, 'I', r.choice([0, 1, 3, 100, 0xffffffff, 0x80000000, r.randrange(1, 1 << 30)]))
    put(transfer, 0x1c, 'I', r.choice([0, 1, 3, 50, 100, r.randrange(1 << 25), r.getrandbits(32)]))
    regions = {LOG: log_region(), SESSION: session, TRANSFER: transfer}
    return Call(0x5b5bf0, [SESSION], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
