# The drawing front end (src/game/gfx/display: 0x5f9780-0x5f9e64, cdecl) against the originals, on the game's draw context (0xfa76b8, random
# but for its pointers): the shapes tested against the clip and handed to the current driver, a table of thiscall recorders (each depth's
# driver its own `this`; the colour's index logged as its byte, the original leaves the rest of its dword as the stack lies); the window and
# clip set from rectangles in either order, partly or wholly off the surface, or none; the surface set from a SurfaceInfo of any bit count or a
# Surface whose GetSurfaceDesc (COM slot 22) fills a random description (surfacedesc.py's) or fails.
# usage: display.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import surfacedesc  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, blob, fill, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x5f9780, 0x5f9eb0), (0x61af60, 0x61b040), (0x61b090, 0x61b1f0), (0x62f890, 0x62f8e0), (0x62f940, 0x62f990)])
STATE = 0xfa76b8
TABLE, INFO, RECT, SURFACE, COM, COM_TABLE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
SLOT_ARGS = {0: 4, 3: 3, 7: 6, 8: 5, 9: 5, 10: 8}   # the slots the front end calls: their stack arguments (the colour's index the last)
GET_DESC = 22
CALLS = {'IsVisible': 0x5f9780, 'Pixel': 0x5f97c0, 'Line': 0x5f97f0, 'HLine': 0x5f9860, 'VLine': 0x5f98d0, 'Triangle': 0x5f9940,
         'FillRect': 0x5f9a30, 'BoxFill': 0x5f9a80, 'SetWindow': 0x5f9ac0, 'SetClip': 0x5f9b60, 'GetClip': 0x5f9ca0, 'SetSurfaceInfo': 0x5f9cb0,
         'SetSurface': 0x5f9d90}


EDGES = []   # the case's clip edges: coordinates land on them or one off now and then


def coord(r):
    if EDGES and r.random() < 0.3:
        return r.choice(EDGES) + r.choice([-1, 0, 1])
    return r.choice([r.randrange(-40, 700), r.randrange(-40, 700), r.randrange(-5, 5), r.getrandbits(32) - (1 << 31)])


def rect(r):
    xs, ys = [coord(r), coord(r)], [coord(r), coord(r)]
    if r.random() < 0.6:
        xs.sort()
        ys.sort()
    return struct.pack('<4i', xs[0], ys[0], xs[1], ys[1])


def info(r):
    """A SurfaceInfo: masks random, a bit count of the four depths or any, a size mostly small, the bits anywhere."""
    i = bytearray(r.bytes(0x28))
    put(i, 0, '4I', *[surfacedesc.mask(r) for _ in range(4)])
    put(i, 0x10, 'I', r.choice([8, 16, 24, 32, 15, 0, r.getrandbits(32)]))
    put(i, 0x1c, 'II', r.choice([640, 800, 64, 0, r.getrandbits(32)]), r.choice([480, 600, 48, 0, r.getrandbits(32)]))
    put(i, 0x24, 'i', r.choice([640, 1600, 2560, -640, 0, r.getrandbits(32) - (1 << 31)]))
    return i


def case(r):
    op = r.deal(OPS)
    log = log_region()
    for slot, n in SLOT_ARGS.items():
        struct.pack_into('<I', log, LOG_BYTES + 4 * slot, 1 << (n - 1))
    state = bytearray(r.bytes(0x74))
    xs, ys = sorted([r.randrange(-20, 100), r.randrange(-20, 100)]), sorted([r.randrange(-20, 100), r.randrange(-20, 100)])
    put(state, 8, '4i', xs[0], ys[0], xs[1], ys[1])   # (the clip in order)
    EDGES[:] = xs + ys
    put(state, 0x18, 'I', STATE + r.choice([0x38, 0x40, 0x44, 0x48]))
    put(state, 0x30, 'I', INFO)
    for at in (0x38, 0x40, 0x44, 0x48):
        put(state, at, 'I', TABLE)
    regions = {STATE: state, TABLE: vtable({k: n for k, n in SLOT_ARGS.items()}, 16), INFO: info(r), LOG: log}
    at = CALLS[op]
    colour = [r.getrandbits(32), r.getrandbits(32)]
    if op in ('IsVisible', 'FillRect', 'BoxFill', 'SetWindow', 'SetClip'):
        regions[RECT] = rect(r)
        arg = RECT if op in ('IsVisible', 'BoxFill') or r.random() < 0.8 else 0
        return Call(at, [arg] + ([] if op in ('IsVisible', 'SetWindow', 'SetClip') else colour), regions, ret='a' if op == 'IsVisible' else 'v')
    if op in ('Pixel', 'Line', 'HLine', 'VLine', 'Triangle'):
        n = {'Pixel': 2, 'Line': 4, 'HLine': 3, 'VLine': 3, 'Triangle': 6}[op]
        return Call(at, [coord(r) & 0xffffffff for _ in range(n)] + colour, regions)
    if op == 'GetClip':
        return Call(at, [], regions, ret='a')
    if op == 'SetSurfaceInfo':
        return Call(at, [r.choice([INFO] * 6 + [0])], regions)
    # SetSurface: the Surface (its info the old one, random), the IDirectDrawSurface's GetSurfaceDesc
    surface = bytearray(r.bytes(0x28)) + info(r)
    put(surface, 0x24, 'I', COM)
    blob(log, GET_DESC, 1, 1)
    fill(log, GET_DESC, 1, surfacedesc.desc(r))
    if r.random() < 0.15:
        set_return(log, GET_DESC, r.choice([0x80000000, 0x887601c2, r.getrandbits(32)]))
    regions.update({SURFACE: surface, COM: struct.pack('<I', COM_TABLE) + r.bytes(0xc), COM_TABLE: vtable({}, com={GET_DESC: 2})})
    return Call(at, [r.choice([SURFACE] * 6 + [0])], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
