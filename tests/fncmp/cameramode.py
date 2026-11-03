# The camera's mode change (src/game/world/camera: Camera_SetMode 0x42ab20) against the original. Its calls are recorders (the driver's
# CameraEnv): Highlight_Suspend 0x45d660 (cdecl 20 of 1), ChatButtons_SetChat 0x481470 (thiscall 21 of 1), Terrain_ProceduralLeave
# 0x457a40 / Enter 0x457a00 (cdecl 22 / 23 of none), Holder_AddToGrid 0x45bcb0 (cdecl 24 of 3) and Holder_Wake 0x45bd00 (cdecl 25 of 1);
# Terrain_ProceduralOn runs as it is. The fixture: gCameraFlags 0x790ab0 (its mode bits and 0x80 / 0x200 / 0x400 / 0x800 / 0x1000, or
# random), 0x790adc / 0x790ae0, 0x7a1a8c, the instance at 0x790a34 (or none; +4's 0x4000), the ride table 0x7a4610 with its count and holes
# (empty slots now and then), three models (their parts' flags 0x4000000 to 0x20000000 and the faces either side of +0x44), the mode
# (2, 4, 0x40 or another), toggle, ride (0 or a slot), force, out (or none) and fresh.
# usage: cameramode.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

HOOKS = {0x45d660: _recorder(20, 1, cdecl=True), 0x481470: _recorder(21, 1), 0x457a40: _recorder(22, 0, cdecl=True),
         0x457a00: _recorder(23, 0, cdecl=True), 0x45bcb0: _recorder(24, 3, cdecl=True), 0x45bd00: _recorder(25, 1, cdecl=True)}
RANGES = [(0x42ab20, 0x42ae63), (0x457a70, 0x457a88)]
B = Bench(RANGES, hooks=HOOKS)
INST, CAM, OBJS, OUT =(RAM + k * 0x1000 for k in range(4))
MODELS = [RAM + (4 + 4 * m) * 0x1000 for m in range(3)]   # each: the model, its parts, faces +0x70, faces +0x74
BITS = [1, 2, 4, 0x10, 0x40, 0x80, 0x200, 0x400, 0x800, 0x1000]
PART_BITS = [0x4000000, 0x8000000, 0x10000000, 0x20000000]


def case(r):
    log = log_region()
    regions = {LOG: log}
    if r.random() < 0.25:
        flags = r.getrandbits(32)
    else:
        flags = r.getrandbits(32) & ~0x1ff7 if r.random() < 0.3 else 0
        for b in BITS:
            if r.random() < 0.3:
                flags |= b
    for m, base in enumerate(MODELS):
        model = bytearray(r.bytes(0x80))
        n = r.randrange(0, 6)
        start = r.randrange(0, 4)
        parts = r.randrange(0, 6)
        put(model, 0x44, '3H', n, start, parts)
        put(model, 0x70, '2I', base + 0x2000, base + 0x3000)
        put(model, 0x7c, 'I', base + 0x1000)
        pb = bytearray(r.bytes(0x14 * max(parts, 1)))
        for j in range(parts):
            w = r.getrandbits(32) & ~0x3c000000
            for b in PART_BITS:
                if r.random() < 0.35:
                    w |= b
            put(pb, 0x14 * j, 'I', w)
        regions[base] = model
        regions[base + 0x1000] = pb
        regions[base + 0x2000] = r.bytes(0xa0 * 10)
        regions[base + 0x3000] = r.bytes(0x58 * 10)
    regions[OBJS] = b''.join(struct.pack('<4I', r.getrandbits(32), MODELS[m], r.getrandbits(32), r.getrandbits(32)) for m in range(3))
    count = r.randrange(0, 14)
    holes = r.randrange(0, 4)
    insts = bytearray(r.bytes(0x10 * 32))
    table = [r.getrandbits(32)] + [0] * 31
    for k in range(1, 32):
        put(insts, 0x10 * k + 8, 'I', OBJS + 0x10 * r.randrange(3))
        if r.random() < 0.8:
            table[k] = INST + 0x10 * k
    inst = bytearray(r.bytes(0xd0))
    put(inst, 4, 'I', r.getrandbits(32))
    regions[INST] = bytes(insts)
    regions[CAM] = inst
    regions[OUT] = r.bytes(4)
    regions.update({0x790ab0: struct.pack('<I', flags), 0x790a34: struct.pack('<I', CAM if r.random() < 0.7 else 0),
                    0x790adc: r.bytes(8), 0x7a1a8c: struct.pack('<I', r.getrandbits(32)), 0x7a4610: struct.pack('<32I', *table),
                    0x7a4084: struct.pack('<I', count), 0x7a3ebc: struct.pack('<I', holes)})
    mode = r.choice([2, 4, 0x40]) if r.random() < 0.9 else r.choice([0, 1, 3, 8, 0x3e, r.getrandbits(32)])
    flag = lambda: r.choice([0, 1]) if r.random() < 0.9 else r.getrandbits(32)  # noqa: E731
    ride = 0 if r.random() < 0.3 else r.randrange(1, 32)
    args = [mode, flag(), ride, flag(), OUT if r.random() < 0.8 else 0, flag()]
    return Call(0x42ab20, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
