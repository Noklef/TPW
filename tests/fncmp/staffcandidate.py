# The staff pool's candidates (src/game/world/staffcandidate: StaffPool_NameTaken 0x5083f0, StaffPoolEntry_PickName 0x507580, StaffPoolEntry_Generate
# 0x507600) against the originals. The world's pool: 32 entries of kinds 0-4 mostly, text ids 0-0x22, most used; its seed and tick; a thing list of up to
# 4 things with handles under 16, staff kinds mostly, each with a name. The text formatter 0x6acd60 the log stub, which gives a buffer the hash of the id
# (name() here). Taken: a name that is a used entry's, an unused one's, a thing's, empty or none's. Pick: now and then every id taken, so all 15 tries
# run. Generate: the kind 0-4 mostly, the folders' counts 1 or more, the average grades (so also 0 and 0xff, where the grade's byte wraps), the timeout
# any (1 and 0 divide by zero in both); 0x481550 a thiscall recorder (20 of 1: driver.cpp's StaffCandidateEnv).
# One, All (0x508330 StaffPool_AddCandidate, 0x5087d0 StaffPool_FillAll): the flags +0x298-+0x29d mostly 0, the pool now and then full, the counts [0x7852ac] small,
# 40 (up to full) or -1.
# Start (0x507bb0): All's. TopUp (0x508170): the kinds' dwords +0x280 and limits [0x7852d4] with each kind at, short of or over its limit; kind 4 short only when the pool can
# take all that is short (else both never end: see top_up()).
# Update (0x5084f0 StaffPool_Update): the ages about their limits, TopUp's limits so it cannot hang (see the case).
# usage: staffcandidate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Taken,Pick,Generate,One,All,Start,TopUp,Update)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _cdecl_log4, _recorder, log_region, put, text_args  # noqa: E402

B = Bench([(0x507bb0, 0x507bc5), (0x508170, 0x508330), (0x508330, 0x508378), (0x5087b0, 0x508978), (0x5083f0, 0x5084f0), (0x507580, 0x507600), (0x507600, 0x5077e8), (0x5074a0, 0x507560), (0x516130, 0x5161c0), (0x516330, 0x516380),
           (0x50b350, 0x50b370), (0x541f70, 0x541fa0), (0x41a960, 0x41a980), (0x506860, 0x506870), (0x67c290, 0x67c320), (0x5da3c0, 0x5da3c1), (0x5084f0, 0x5087b0), (0x508000, 0x508170)],
          hooks={0x6acd60: _cdecl_log4, 0x481550: _recorder(20, 1)})
CALLS = {'Taken': 0x5083f0, 'Pick': 0x507580, 'Generate': 0x507600, 'One': 0x508330, 'All': 0x5087d0, 'Start': 0x507bb0, 'TopUp': 0x508170, 'Update': 0x5084f0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
WORLD, ENTRY, NAME, THINGS = RAM + 0x100000, RAM, RAM + 0x100, RAM + 0x1000
THING = 0x300


def name(text_id):
    """What the log stub writes for this text id: 8 hex digits of its hash, wide, and the end"""
    h = ((0x811c9dc5 ^ text_id) * 0x01000193) & 0xffffffff
    return ('%08X' % h).encode('utf-16-le') + bytes(2)


def wide(r):
    return bytes(x for _ in range(r.randrange(1, 12)) for x in (r.randrange(0x41, 0x5b), 0)) + bytes(2)


def world(r, every=False, full=False):
    """The regions of the world, its pool and its things; returns them, the used ids, the unused ids and the things' names. every: all 35 ids taken."""
    pool = bytearray(r.bytes(0x2a0))
    pool[0x298:0x29e] = bytes(r.choice([0, 0, 0, 1]) for _ in range(6))
    ids = list(range(0x20)) if every else [r.randrange(0x23) for _ in range(0x20)]
    for i in range(0x20):
        put(pool, 0x14 * i, 'I', r.randrange(5) if every or r.random() < 0.95 else r.choice([5, 0xff, r.getrandbits(32)]))
        put(pool, 0x14 * i + 4, 'I', ids[i])
        pool[0x14 * i + 0xa] = r.choice([1, 1, 1, 0xff]) if every or full or r.random() < 0.8 else 0
    used = [ids[i] for i in range(0x20) if pool[0x14 * i + 0xa]]
    unused = [ids[i] for i in range(0x20) if not pool[0x14 * i + 0xa]]
    regions = {WORLD: pool, WORLD + 0x1da708: r.bytes(8), 0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD), 0x78baa8: r.bytes(20),
               0x7cee18: r.bytes(0x100), 0x7cef20: r.bytes(0x100)}
    table = bytearray(r.bytes(20 * 16))
    n = 3 if every else r.choice([0, 0, 1, 2, 3, 4])
    handles = r.sample(range(1, 16), n)
    names = []
    for k, h in enumerate(handles):
        thing = bytearray(r.bytes(THING))
        put(thing, 0, 'H', h)
        thing[2] = r.choice([4, 5, 6, 7, 8]) if every or r.random() < 0.8 else r.choice([0, 1, 3, 9, r.getrandbits(8)])
        text = name(0x20 + k) if every else r.choice([name(r.randrange(0x23)), wide(r)])
        thing[0x1a0:0x1a0 + len(text)] = text
        if thing[2] in (4, 5, 6, 7, 8):
            names.append(text)
        regions[THINGS + THING * k] = thing
        put(table, 20 * h, 'III', THINGS + THING * k, h, 0x7cfb90 + 20 * handles[k + 1] if k + 1 < n else 0)
    regions[0x7cfb90] = table
    regions[0x7cf56c] = struct.pack('<I', 0x7cfb90 + 20 * handles[0] if n else 0)
    return regions, used, unused, names


def top_up(r, regions):
    """TopUp's limits. A kind short that cannot be added goes on for ever: so a kind is short only by what the free entries still cover, with its flag clear. At the sign: a sum
    just above it with a limit just under wraps to "not short" (kept); a sum at the bottom with a limit under it would wrap to far short (not made)."""
    pool = regions[WORLD]
    used = [sum(1 for i in range(0x20) if pool[0x14 * i + 0xa] and struct.unpack_from('<I', pool, 0x14 * i)[0] == k) for k in range(5)]
    free = sum(1 for i in range(0x20) if not pool[0x14 * i + 0xa])
    limits = bytearray(20)
    for k in r.sample(range(5), 5):
        base = r.choice([0, 0, 1, 2, 5, 0x7ffffffe, 0x7fffffff, r.getrandbits(32)])
        total = struct.unpack('<i', struct.pack('<I', (base + used[k]) & 0xffffffff))[0]
        wanted = r.choice([1, 1, 2, 3])
        short = wanted if not pool[0x299 + k] and free >= wanted and total + wanted <= 0x7fffffff else r.choice([0, 0, -1, -5, 0x80000000 - total if total > 0 else 0])
        if short > 0:
            free -= wanted if short == wanted else 0
        limit = total + short
        put(pool, 0x280 + 4 * k, 'I', base)
        put(limits, 4 * k, 'I', (limit if limit >= -0x80000000 else total) & 0xffffffff)
    regions[0x7852d4] = limits


def case(r):
    op = r.deal(OPS)
    log = log_region()
    text_args(log, 1, only=(0xfffe, 0xffff))
    if op == 'Taken':
        regions, used, unused, names = world(r)
        pick = r.choice(['used', 'used', 'unused', 'thing', 'thing', 'empty', 'other'])
        text = {'used': name(r.choice(used)) if used else b'\0\0', 'unused': name(r.choice(unused)) if unused else b'\0\0',
                'thing': r.choice(names) if names else b'\0\0', 'empty': b'\0\0', 'other': wide(r)}[pick]
        regions.update({LOG: log, NAME: text + bytes(8)})
        return Call(CALLS[op], [WORLD, NAME], regions, conv='t', ret='b')
    regions, _, _, _ = world(r, every=r.random() < 0.1 and op in ('Pick', 'Generate'), full=op in ('One', 'All', 'TopUp', 'Update') and r.random() < 0.2)
    entry = bytearray(r.bytes(0x14))
    regions.update({LOG: log, ENTRY: entry})
    if op == 'Pick':
        put(entry, 0, 'I', r.randrange(5) if r.random() < 0.9 else r.choice([5, 0xff, r.getrandbits(32)]))
        return Call(CALLS[op], [ENTRY], regions, conv='t')
    folders = bytearray(r.bytes(14 * 12))
    for f in range(14):
        put(folders, 12 * f + 4, 'I', r.choice([1, 2, 7, r.randrange(1, 0x10000)]))
    regions[0x763f8c] = folders
    regions[0x785298] = b''.join(struct.pack('<I', r.choice([0, 1, 2, 3, 4, 5, 6, 0xff, r.getrandbits(32)])) for _ in range(5))
    regions[0x78530c] = struct.pack('<i', r.choice([2, 3, 100, 1000, 3000, 0x7fffffff, -2, -1000, -0x80000000] * 4 + [0, 1, r.getrandbits(31), -r.getrandbits(31) - 2]))
    if op == 'One':
        return Call(CALLS[op], [WORLD, r.randrange(5)], regions, conv='t', ret='b')
    if op in ('All', 'Start'):
        regions[0x7852ac] = b''.join(struct.pack('<i', r.choice([0, 0, 1, 2, 3, 40, -1])) for _ in range(5))
        return Call(CALLS[op], [WORLD], regions, conv='t', ret='b' if op == 'All' else 'v')
    if op == 'Update':   # (the ages about their limits, the balance's wants, small, the clock old mostly; TopUp's limits)
        pool = regions[WORLD]
        now = r.getrandbits(32)
        ticks = bytearray(regions[WORLD + 0x1da708])
        put(ticks, 4, 'I', now)
        regions[WORLD + 0x1da708] = ticks
        for i in range(0x20):
            limit = r.choice([0, 1, 5, 0x40000000, r.getrandbits(8)])
            put(pool, 0x14 * i + 0x10, 'I', limit)
            put(pool, 0x14 * i + 0xc, 'I', (now - limit * 4 - r.choice([0, 1, 1])) & 0xffffffff)
            pool[0x14 * i + 0xb] = r.choice([0, 0, 0, 1])
        pause = r.choice([0, 1, 10])
        regions[0x785304] = struct.pack('<II', pause, r.choice([0, 1, 2, 3, 5, 40, 0xffffffff]))
        put(pool, 0x294, 'I', (now - pause * 4 - r.choice([0, 1, 1, 1, 1])) & 0xffffffff)
        regions[0x7852c0] = b''.join(struct.pack('<i', r.choice([0, 1, 2, 3, 5, 8, 8, -1])) for _ in range(5))
        # (TopUp's limits: a kind short with a flag set or no entry free never ends, in both, and Update's new candidates and Staff_CountTypes change all that top_up() reckons
        # with. So: none short, the flags' limits any; or room for all that can be short, no flags)
        limits, counted = [-0x80000000] * 5, [r.choice([1, 2, 40]) for _ in range(5)]
        if r.random() < 0.3:
            for i in range(10, 0x20):
                pool[0x14 * i + 0xa] = 0
            regions[0x785304] = struct.pack('<II', pause, r.choice([0, 1, 2, 3, 5]))
            limits, counted = [r.choice([-0x80000000, 0, 1, 2]) for _ in range(5)], [40] * 5
        regions[0x7852d4] = struct.pack('<5i', *limits)
        regions[0x7852e8] = struct.pack('<5i', *counted)
        return Call(CALLS[op], [WORLD], regions, conv='t', ret='v', steps=400000)
    if op == 'TopUp':
        top_up(r, regions)
        return Call(CALLS[op], [WORLD], regions, conv='t', ret='v')
    kind = r.choice([0, 1, 2, 3, 4] * 3 + [5, 0xff, 0x100, r.getrandbits(32)])
    return Call(CALLS[op], [ENTRY, kind, r.getrandbits(32)], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
