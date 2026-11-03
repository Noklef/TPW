# What a coaster type learns from its model (src/game/coaster/typemodel: CoasterType_ReadModel 0x430590) against the original. Its calls are cdecl
# recorders (the driver's TypeModelEnv): Ride_Create 0x463060 (1 of 10, answering the instance's index; its desc logged), Layer_Play 0x472f60 (2 of 6),
# Animator_Update 0x473c70 (3 of 3), Ride_Destroy 0x463c80 (4 of 1). Animator_HasMotion, Model_NthPart and Model_CountParts run as they are.
# The world: a model of up to 12 parts with flags and kinds, its banks' flags and counts, an instance with its layers and part objects, the block.
# usage: typemodel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fb, fill, log_region, put, set_return  # noqa: E402

HOOKS = {0x463060: _recorder(1, 10, cdecl=True), 0x472f60: _recorder(2, 6, cdecl=True), 0x473c70: _recorder(3, 3, cdecl=True),
         0x463c80: _recorder(4, 1, cdecl=True)}
B = Bench([(0x430590, 0x430ac0), (0x44b270, 0x44b2e0), (0x472d10, 0x472d43)], hooks=HOOKS)
BLOCK, HOLDER, DEF, MODEL, PARTS = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
INSTANCE, IDEF, IMODEL, OBJECTS, LIST, LAYERS = RAM + 0x800, RAM + 0xa00, RAM + 0xb00, RAM + 0xc00, RAM + 0x1800, RAM + 0x1c00
MASKS = [0x1000000, 0x800000, 0x2000000, 0x100000, 0x80000, 0x400000]
N_PARTS = 12


def height(r):
    return r.f(9) if r.random() < 0.15 else fb(r.choice([0.0, -0.0, 1.0, 2.5, -3.0, r.uniform(-9, 9)]))


def banks(r, block):
    for b in (2, 3, 9, 10):
        put(block, (b + 2) * 12, 'I', r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]))
        put(block, b * 12 + 0x1c, 'I', r.choice([0, 1, 2, r.getrandbits(8)]))


def case(r):
    log = log_region()
    block, holder, def_, model, parts = bytearray(0x100), bytearray(0x100), bytearray(0x10), bytearray(0x80), bytearray(0x14 * N_PARTS)
    for at in range(0, 0xe0, 4):
        put(block, at, 'I', r.getrandbits(32))
    put(block, 0, 'I', r.choice([0, 8, 0xffffffff, r.getrandbits(32)]) & ~r.choice([0, 8, 0x3fe0, 0x3fe8]))
    put(block, 0x2c, 'I', r.randrange(N_PARTS))
    put(block, 0x68, 'I', height(r))
    low = r.uniform(-1, 2)
    put(block, 0xd0, '4I', fb(low), fb(low + r.choice([0.0, 1.0, r.uniform(0, 5)])), fb(r.uniform(-2, 4)), fb(r.uniform(-2, 4)))
    if r.random() < 0.15:
        put(block, 0xd0 + 4 * r.randrange(4), 'I', r.f(5))
    if r.random() < 0.1:
        put(block, 0xd4, 'I', 0x7fc00000)   # (no greatest: the clamp's compare)
    n = r.deal([0, 1, 2, 3, 5, 8, 12])
    banks(r, holder)
    put(holder, 8, 'I', DEF)
    put(def_, 4, 'I', MODEL)
    put(model, 0x48, 'H', n)
    put(model, 0x7c, 'I', PARTS)
    lone = r.choice(MASKS + [0, 0])
    for p in range(n):
        flags = lone if r.random() < 0.5 else sum(m for m in MASKS if r.random() < 0.2)
        put(parts, 0x14 * p, 'I', flags | (r.getrandbits(32) & 0xc00000ff))
        put(parts, 0x14 * p + 4, 'I', r.choice([0, 1, 2, 3, 4, 5, 1, 3, r.getrandbits(32)]))
    ride = r.randrange(4)
    instance = bytearray(0x100)
    banks(r, instance)
    put(instance, 4, 'I', r.getrandbits(32))
    put(instance, 8, 'I', IDEF)
    put(instance, 0x10, 'I', LAYERS)
    idef, imodel, objects, plist, layers = bytearray(0x40), bytearray(0x10), bytearray(0x40 * N_PARTS), bytearray(0x14 * N_PARTS), bytearray(0x38 * 5)
    put(idef, 0x28, 'I', IMODEL)
    put(imodel, 4, 'I', LIST)
    for p in range(N_PARTS):
        put(plist, 0x14 * p + 4, 'I', 0 if r.random() < 0.2 else OBJECTS + 0x40 * p)
        put(objects, 0x40 * p + 0x34, 'I', height(r))
    put(layers, 0x1c, '2I', height(r), r.getrandbits(32))
    table, models = bytearray(4 * 4), bytearray(4 * 4)
    put(table, 4 * ride, 'I', INSTANCE)
    kind = r.randrange(4)
    if r.random() > 0.05:
        put(models, 4 * kind, 'I', HOLDER)
    blob(log, 1, 2, 0xf)
    set_return(log, 1, ride)
    # (Animator_Update: the rate it finds in layer 0; each call moves the block's part)
    put(instance, 0xf0, 'I', OBJECTS + 0x40 * struct.unpack_from('<I', block, 0x2c)[0] + 0x34)
    blob_bytes(log, 3, 0, 0, inner=(0x10, 0xf0, 28))
    fill(log, 3, 0, struct.pack('<2I', height(r), height(r)), records=2, through=0xf0)
    regions = {LOG: log, BLOCK: block, HOLDER: holder, DEF: def_, MODEL: model, PARTS: parts, INSTANCE: instance, IDEF: idef, IMODEL: imodel,
               OBJECTS: objects, LIST: plist, LAYERS: layers, 0x7a4610: table, 0x7ae6a8: models, 0x791050: bytes(8), 0x791ce8: bytes(16)}
    return Call(0x430590, [r.getrandbits(32), BLOCK, kind], regions)


if __name__ == '__main__':
    B.main(case)
