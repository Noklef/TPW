# A mouse mode entered (src/game/world/mousemode: 0x497bc0) against the original, on focus.py's Enter6fead0 set-up (modes_case: the
# holder [0x7b05e8] set or not, the current mode [0x7b05d8] a recorder object or none, the mode setting 1, the heap failing now and
# then). Ensure6feaa0: the current mode's slot 9 answering 8 half the time (else 9, 1, 0 or any).
# usage: modeenter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ensure6feaa0)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import focus  # noqa: E402
from fncmp import LOG, Bench, Call, set_return  # noqa: E402

CALLS = {'Ensure6feaa0': 0x497bc0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(dialog7cc5f4.merged(focus.RANGES + focus.FACTORY + focus.MODES + [(0x497bc0, 0x497d1c), (0x46cfc0, 0x46cfd0), (0x40f3f0, 0x40f3f6)]))


def case(r):
    op = r.deal(OPS)
    c = focus.case(r, ['Enter6fead0'])
    set_return(c.regions[LOG], 9, r.choice([8, 8, 8, 8, 9, 1, 0, r.getrandbits(32)]))
    return Call(CALLS[op], [], c.regions, ret='v')


if __name__ == '__main__':
    B.main(case)
