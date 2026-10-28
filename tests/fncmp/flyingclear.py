# A flying ride's slot cleared (src/game/world/flyingrides: 0x55cda0 FlyingRide_ClearSlot) against the original: a record (any bytes) at
# an index of the table 0x8791f8 (1-99 mostly), a slot 0-19 (now and then past them, still inside the mapped record); its particle system
# (+0xc4) none or a live, stale or any handle on particles.py's Spawn state (moved), its ride (+0xc0; Ride_Destroy 0x463c80 a cdecl
# recorder, 31) and its sound (+0xe0) none or a value for Sound_Forward6 on soundset.py's sender world (0x51c2c0).
# Free (0x55d3d0 FlyingRide_Free): see free().
# usage: flyingclear.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Free)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import particles  # noqa: E402
import partstate  # noqa: E402
import soundset  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops
particles.NODES, particles.SINES = RAM + 0x60000, RAM + 0x61000   # (clear of the sound object's page)
REC = RAM + 0x20000
HOLDER, MODEL, PARTS = RAM + 0x22000, RAM + 0x22100, RAM + 0x22200


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = merged(particles.RANGES + soundset.RANGES + partstate.RANGES + [(0x55cda0, 0x55ce5c), (0x55d3d0, 0x55d4c9), (0x44b220, 0x44b267), (0x44b4c0, 0x44b505),
                                                                               (0x475550, 0x47558b), (0x5f5680, 0x5f5690)])
HOOKS = {0x463c80: _recorder(31, 1, cdecl=True)}
B = Bench(RANGES, hooks=HOOKS)


def clear(r):
    regions = dict(particles.spawn(r).regions)
    sound = soundset.sender_case(r, 0x51c2c0)
    regions.update(sound.regions)
    index = r.choice([1, 2, 99, r.randrange(1, 100)] * 5 + [0, 100])
    table = bytearray(r.bytes(0x194))
    put(table, index * 4, 'I', REC)
    slot = r.choice([r.randrange(20)] * 9 + [20, 21])
    rec = bytearray(r.bytes(0x5c + 22 * 0xe4))
    s = 0x5c + slot * 0xe4
    systems = regions[0x80cefc]
    live = r.randrange(6)
    put(rec, s + 0xc4, 'I', r.choice([0, 0, struct.unpack_from('<H', systems, 0x140 * live + 0xa)[0] << 16 | live] * 2 + [live, r.getrandbits(32)]))
    put(rec, s + 0xc0, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))
    put(rec, s + 0xe0, 'I', r.choice([0, 0, sound.args[0], r.getrandbits(32)]))
    regions.update({0x8791f8: table, REC: rec})
    return Call(0x55cda0, [index, slot], regions)


def free(r):
    """0x55d3d0 (index): a record (any bytes) at an index of the table (1-99 mostly, 0 / 100 / an empty entry now and then), its 20 slots in
    use (+0x80) or not; each one in use with a step count (+0x8c) of 0-3 either sign, a base (+0xd8) and a part count (+0xdc; 0 or -1 now
    and then), its particle system, ride and sound as Clear's. Either soundset.py's sender world for the sounds (no part found: the count
    0), or partstate.py's activation (no sounds) with the rides 1-3 a holder of its instance, whose model has 0-8 parts keyed 1-8, mostly
    flag 0x80 (as flyingslots.py)."""
    regions = dict(particles.spawn(r).regions)
    regions[HEAP] = heap()
    systems = regions[0x80cefc]
    parts = r.random() < 0.5
    if parts:
        partstate.activation(r, regions)
        regions[LOG] = log_region()   # (Ride_Destroy's recorder)
        put(regions[partstate.INSTANCE], 4, 'I', MODEL)
        model = put(bytearray(r.bytes(0x100)), 0x48, 'H', r.choice([0, 1, 4, 8]))
        put(model, 0x7c, 'I', PARTS)
        pt = bytearray(r.bytes(0xa0))
        for i in range(8):
            put(pt, i * 0x14, '2I', r.choice([0x80, 0x80, 0x81, 1, 0, r.getrandbits(32)]), r.choice([1, 2, 3, 4, 5, 8]))
        regions.update({0x7a4610: struct.pack('<4I', 0, HOLDER, HOLDER, HOLDER), HOLDER: put(bytearray(r.bytes(0x40)), 8, 'I', partstate.INSTANCE),
                        MODEL: model, PARTS: pt})
        sound = None
    else:
        sound = soundset.sender_case(r, 0x51c2c0)
        regions.update(sound.regions)
        table = bytearray(regions[0x6fd000])   # (GlobalFree served too)
        for at, v in global_heap().items():
            table[at - 0x6fd000:at - 0x6fd000 + 4] = v
        regions[0x6fd000] = table
    index = r.choice([1, 2, 99, r.randrange(1, 100)] * 5 + [0, 100])
    table = bytearray(r.bytes(0x194))
    put(table, index * 4, 'I', REC if r.random() < 0.95 else 0)
    rec = bytearray(r.bytes(0x5c + 20 * 0xe4))
    for i in range(20):
        s = 0x5c + i * 0xe4
        put(rec, s + 0x80, 'I', r.getrandbits(32) if r.random() < 0.3 else 0)
        put(rec, s + 0x8c, 'i', r.choice([0, 1, 2, 3, -1, -2, -3]))
        put(rec, s + 0xd8, 'i', r.choice([0, 1, r.randrange(-1000, 1000)]))
        put(rec, s + 0xdc, 'i', r.choice([1, 2, 3, 4, 8, 0, -1]) if parts else 0)
        put(rec, s + 0xc0, 'I', r.choice([1, 2, 3, 3, 0]) if parts else r.choice([0, 1, 5, r.getrandbits(32)]))
        live = r.randrange(6)
        put(rec, s + 0xc4, 'I', r.choice([0, 0, struct.unpack_from('<H', systems, 0x140 * live + 0xa)[0] << 16 | live] * 2 + [live, r.getrandbits(32)]))
        put(rec, s + 0xe0, 'I', r.choice([0, 0, sound.args[0], r.getrandbits(32)]) if sound else 0)
    regions.update({0x8791f8: table, REC: rec})
    return Call(0x55d3d0, [index], regions)


OPS_ALL = {'Clear': clear, 'Free': free}
OPS = [OPS_ALL[k] for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


def case(r):
    return r.choice(OPS)(r)


if __name__ == '__main__':
    B.main(case)
