# A ride's instance destroyed (src/game/world/rides: 0x463c80 Ride_Destroy) against the original: holder.py's Instance_Dispose world (its
# instance in the table 0x7a4610 at the ride, now and then an empty slot or ride 0), the instance's holder's model with no meshes (its base
# +0x9c, flags +0x30 with 0x100000 or not: Highlight_Unflash's two ways); the instance highlighted (0x7a1ce0, flag 0x20000 either way) or
# not, 0x7afda0 it or not, flashing (+4 flag 0x4000000; 0x7afe14 it) or not (never both flashing and highlighted-flagged unless current:
# Highlight_Update's world is not set up); the camera (0x790ab0 bit 4, 0x790988, 0x790a44 the ride or not); the env's 0x42a190 / 0x436fa0
# cdecl recorders (18, 19); the table's counts (0x7a4084, holes 0x7a3ebc, first hole 0x7aedf4) about the ride's slot, the slots below
# the end empty or not.
# usage: ridedestroy.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, _recorder, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import holder  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(holder.RANGES + [(0x463c80, 0x463dab), (0x469470, 0x469507), (0x42a410, 0x42a43a), (0x469650, 0x46983a)],
          hooks={**holder.HOOKS, 0x42a190: _recorder(18, 0, cdecl=True), 0x436fa0: _recorder(19, 1, cdecl=True)})
MODEL, BASE = RAM + 0x40000, RAM + 0x40100
SLOTS = 0x30


def case(r):
    regions = dict(holder.instance_dispose(r).regions)
    inst_at = holder.INST
    inst = bytearray(regions[inst_at])
    ride = r.choice([r.randrange(1, 0x20)] * 8 + [0, 1])
    current = r.choice([inst_at, 0, BASE])
    flags = struct.unpack_from('<I', inst, 4)[0] & ~0x4020000 | r.choice([0, 0x20000])
    if r.random() < 0.3:
        flags |= 0x4000000
        if current != inst_at:
            flags &= ~0x20000
    put(inst, 4, 'I', flags)
    regions[inst_at] = inst
    h2 = bytearray(regions[RAM])
    put(h2, 4, 'I', MODEL)
    regions[RAM] = h2
    model = bytearray(r.bytes(0xa0))
    put(model, 0x44, 'H', 0)
    put(model, 0x9c, 'I', BASE)
    put(model, 0x30, 'I', r.getrandbits(32) & ~0x100000 | r.choice([0, 0x100000]))
    table = bytearray(SLOTS * 4)
    for k in range(SLOTS):
        put(table, 4 * k, 'I', r.choice([0, 0, BASE]))
    put(table, 4 * ride, 'I', inst_at if r.random() < 0.92 else 0)
    slot = ride - 1
    holes = r.choice([0, 0, 1, 2, 3, r.randrange(6)])
    count = max(0, r.choice([slot + 1 - holes, slot + 1 - holes, slot + 1, slot + 3, r.randrange(SLOTS - 8)]))
    first = r.choice([slot, slot - 1, slot + 1, 0, r.randrange(SLOTS)]) & 0xffffffff
    regions.update({MODEL: model, BASE: r.bytes(0x100), 0x7a4610: table, 0x7a4084: struct.pack('<I', count),
                    0x7a3ebc: struct.pack('<I', holes), 0x7aedf4: struct.pack('<I', first), 0x7a1ce0: struct.pack('<I', current),
                    0x7afda0: struct.pack('<I', r.choice([inst_at, 0, BASE])), 0x7afe14: struct.pack('<I', inst_at),
                    0x790ab0: bytes([r.getrandbits(8) & ~4 | r.choice([0, 4, 4])]), 0x790988: struct.pack('<I', r.choice([0, 1, 1])),
                    0x790a44: struct.pack('<I', r.choice([ride, ride, 0, ride + 1]))})
    return Call(0x463c80, [ride], regions)


if __name__ == '__main__':
    B.main(case)
