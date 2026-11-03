# The camera following an object (src/game/world/object: Object_FollowCamera, 0x4e15b0) against the original, with ridestate.py's track rides, ride
# records and flying rides and thingenv.py's definitions (found by the object's id, +0x80 and +0x11c read). Camera_FollowThing (0x42a560,
# agent B's) is the cdecl recorder 26 of 6 (ObjectEnv in driver.cpp).
# usage: objectcamera.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridestate  # noqa: E402
import thingenv  # noqa: E402
from fncmp import LOG, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x4e15b0, 0x4e1668), (0x412e90, 0x412f30), (0x549d30, 0x549dc0), (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0), (0x5da3c0, 0x5da3c1)],
          hooks={0x42a560: _recorder(26, 6, cdecl=True)})


def case(r):
    c = ridestate.case(r)
    defs, _, _ = thingenv.park(r, count=0)
    regions = dict(c.regions)
    regions[thingenv.DEFS], regions[thingenv.ITEMS] = defs[thingenv.DEFS], defs[thingenv.ITEMS]
    log = log_region()
    regions[LOG] = log
    set_return(log, 26, r.getrandbits(32))
    n = struct.unpack_from('<i', defs[thingenv.DEFS], 0x528)[0]
    obj = regions[ridestate.OBJ]
    put(obj, 0x20, 'I', r.getrandbits(32))
    item = r.randrange(n)
    put(obj, 0xe, 'H', struct.unpack_from('<H', defs[thingenv.ITEMS], item * thingenv.ITEM)[0])
    # (ids of records that exist: Rides_Get9c of no record reads through null, in both)
    nodes, ids = regions[ridestate.NODES], []
    if struct.unpack_from('<I', regions[0x8791a0])[0] and struct.unpack_from('<I', regions[0x8791b0])[0]:
        at = ridestate.NODES
        while at:
            ids.append(struct.unpack_from('<I', nodes, at - ridestate.NODES + 8)[0])
            at = struct.unpack_from('<I', nodes, at - ridestate.NODES)[0]
    else:
        regions[0x8791a0] = struct.pack('<I', 1)
        regions[0x8791b0] = struct.pack('<I', ridestate.NODES)
        put(nodes, 0, 'I', 0)
        put(nodes, 8, 'I', 1)
        ids = [1]
    put(obj, 0x24, 'I', r.choice(ids) if r.random() < 0.97 else r.getrandbits(32))
    return Call(0x4e15b0, [ridestate.OBJ, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
