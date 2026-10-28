# A flying ride's slot cleared (src/game/world/flyingrides: 0x55cda0 FlyingRide_ClearSlot) against the original: a record (any bytes) at
# an index of the table 0x8791f8 (1-99 mostly), a slot 0-19 (now and then past them, still inside the mapped record); its particle system
# (+0xc4) none or a live, stale or any handle on particles.py's Spawn state (moved), its ride (+0xc0; Ride_Destroy 0x463c80 a cdecl
# recorder, 31) and its sound (+0xe0) none or a value for Sound_Forward6 on soundset.py's sender world (0x51c2c0).
# usage: flyingclear.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, _recorder, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import particles  # noqa: E402
import soundset  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops
particles.NODES, particles.SINES = RAM + 0x60000, RAM + 0x61000   # (clear of the sound object's page)
REC = RAM + 0x20000


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(particles.RANGES + soundset.RANGES + [(0x55cda0, 0x55ce5c)]), hooks={0x463c80: _recorder(31, 1, cdecl=True)})


def case(r):
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


if __name__ == '__main__':
    B.main(case)
