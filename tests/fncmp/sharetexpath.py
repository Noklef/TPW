# The share textures' folder path (src/game/files/gamepaths: 0x46e650) against the original: the CRT's sprintf real, the root (0x40ef40) a recorder (driver.cpp's kShareTexCalls).
# usage: sharetexpath.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

ROOT = 1
B = Bench([(0x46e650, 0x46e66e), (0x67a400, 0x690000), (0x5da3c0, 0x5da3c1)], hooks={0x40ef40: _recorder(ROOT, 0, cdecl=True)})
OUT, TEXT = RAM, RAM + 0x400
NAME = 0x7af720


def text(r):
    return r.choice([b'', b'a', b'c:\\tpw', b'data\\sharetex', b'%s%d', b'Share Tex', bytes(r.randrange(1, 256) for _ in range(r.randrange(40)))])


def case(r):
    log = log_region()
    root = text(r)
    set_return(log, ROOT, TEXT)
    regions = {LOG: log, HEAP: heap(0x400), TEXT: root + bytes(4), NAME: text(r) + bytes(4), OUT: r.bytes(0x200)}
    return Call(0x46e650, [OUT], regions, ret='a')


if __name__ == '__main__':
    if '--table' in sys.argv:
        print('    {0x40ef40, jpeg_recorder(%d, 0)},' % ROOT)
        sys.exit(0)
    B.main(case)
