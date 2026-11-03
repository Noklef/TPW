# The handler of the dialog [0x7ca150] (src/game/ui/dialog7ca150: 0x495290) against the original: every call a recorder numbered as Dialog7ca150PageEnv from 10.
# usage: dialog7ca150page.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x495290, 0x495358)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x4a52a0: _recorder(11, 0, cdecl=True), 0x496620: _recorder(12, 2, cdecl=True), 0x493530: _recorder(13, 1, cdecl=True), 0x486b40: _recorder(14, 1, cdecl=True), 0x488ba0: _recorder(15, 4, cdecl=True)})


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[0x7ca150] = r.bytes(4)
    set_return(log, 15, r.getrandbits(32))
    message = r.deal([5, 0x14, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x13, 0x101, r.getrandbits(32)])
    a = r.choice([0xfffffffe, 0xffffffff, 0x12c4be, 0x12c4bf, 0x12c4c0, 0x12c4c1, 0x12c4c2, r.getrandbits(32)])
    return Call(0x495290, [RAM, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
