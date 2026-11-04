# The help background started (src/game/frontend/helpbgstart: 0x492470) against the original. Every call a recorder (their own benches have them).
# usage: helpbgstart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region  # noqa: E402

BUILD, STOP, SHOWN, POINTER = range(10, 14)
B = Bench([(0x492470, 0x4924a9)], hooks={0x492180: _recorder(BUILD, 0), 0x66a016: _recorder(STOP, 0), 0x65d9dd: _recorder(SHOWN, 1), 0x6595b3: _recorder(POINTER, 1, cdecl=True)})


def case(r):
    log = log_region()
    regions = {LOG: log, 0x7ca0cc: struct.pack('<H', r.deal([0, 1, 0x100, r.getrandbits(16)])), 0x7ca0d8: r.bytes(4)}
    return Call(0x492470, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
