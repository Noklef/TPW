# The research lab's group and project queries (src/game/world/research: 0x503850, 0x5039f0,
# 0x503b30, 0x504040, 0x504750) against the originals: current projects none or any, categories
# matching or 5, mResearchDone about mCost (equal, NaNs, infinities), project counts 0 to 0x100 and
# negative. Next (0x504580) and Fill (0x5031a0): see defs() below; Fill's project counts also past 0x100, where the projects run into
# the groups and mNumProjects. AllDone (0x4ca6b0): the lab (through the table at 0x7cfb90) with none to a few projects, done or not,
# then defs()'s definitions and records. Flag (0x503ff0): defs()'s records, the lab's 0-12 projects (or a negative count) with ids mostly
# among them.
# usage: research.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Effort,ObjId,Count,NotDone,Ctor,Next,Fill,AllDone,Flag)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x503850, 0x503861), (0x5039f0, 0x503a20), (0x503b30, 0x503b79), (0x504040, 0x504081),
           (0x504750, 0x50475e), (0x504580, 0x504627), (0x5031a0, 0x503325), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f),
           (0x4d3d10, 0x4d3d80), (0x5da3c0, 0x5da3c1), (0x4ca6b0, 0x4ca733), (0x4d3d80, 0x4d3df0), (0x519550, 0x51958d), (0x503ff0, 0x504037)])
LAB = RAM
CALLS = {'Effort': 0x503850, 'ObjId': 0x5039f0, 'Count': 0x503b30, 'NotDone': 0x504040, 'Ctor': 0x504750, 'Next': 0x504580, 'Fill': 0x5031a0, 'AllDone': 0x4ca6b0,
         'Flag': 0x503ff0}
TABLE, ITEMS, DEFS = RAM + 0x2000, RAM + 0x4000, 0x7890a0   # (the game's object control table, the object definitions and their list)
IDS = [1, 2, 3, 4, 5, 9]
FLOATS = [0.0, -0.0, 1.0, 100.0, 1e30, float('inf'), float('-inf'), float('nan')]


def fbits(r):
    if r.random() < 0.1:
        return r.choice([0x7fa00000, 0xffc00001, 0x7f800001])  # signalling and other NaNs
    return struct.unpack('<I', struct.pack('<f', r.choice(FLOATS + [r.uniform(-50, 200)])))[0]


def defs(r, regions):
    """Up to 6 object definitions (ids 1-5 and 9; +0x17c 0-5; costs +0x180 / +0x1c0 / +0x200 none, negative or some; +0x4ac and +0x508
    around their limits) in the list at 0x7890a0, and the game's control table (records for most ids, their +0x14 levels -1 to 3).
    Returns the definitions' addresses."""
    n = r.choice([0, 1, 2, 4, 6])
    lst, items = put(bytearray(r.bytes(0x740)), 0x528, 'i', n), bytearray(r.bytes(6 * 0x600))
    for i in range(6):
        d = 0x600 * i
        put(lst, i * 4, 'I', ITEMS + d)
        put(items, d, 'H', r.choice(IDS))
        put(items, d + 0x17c, 'i', r.choice([0, 1, 2, 3, 4, 4, 5]))
        for off in (0x180, 0x1c0, 0x200):
            put(items, d + off, 'I', r.choice([0, 0, 0xffffffff, 1, 250, 1000, 0x7fffffff, r.getrandbits(32)]))
        put(items, d + 0x4ac, 'i', r.choice([0, 1, 3, 3, 4, 5, -1]))
        put(items, d + 0xc4, 'i', r.choice([0, 0, 1, 5, -1]))
        put(items, d + 0x508, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    table = bytearray(r.bytes(0x1400))
    have = r.choice([6, 6, 6, 4])
    put(table, 0x12c0, 'i', have)
    for k, i in enumerate(r.sample(IDS, 6)[:have]):
        put(table, 0x20 * k, 'H', i)
        put(table, 0x20 * k + 0x14, 'I', r.choice([0xffffffff, 0, 0, 1, 2, 3, r.getrandbits(32)]))
        put(table, 0x20 * k + 0x18, 'i', r.choice([0, 0, 1, -1, 7]))
    put(table, 0x12c4, 'H', r.choice([0, 0, 1, 9]))
    put(table, 0x12c8, 'I', TABLE + 0x20 * r.randrange(have))
    regions.update({DEFS: lst, ITEMS: items, TABLE: table, 0x80239c: struct.pack('<I', TABLE - 0x1d943c)})
    return [ITEMS + 0x600 * i for i in range(6)]


def case(r):
    op = r.choice(OPS)
    lab = bytearray(r.bytes(0x1800))
    for i in range(0x100):
        p = 0xc + i * 20
        put(lab, p + 4, 'I', r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
        cost = fbits(r)
        put(lab, p + 0xc, 'II', cost, r.choice([cost, cost, fbits(r)]) if r.random() < 0.5 else fbits(r))
    for g in range(5):
        put(lab, 0x140c + g * 16 + 8, 'i', r.choice([-1, -2, 0, r.randrange(0x100), r.randrange(0x100)]))
    put(lab, 0x145c, 'i', r.choice([0, 1, 2, r.randrange(0x101), 0x100, -1, -5]))
    group = r.choice([0, 1, 2, 3, 4, r.randrange(5)])
    if op == 'Flag':   # the projects' ids mostly ones the table has (one missing faults in both)
        regions = {LAB: lab}
        defs(r, regions)
        table = regions[TABLE]
        known = [struct.unpack_from('<H', table, 0x20 * k)[0] for k in range(struct.unpack_from('<i', table, 0x12c0)[0])]
        n = r.choice([0, 1, 2, 3, 5, 8, 12, -1, -7])
        for i in range(max(n, 0)):
            put(lab, 0xc + i * 20, 'H', r.choice(known) if r.random() < 0.97 else r.choice(IDS))
        put(lab, 0x145c, 'i', n)
        return Call(0x503ff0, [LAB], regions, conv='t')
    if op in ('Next', 'Fill', 'AllDone'):
        regions = {LAB: lab}
        items = defs(r, regions)
        if op == 'AllDone':   # (the game object's word +0x1da722, in the table's region, picks the lab's entry)
            k = r.randrange(4)
            put(regions[TABLE], 0x1da722 - 0x1d943c, 'H', k)
            regions[0x7cfb90] = put(bytearray(r.bytes(80)), 20 * k, 'I', LAB)
            put(lab, 0x145c, 'i', r.choice([0, 0, 0, 1, 2, 5, -1]))
            return Call(0x4ca6b0, [], regions, conv='c', ret='b')
        if op == 'Next':
            return Call(0x504580, [LAB + 0xc + 20 * r.randrange(0x100), r.choice(items)], regions, conv='t', ret='b')
        n = r.choice([0, 1, 5, r.randrange(0x100), 0xfc, 0xff, 0x100, 0x101, 0x102, 0x103, 0x104])
        put(lab, 0x145c, 'i', n)
        put(lab, 0x1464, 'i', r.choice([-1, -7, 0, max(0, n - 1), r.randrange(n + 2)]))
        for g in range(5):
            put(lab, 0x140c + g * 16, 'I', r.choice([0, 0, 1, 7, 100, 0x3000000, r.getrandbits(32)]))
        put(lab, 0x1460, 'I', r.choice([0, 1, 100, 250, 0x10000, r.getrandbits(32)]))
        return Call(0x5031a0, [LAB], regions, conv='t')
    if op == 'Ctor':
        return Call(0x504750, [LAB + 0x140c + group * 16], {LAB: lab}, conv='t', ret='a')
    if op == 'Count':
        return Call(0x503b30, [LAB, r.choice([0, 1, 2, 3, 4, 5, 5, r.getrandbits(32)])], {LAB: lab}, conv='t', ret='a')
    if op == 'ObjId':
        return Call(0x5039f0, [LAB, RAM + 0x1500 + r.randrange(4), group], {LAB: lab, RAM + 0x1500: r.bytes(8)},
                    conv='t', ret='a')
    return Call(CALLS[op], [LAB, group], {LAB: lab}, conv='t', ret='b' if op == 'NotDone' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
