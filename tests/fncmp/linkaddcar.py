# A car piece added to the current coaster link (src/game/coaster/registry: CoasterLink_AddCar 0x434060) against the original, with what
# it calls: GlobalMem_Alloc (global_heap()), TrackPieces_Renumber, Ride_Claim (its object definitions without one), Layer_Play,
# Car_SetupCoupling (coastersetup.py's holder layout), Map_SetRideCell (only its bounds: the map is 0 wide) and Train_InsertCar. The ride
# made (0x463060) is the cdecl recorder k 30 (driver.cpp's CoasterLinksEnv), answering a ride index.
# - Insert: insertcar.py's train, cars and rides (its car the new piece; the rides enlarged to 0x100 bytes, their bank flags without 1 or 2
#   so that Layer_Play is not reached), without the spare flag 2, so that Train_InsertCar runs; the train's +4 bit 1 now and then.
# - Spare: animator.py's world, the ride its animator (a fifth layer added), flag 2 always (no Train_InsertCar), so Layer_Play runs.
# Both: the link's +0x3c flag 2 (a piece P with a next one Q) or not, its chain's first / last P or none, desc's flags (0x4000, 8, the five
# bank bits 0x20 << i), its type 0..3, its two coupling specs (0..3 seats), its clamp range and factors (NaNs now and then).
# usage: linkaddcar.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fb, global_heap, heap, log_region, put, set_return  # noqa: E402
import animator  # noqa: E402
import insertcar  # noqa: E402

INSERT_RANGES = [(0x433bd0, 0x433f64), (0x432df0, 0x433220), (0x433220, 0x433240), (0x433280, 0x433300), (0x430ca0, 0x430ee0),
                 (0x433790, 0x4338e0), (0x433a90, 0x433bd0), (0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0),
                 (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)]
ANIM_RANGES = [(0x46f150, 0x475010), (0x67a400, 0x690000), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172),
               (0x4031e0, 0x403202), (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)]
B = Bench(sorted(set(INSERT_RANGES + ANIM_RANGES + [
    (0x434060, 0x4344b5), (0x4344c0, 0x434566), (0x44b140, 0x44b2e0), (0x444700, 0x444740), (0x463ab0, 0x463c80), (0x412e90, 0x412eea),
    (0x452d30, 0x452e03), (0x4d0af0, 0x4d0b40), (0x4d8300, 0x4d8328), (0x45b9d0, 0x45ba30), (0x5da3c0, 0x5da3d0)])),
    hooks={0x473c70: _recorder(27, 3, cdecl=True), 0x463060: _recorder(30, 10, cdecl=True)})
NR = 3
BANKS = (3, 0xa, 2, 9, 0xa)


def link_world(r, base, regions, owner=b''):
    """the link at base (0x790fec), its pieces P, Q, the desc and the coupling holder; returns (desc, the link's +0x3c flag 2)."""
    link, pcs, desc, holder = base, base + 0x200, base + 0x400, base + 0x600
    lk = bytearray(r.bytes(0x140))
    lk[:len(owner)] = owner
    flag2 = r.random() < 0.3
    put(lk, 0x3c, 'I', r.getrandbits(32) & ~6 | (2 if flag2 else 0) | r.choice([0, 4]))
    put(lk, 0x44, 'I', r.choice([0, 1, 2, r.randrange(100), r.getrandbits(32)]))
    put(lk, 0x48, 'I', r.f(50))
    first = r.choice([0, pcs])
    put(lk, 0x54, 'II', first, r.choice([0, pcs]) if first else r.choice([0, 0, pcs]))
    put(lk, 0x60, 'I', pcs)
    p = bytearray(r.bytes(0x100))
    put(p, 0x64, 'II', pcs + 0x80, r.choice([0, pcs + 0x80]))
    put(p, 0x80 + 0x64, 'II', 0, pcs)
    d = bytearray(r.bytes(0x100))
    put(d, 0, 'I', r.getrandbits(32) & ~0x4008 | r.choice([0, 0x4000]) | r.choice([0, 8]))
    put(d, 0x14, 'I', r.randrange(4))
    for spec in (0x30, 0x44):
        put(d, spec + 0xc, 'II', r.choice([0, 1, 1, 2, 3]), r.getrandbits(32) & 0xff)
    lo = r.uniform(-2, 2)
    put(d, 0xd0, 'II', r.choice([fb(lo)] * 8 + [r.f()]), r.choice([fb(lo + r.uniform(-1, 3))] * 8 + [r.f()]))
    put(d, 0xd8, 'II', r.choice([r.f(), r.f(), 0x7f800000, 0x7fa00000]), r.choice([fb(r.uniform(-4, 4)), r.f(), 0x7fa00000]))
    hd, model, parts = bytearray(r.bytes(0x40)), bytearray(r.bytes(0x80)), bytearray(r.bytes(12 * 0x14))   # (coastersetup.py's)
    put(hd, 0, 'I', holder + 0x20)
    put(hd, 8, 'I', holder + 0x20)
    put(hd, 0x24, 'I', holder + 0x40)
    put(model, 0x48, 'H', r.choice([0, 3, 12]))
    put(model, 0x7c, 'I', holder + 0x100)
    for i in range(12):
        put(parts, i * 0x14, 'I', r.getrandbits(32) | r.choice([0, 0x400000, 0x400000]))
        put(parts, i * 0x14 + 4, 'I', r.getrandbits(32) & 0xff if r.random() < 0.3 else (struct.unpack_from('<I', d, 0x40)[0] + r.randrange(-1, 4)) & 0xffffffff)
    defs = put(bytearray(0x738), 0x730, 'H', 0xffff)
    regions.update({link: lk, pcs: p, desc: d, holder: hd + model + bytearray(0x40) + parts, 0x7890a0: defs,
                    0x7ae6a8: put(bytearray(16), 0, '4I', *[holder] * 4), 0x790fec: link.to_bytes(4, 'little'),
                    0x7a0314: bytes(4), 0x7a0374: bytes(4), 0x791ce8: r.bytes(0x10), 0x791050: r.bytes(8), HEAP: heap(0x10000), **global_heap()})
    return desc, flag2


def flags(r, spare):
    f = r.getrandbits(10) & ~3 | (2 if spare else 0) | r.choice([0, 0, 1])
    return f if r.random() < 0.8 else f | r.getrandbits(32) & ~0x3ff


def insert_case(r):
    c = insertcar.case(r)
    regions, log = c.regions, c.regions[LOG]
    owner = bytes(regions[insertcar.OWNER])
    desc, _ = link_world(r, RAM + 0x4000, regions, owner)
    rides = RAM + 0x5000
    rd = bytearray(r.bytes(0x100 * NR))
    for k in range(NR):
        rd[0x100 * k:0x100 * k + 0x20] = regions[insertcar.RIDES][0x20 + 0x20 * k:0x40 + 0x20 * k]
        put(rd, 0x100 * k + 4, 'I', struct.unpack_from('<I', rd, 0x100 * k + 4)[0] & ~0x40000)
        for b in BANKS:
            rd[0x100 * k + (b + 2) * 0xc] &= ~3
    regions[rides] = rd
    regions[0x7a4610] = struct.pack('<3I', *[rides + 0x100 * k for k in range(NR)])
    train = regions[insertcar.TRAIN]
    train[4] = train[4] & ~1 | (1 if r.random() < 0.25 else 0)
    put(train, 0xc, 'II', r.randrange(-5, 300) & 0xffffffff, r.randrange(-5, 300) & 0xffffffff)
    set_return(log, 30, r.randrange(NR))
    height, slot = r.choice([c.args[4]] * 9 + [0x7f800001 | r.getrandbits(22)]), c.args[3]   # (a signalling NaN now and then)
    return Call(0x434060, [insertcar.TRAIN, r.getrandbits(32), r.getrandbits(32), height, flags(r, False), desc, r.choice([0, 1]), slot,
                           r.randrange(1, NR)], regions, ret='a')


def spare_case(r):
    regions, nlayers, blocks = animator.world(r)
    log = log_region()
    regions[LOG] = log
    anim = regions[animator.ANIM]
    regions[animator.LAYERS] = regions[animator.LAYERS] + regions[animator.LAYERS][0x38 * r.randrange(4):][:0x38]
    for k in range(5):   # (each layer with a block and the banks played two entries with blocks: animator.py's null ones fault in both)
        put(regions[animator.LAYERS], 0x38 * k + 0x34, 'I', r.choice(blocks))
    tables = RAM + 0xbc00
    regions[tables] = struct.pack('<16I', *[v for _ in range(8) for v in (0, r.choice(blocks))])
    for i, b in enumerate(sorted(set(BANKS))):
        put(anim, 0x18 + 12 * b + 4, 'II', 2, tables + 0x10 * i)
    put(anim, 4, 'I', struct.unpack_from('<I', anim, 4)[0] & ~0x40000)
    base = RAM + 0xb000
    desc, _ = link_world(r, base, regions)
    train = base + 0x800
    tr = bytearray(r.bytes(0x40))
    tr[4] &= ~1
    put(tr, 0x18, 'I', r.choice([0, base + 0x400]))
    regions[train] = tr
    regions[0x7a4610] = struct.pack('<3I', *[animator.ANIM] * NR)
    set_return(log, 30, r.randrange(NR))
    return Call(0x434060, [train, r.getrandbits(32), r.getrandbits(32), r.f(), flags(r, True), desc, r.choice([0, 1]), r.randrange(4),
                           r.randrange(1, NR)], regions, ret='a')


OPS_ALL = ['Insert', 'Insert', 'Spare']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


def case(r):
    return insert_case(r) if r.choice(OPS) == 'Insert' else spare_case(r)


if __name__ == '__main__':
    B.main(case)
