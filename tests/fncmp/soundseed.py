# The random number's seed (SoundRandom_Seed 0x6bb940, src/game/core/soundseed) against the original: Time_GetMs a cdecl recorder (k 1) answering any value; the seed's dword at 0xfb1f20 compared.
# usage: soundseed.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x6bb940, 0x6bb94b)], hooks={0x5f5fa0: _recorder(1, 0, cdecl=True)})


def case(r):
    log = log_region()
    set_return(log, 1, r.getrandbits(32))
    return Call(0x6bb940, [], {LOG: log, 0xfb1f20: r.bytes(4)}, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
