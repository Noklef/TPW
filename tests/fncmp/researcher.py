# A researcher's tick (Researcher_Update 0x502960, src/game/world/researcher) against the original: Mode19c_ToIndex (k 1, its answer in and about 3-5), the lab 0x519550 (k 2) and
# ResearchLab_Spend (k 3) and Person_TickSpeed (k 4) recorders; Thing_GetHandle real in the original (its handle only printed); the tick about a multiple of 20.
# usage: researcher.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x502960, 0x5029e1), (0x50b350, 0x50b370), (0x5da3c0, 0x5da3c1)],
          hooks={0x5064f0: _recorder(1, 0), 0x519550: _recorder(2, 0), 0x503430: _recorder(3, 1), 0x505490: _recorder(4, 0)})
PERSON, TICKS = RAM, RAM + 0x400
GAME = TICKS - 0x1da70c
WAGES = 0x785400


def case(r):
    log = log_region()
    set_return(log, 1, r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)]))
    set_return(log, 2, r.getrandbits(32))
    person = bytearray(r.bytes(0x200))
    put(person, 0x1e4, 'I', r.randrange(10))
    tick = r.choice([0, 20, 40, 1000, 19, 21, 10, 0x80000000, 0xfffffff0, 20 * r.randrange(1 << 20), r.getrandbits(32)])
    return Call(0x502960, [PERSON], {LOG: log, PERSON: person, TICKS: struct.pack('<I', tick), 0x80239c: struct.pack('<I', GAME), WAGES: r.bytes(0x100)}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
