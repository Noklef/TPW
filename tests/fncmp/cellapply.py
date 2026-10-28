# A map cell's model applied (src/game/world/cellmodel: 0x53afa0 MapCell_ApplyModel) against the original: cellmodel.py's cell and tables
# (the tables' models now of the groups the code tells apart: 0, 3, 4, 5 or another; the index now and then 0 or 0x37), its handle's
# 0x44-byte cell (gMapCells 0x8023a0 set so that it is MAPC; its +8 0x15 or not), the byte 0x81b4cc; the cell's ride (+4) none or one
# (Ride_Destroy 0x463c80 a cdecl recorder, 31, as queuepieces.py's); the placers 0x522b90 / 0x522d50 / 0x522f20 cdecl recorders (20, 21,
# 22 of 6), their results now and then set (0, a ride of the instances' table 0x7a4610: animator.py's animator, or another); the animator
# world animator.py's. The variant counter (0x820aec) stays under 0x1e: past it MapCell_PickModel's variant is the angle's pointer
# (sic), here a stack local, whose address the original and the port do not share.
# Place (0x53b1a0 MapCell_PlaceRide): Apply's world, the cell's group (+0x14) 3, 4, 5 mostly, else 0, 2 or any, its flags (+0xe)
# with 0x10 or not, its +0x18 / +0x1c any; the 0x44-byte cell's +8 0x15 or not.
# usage: cellapply.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Apply,Place)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import animator  # noqa: E402
import cellmodel  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

cellmodel.CELL, cellmodel.OUT = RAM + 0x40000, RAM + 0x41000   # (animator.py's world uses RAM .. RAM + 0xa000)
MAPC = RAM + 0x42000


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = merged([   # (animator.py's)
    (0x46f150, 0x475010), (0x67a400, 0x690000), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172), (0x4031e0, 0x403202),
    (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)]) + [
    (0x53a540, 0x53aa88), (0x522700, 0x5228b5), (0x516330, 0x516370), (0x4d8c00, 0x4d8c1f), (0x4d0b10, 0x4d0b30), (0x522a90, 0x522a9e),
    (0x53afa0, 0x53b19e)]
HOOKS = {0x463c80: _recorder(31, 1, cdecl=True), 0x522b90: _recorder(20, 6, cdecl=True), 0x522d50: _recorder(21, 6, cdecl=True),
         0x522f20: _recorder(22, 6, cdecl=True)}
B = Bench(RANGES + [(0x53b1a0, 0x53b279)], sines=256, hooks=HOOKS)


def apply_case(r, log=None):
    regions, _, _ = animator.world(r)
    regions.update(cellmodel.case(r).regions)
    for table, _, most in cellmodel.TABLES:   # (the models' groups the code tells apart)
        data = bytearray(regions[table])
        for i in range(most):
            group = r.choice([0, 3, 4, 5, 0, 3, 4, 5, 2, 8, r.getrandbits(16)])
            index = r.choice([r.randrange(0x20)] * 3 + [0, 0x37, r.getrandbits(16)])
            if r.random() < 0.97:
                put(data, i * 12, 'I', group << 16 | index)
        regions[table] = data
    # the variant counter under 0x1e (World_Rand % 30 keeps it there): from 0x1e the variant is the angle's pointer (sic), here a local
    counter = r.choice([r.randrange(0x1e), r.randrange(0x1e), 0x11, 0x12, 0x17, 0x18, 0x1d, -1])
    regions[0x820aec] = struct.pack('<i', counter) + regions[0x820aec][4:]
    cell = bytearray(regions[cellmodel.CELL])
    put(cell, 4, 'I', r.choice([0, 0, 5, r.getrandbits(32)]))
    k = r.random()
    if k < 0.3:   # (the default model: kind not positive, or 5)
        put(cell, 8, 'i', r.choice([0, 5, -3, -0x19]))
    elif k < 0.45:   # (the kinds the flag 0x81b4cc leaves)
        put(cell, 8, 'i', r.choice([0xb, 0x10, 0xc, 0x11]))
    handle = struct.unpack_from('<H', cell, 0)[0]
    regions[cellmodel.CELL] = cell
    mapc = bytearray(r.bytes(0x44))
    put(mapc, 8, 'I', r.choice([0x15, 0, 1, 0x14, r.getrandbits(32)]))
    instances = bytearray(r.bytes(0x40))
    for k in range(16):
        put(instances, 4 * k, 'I', animator.ANIM)
    log = log_region() if log is None else log
    for k in (20, 21, 22):
        set_return(log, k, r.choice([0, 1, 2, 15, r.randrange(16)]))
    regions.update({LOG: log, MAPC: mapc, 0x8023a0: struct.pack('<I', (MAPC - (handle - 1) * 0x44) & 0xffffffff), 0x7a4610: instances,
                    0x820ad8: r.bytes(12)})
    return Call(0x53afa0, [cellmodel.CELL], regions, conv='t', ret='v')


def place(r):
    c = apply_case(r)
    cell = bytearray(c.regions[cellmodel.CELL])
    put(cell, 0x14, 'I', r.choice([3, 3, 4, 4, 5, 5, 0, 2, r.getrandbits(32)]))
    cell[0xe] = cell[0xe] & ~0x10 | r.choice([0, 0, 0x10])
    put(cell, 0x18, 'II', r.choice([0, 1, 0x37, r.getrandbits(16)]), r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    c.regions[cellmodel.CELL] = cell
    return Call(0x53b1a0, [cellmodel.CELL], c.regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Apply,Place').split(',') if k]


def case(r):
    return apply_case(r) if r.deal(OPS) == 'Apply' else place(r)


if __name__ == '__main__':
    B.main(case)
