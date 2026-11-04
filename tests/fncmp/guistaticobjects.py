# The GUI library's static objects (src/game/ui/guistaticobjects) against the originals: the class's constructor and destructor and atexit recorders as driver.cpp's kGuiStaticObjectsEnv, timeGetTime's slot a recorder; the clock's memory random.
# usage: guistaticobjects.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, import_table, log_region, set_return  # noqa: E402

MAKE, DESTROY, ATEXIT, TIME = 10, 11, 12, 13
B = Bench([(0x6584a0, 0x6584d0), (0x6584d0, 0x6584df), (0x65964a, 0x659674), (0x659750, 0x659770), (0x659770, 0x659790)],
          hooks={0x669fa8: _recorder(MAKE, 0), 0x669fee: _recorder(DESTROY, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})


def case(r):
    log = log_region()
    imports = import_table(log, {'timeGetTime': (TIME, 0x6fd3ec, 0, 0, 0)})
    set_return(log, TIME, r.getrandbits(32))
    regions = {LOG: log, IMPORT_TABLE: imports, RAM: r.bytes(0x20), 0xfaa580: r.bytes(0x20)}
    make = r.deal([0, 1, 2])
    if make == 0:
        return Call(r.choice([0x6584a0, 0x6584af, 0x6584be, 0x6584d0, 0x65965b, 0x659665]), [], regions, ret='v')
    if make == 1:
        return Call(0x65964a, [RAM], regions, conv='t', ret='a')
    return Call(0x659750, [RAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
