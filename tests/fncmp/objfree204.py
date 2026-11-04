# A block at +0x204 freed (src/game/core/accessors: 0x4056d0 Obj_FreeAt204) against the original: CRT_free a recorder (k 3).
# usage: objfree204.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x4056d0, 0x4056e0)], hooks={0x67ad90: _recorder(3, 1, cdecl=True)})


def case(r):
    return Call(0x4056d0, [RAM], {LOG: log_region(), RAM: r.bytes(0x210)}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
