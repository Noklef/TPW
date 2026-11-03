# The ground shadows' list drawn (src/game/render/groundshadow: GroundShadows_DrawAll 0x57c2a0) against the original: GroundShadow_Draw (k 2) a recorder answering in turn, now and then 0; a count of none to 6 or negative.
# usage: groundshadows.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put  # noqa: E402

B = Bench([(0x57c2a0, 0x57c2e8)], hooks={0x57be90: _recorder(2, 0)})
LIST, ANSWERS = RAM, RAM + 0x100


def case(r):
    log = log_region()
    answers(log, 2, ANSWERS)
    n = r.choice([0, 1, 2, 3, 4, 6, -1 & 0xffffffff, -5 & 0xffffffff])
    regions = {LOG: log, LIST: put(bytearray(r.bytes(0x40)), 0, 'I', n),
               ANSWERS: b''.join(struct.pack('<I', r.choice([1, 1, 1, 1, 1, 0, r.getrandbits(32) | 1])) for _ in range(48))}
    return Call(0x57c2a0, [LIST], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
