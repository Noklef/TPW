# The box's outer handler (src/game/ui/dialogbase: 0x47f7a0 Dialog_BoxOuterHandler) against the original: message 0x14 clears [0x7b4ba0], Dialog_BoxHandler a
# recorder (k 3) whose answer it returns.
# usage: dialogboxclose.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

HANDLER = 3
B = Bench([(0x47f7a0, 0x47f7cc)], hooks={0x47eed0: _recorder(HANDLER, 4, cdecl=True)})


def case(r):
    log = log_region()
    set_return(log, HANDLER, r.getrandbits(32))
    message = r.choice([0x14, 0x14, 0x13, 0x15, 0x100, 0x114, r.getrandbits(32)])
    args = [r.getrandbits(32), message, r.getrandbits(32), r.getrandbits(32)]
    return Call(0x47f7a0, args, {LOG: log, 0x7b4ba0: r.bytes(4)}, ret='a')


if __name__ == '__main__':
    B.main(case)
