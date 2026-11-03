# The advisor's table at 0x76e300 (src/game/world/advisortable: AdvisorTable_Fill 0x5a0a50) against the original: the whole table's bytes, from before its start to past its end,
# random going in, so what it does not write (+0x34 of each record, record 0's first four fields) is compared too.
# usage: advisortable.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x5a0a50, 0x5a86fc)])
START, END = 0x76e300 - 0x40, 0x76e300 + 351 * 0x38 + 0x40


def case(r):
    return Call(0x5a0a50, [], {START: r.bytes(END - START)})


if __name__ == '__main__':
    B.main(case)
