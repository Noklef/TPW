# The screen's surface as a drawing target and the pixel formats' tables (src/game/gfx/screenview) against the originals: Screen_GetDirectDraw, Surface_Refresh and the object's slot 8 are recorders.
# usage: screenview.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return, vtable  # noqa: E402

ADDRESS = {'Pal8': (0x6316c0, 43), 'I555': (0x631700, 53), 'I888': (0x631750, 53), 'I8888': (0x6317a0, 61), 'Surface': (0x6317e0, 6), 'Field': (0x6317f0, 6), 'Rect': (0x631800, 83), 'Flag': (0x631860, 11)}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
DIRECTDRAW, REFRESH = 20, 21
B = Bench([(a, a + n) for a, n in ADDRESS.values()], hooks={0x5fa040: _recorder(DIRECTDRAW, 1, cdecl=True), 0x61b190: _recorder(REFRESH, 0)})
OBJ, OUT, SIZE, VT = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x200


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, OBJ: struct.pack('<I', VT), OUT: r.bytes(0x10), SIZE: r.bytes(0x40), VT: vtable({})}
    address = ADDRESS[op][0]
    regions[0xfa7780] = r.bytes(0x170)   # (the screen: +0x1c the flags, +0x164 a field)
    if op in ('Pal8', 'I555', 'I888', 'I8888'):
        regions[0xfaa268] = r.bytes(0x60)
        return Call(address, [], regions, conv='c', ret='v')
    if op == 'Rect':
        set_return(log, DIRECTDRAW, r.choice([0, 1, 0x80000000]))
        set_return(log, 2, 0x12340)
        set_return(log, REFRESH, SIZE)
        return Call(address, [OBJ, OUT], regions, conv='t', ret='a')
    return Call(address, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
