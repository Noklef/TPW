# The camera set to follow a ride (src/game/world/camera: Camera_FollowThing 0x42a560) against the original. Its calls are recorders (the
# driver's CameraEnv): Model_FindPartByKey4 0x44b220 (cdecl 26 of 3: a part 0-4, or -1 now and then), PartStates_Update 0x44ab30 (cdecl 27
# of 1), Sprites_UseGroundFrames 0x542420 / UseParkFrames 0x542640 (cdecl 28 / 29), and Camera_SetMode's (20 to 25 as
# tests/fncmp/cameramode.py); Camera_SetMode, Camera_PartFace, Ride_Bit18, CoasterLink_IdValue and Terrain_ProceduralOn run as they are.
# The fixture: gCameraFlags 0x790ab0, the camera's part (0x790940) and instance (0x790988) or none, 0x790a44 / 0x790ab4 / 0x790a30, the
# ride table 0x7a4610 (count and holes; slots empty now and then; +4 with bit 18 and 0x2000 now and then), three models with their parts'
# flags, faces either side of +0x44 and part states (+0x28: or none), the coaster links 0x790fe0 (one whose id is the ride's now and then,
# flag 0x800, its +0x124 -> +0x34 -> +0xc a ride), the camera's instance 0x790a34 and the arguments (the ride, 1 or another, force, out or
# none, key 1 or another, fresh).
# usage: camerafollow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

HOOKS = {0x45d660: _recorder(20, 1, cdecl=True), 0x481470: _recorder(21, 1), 0x457a40: _recorder(22, 0, cdecl=True),
         0x457a00: _recorder(23, 0, cdecl=True), 0x45bcb0: _recorder(24, 3, cdecl=True), 0x45bd00: _recorder(25, 1, cdecl=True),
         0x44b220: _recorder(26, 3, cdecl=True), 0x44ab30: _recorder(27, 1, cdecl=True), 0x542420: _recorder(28, 0, cdecl=True),
         0x542640: _recorder(29, 0, cdecl=True)}
RANGES = [(0x42a560, 0x42a712), (0x42a720, 0x42a760), (0x42ab20, 0x42ae63), (0x436840, 0x4368ba), (0x457a70, 0x457a88)]
B = Bench(RANGES, hooks=HOOKS)
INST, CAM, OBJS, OUT, LINKS, MISC = (RAM + k * 0x1000 for k in range(6))
MODELS = [RAM + (6 + 5 * m) * 0x1000 for m in range(3)]   # each: the model, its parts, faces +0x70, faces +0x74, part states
BITS = [1, 2, 4, 0x10, 0x40, 0x80, 0x200, 0x400, 0x800, 0x1000]
PART_BITS = [0x4000000, 0x8000000, 0x10000000, 0x20000000]


def case(r):
    log = log_region()
    regions = {LOG: log}
    first = r.randrange(0, 5) if r.random() < 0.9 else 0xffffffff
    set_return(log, 26, first)
    if r.random() < 0.25:
        flags = r.getrandbits(32)
    else:
        flags = r.getrandbits(32) & ~0x1ff7 if r.random() < 0.3 else 0
        for b in BITS:
            if r.random() < 0.3:
                flags |= b
    for m, base in enumerate(MODELS):
        model = bytearray(r.bytes(0x80))
        parts = r.randrange(0, 6)
        put(model, 0x44, '3H', r.randrange(0, 8), r.randrange(0, 4), parts)
        put(model, 0x70, '2I', base + 0x2000, base + 0x3000)
        put(model, 0x7c, 'I', base + 0x1000)
        pb = bytearray(r.bytes(0x14 * max(parts, 1)))
        for j in range(parts):
            w = r.getrandbits(32) & ~0x3c000000
            for b in PART_BITS:
                if r.random() < 0.35:
                    w |= b
            put(pb, 0x14 * j, 'I', w)
        states = bytearray(r.bytes(0x10))
        put(states, 4, 'I', base + 0x4100)
        regions.update({base: model, base + 0x1000: pb, base + 0x2000: r.bytes(0xa0 * 12), base + 0x3000: r.bytes(0x58 * 12),
                        base + 0x4000: states, base + 0x4100: r.bytes(0x14 * 8)})
    regions[OBJS] = b''.join(struct.pack('<11I', r.getrandbits(32), MODELS[m], *[r.getrandbits(32) for _ in range(8)],
                                         # (no part states: only leaving follow mode runs on past them; entering reads them unchecked)
                                         MODELS[m] + 0x4000 if r.random() < (0.5 if flags & 4 else 0.98) else 0) + bytes(0x14)
                           for m in range(3))
    count = r.randrange(0, 14)
    holes = r.randrange(0, 4)
    insts = bytearray(r.bytes(0x10 * 32))
    table = [INST] + [0] * 31
    for k in range(32):
        w = r.getrandbits(32)
        if r.random() < 0.7:
            w &= ~0x40000
        put(insts, 0x10 * k + 4, '2I', w, OBJS + 0x40 * r.randrange(3))
        if r.random() < 0.85:
            table[k] = INST + 0x10 * k
    ride = r.randrange(0, 32)
    table[ride] = INST + 0x10 * ride   # (Ride_Bit18 reads the ride's slot unchecked: the callers' rides are there)
    links = bytearray(r.bytes(0x400))
    for j in range(2):
        put(links, 0x200 * j, 'I', r.getrandbits(32))
        put(links, 0x200 * j + 0x10, 'I', ride if r.random() < 0.6 else r.randrange(0, 32))
        put(links, 0x200 * j + 0x18, 'I', LINKS + 0x200 if j == 0 and r.random() < 0.6 else 0)
        put(links, 0x200 * j + 0x124, 'I', MISC)
    misc = bytearray(r.bytes(0x40))
    put(misc, 0x34, 'I', MISC + 0x20)
    put(misc, 0x2c, 'I', r.randrange(0, 32))
    regions.update({INST: bytes(insts), CAM: r.bytes(0xd0), OUT: r.bytes(4), LINKS: links, MISC: misc,
                    0x790ab0: struct.pack('<I', flags), 0x790a34: struct.pack('<I', CAM if r.random() < 0.7 else 0),
                    0x790adc: r.bytes(8), 0x7a1a8c: struct.pack('<I', r.getrandbits(32)), 0x7a4610: struct.pack('<32I', *table),
                    0x7a4084: struct.pack('<I', count), 0x7a3ebc: struct.pack('<I', holes),
                    0x790940: struct.pack('<I', MISC + 0x38 if r.random() < 0.6 else 0),
                    0x790988: struct.pack('<I', INST + 0x10 * r.randrange(1, 32) if r.random() < 0.6 else 0),
                    0x790a44: r.bytes(4), 0x790ab4: r.bytes(4), 0x790a30: r.bytes(4),
                    0x790fe0: struct.pack('<I', LINKS if r.random() < 0.8 else 0)})
    flag = lambda: r.choice([0, 1]) if r.random() < 0.9 else r.getrandbits(32)  # noqa: E731
    args = [ride, flag(), flag(), OUT if r.random() < 0.8 else 0, r.choice([1, 1, 0, 2, r.getrandbits(32)]), flag()]
    return Call(0x42a560, args, regions)


if __name__ == '__main__':
    B.main(case)
