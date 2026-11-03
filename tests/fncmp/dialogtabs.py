# DialogTabs_Reopen (src/game/ui/dialogtabs, 0x4a0940) against the original: its calls recorders as DialogTabsEnv's from 10, the group 0-4 and the three tabs mostly in range.
# usage: dialogtabs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x4a0940, 0x4a09f2)], hooks={a: _recorder(10 + k, n, cdecl=True) for k, (a, n) in enumerate(
    [(0x48c8d0, 0), (0x4acc70, 2), (0x49bdd0, 1), (0x4a52a0, 0), (0x496620, 2), (0x495aa0, 2), (0x493530, 1), (0x49ac60, 0), (0x49fb30, 0), (0x4b2750, 0), (0x498d80, 0)])})


def case(r):
    log = log_region()
    set_return(log, 10, r.choice([0] * 12 + [1, r.getrandbits(32) | 1]))
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 0b10 << 16)   # (the second's word: its upper half the stack's)
    group = r.deal([1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 0, 4, r.getrandbits(32)])
    regions = {LOG: log, 0x7cb28c: struct.pack('<I', r.choice([0, 1, 2, 3, 1, 2])), 0x7cb290: struct.pack('<I', r.deal([6, 7, 8, 9, 10, 11, r.getrandbits(32)])), 0x7cb2a0: struct.pack('<I', r.deal([2, 3, 4, 5, 6, 7, r.getrandbits(32)]))}
    return Call(0x4a0940, [group], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
