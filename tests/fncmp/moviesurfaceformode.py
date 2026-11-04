# The movie's surfaces made for its mode (src/game/gfx/moviesurface: MovieSurface_CreateForMode 0x6719e0) against the original. The creators real (0x670ad0, 0x671ea0, 0x671cb0), the IDirectDraw and the surfaces objects over vtables of
# COM recorders as in moviesurfacecreate.py, _nh_malloc a recorder (driver.cpp's CRT, k 28 as crt_thread.py). Each path by a deck: the FourCC list failing, the creators failing at their steps, the primary's fill logged by its rectangle or its effects.
# usage: moviesurfaceformode.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, recorder, set_return, strings  # noqa: E402
from moviesurfacecreate import ATTACHED, BACK, BQI, BRELEASE, CREATE, DRAW, FBLT, FKEY, FRONT, LATER, LBLT, LKEY, SQI, SRELEASE, SURFACE, VTABLES, vtable  # noqa: E402

CODES, PBLT = 20, 21
NHMALLOC = 28
PLAYER, SETTINGS, PRIMARY = RAM + 0x1000, RAM + 0x2000, RAM + 0x60
B = Bench([(0x6719e0, 0x671cb0), (0x670ad0, 0x670c15), (0x671ea0, 0x671f9f), (0x671cb0, 0x671e9e), (0x67dcd0, 0x67dcf0)], hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True)})


def case(r):
    log = log_region()
    regions = {LOG: log, 0xfb1548: r.bytes(4)}
    for at, vt in ((DRAW, 0), (SURFACE, 1), (BACK, 2), (FRONT, 3), (LATER, 4), (PRIMARY, 5)):
        regions[at] = struct.pack('<I', VTABLES + 0x100 * vt) + r.bytes(0xc)
    regions[VTABLES] = vtable(a=(0x18, recorder(CREATE, 4, True)), b=(0x34, recorder(CODES, 3, True)))
    regions[VTABLES + 0x100] = vtable(a=(0, recorder(SQI, 3, True)), b=(8, recorder(SRELEASE, 1, True)), c=(0x30, recorder(ATTACHED, 3, True)))
    regions[VTABLES + 0x200] = vtable(a=(0, recorder(BQI, 3, True)), b=(8, recorder(BRELEASE, 1, True)))
    regions[VTABLES + 0x300] = vtable(a=(0x14, recorder(FBLT, 6, True)), b=(0x74, recorder(FKEY, 3, True)))
    regions[VTABLES + 0x400] = vtable(a=(0x14, recorder(LBLT, 6, True)), b=(0x74, recorder(LKEY, 3, True)))
    regions[VTABLES + 0x500] = vtable(a=(0x14, recorder(PBLT, 6, True)))

    settings = bytearray(r.bytes(0x70))
    put(settings, 4, 'I', r.getrandbits(32) & ~0x5c0 | r.deal([0x400, 0x400, 0x400, 0x400, 0, 0x40, 0x80, 0x100, 0x140, 0x180]))
    put(settings, 0x10, 'I', DRAW)
    put(settings, 0x14, 'I', PRIMARY)
    put(settings, 0x1c, 'I', r.deal([0, 0, 0, 0, 0, 0, 0, SURFACE]))
    put(settings, 0x60, 'I', r.deal([240, 480, 1, r.getrandbits(32)]))
    put(settings, 0x64, 'I', r.deal([320, 640, 1, r.getrandbits(32)]))
    regions[SETTINGS] = settings
    player = bytearray(r.bytes(0x650))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x4c, 'I', r.deal([2, 2, 2, 4, 4, 1, 0, 3]))
    regions[PLAYER] = player

    blob(log, CODES, 1, 1)
    strings(log, CODES, 0, 1 << 2)
    blob(log, CREATE, 1, (1 << 27) - 1)
    strings(log, CREATE, 0, 1 << 2)
    fill(log, CREATE, 2, struct.pack('<2I', SURFACE, BACK), records=2)
    fill(log, ATTACHED, 2, struct.pack('<I', BACK), which=1)
    blob(log, ATTACHED, 1, 1)
    strings(log, ATTACHED, 0, 1 << 2)
    fill(log, SQI, 2, struct.pack('<I', r.deal([FRONT, LATER])), which=2)
    strings(log, SQI, 0, 1 << 2)
    fill(log, BQI, 2, struct.pack('<I', LATER), which=3)
    strings(log, BQI, 0, 1 << 2)
    for key, blt in ((FKEY, FBLT), (LKEY, LBLT)):
        blob(log, key, 2, 3)
        blob(log, blt, 5, (1 << 25) - 1)
    arg, mask = r.deal([(1, 0xf), (5, (1 << 25) - 1)])
    blob(log, PBLT, arg, mask)
    strings(log, PBLT, 0, 1 << (6 - arg))
    set_return(log, PBLT, r.choice([0, 0, 1]))
    set_return(log, NHMALLOC, r.getrandbits(32))
    fail = r.deal([None, None, None, CODES, CREATE, ATTACHED, ATTACHED, ATTACHED, SQI, FKEY])
    if fail is not None:
        set_return(log, fail, r.choice([1, 0x80004005]))
    return Call(0x6719e0, [PLAYER], regions, conv='f', ret='a')


if __name__ == '__main__':
    B.main(case)
