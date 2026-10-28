# A track ride stripped (src/game/world/trackrides: TrakRide_Strip 0x545450) and destroyed (TrakRide_Destroy 0x545610) against the
# originals, with their callees run: trackcar_remove.py's car and ride (the car one of 1-4 records of the pool [0x877b68], the others used
# or not and on other rides, or on this one unused; the used count exact or one short; the module loaded or not as it deals it), the
# handle the ride's name mostly; TrakRide_Close's list (+0xc8, next +0x10, or none) and state (+0x50); the ride's +0xa8 pieces (0-4,
# flags 2 / 4 either way), +0xbc chunks (0-3, each with 0-3 nodes at +0x1c) and +0xc0 nodes (0-3), all chained by +0x14 (chunks by +0x24).
# All (0x5445d0 TrakRides_DestroyAll): the module loaded, the other rides unused (+0 clear) but for one now and then (no lists, no used
# car on it), the used count those (or one short: the walk stops early), the free records' block [0x877b88] (0x400 of 0x14 bytes).
# Cars (0x544b50 TrakRide_RemoveCars): Strip's world.
# usage: trakride_destroy.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Strip,Destroy,All,Cars)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import trackcar_remove as tcr  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(tcr.merged(tcr.RANGES + [(0x545450, 0x5457db), (0x544a10, 0x544b50), (0x5445d0, 0x54483d), (0x5457e0, 0x545809), (0x544b50, 0x544bf3)]), hooks=tcr.HOOKS)
RECS, LISTS, FREE = RAM + 0x70000, RAM + 0x71000, RAM + 0x72000


def chain(r, buf, base, at, n, stride, link):
    """n nodes of stride bytes from buf's offset at (address base + at), each pointing at the next by link; returns the head or 0."""
    for k in range(n):
        put(buf, at + stride * k + link, 'I', base + at + stride * (k + 1) if k + 1 < n else 0)
    return base + at if n else 0


def case(r):
    c = tcr.case(r)
    regions = c.regions
    car = regions.pop(tcr.CAR)
    ride = struct.unpack_from('<I', car, 0x9c)[0]
    index = (ride - tcr.RIDES) // 0xd0
    rides = regions[tcr.RIDES]
    module = bytearray(regions[0x877b58])
    if r.random() < 0.8:   # (mostly loaded: not loaded, only the handle 0 goes on, to the null ride's fault)
        put(module, 0, 'I', 0x4a454647)
    loaded = struct.unpack_from('<I', module)[0] == 0x4a454647
    kind = struct.unpack_from('<i', rides, 0xd0 * index)[0]
    name = (index | (kind & 0xffffffff) << 8) & 0xffffffff if loaded else 0
    # the pool: the car among 1-4 records
    n, at = r.randrange(1, 5), None
    at = r.randrange(n)
    recs = bytearray(r.bytes(0xac * n))
    used = 0
    for k in range(n):
        if k == at:
            continue
        flag = r.random() < 0.5
        recs[0xac * k] = recs[0xac * k] & ~1 | flag
        others = [tcr.RIDES + 0xd0 * j for j in range(4) if j != index]
        put(recs, 0xac * k + 0x9c, 'I', r.choice(others + [r.getrandbits(32)]) if flag else r.choice([ride, r.getrandbits(32)]))
        used += flag
    recs[0xac * at:0xac * at + 0xac] = car
    used += car[0] & 1
    if not car[0] & 1 and r.random() < 0.8:
        recs[0xac * at] |= 1   # (mostly a used car, so that it is taken off)
        used += 1
    put(module, 0x10, 'II', RECS if r.random() < 0.9 else 0, r.choice([used] * 4 + [max(used - 1, 0)]))
    put(module, 0xc, 'I', r.choice([1, 2, r.getrandbits(32)]))
    regions.update({RECS: recs, 0x877b58: module})
    # the ride's lists
    lists = bytearray(r.bytes(0x400))
    pieces = r.choice([0, 1, 2, 3, 4])
    put(rides, 0xd0 * index + 0xa8, 'I', chain(r, lists, LISTS, 0, pieces, 0x20, 0x14))
    for k in range(pieces):
        lists[0x20 * k] = lists[0x20 * k] & ~6 | r.choice([0, 0, 2, 4, 6])
    chunks = r.choice([0, 1, 2, 3])
    put(rides, 0xd0 * index + 0xbc, 'I', chain(r, lists, LISTS, 0x80, chunks, 0x30, 0x24))
    for k in range(chunks):
        put(lists, 0x80 + 0x30 * k + 0x1c, 'I', chain(r, lists, LISTS, 0x200 + 0x60 * k, r.choice([0, 1, 2, 3]), 0x20, 0x14))
    put(rides, 0xd0 * index + 0xc0, 'I', chain(r, lists, LISTS, 0x140, r.choice([0, 1, 2, 3]), 0x18, 0x14))
    put(rides, 0xd0 * index + 0x50, 'I', r.choice([0, 1, 4]))
    put(rides, 0xd0 * index + 0xc8, 'I', chain(r, lists, LISTS, 0x340, r.choice([0, 0, 1, 2]), 0x18, 0x10))
    regions[LISTS] = lists
    handle = r.choice([name] * 6 + [name ^ 0x100, name ^ 1, r.getrandbits(32)])
    func = r.choice(FUNCS)
    if func == 0x5445d0:
        put(module, 0, 'I', 0x4a454647)
        busy = {struct.unpack_from('<I', recs, 0xac * k + 0x9c)[0] for k in range(n) if recs[0xac * k] & 1}
        spare = [j for j in range(4) if j != index and tcr.RIDES + 0xd0 * j not in busy]
        for j in range(4):
            if j != index:
                put(rides, 0xd0 * j, 'I', 0)
        if spare and r.random() < 0.4:
            j = r.choice(spare)
            put(rides, 0xd0 * j, 'I', r.choice([1, -5 & 0xffffffff, r.getrandbits(32) | 1]))
            for o in (0xa8, 0xbc, 0xc0, 0xc8):
                put(rides, 0xd0 * j + o, 'I', 0)
        used = sum(1 for j in range(4) if struct.unpack_from('<I', rides, 0xd0 * j)[0])
        put(module, 0xc, 'I', r.choice([used] * 4 + [max(used - 1, 0)]))
        put(module, 0x30, 'I', FREE)
        regions[FREE] = r.bytes(0x5000)
        return Call(func, [], regions, ret='v')
    return Call(func, [handle], regions)


OPS_ALL = {'Strip': 0x545450, 'Destroy': 0x545610, 'All': 0x5445d0, 'Cars': 0x544b50}
FUNCS = [OPS_ALL[k] for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
