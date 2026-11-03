# The handler of the child 0x25 of the window [0x7cb2b8] (src/game/ui/dialog7cb2b8tabs: Dialog7cb2b8_TabsHandler, 0x4a0840) against the original. Recorders (driver.cpp's kDialog7cb2b8TabsEnv): DialogTabs_Reopen 10, Dialog7ca1e0_Post 11, Dialog7ca1e0_Cancel 12, 0x5f0b40 13, ResearchPanel_Open 14, Window_CallDefault 15.
# usage: dialog7cb2b8tabs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x4a0840, 0x4a091a)], hooks={0x4a0940: _recorder(10, 1, cdecl=True), 0x498bb0: _recorder(11, 0, cdecl=True), 0x498bd0: _recorder(12, 0, cdecl=True), 0x5f0b40: _recorder(13, 0), 0x4aa480: _recorder(14, 0, cdecl=True), 0x65f66e: _recorder(15, 3)})


def case(r):
    log = log_region()
    set_return(log, 15, r.getrandbits(32))
    message = r.choice([0x100] * 8 + [0xff, 0x101, r.getrandbits(32)])
    a = r.choice(list(range(0x25, 0x2d)) + [r.getrandbits(32)])
    return Call(0x4a0840, [r.getrandbits(32), message, a, r.choice([0, 1, 1, 2, r.getrandbits(32)])], {LOG: log}, ret='a')


if __name__ == '__main__':
    B.main(case)
