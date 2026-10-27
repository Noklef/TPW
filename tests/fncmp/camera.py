# The camera (src/game/world/camera: 0x42a0f0-0x42a182 and the leaves 0x42a410-0x42cd40)
# against the original: the static initialisers, the follow tests, the part face, the view
# setter, flag 0x80, the control flags' handlers 0x42a500 / 0x42a520, the matrix closeness test 0x42b070 and the cell clamp. CentreCell (0x4868d0) sets the view's point from a cell id (0, 1,
# 0xffff or any); CentreThing (0x4867b0) from a thing's cell (a handle naming a slot of a thing, an empty slot or 0, high bits random);
# CentreGlobal (0x48ce80) the same for the handle of the thing [0x7c2658] points to. Handler3b0 (0x4b5c80, src/game/ui/panel7cc3b0):
# CentreGlobal's surroundings, the message 0x10006 mostly, the window's default handler +0x118 fncmp's logger (its answer now and then set).
# Reset (0x42a990): the camera back to its defaults (see reset()). SnapYaw (0x42aff0): the yaw to the nearest quarter turn.
# Input (0x42a760): the mouse messages (see the case).
# usage: camera.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, Bench, Call, RAM, fb, log_region, put, set_return  # noqa: E402

B = Bench([(0x42a0f0, 0x42a183), (0x42a410, 0x42a43a), (0x42a4e0, 0x42a4fb), (0x42a720, 0x42a75f), (0x42aab0, 0x42ab1d),
           (0x42afd0, 0x42afed), (0x42a500, 0x42a553), (0x42b070, 0x42b1b6), (0x42cd40, 0x42cd7f), (0x42cd80, 0x42cdb8), (0x4bd020, 0x4bd08f), (0x4867b0, 0x486858), (0x4868d0, 0x48691f), (0x48ce80, 0x48ce9f), (0x50b350, 0x50b35d),
           (0x4b5c80, 0x4b5cad), (0x65f66e, 0x65f697),
           (0x5da3c0, 0x5da3c1),
           (0x42a990, 0x42aaac), (0x46f150, 0x46f190), (0x56b790, 0x56b827), (0x578bc0, 0x578bd4), (0x578be0, 0x578d20), (0x583220, 0x583280),
           (0x402d80, 0x403220), (0x4033a0, 0x4033f0), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x42aff0, 0x42b063), (0x42a760, 0x42a949)])
INITS = {'One': (0x42a0f0, 0x7909e8, 4), 'Epsilon': (0x42a110, 0x790a94, 4), 'Zoom': (0x42a130, 0x7909ec, 4),
         'Yaw': (0x42a140, 0x790a38, 4), 'E0': (0x42a150, 0x7909e0, 4), 'Point': (0x42a160, 0x7908f0, 12)}


def case(r):
    op = r.deal(OPS)
    if op == 'Reset':
        return reset(r)
    if op == 'Input':   # 0x42a760: every message 0x1e-0x22 and 0x10001-0x1000d and a few beyond; param a point (any words), a button
        # (0-3 or any) or a wheel turn (either sign); the screen's width and height (signed words) usual, odd, 0 or negative; the camera's
        # globals (0x7909e0-0x790acf) any bytes but floats where they are read
        message = r.choice(list(range(0x1e, 0x23)) * 2 + list(range(0x10001, 0x1000e)) + [0x1d, 0x23, 0x10000, 0x1000e, 0, r.getrandbits(32)])
        param = r.choice([r.getrandbits(32), r.randrange(0x10000) << 16 | r.randrange(0x10000), 0, 1, 2, 3, 0xffffffff, 0x80000000,
                          r.choice([120, -120, 240, -240]) & 0xffffffff])
        if message == 0x1000d and r.random() < 0.3:   # (the wheel at 0: not negative)
            param = 0
        cam =bytearray(r.bytes(0xf0))
        for at in (0x9ec, 0xa38, 0xac8, 0xacc):
            put(cam, at - 0x9e0, 'I', r.choice([fb(r.uniform(-1000, 1000)), fb(r.uniform(0, 1000)), fb(110.0), r.f()]))
        screen = bytearray(r.bytes(8))
        put(screen, 0, 'h', r.choice([640, 800, 1024, 1600, 1, 0, -640, r.randrange(-0x8000, 0x8000)]))
        put(screen, 4, 'h', r.choice([480, 600, 768, 1200, 1, 0, -480, r.randrange(-0x8000, 0x8000)]))
        return Call(0x42a760, [message, param], {0x7909e0: cam, 0xfaa5c0: screen,
                                                 0x74c9c0: struct.pack('<I', r.choice([fb(0.001), fb(r.uniform(-0.01, 0.01)), r.f()]))}, ret='v')
    if op == 'SnapYaw':   # 0x42aff0: yaws about the quarter turns (and the eighths between), any, or special
        q = r.randrange(-12, 13) * math.pi / 4
        yaw = r.choice([fb(q), fb(q + r.uniform(-0.01, 0.01)), fb(q + r.uniform(-1, 1)), fb(r.uniform(-1e4, 1e4)), r.f(), r.f(), 0x80000000,
                        0x7f800000, 0xff800000, 0x7fc00000, 0x7f800001, 0xffc12345])
        return Call(0x42aff0, [], {0x790a38: struct.pack('<I', yaw)}, ret='v')
    if op in ('CentreCell', 'CentreThing', 'CentreGlobal', 'Handler3b0'):
        regions = {0x790a88: bytearray(r.bytes(12)), 0x7908f0: bytearray(r.bytes(12)), 0x7909ec: bytearray(r.bytes(4)),
                   0x790a38: bytearray(r.bytes(4)), 0x74c9bc: bytearray(r.bytes(4))}
        if op == 'CentreCell':
            regions[RAM] = struct.pack('<H', r.choice([0, 1, 2, 0x80, 0x81, 0xffff, r.getrandbits(16), r.getrandbits(16)])) + r.bytes(2)
            return Call(0x4868d0, [RAM], regions)
        table = bytearray(r.bytes(20 * 6))
        for h in range(6):
            thing = RAM + 0x100 + 0x10 * h
            struct.pack_into('<I', table, 20 * h, thing if r.random() < 0.8 else 0)
            regions[thing] = r.bytes(0x10)
        regions[0x7cfb90] = table
        handle = r.choice([0, 1, 2, 3, 4, 5, 5]) | (r.choice([0, 0, r.getrandbits(16)]) << 16)
        if op in ('CentreGlobal', 'Handler3b0'):   # (the thing's handle word at +0; the pointer's high word goes along)
            regions[RAM + 0x400] = struct.pack('<I', handle)
            regions[0x7c2658] = struct.pack('<I', RAM + 0x400)
            if op == 'Handler3b0':
                log = regions[LOG] = log_region()
                regions[RAM + 0x800] = put(bytearray(r.bytes(0x120)), 0x118, 'I', CDECL_LOG4)
                if r.random() < 0.5:
                    set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
                message = r.choice([0x10006] * 4 + [0x10005, 0x10007, 6, 0x14, r.getrandbits(32)])
                return Call(0x4b5c80, [RAM + 0x800, message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')
            return Call(0x48ce80, [], regions)
        return Call(0x4867b0, [handle], regions)
    if op == 'Close':  # two matrices, often nearly equal so the 1e-5 threshold decides
        scale = r.choice([1, 1, 2, None])  # near 1, a nudge of a few ulps is near 1e-5
        a = [r.f(scale) for _ in range(16)]
        b = [r.choice([v, v, v, v, v, r.f(scale)]) for v in a]
        ma, mb = bytearray(64), bytearray(64)
        for i in range(16):
            put(ma, 4 * i, 'I', a[i])
            put(mb, 4 * i, 'I', b[i])
            if r.random() < 0.2:  # a nudge of a few units in the last place
                put(mb, 4 * i, 'I', (a[i] + r.randrange(-40, 41)) & 0xffffffff)
        return Call(0x42b070, [RAM, RAM + 0x100], {RAM: ma, RAM + 0x100: mb}, ret='a')
    if op == 'Face':  # instance -> X, X + 4 -> model; the index straddles the +0x44 count
        first, base = r.choice([0, 1, 5, 0xffff, r.getrandbits(16)]), r.choice([0, 2, r.getrandbits(16)])
        part = r.choice([0, 1, 3, 7, r.getrandbits(32), (first - base) & 0xffffffff])
        model = put(bytearray(r.bytes(0x78)), 0x44, 'HH', first, base)
        regions = {RAM: put(bytearray(4), 0, 'I', RAM + 0x100), RAM + 0x100: put(bytearray(8), 4, 'I', RAM + 0x200),
                   RAM + 0x200: model, RAM + 0x300: put(bytearray(4), 0, 'I', part)}
        return Call(0x42a720, [RAM, RAM + 0x300], regions, ret='a')
    if op == 'View':  # each pointer null or a value; z is read whenever x is given
        regions = {0x790a88: bytearray(r.bytes(12)), 0x7908f0: bytearray(r.bytes(12)), 0x7909ec: bytearray(r.bytes(4)),
                   0x790a38: bytearray(r.bytes(4)), 0x74c9bc: bytearray(r.bytes(4)), RAM: bytearray(r.bytes(0x20))}
        x = r.choice([0, RAM])
        z = RAM + 4 if x else r.choice([RAM + 4, 0])
        return Call(0x42aab0, [x, z] + [r.choice([0, RAM + 8 + 4 * i]) for i in range(3)], regions)
    if op in ('Control100', 'Control200'):   # 0x42a500 / 0x42a520: the control flags 0x790aac (bit 0x200 set or not)
        return Call(0x42a500 if op == 'Control100' else 0x42a520, [], {0x790aac: bytearray(r.bytes(4))}, ret='a')
    if op == 'Flag80':
        return Call(0x42afd0, [r.choice([0, 1, r.getrandbits(32)])], {0x790ab0: bytearray(r.bytes(4))})
    if op == 'Pack':   # coordinates about the map, negative, huge or not numbers; the outputs apart or the same
        cs = [fb(r.choice([0.0, 5.0, 10.0, 12.75, -3.0, r.uniform(0, 2560), r.uniform(0, 2560), r.uniform(-1e3, 1e4), 1e12, float('nan')]))
              for _ in range(2)]
        return Call(0x4bd020, cs + [RAM + 0x10, r.choice([RAM + 0x14, RAM + 0x14, RAM + 0x10])], {RAM: r.bytes(0x20)})
    if op == 'CellOf':   # positions about the map's cells, negative, huge or not numbers; either output left out
        pos = [fb(r.choice([0.0, 5.0, 9.99, 10.0, -0.5, -10.0, r.uniform(0, 2560), r.uniform(-1e3, 1e3), 3e9, -3e10, 1e30, float('nan')]))
               for _ in range(3)]
        return Call(0x42cd80, [RAM, r.choice([RAM + 0x10, RAM + 0x10, 0]), r.choice([RAM + 0x14, RAM + 0x14, 0])],
                    {RAM: struct.pack('<3I', *pos) + r.bytes(0x10)})
    if op == 'Clamp':  # signed cells against the map size; a size of 0 or less clamps to it - 1
        def size():
            return r.choice([0, 1, 64, 128, 256, r.getrandbits(32)])

        def cell():
            return r.choice([0, -1, 1, 63, 64, 127, 128, 255, 256, r.getrandbits(32) - (1 << 31)]) & 0xffffffff
        regions = {RAM: put(bytearray(8), 0, 'II', cell(), cell()), 0x7a0314: put(bytearray(4), 0, 'I', size()),
                   0x7a0374: put(bytearray(4), 0, 'I', size())}
        return Call(0x42cd40, [RAM, RAM + 4], regions)
    if op in INITS:
        at, var, size = INITS[op]
        return Call(at, [], {var: bytearray(r.bytes(size))})
    followed = r.choice([0, 0x1234560, r.getrandbits(32)])
    thing = r.choice([followed, followed, 0, r.getrandbits(32)])
    flags = bytearray(r.bytes(4))
    if r.random() < 0.6:
        flags[0] |= 4
    regions = {0x790ab0: flags, 0x790988: bytearray(r.choice([bytes(4), r.bytes(4), r.bytes(4)])),
               0x790a44: bytearray(4)}
    put(regions[0x790a44], 0, 'I', followed)
    if op == 'Followed':
        return Call(0x42a4e0, [], regions, ret='a')
    return Call(0x42a410, [thing], regions, ret='a')


def reset(r):
    """0x42a990: the camera's globals (0x7908e0-0x790ab7) any bytes; its defaults (0x6fdd40-0x6fdd53: zoom, yaw, point) the game's or any
    floats (signalling NaNs among them); the field of view's factors [0x74c9b8] (pi / 2) and [0x6fdd80] (180 / pi) the game's or others; the
    perspective's globals and the view quad's aspect any; the game clock's real part paused (its time needs no timer)."""
    def num():
        return r.choice([r.f(), r.f(1000), fb(r.uniform(-500, 1500)), 0x7f800001, 0xff812345, 0x7fc00000])
    defaults = struct.pack('<5I', *[num() for _ in range(5)]) if r.random() < 0.6 else struct.pack('<5f', 110.0, 0.0, 475.0, 0.0, 175.0)
    half_pi = r.choice([0x3fc90fdb] * 4 + [fb(r.uniform(0.01, 3)), r.f(), 0x7f800001])
    factor = r.choice([0x42652ee0] * 4 + [fb(r.uniform(1, 100)), r.f(), 0xff800001])
    clock = put(bytearray(r.bytes(0xa0)), 0x68, 'I', r.choice([1, 1, r.getrandbits(32) | 1]))
    regions = {0x7908e0: r.bytes(0x1d8), 0x6fdd40: defaults, 0x6fdd80: struct.pack('<I', factor), 0x74c9b8: struct.pack('<I', half_pi),
               0x87afe8: struct.pack('<I', RAM), RAM: r.bytes(0x40), 0x87b110: r.bytes(0xd0), 0x8bcbc8: r.bytes(8), 0x768acc: r.bytes(4),
               0x87af58: r.bytes(4), 0x879d54: struct.pack('<I', RAM + 0x100), RAM + 0x100: r.bytes(0x30), 0x785970: clock}
    return Call(0x42a990, [], regions, ret='v')


OPS_ALL = list(INITS) + ['Follow'] * 6 + ['Close'] * 6 + ['Followed'] * 3 + ['Face', 'View', 'Clamp', 'CellOf', 'Pack'] * 4 + ['Flag80', 'Control100', 'Control200'] * 2 + ['CentreCell', 'CentreThing', 'CentreGlobal', 'Handler3b0'] * 3 + ['Reset'] * 3 + ['SnapYaw'] * 4 + ['Input'] * 4
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
