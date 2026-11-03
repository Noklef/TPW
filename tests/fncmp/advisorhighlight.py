# The advisor's highlight applied (src/game/world/advisor: Advisor_UpdateHighlight 0x598dc0) against the original. model_slot_selection.py's world (a holder with parts, the slot table) or
# no holder for the slot; the advisor's block (0xf79660): the flag bits of each arm alone, now and then mixed, with +0xc on or off, the kinds, up to 5 parts to mark with indexes in the
# holder's 16, the time's index past the part rows; the real pool 0 (its count 0-5, its handles real, none or stale) on particlepools.py's Spawn world, its position block, the clock's
# time (gameclock.py's less_pauses) and the sound globals with the manager's slot 2 a recorder (soundplay.py's, so the arm's sound is compared).
# usage: advisorhighlight.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import particles  # noqa: E402
import scene  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, vtable  # noqa: E402
from gameclock import less_pauses  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops



def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = merged([(0x402d80, 0x403220), (0x4033a0, 0x4033f0), (0x429ee0, 0x42a010), (0x44b220, 0x44b280), (0x51bfc0, 0x51c060), (0x51feb0, 0x51ff70), (0x520560, 0x520e10), (0x521d60, 0x5222f0),
                 (0x522360, 0x522500), (0x5988e0, 0x598970), (0x598ca0, 0x599060), (0x5ed5a0, 0x5ed780), (0x5f5e20, 0x5f5e90), (0x5f5f10, 0x5f5fc0), (0x402d70, 0x402e00), (0x5da3c0, 0x5da3c1),
                 (0x44a590, 0x44b2e0), (0x44b3c0, 0x44b410), (0x44b4c0, 0x44b505), (0x475550, 0x47558b), (0x67a830, 0x67a860)])
B = Bench(RANGES, sines=256)
BLOCK = 0xf79660
MANAGER, VT = RAM + 0x9000, RAM + 0x9100
NODES, SINES = RAM + 0xa000, RAM + 0xb000   # (particles.py's, moved off scene.py's model and parts)
ARMS = [0x20000, 0x40000, 0x80000, 0x100000, 0x200000]


def case(r):
    saved = scene.OPS
    scene.OPS = [r.choice(['SlotClear', 'SlotSet'])]
    try:
        c = scene.case(r)
    finally:
        scene.OPS = saved
    key, slot = c.args
    regions = dict(c.regions)
    if r.random() < 0.05:   # (no holder for the slot)
        regions[0x790398] = put(bytearray(regions[0x790398]), slot * 0xb0 + 4, 'I', 0)

    block = bytearray(r.bytes(0x168))
    arm = r.deal(ARMS)
    put(block, 0, 'I', r.choice([arm, arm, arm, arm | 0x10000, r.getrandbits(32) & ~0x3e0000 | arm, 0, 0x10000, r.getrandbits(32)]))
    put(block, 0xc, 'I', r.choice([1, 1, 1, 1, r.getrandbits(32) | 1, 0]))
    put(block, 0x14, 'I', slot)
    for off in (0x24, 0x28):   # (the kinds)
        put(block, off, 'I', r.choice([0, 1, 3, 8, 14, -1, key]) & 0xffffffff)
    for i in range((0x70 - 0x38) // 4):   # (0xf79698 by slot, 0xf796a0 by row)
        put(block, 0x38 + 4 * i, 'I', r.randrange(16))
    put(block, 0x10c, 'I', r.randrange(10, 16))   # (the time's index, past the rows)
    for i in range(10, 16):
        put(block, 0x68 + 4 * i, 'I', r.choice([0, 1, 40, -3, r.getrandbits(32)]) & 0xffffffff)
    put(block, 0x110, 'I', r.choice([0, 1, 2, 5]))
    block[0x144:0x164] = bytes(regions.pop(0xf797a4)) if 0xf797a4 in regions else r.bytes(0x20)
    pauses = less_pauses(r).regions
    block[0x11c:0x128] = pauses.pop(0xf7977c)
    regions.update(pauses)
    regions[BLOCK] = block

    world = particles.spawn(r).regions
    systems, defs = world[0x80cefc], world[0x804370]
    for k in range(0x69):   # (no voice links, as particlepools.py)
        if defs[0x140 * k + 0xba]:
            defs[0x140 * k + 0xba] = 0
            put(defs, 0x140 * k + 0xb4, 'i', -1)
    world[NODES], world[SINES] = world.pop(particles.NODES), world.pop(particles.SINES)
    put(world[0x80ced0], 0x1c, 'I', SINES)
    put(world[0x80ced0], 0x28, 'I', NODES)
    regions.update(world)
    pool = bytearray(r.floats(bytearray(r.bytes(0x70)), range(0, 0x18, 4)))
    put(pool, 0x2c, 'i', r.choice([0, 0, 1, 2, 3, 5]))
    for k in range(16):
        s = r.randrange(6)
        serial = struct.unpack_from('<H', systems, 0x140 * s + 0xa)[0]
        put(pool, 0x30 + 4 * k, 'I', r.choice([serial << 16 | s, serial << 16 | s, 0, 0, (serial ^ 1) << 16 | s]))
    regions[0xf86900] = pool

    log = log_region()
    blob(log, 2, 1, 0x7f)
    g = bytearray(r.bytes(0x20))
    put(g, 8, 'I', r.choice([1, 1, 1, 0]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([1, 1, 1, 0]))
    regions.update({LOG: log, 0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4), 0x803a2c: r.bytes(4)})
    return Call(0x598dc0, [], regions)


if __name__ == '__main__':
    B.main(case)
