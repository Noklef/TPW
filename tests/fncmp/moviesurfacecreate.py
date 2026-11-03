# The movie's surface creators and overlay update (src/game/gfx/moviesurface: 0x670ad0, 0x671ea0, 0x671cb0, 0x66fa70) against the originals. The IDirectDraw and the surfaces are objects over vtables of COM recorders (each object its own, so a fill
# gives its next object), the stack structures logged by their contents (UpdateOverlay has three: one of them a case), each step failing now and then; the error of UpdateOverlay one of those it tells apart, 0 mostly.
# usage: moviesurfacecreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, recorder, set_return, strings  # noqa: E402

CALLS = {'Overlay': 0x670ad0, 'Offscreen': 0x671ea0, 'Flip': 0x671cb0, 'Update': 0x66fa70}
B = Bench([(0x670ad0, 0x670c15), (0x671ea0, 0x671f9f), (0x671cb0, 0x671e9e), (0x66fa70, 0x66fb57)])
DRAW, SURFACE, BACK, FRONT, LATER, OUT, VTABLES, PLAYER, SETTINGS = RAM, RAM + 0x10, RAM + 0x20, RAM + 0x30, RAM + 0x40, RAM + 0x50, RAM + 0x100, RAM + 0x1000, RAM + 0x1100
CREATE, SQI, ATTACHED, SRELEASE, BQI, BRELEASE, FKEY, FBLT, LKEY, LBLT, OVERLAY, RESTORE = range(1, 13)
ERRORS = [0x80070057, 0x88760168, 0x80004001, 0x80004005, 0x88760082, 0x887601c2]


def vtable(**slots):
    table = bytearray(0x88)
    for offset, stub in slots.values():
        struct.pack_into('<I', table, offset, stub)
    return table


def step(r, log, k):
    set_return(log, k, r.choice([0] * 12 + [1, 0x80004005]))


def objects(r, regions):
    for at, vt in ((DRAW, 0), (SURFACE, 1), (BACK, 2), (FRONT, 3), (LATER, 4)):
        regions[at] = struct.pack('<I', VTABLES + 0x100 * vt) + r.bytes(0xc)
    regions[VTABLES] = vtable(a=(0x18, recorder(CREATE, 4, True)))
    regions[VTABLES + 0x100] = vtable(a=(0, recorder(SQI, 3, True)), b=(8, recorder(SRELEASE, 1, True)), c=(0x30, recorder(ATTACHED, 3, True)))
    regions[VTABLES + 0x200] = vtable(a=(0, recorder(BQI, 3, True)), b=(8, recorder(BRELEASE, 1, True)))
    regions[VTABLES + 0x300] = vtable(a=(0x14, recorder(FBLT, 6, True)), b=(0x74, recorder(FKEY, 3, True)))
    regions[VTABLES + 0x400] = vtable(a=(0x14, recorder(LBLT, 6, True)), b=(0x74, recorder(LKEY, 3, True)))


def create_case(r, op):
    log = log_region()
    regions = {LOG: log}
    objects(r, regions)
    regions[OUT] = r.bytes(8)
    blob(log, CREATE, 1, (1 << 27) - 1)
    strings(log, CREATE, 0, 1 << 2)
    fill(log, CREATE, 2, struct.pack('<I', SURFACE))
    fill(log, ATTACHED, 2, struct.pack('<I', BACK), which=1)
    blob(log, ATTACHED, 1, 1)
    strings(log, ATTACHED, 0, 1 << 2)
    fill(log, SQI, 2, struct.pack('<I', FRONT if op == 'Flip' or r.random() < 0.5 else LATER), which=2)
    strings(log, SQI, 0, 1 << 2)
    fill(log, BQI, 2, struct.pack('<I', LATER), which=3)
    strings(log, BQI, 0, 1 << 2)
    for key, blt in ((FKEY, FBLT), (LKEY, LBLT)):
        blob(log, key, 2, 3)
        blob(log, blt, 5, (1 << 25) - 1)
    for k in (CREATE, ATTACHED, SQI, BQI, FKEY, LKEY):
        step(r, log, k)
    args = [DRAW, OUT] + ([OUT + 4] if op == 'Flip' else []) + [r.choice([0, 1, 320, 640, r.getrandbits(32)]), r.choice([0, 1, 240, 480, r.getrandbits(32)]), r.choice([0x32595559, 0x59565955, r.getrandbits(32)])]
    return Call(CALLS[op], args, regions, conv='f', ret='a')


def update_case(r):
    log = log_region()
    regions = {LOG: log}
    regions[SURFACE] = struct.pack('<I', VTABLES) + r.bytes(0xc)
    regions[VTABLES] = vtable(a=(0x84, recorder(OVERLAY, 6, True)), b=(0x6c, recorder(RESTORE, 1, True)))
    regions[SETTINGS] = struct.pack('<26I', *[r.getrandbits(32) for _ in range(26)])
    settings = bytearray(regions[SETTINGS])
    struct.pack_into('<I', settings, 0x1c, SURFACE)
    regions[SETTINGS] = settings
    player = bytearray(r.bytes(0x60))
    struct.pack_into('<I', player, 0, SETTINGS)
    regions[PLAYER] = player
    arg, mask = r.choice([(1, 0xf), (3, 0xf), (5, (1 << 14) - 1)])
    blob(log, OVERLAY, arg, mask)
    strings(log, OVERLAY, 0, sum(1 << a for a in (1, 3, 5) if a != arg))
    set_return(log, OVERLAY, r.choice([0] * 5 + ERRORS * 2 + [0x887601c2, 1, 0x80000000, r.getrandbits(32)]))
    return Call(CALLS['Update'], [PLAYER, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='f', ret='a')


def case(r):
    op = r.deal(list(CALLS))
    return update_case(r) if op == 'Update' else create_case(r, op)


if __name__ == '__main__':
    B.main(case)
