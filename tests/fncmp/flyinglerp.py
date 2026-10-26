# The flying rides' slots placed between their old and new positions (src/game/world/flyingrides: 0x55ce60) against the original: t about
# 0-2 (or past 2, or anything with --wild), 0-2 records in the table (0x8791fc-0x879388) of 20 slots each, in use or not, new or not (+0xc8),
# positions and their old ones near or far, 12-bit angles and their old ones either side of the 0x800 wrap (or any), last positions and
# angles near or not; a slot's ride 0 or one of three sharing a holder whose matrix gets the rotation (the matrix library's sine table),
# and its particle system 0 or a handle on the particles bench's systems (live or not), the part keyed 1 among the flag-0x100 parts found
# or not, its entry null or not.
# usage: flyinglerp.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import particles  # noqa: E402
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench(particles.RANGES + [(0x55ce60, 0x55d3d0), (0x46f150, 0x470990), (0x44b220, 0x44b267), (0x67a400, 0x690000)], sines=256)
HOLDER, INSTANCE, MODEL, PARTS, STATE, ENTRIES, ENTRY, MATRIX = (RAM + 0x20000 + 0x100 * k for k in range(8))
RECS = RAM + 0x30000


def angle(r):
    return r.choice([r.randrange(0x1000), 0, 0x7ff, 0x800, 0x801, 0xfff, r.randrange(-0x3000, 0x3000), r.getrandbits(32)]) & 0xffffffff


def near(r, v, far):
    return (v + r.choice([0, 1, -1, r.randrange(-50, 50), r.randrange(-far, far)])) & 0xffffffff


def slot(r, rec, base):
    put(rec, base + 0x80, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    put(rec, base + 0xc8, 'I', r.choice([0, 0, 0, 1]))
    for k in range(3):
        v = r.choice([r.randrange(-300000, 300000), r.randrange(-3000, 3000), r.getrandbits(32) - (1 << 31)])
        put(rec, base + 4 + 4 * k, 'I', v & 0xffffffff)
        put(rec, base + 0x1c + 4 * k, 'I', near(r, v, 1 << 20))
        put(rec, base + 0x34 + 4 * k, 'I', fb(v / 300 + r.choice([0, 0, r.uniform(-50, 50)])) if r.random() < 0.9 else r.f())
    for k in range(3):
        v = angle(r)
        put(rec, base + 0x10 + 4 * k, 'I', v)
        put(rec, base + 0x28 + 4 * k, 'I', near(r, v, 0x1000) if r.random() < 0.8 else angle(r))
        put(rec, base + 0x40 + 4 * k, 'I', fb(r.choice([r.randrange(0x1000), r.uniform(-8000, 8000), 0x800, 0])) if r.random() < 0.9 else r.f())
    put(rec, base + 0xc0, 'I', r.choice([0, 1, 2, 3, 3]))
    put(rec, base + 0xc4, 'I', 0)


def case(r):
    regions = {}
    table = bytearray(4 * 100)
    for i in range(r.choice([0, 1, 1, 2, 2])):
        rec = bytearray(r.bytes(0x122c))
        for k in range(20):
            slot(r, rec, 0x5c + k * 0xe4)
        regions[RECS + 0x1300 * i] = rec
        put(table, 4 * r.choice([1, 99, r.randrange(1, 100)]), 'I', RECS + 0x1300 * i)
    regions[0x8791f8] = table
    regions[0x8791c0] = r.bytes(0x38)
    # particle systems: as particles.py's VecA, each slot with a system (one in three) on a handle of its own
    setup = bytearray(r.bytes(0x10))
    put(setup, 0, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    systems = bytearray(r.bytes(0x78 * 0x140))
    for base in list(regions):
        if base < RECS:
            continue
        rec = regions[base]
        for k in range(20):
            if r.random() < 0.35:
                n = r.randrange(0x78)
                serial = r.getrandbits(16)
                put(systems, n * 0x140 + 0xa, 'H', serial if r.random() < 0.9 else serial ^ 1)
                put(rec, 0x5c + k * 0xe4 + 0xc4, 'I', serial << 16 | n)
    regions.update({particles.SETUP: setup, particles.SYSTEMS: systems})
    rides = bytearray(r.bytes(0x10))
    for k in range(1, 4):
        put(rides, 4 * k, 'I', HOLDER)
    regions[0x7a4610] = rides
    regions[HOLDER] = put(bytearray(r.bytes(0x40)), 8, 'I', INSTANCE)
    instance = bytearray(r.bytes(0x40))
    put(instance, 4, 'I', MODEL)
    put(instance, 0x28, 'I', STATE)
    regions[INSTANCE] = instance
    model = bytearray(r.bytes(0x100))
    put(model, 0x48, 'H', r.choice([0, 1, 4, 8]))
    put(model, 0x78, 'I', MATRIX)
    put(model, 0x7c, 'I', PARTS)
    regions[MODEL] = model
    parts = bytearray(r.bytes(0xa0))
    for i in range(8):
        put(parts, i * 0x14, '2I', r.choice([0x100, 0x100, 0x101, 0x80, 0, r.getrandbits(32)]), r.choice([1, 1, 2, 3]))
    regions[PARTS] = parts
    regions[STATE] = put(bytearray(r.bytes(0x10)), 4, 'I', ENTRIES)
    entries = bytearray(r.bytes(0xa0))
    for i in range(8):
        put(entries, i * 0x14 + 4, 'I', ENTRY if r.random() < 0.85 else 0)
    regions[ENTRIES] = entries
    entry = bytearray(r.bytes(0x40))
    for k in (0x30, 0x34, 0x38):
        put(entry, k, 'I', fb(r.choice([0, 1.5, -1.5, r.uniform(-3000, 3000), r.uniform(-1e6, 1e6)])) if r.random() < 0.9 else r.f())
    regions[ENTRY] = entry
    matrix = bytearray(0x60)
    for k in range(0, 0x60, 4):
        put(matrix, k, 'I', fb(r.uniform(-2, 2)) if r.random() < 0.95 else r.f())
    regions[MATRIX] = matrix
    t = r.choice([0.0, 0.5, 1.0, 2.0, 2.5, r.uniform(0, 2), r.uniform(0, 2), r.uniform(-1, 3)])
    return Call(0x55ce60, [fb(t) if r.random() < 0.95 else r.f()], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
