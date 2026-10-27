# The object's state and its queue's head (src/game/world/object: 0x4e0450, 0x4e0e60) against the originals, over objectmsg.py's queue
# (Remove's environment: a queue of persons in states 11, 12, 14, 8, 3 or 15, the object's head +0x3c one of them or 0; a ride record of id
# +0x24 or not, its element 0 the head, another person or any), the record's count 0-8 or -1 (so element 6 is there or not), the thing slot 0
# a person now and then (a head of 0). Restart's strip object ([0x7a4610 + 4 * +0x20], the low 2 bits) has no strips (+0xb0 0), and the
# message centre [0x788d3c] is one. 0x40fb10 (agent B's), 0x500870 and 0x5012f0 (agent C's), all unported, are thiscall recorders 21 (the
# message logged as its first 10 bytes), 22 and 23 (driver.cpp's kObjectEnv). Dismiss: 0x4e0450 alone. State: 0x4e0e60, the states 0-5 and any.
# usage: objectstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dismiss,State)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put  # noqa: E402
from thingenv import POOL  # noqa: E402

B = Bench([(0x40f380, 0x40f3b0), (0x40f3d0, 0x40f400), (0x40f420, 0x40f450), (0x4217f0, 0x421810), (0x454550, 0x4547d0), (0x475a10, 0x475b90),
           (0x4d4140, 0x4d41a0), (0x4d8610, 0x4d8660), (0x4dd4e0, 0x4dd510), (0x4ddd20, 0x4ddf50), (0x4e0450, 0x4e0570), (0x4e0a70, 0x4e0ab0),
           (0x4e0e60, 0x4e11e4), (0x4e2670, 0x4e2720), (0x50b350, 0x50b370), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x67acf0, 0x67ada0),
           (0x5da3c0, 0x5da3c1)],
          hooks={0x40fb10: _recorder(21, 1), 0x500870: _recorder(22, 0), 0x5012f0: _recorder(23, 0)})
CALLS = {'Dismiss': 0x4e0450, 'State': 0x4e0e60}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
OBJ, RIDES = objectmsg.OBJ, RAM + 0x20000
STRIP, CENTRE = RAM + 0x30000, RAM + 0x31000


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
    regions[0x7cfb90] = slots
    table = bytearray(r.bytes(0x10))
    put(table, 4 * k, 'I', STRIP)
    regions[0x7a4610] = table
    regions[STRIP] = put(bytearray(r.bytes(0xc0)), 0xb0, 'I', 0)
    regions[0x788d3c] = struct.pack('<I', CENTRE)
    log = log_region()
    blob_bytes(log, 21, 0, 0x3ff)
    regions[LOG] = log
    if op == 'Dismiss':
        return Call(0x4e0450, [OBJ], regions, conv='t', ret='v')
    return Call(0x4e0e60, [OBJ, r.choice([0, 1, 1, 2, 2, 3, 4, 4, 5, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
