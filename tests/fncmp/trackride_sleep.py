# A ride put to sleep and woken (src/game/world/trackrides: 0x548e80 TrakRide_SetAsleep) against the original, with its callees run: the ride's instances behind
# [0x7a4610 + 4 * index] (+8 and, with flag 0x2002000, +0xc) taken out of the renderer by their rectangle or by Holder_Sleep (holder.py's Unlink environment, whose
# part states the node list's parts release) or put back by Holder_AddToGrid / Holder_Wake (holder.py's RideAcquire environment, with partstate.py's activation
# moved clear of it for the parts the nodes activate); the flag 0x200 already as asked (nothing done).
# usage: trackride_sleep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Sleep,Wake)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
import partstate  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench(holder.RANGES + partstate.RANGES + [(0x548e80, 0x54901e)], hooks=holder.HOOKS)
RIDE, NODES = RAM + 0x20000, RAM + 0x20100
# partstate.py's activation, moved clear of holder.py's environments
partstate.INSTANCE, partstate.PS, partstate.ENTRIES, partstate.THINGS, partstate.TABLE, partstate.SPRITES, partstate.WORLD = (
    RAM + o for o in (0x21000, 0x21100, 0x21200, 0x21400, 0x21800, 0x22000, 0x23000))


def ride(r, regions, index, other, flags, parts):
    """The ride at RIDE: flags, the instances' indices at +8 / +0xc, a list of 0-3 nodes at +0x30 (parts -1 or below `parts`)."""
    rec = bytearray(r.bytes(0x40))
    n = r.choice([0, 1, 2, 3, 3])
    nodes = bytearray(r.bytes(0x20 * 3))
    for k in range(n):
        put(nodes, 0x20 * k + 8, 'i', r.choice([-1, r.randrange(parts), r.randrange(parts)]))
        put(nodes, 0x20 * k + 0x10, 'I', NODES + 0x20 * (k + 1) if k + 1 < n else 0)
    put(rec, 0, 'I', flags)
    put(rec, 8, 'II', index, other)
    put(rec, 0x30, 'I', NODES if n else 0)
    regions.update({RIDE: rec, NODES: nodes})


def flags(r, asleep):
    return r.getrandbits(32) & ~0x2002200 | r.choice([0, 0x2000, 0x2000000, 0x2002000]) | (0x200 if asleep else 0)


def sleep(r):   # holder.py's Unlink: its holder (part states with 8 entries) behind TARGET, at index and maybe a second index
    c = holder.unlink(r)
    regions = c.regions
    index, other = r.randrange(4), r.randrange(4)
    regions[0x7a4610] = b''.join(struct.pack('<I', holder.TARGET if k in (index, other) else 0) for k in range(4))
    ride(r, regions, index, r.choice([0, other, other]), flags(r, r.random() < 0.15), 8)
    return Call(0x548e80, [RIDE, r.choice([1, 1, 5, 0xffffffff])], regions, conv='c', ret='v')


def wake(r):    # holder.py's RideAcquire: its target at index (the rectangle or Holder_Wake); the holder's part states partstate.py's
    c = holder.acquire(r, True)
    regions = dict(c.regions)
    index = struct.unpack_from('<I', regions[holder.ACQ_USER], 4)[0]
    partstate.activation(r, regions)
    target = regions[holder.ACQ_TARGET]
    h = struct.unpack_from('<I', target, 8)[0]
    regions[h] = put(bytearray(regions[h]), 0x28, 'I', partstate.PS)
    regions[HEAP] = heap()
    asleep = r.random() < 0.9
    ride(r, regions, index, r.choice([0, index]), flags(r, asleep), 4)
    return Call(0x548e80, [RIDE, 0], regions, conv='c', ret='v')


CASES = {'Sleep': sleep, 'Wake': wake}
OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
