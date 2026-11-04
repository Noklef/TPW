# The text button object's window made (src/game/ui/textbuttonwindow: TextButton_CreateWindow 0x492e80) against the original. The heap (an allocation now and then failing) and rand real, the window's calls recorders as driver.cpp's kTextButtonWindowEnv; the object with a window or not, the constructor answering a window or 0, second 0 or not.
# usage: textbuttonwindow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, RAND_SEED, Bench, Call, _recorder, heap, log_region, put, set_return, strings  # noqa: E402

CTOR, HANDLER, FIND, OBJECT98, DWORD, SHOWN = range(10, 16)
B = Bench([(0x492e80, 0x492f60)], hooks={0x65bcad: _recorder(CTOR, 7), 0x65e526: _recorder(HANDLER, 1), 0x659bc9: _recorder(FIND, 1, cdecl=True), 0x65d070: _recorder(OBJECT98, 1), 0x65e6d5: _recorder(DWORD, 2), 0x65d9dd: _recorder(SHOWN, 1)})
OBJECT, WINDOW = RAM, RAM + 0x100


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x400, r.deal([0, 0, 0, 0, 0, 1])), RAND_SEED: r.bytes(4)}
    object_ = bytearray(r.bytes(0x20))
    put(object_, 0x10, 'I', r.deal([0, 0, 0, 0, 0, 0, 0, r.getrandbits(32)]))
    regions[OBJECT] = object_
    set_return(log, CTOR, r.deal([WINDOW, WINDOW, WINDOW, WINDOW, 0]))
    set_return(log, FIND, r.getrandbits(32))
    return Call(0x492e80, [OBJECT, r.getrandbits(32), r.deal([0, r.getrandbits(32) | 1])], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
