# The deleting destructor of the vtable 0x6fefb8's class (src/game/core/vtables: 0x47e750 Vtbl6fefb8_Delete) against the original: the flags' bit 0, Mem_Free a recorder (k 2).
# usage: vtbl6fefb8del.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x47e750, 0x47e76e), (0x4bc420, 0x4bc430)], hooks={0x67acb0: _recorder(2, 1, cdecl=True)})


def case(r):
    return Call(0x47e750, [RAM, r.choice([0, 1, 2, 3, r.getrandbits(32)])], {LOG: log_region(), RAM: r.bytes(0x20)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
