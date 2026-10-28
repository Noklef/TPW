# The object's state and its queue's head (src/game/world/object: 0x4e0450, 0x4e0e60) against the originals, over objectmsg.py's queue
# (Remove's environment: a queue of persons in states 11, 12, 14, 8, 3 or 15, the object's head +0x3c one of them or 0; a ride record of id
# +0x24 or not, its element 0 the head, another person or any), the record's count 0-8 or -1 (so element 6 is there or not), the thing slot 0
# a person now and then (a head of 0; else a pointer with the top bit set). Restart's strip object ([0x7a4610 + 4 * +0x20], the low 2 bits) has no strips (+0xb0 0), and the
# message centre [0x788d3c] is one. 0x40fb10 (agent B's), 0x500870 and 0x5012f0 (agent C's), all unported, are thiscall recorders 21 (the
# message logged as its first 10 bytes), 22 and 23 (driver.cpp's kObjectEnv). Dismiss: 0x4e0450 alone. State: 0x4e0e60, the states 0-5 and any.
# Open (0x4e0050): the argument 0, 1 or any; the flags +0x32 bit 2 set or not; the state 2 mostly, 0, 1, 4 or any; for a close the strip
# object's +0xb0 (strips) set now and then (the shrink's flags and speed), for an open too: then the strip object's flags (+4) have bit 1
# clear (no Show) and its owner (+0xb4) chains to the frame pointer written (+8 -> +0x2c) and the frame list read (+0x24 -> +4 -> +0x50).
# Call (0x4e1220): +0x68 0 now and then; the record's count 10 mostly (elements 2, 5 and 9 there), its element 0 mostly 0, 2 and 5 small
# (signed) and 9 0 or not; the definition (ObjectDefs_Find's cache at 0x7897d0 holding the id +0xe) of kind 3, 2 or any; the flags +0x33
# random; the rider +0x6c 0, the head or another person; the head's state 11 and queue position (+0x1f1) 0 now and then, the rider's state
# 0xd now and then (the head mostly in state 11 at position 0; the rider 0x100 now and then: past the slots, a fault when it is looked at).
# usage: objectstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dismiss,State,Open,Call)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put  # noqa: E402
from thingenv import POOL  # noqa: E402

RANGES = ([(0x40f380, 0x40f3b0), (0x40f3d0, 0x40f400), (0x40f420, 0x40f450), (0x4217f0, 0x421810), (0x454550, 0x454800), (0x475a10, 0x475b90),
           (0x4d4140, 0x4d41a0), (0x4d8610, 0x4d8660), (0x4dd4e0, 0x4dd510), (0x4ddd20, 0x4ddf50), (0x4e0450, 0x4e0570), (0x4e0a70, 0x4e0ab0),
           (0x4e0e60, 0x4e11e4), (0x4e2670, 0x4e2720), (0x50b350, 0x50b370), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x67acf0, 0x67ada0),
           (0x5da3c0, 0x5da3c1), (0x4e0050, 0x4e01f0), (0x4e1220, 0x4e1406), (0x412e90, 0x412f00), (0x501290, 0x5013a0)])
HOOKS = {0x40fb10: _recorder(21, 1), 0x500870: _recorder(22, 0), 0x5012f0: _recorder(23, 0)}
B = Bench(RANGES, hooks=HOOKS)
CALLS = {'Dismiss': 0x4e0450, 'State': 0x4e0e60, 'Open': 0x4e0050, 'Call': 0x4e1220}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
OBJ, RIDES = objectmsg.OBJ, RAM + 0x20000
STRIP, CENTRE = RAM + 0x30000, RAM + 0x31000
DEF, OWNER, SHAPE, FRAMES, LIST = RAM + 0x32000, RAM + 0x33000, RAM + 0x33100, RAM + 0x33200, RAM + 0x33300


def case(r):
    op = r.deal(OPS)
    c = objectmsg.queue(r, 'Remove')
    regions = dict(c.regions)
    obj = bytearray(regions[OBJ]) + bytearray(r.bytes(0x100))
    k = r.randrange(4)
    put(obj, 0x20, 'I', k)
    regions[OBJ] = obj
    node = bytearray(regions[RIDES])
    put(node, 0x8c, 'i', r.choice([0, 1, 2, 6, 7, 8, -1]))
    regions[RIDES] = node
    regions[RIDES + 0x100] = bytes(regions[RIDES + 0x100][:4]) + r.bytes(0x3c)
    slots = bytearray(regions[0x7cfb90])
    if r.random() < 0.3:
        put(slots, 0, 'I', POOL)
    else:   # (else a pointer with the top bit set: a low random one can be the driver's memory natively, a fault only in the emulator)
        put(slots, 0, 'I', struct.unpack_from('<I', slots, 0)[0] | 0x80000000)
    regions[0x7cfb90] = slots
    table = bytearray(r.bytes(0x10))
    put(table, 4 * k, 'I', STRIP)
    regions[0x7a4610] = table
    regions[STRIP] = put(bytearray(r.bytes(0xc0)), 0xb0, 'I', 0)
    regions[0x788d3c] = struct.pack('<I', CENTRE)
    log = log_region()
    blob_bytes(log, 21, 0, 0x3ff)
    regions[LOG] = log
    if op == 'Open':
        put(obj, 0x19c, 'I', r.choice([2, 2, 2, 0, 1, 4, r.getrandbits(32)]))
        obj[0x32] = r.choice([4, 0, r.getrandbits(8)])
        opening = r.choice([0, 0, 1, 0x100, r.getrandbits(32)])
        if r.getrandbits(1):
            put(regions[STRIP], 0xb0, 'I', r.getrandbits(32) | 1)
            if opening:
                put(regions[STRIP], 4, 'I', r.getrandbits(32) & ~2)
                put(regions[STRIP], 0xb4, 'I', OWNER)
                regions[OWNER] = put(put(bytearray(r.bytes(0x30)), 8, 'I', SHAPE), 0x24, 'I', FRAMES)
                regions[SHAPE] = r.bytes(0x40)
                regions[FRAMES] = put(bytearray(r.bytes(8)), 4, 'I', LIST)
                regions[LIST] = r.bytes(0x60)
        return Call(0x4e0050, [OBJ, opening], regions, conv='t', ret='a')
    if op == 'Call':
        handles = [h for h in range(1, 12) if POOL <= struct.unpack_from('<I', slots, 20 * h)[0] < POOL + 6 * 0x400]
        if r.random() < 0.15:
            put(obj, 0x68, 'I', 0)
        put(node, 0x8c, 'i', r.choice([10, 10, 10, 10, 6, 3, 0, -1]))
        head = struct.unpack_from('<H', obj, 0x3c)[0]
        el = bytearray(regions[RIDES + 0x100])
        put(el, 0, 'I', r.choice([0, 0, 0, 0, head, r.getrandbits(32)]))
        for i in (2, 5):
            put(el, 4 * i, 'i', r.choice([0, 1, 2, 3, -1, r.getrandbits(32) - (1 << 31)]))
        put(el, 4 * 9, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        regions[RIDES + 0x100] = el
        put(obj, 0x6c, 'H', r.choice([0, 0, 0, 0, head, 0x100] + (handles and [r.choice(handles)])))
        pool = bytearray(regions[POOL])
        for h in handles:
            t = struct.unpack_from('<I', slots, 20 * h)[0] - POOL
            if r.random() < (0.8 if h == head else 0.5):
                put(pool, t + 0x220, 'i', r.choice([11, 11, 0xd]) if h != head else 11)
            if r.random() < (0.8 if h == head else 0.5):
                pool[t + 0x1f1] = 0
        regions[POOL] = pool
        oid = struct.unpack_from('<H', obj, 0xe)[0]
        regions[0x7897d0] = struct.pack('<HHI', oid, r.getrandbits(16), DEF)
        regions[DEF] = put(bytearray(r.bytes(0x100)), 0x9c, 'i', r.choice([3, 2, 2, 0, 1, r.getrandbits(32) - (1 << 31)]))
        return Call(0x4e1220, [OBJ], regions, conv='t', ret='v')
    if op == 'Dismiss':
        return Call(0x4e0450, [OBJ], regions, conv='t', ret='v')
    return Call(0x4e0e60, [OBJ, r.choice([0, 1, 1, 2, 2, 3, 4, 4, 5, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
