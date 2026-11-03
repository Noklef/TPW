# A sign in the world (src/game/world/sign: Sign_Create 0x4689f0) against the original. Its calls are cdecl recorders (the driver's SignEnv): Ride_Create
# 0x463060 (1 of 10), Model_FindSignEntries 0x468950 (2 of 3, filling its outs: 0 sometimes), Holder_SetScale 0x45bf20 (3 of 2, the scale logged),
# Ride_MarkModel 0x461ed0 (4 of 1, changing the ride now and then), Holder_FlagModel 0x45bed0 (5 of 1), Holder_WakeWithMatrix 0x45bd30 (6 of 3, the
# matrix logged), Animator_Play 0x4732a0 (7 of 6). Vec3_Distance and Matrix_Identity run as they are.
# The instance's +0xc8 is the local matrix's address, which differs between the sides (masked; nothing here reads it).
# usage: sign.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, Mask, _recorder, blob, fb, fill, log_region, put, set_return, strings  # noqa: E402

HOOKS = {0x463060: _recorder(1, 10, cdecl=True), 0x468950: _recorder(2, 3, cdecl=True), 0x45bf20: _recorder(3, 2, cdecl=True),
         0x461ed0: _recorder(4, 1, cdecl=True), 0x45bed0: _recorder(5, 1, cdecl=True), 0x45bd30: _recorder(6, 3, cdecl=True),
         0x4732a0: _recorder(7, 6, cdecl=True)}
B = Bench([(0x4689f0, 0x468e4a), (0x470940, 0x47098f), (0x46f150, 0x46f176)], hooks=HOOKS)
INSTANCES, HOLDER, MODEL, TYPE, TDEF, TMODEL, ROOT, BOX, NODES, FACES = RAM, RAM + 0x400, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900, RAM + 0xa00, \
    RAM + 0xb00, RAM + 0xc00, RAM + 0x1000
N_INSTANCES = 2
ENTRIES = [0x1111, 0x2222, 0x3333, 0x4444]


def value(r, scale=None):
    return r.f(scale) if r.random() < 0.15 else fb(r.choice([0.0, -0.0, 1.0, 0.5, 2.0, r.uniform(-9, 9)]))


def case(r):
    log = log_region()
    instances, holder, model = bytearray(0x100 * N_INSTANCES), bytearray(0x10), bytearray(0x80)
    first = r.deal([0, 1])
    second = first if r.random() < 0.9 else 1 - first
    for i in range(N_INSTANCES):
        put(instances, 0x100 * i + 8, 'I', HOLDER)
        put(instances, 0x100 * i + 0xcc, 'I', BOX)
    table = bytearray(8)
    put(table, 0, '2I', INSTANCES, INSTANCES + 0x100)
    put(holder, 4, 'I', MODEL)
    put(model, 0x78, 'I', ROOT)
    root = bytearray(r.getrandbits(8) for _ in range(0x60))
    box = bytearray(0x40)
    for k in range(3):
        low = r.uniform(-9, 9)
        put(box, 0x18 + 4 * k, 'f', low)
        put(box, 0x24 + 4 * k, 'f', low + r.choice([0.0, 1.0, r.uniform(0, 9)]))
    for _ in range(2):
        if r.random() < 0.1:
            put(box, r.choice([0x18, 0x20, 0x24, 0x2c]), 'I', r.f(9))
    n_nodes = r.choice([0, 1, 2, 3])
    nodes, faces = bytearray(0xa0 * max(n_nodes, 1)), bytearray(0x10 * 12)
    put(model, 0x44, 'H', n_nodes)
    put(model, 0x70, 'I', NODES)
    for n in range(n_nodes):
        count = r.randrange(5)
        put(nodes, 0xa0 * n + 0x5a, 'H', count)
        put(nodes, 0xa0 * n + 0x6c, 'I', FACES + 0x10 * 4 * n)
    for k in range(12):
        put(faces, 0x10 * k, 'I', r.choice(ENTRIES + [0, 5]))
        put(faces, 0x10 * k + 4, 'I', r.choice(ENTRIES))
    type_, tdef, tmodel, models = bytearray(0x100), bytearray(0x10), bytearray(0x10), bytearray(16)
    kind = r.randrange(4)
    put(models, 4 * kind, 'I', TYPE)
    put(type_, 0xd0, 'I', r.choice([0, 1, 1, 1, 0x100]))
    put(type_, 8, 'I', TDEF)
    put(tdef, 4, 'I', TMODEL)
    outs = [r.choice([v, v, v, 0]) if r.random() < 0.2 else v for v in ENTRIES]
    strings(log, 2, 0, 6)
    fill(log, 2, 1, struct.pack('<2I', outs[0], outs[2]), which=0, records=2)
    fill(log, 2, 2, struct.pack('<2I', outs[1], outs[3]), which=1, records=2)
    blob(log, 3, 1, 7)
    blob(log, 6, 1, 0xffff)
    fill(log, 4, 0, struct.pack('<I', second), which=2)
    set_return(log, 1, first)
    glob = bytearray(0x90)
    for at in range(0, 0x90, 4):
        put(glob, at, 'I', r.getrandbits(32))
    put(glob, 0x48, 'I', value(r, 30))
    mask = [Mask(INSTANCES, [0x100 * i + 0xc8 + k for i in range(N_INSTANCES) for k in range(4)], 'the local matrix\'s address, kept (sic)')]
    regions = {LOG: log, INSTANCES: instances, HOLDER: holder, MODEL: model, TYPE: type_, TDEF: tdef, TMODEL: tmodel, ROOT: root, BOX: box, NODES: nodes,
               FACES: faces, 0x7a4610: table, 0x7ae6a8: models, 0x7afd08: glob}
    args = [r.getrandbits(32), value(r, 9), r.getrandbits(32), value(r, 9), value(r, 9), kind]
    return Call(0x4689f0, args, regions, ret='a', masks=mask)


if __name__ == '__main__':
    B.main(case)
