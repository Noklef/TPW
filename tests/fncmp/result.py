# The Win32 error as a COM result (src/game/core/result: Result_FromLastError 0x5fa650) against the original: GetLastError (a recorder in the
# import table) answering 0, a common code, one with high bits or any; out any.
# usage: result.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x5fa650, 0x5fa678)])
LAST = 40
IMPORTS = {'GetLastError': (LAST, 0x6fd138, 0, 0, 0)}


def case(r):
    log = log_region()
    set_return(log, LAST, r.choice([0, 2, 5, 0xb7, 0x8007000e, 0xffff, 0x10000, r.getrandbits(32)]))
    return Call(0x5fa650, [RAM], {RAM: r.bytes(8), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}, ret='a')


if __name__ == '__main__':
    B.main(case)
