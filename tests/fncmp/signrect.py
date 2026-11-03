# The sign over a window's rectangle (src/game/ui/signrect: Sign_PlaceOverRect 0x486410, Sign_Swing 0x4864f0) against the originals: Sprite_FlipY real, 0x4690a0 10, 0x4689f0 11 and RideObject_Swing 12 cdecl recorders.
# usage: signrect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x486410, 0x4864e9), (0x4864f0, 0x486503), (0x476a70, 0x476a81)], hooks={0x4690a0: _recorder(10, 1, cdecl=True), 0x4689f0: _recorder(11, 6, cdecl=True), 0x468e50: _recorder(12, 1, cdecl=True)})
RECT = RAM


def case(r):
    log = log_region()
    set_return(log, 11, r.getrandbits(32))
    def word():
        return r.choice([0, 0x400, 0x300, 0x7fff, 0x8000, 0xffff, r.randrange(0, 0x800), r.getrandbits(16)])
    regions = {LOG: log, RECT: r.bytes(4) + struct.pack('<4H', word(), word(), word(), word())}
    regions[0x7b9f4c] = struct.pack('<I', r.choice([0, 0, r.getrandbits(32)]))
    regions[0x74f898] = struct.pack('<f', r.choice([480.0, 600.0, 768.0, r.uniform(1, 2000)]))
    if r.deal(['place', 'place', 'place', 'swing']) == 'swing':
        return Call(0x4864f0, [], regions, ret='v')
    return Call(0x486410, [RECT, r.getrandbits(32), r.choice([0, 1, 100, 0x7fff, r.getrandbits(16), r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
