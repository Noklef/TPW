# A flying ride's new flyer (src/game/world/flyinglaunch: FlyingRide_Launch 0x55a720) against the original. Its calls are cdecl recorders (the driver's
# FlyingLaunchEnv): Model_FindPartByKey4 0x44b220 (1 of 3, -1 after some calls), Ride_Create 0x463060 (2 of 10), Sound_PlayRideElement 0x51eeb0 (3 of 7),
# FlyingRide_PlaySlotMotion 0x55aa80 (4 of 5). World_Rand and ObjectDefs_Find run as they are.
# usage: flyinglaunch.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_after, log_region, put, set_return  # noqa: E402

HOOKS = {0x44b220: _recorder(1, 3, cdecl=True), 0x463060: _recorder(2, 10, cdecl=True), 0x51eeb0: _recorder(3, 7, cdecl=True),
         0x55aa80: _recorder(4, 5, cdecl=True)}
B = Bench([(0x55a720, 0x55aa75), (0x516330, 0x516370), (0x516130, 0x5161b0), (0x50b350, 0x50b360), (0x412e90, 0x412eea), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
RECORD, STATE, DEF, OBJECT, RECORD2 = RAM, RAM + 0x3000, RAM + 0x3400, RAM + 0x3a00, RAM + 0x4000
SIZE, SLOTS, SLOT = 0x5c + 20 * 0xe4, 20, 0xe4


def case(r):
    log = log_region()
    record = bytearray(r.bytes(SIZE))
    free = r.deal([0, 1, 7, 19, 20, 3, 12, 5])
    for s in range(SLOTS):
        put(record, 0x5c + SLOT * s + 0x80, 'I', 0 if s == free else r.choice([1, 2, 4, 5, 0xffffffff]))
    used = r.deal([0, 0, 1, 2, r.getrandbits(32), 0xffffffff])
    put(record, 0xc, 'I', used)
    object_id = r.randrange(4)
    put(record, 0x58, 'H', object_id)
    id2 = r.choice([1, 2, 0x105])
    table = bytearray(0x14 * 4)
    put(table, 0x14 * object_id, 'I', OBJECT)
    obj = bytearray(0x10)
    put(obj, 0xe, 'H', id2)
    defs = bytearray(0x738)
    put(defs, 0, 'I', DEF)
    put(defs, 0x528, 'i', 1)
    put(defs, 0x730, 'H', id2 if r.random() < 0.5 else 0xffff)
    put(defs, 0x734, 'I', DEF)
    definition = bytearray(0x500)
    put(definition, 0, 'H', id2)
    put(definition, 0x4bc, 'I', r.getrandbits(32))
    bounds = bytearray(0x10)
    x_low, z_low = r.randrange(-100000, 100000), r.randrange(-100000, 100000)
    put(bounds, 0, '4i', x_low + r.choice([1, 7, 100, 1000, r.randrange(1, 400000)]), x_low, z_low + r.choice([1, 13, 100, 1000, r.randrange(1, 400000)]), z_low)
    instances = bytearray(16)
    for i in range(4):
        put(instances, 4 * i, 'I', r.getrandbits(32))
    fail_after(log, 1, r.choice([0, 1, 2, 3, 5]))
    set_return(log, 2, r.randrange(4))
    set_return(log, 3, r.getrandbits(32))
    state = struct.pack('<I', r.getrandbits(32))
    records = bytearray(8)
    index = r.randrange(2)
    put(records, 4 * index, 'I', RECORD)
    regions = {LOG: log, RECORD: record, DEF: definition, OBJECT: obj, 0x7890a0: defs, 0x7cfb90: table, 0x7660b8: bounds, 0x7a4610: instances, 0x8791f8: records,
               0x803a3c: r.bytes(4), STATE: state, 0x7cf83c: struct.pack('<I', (STATE - 0x1da708) & 0xffffffff)}
    return Call(0x55a720, [index], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
