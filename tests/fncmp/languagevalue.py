# The text metrics' constructor with 0 (src/game/core/language: 0x419680 Language_CtorValue0) against the original: Obj_CtorValue a recorder (k 20) whose answer it returns.
# usage: languagevalue.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

CTOR = 20
B = Bench([(0x419680, 0x419688)], hooks={0x47f940: _recorder(CTOR, 1)})


def case(r):
    log = log_region()
    set_return(log, CTOR, r.getrandbits(32))
    return Call(0x419680, [RAM], {LOG: log, RAM: r.bytes(0x10)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
