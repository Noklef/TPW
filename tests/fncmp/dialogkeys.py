# The dialogs' key handler (src/game/ui/dialogbase: 0x488ba0) against the original: every call a recorder numbered as DialogKeyEnv from 10.
# usage: dialogkeys.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x488ba0, 0x488c5d)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x5194d0: _recorder(11, 0), 0x59ab50: _recorder(12, 0), 0x40c900: _recorder(13, 2), 0x40c990: _recorder(14, 2), 0x485b70: _recorder(15, 1, cdecl=True), 0x65f66e: _recorder(16, 3)})


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[0x7cf83c] = r.bytes(4)
    regions[0x78721c] = r.bytes(4)
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 16, r.getrandbits(32))
    message = r.deal([0x1000b, 0x1000b, 0x1000b, 0x1000b, 0x1000a, 0x1000a, 0x14, 0x13, 0x15, 0x10009, 0x1000c, r.getrandbits(32)])
    a = r.choice([0x1b, 0x1b, 0x70, 0x70, 0x1c, r.getrandbits(32)])
    b = r.choice([0, 0x10001, 0xff00ffff, 0xff0000, 0x10000]) | r.getrandbits(32) & r.choice([0, 0xff00ffff])
    return Call(0x488ba0, [RAM, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
