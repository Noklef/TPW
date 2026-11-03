# A coaster's cars and pieces posed (src/game/coaster/carpose: 0x430ed0) against the original, with its vector helpers, TrackOrientation_Init, CoasterCar_SwingRate and Coaster_SpeedAfterDrop.
# Recorders (driver.cpp's kCoasterPoseEnv): TrackPiece_Handles 1, Car_SetChannelRates 2, Animator_Update 3, TrackPiece_NodeHandles 4. World: 0-2 groups of 0-3 cars, 0-3 pieces of 0-3 nodes, a shared bone table.
# usage: carpose.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put  # noqa: E402

HOOKS = {0x431660: _recorder(1, 3, cdecl=True), 0x430ca0: _recorder(2, 2, cdecl=True), 0x473c70: _recorder(3, 3, cdecl=True), 0x4321c0: _recorder(4, 2, cdecl=True)}
B = Bench([(0x430ed0, 0x431660), (0x43bfb0, 0x43c010), (0x43c160, 0x43c200), (0x43e540, 0x43e6c0), (0x46f150, 0x4709a0), (0x67a400, 0x690000),
           (0x5da3c0, 0x5da3c1)], hooks=HOOKS, sines=256)
COASTER, MODE, RULES, ANIM, H1, H2, ZS, BONES, KINDS, TYPES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x380, RAM + 0x400, RAM + 0x500, RAM + 0x800, RAM + 0xa00
GROUPS, CARS, PIECES, NODES = RAM + 0x1000, RAM + 0x1200, RAM + 0x2000, RAM + 0x3000
TABLE = 0x7a4610


def f(r, nan):
    if r.random() < nan:
        return r.choice([0x7fc00000, 0xffc00000, 0x7fa00000, 0x7f800000, 0xff800000])
    return fb(r.choice([0.0, 1.0, -1.0, r.uniform(-1, 1), r.uniform(-8, 8), r.uniform(-8, 8)]))


def floats(r, nan, n):
    return struct.pack('<%dI' % n, *[f(r, nan) for _ in range(n)])


def body(r, nan, size):
    blob = bytearray(r.bytes(size))
    for at in (0x34, 0x38, 0x3c, 0x4c, 0x50, 0x54, 0x94, 0xd8, 0xdc, 0xe0, 0xe8):
        put(blob, at, 'I', f(r, nan))
    return blob


def case(r):
    nan = r.deal([0, 0, 0, 0, 0.03])
    regions = {0xfb1330: b'\1\0\0\0'}   # (the CRT's flag: acos past 1 returns its NaN without the math error path, whose start-up tables a bench does not have)
    log = log_region()
    regions[LOG] = log
    kinds, types = bytearray(r.bytes(0x200)), bytearray(r.bytes(0x400))
    for k in range(4):
        kind, ty = KINDS + 0x80 * k, TYPES + 0x100 * k
        put(types, 0x100 * k + 0x2c, 'I', 0xffffffff if r.random() < 0.15 else r.getrandbits(31))
        put(types, 0x100 * k + 0xbc, 'III', f(r, nan), f(r, nan), f(r, nan))
        put(kinds, 0x80 * k + 4, 'B', r.choice([0, 0, 0, 1]) | r.getrandbits(8) & 0xfe)
        put(kinds, 0x80 * k + 0x18, 'I', ty)
        kinds[0x80 * k + 0x1c:0x80 * k + 0x28] = floats(r, nan, 3)
    # Cars of the groups.
    ngroups = r.choice([0, 1, 1, 2])
    groups = bytearray(0x100)
    cars = bytearray()
    ncars = 0
    for g in range(ngroups):
        count = r.choice([0, 1, 2, 3])
        put(groups, 0x80 * g + 0x28, 'I', GROUPS + 0x80 * (g + 1) if g + 1 < ngroups else 0)
        put(groups, 0x80 * g + 0x30, 'I', count)
        for k in range(count):
            put(groups, 0x80 * g + 0x34 + 4 * k, 'I', CARS + 0x110 * ncars)
            car = body(r, nan, 0x110)
            put(car, 8, 'I', r.getrandbits(32) | 0x2000 if r.random() < 0.15 else r.getrandbits(32) & ~0x2000)
            put(car, 4, 'I', r.choice([0, 0, 1, 2]))
            put(car, 0x100, 'I', KINDS + 0x80 * r.randrange(4))
            cars += car
            ncars += 1
    for k in range(4):
        for seat in range(3):
            put(kinds, 0x80 * k + 0x30 + 4 * seat, 'I', CARS + 0x110 * r.randrange(ncars) if ncars else MODE)
    regions[KINDS], regions[TYPES], regions[GROUPS] = kinds, types, groups
    if ncars:
        regions[CARS] = cars
    # The bones: an animator's chain to a table of 4 of them (a turn writes into them).
    regions[ANIM] = put(bytearray(r.bytes(0x10)), 8, 'I', H1)
    regions[H1] = put(bytearray(r.bytes(0x40)), 0x28, 'I', H2)
    regions[H2] = put(bytearray(r.bytes(0x10)), 4, 'I', ZS)
    zs, bones = bytearray(r.bytes(0x60)), bytearray()
    for k in range(4):
        put(zs, 20 * k + 4, 'I', BONES + 0x40 * k)
        bones += floats(r, nan, 16)
    regions[ZS], regions[BONES] = zs, bones
    regions[TABLE] = struct.pack('<8I', *[ANIM] * 8)
    # The pieces and their nodes.
    npieces = r.choice([0, 1, 2, 2, 3])
    pieces = bytearray()
    nodes = bytearray()
    for p in range(npieces):
        piece = body(r, nan, 0x110)
        at = PIECES + 0x110 * p
        flags = r.getrandbits(32)
        for bit, chance in ((1, 0.3), (2, 0.3), (4, 0.2), (0x40, 0.5), (0x100, 0.2), (0x2000, 0.2), (0x10000000, 0.3)):
            flags = flags | bit if r.random() < chance else flags & ~bit
        put(piece, 8, 'I', flags)
        put(piece, 0x64, 'I', at + 0x110 if p + 1 < npieces else 0)
        put(piece, 0x68, 'I', at - 0x110 if p and r.random() < 0.7 else at)
        put(piece, 0x70, 'I', r.randrange(8))
        put(piece, 0x100, 'I', KINDS + 0x80 * r.randrange(4))
        count = r.choice([0, 1, 2, 3])
        n = len(nodes)
        list_at = NODES + n
        lists = bytearray(0x10)
        put(lists, 0, 'III', count, list_at + 0x10, list_at + 0x10 + 0x58 * 4)
        node = bytearray()
        for k in range(8):
            one = bytearray(r.bytes(0x58))
            put(one, 4, 'I', r.randrange(4))
            node += one
        nodes += lists + node
        put(piece, 0x90, 'I', list_at)
        pieces += piece
    if npieces:
        regions[PIECES] = pieces
        regions[NODES] = nodes
    put_list = bytearray(r.bytes(0x20))
    put(put_list, 0x10, 'I', GROUPS if ngroups else 0)
    put(put_list, 0x18, 'I', PIECES if npieces else 0)
    put(put_list, 4, 'I', MODE)   # (the coaster also serves CoasterCar_SwingRate's type through the mode block)
    regions[COASTER] = put_list
    rules = bytearray(r.bytes(0x60))
    put(rules, 0, 'I', r.getrandbits(32) | 0x10 if r.random() < 0.6 else r.getrandbits(32) & ~0x10)
    for at in (0x24, 0x28, 0x2c):
        put(rules, at, 'I', f(r, nan))
    put(rules, 0x48, 'I', f(r, nan))
    regions[RULES] = rules
    mode = bytearray(r.bytes(0x40))
    put(mode, 4, 'I', RULES)
    put(mode, 0x10, 'I', f(r, nan))
    regions[MODE] = mode
    regions[0x790fe4] = struct.pack('<I', COASTER)
    regions[0x792218] = struct.pack('<I', MODE)
    regions[0x792220] = r.bytes(4)
    return Call(0x430ed0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
