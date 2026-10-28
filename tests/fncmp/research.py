# The research lab's group and project queries (src/game/world/research: 0x503850, 0x5039f0,
# 0x503b30, 0x504040, 0x504750) against the originals: current projects none or any, categories
# matching or 5, mResearchDone about mCost (equal, NaNs, infinities), project counts 0 to 0x100 and
# negative. Next (0x504580) and Fill (0x5031a0): see defs() below; Fill's project counts also past 0x100, where the projects run into
# the groups and mNumProjects. AllDone (0x4ca6b0): the lab (through the table at 0x7cfb90) with none to a few projects, done or not,
# then defs()'s definitions and records. Flag (0x503ff0): defs()'s records, the lab's 0-12 projects (or a negative count) with ids mostly
# among them. Choose / ChooseBack (0x5034f0 / 0x5036a0): defs()'s definitions (their +0x284 ids none or known ones) and records (their
# +0x10 bytes 0 or set), the global save info's keys (a red-black tree of stdtree.py's of 1-8 of the ids 0-11, the instance [0xf7c58c]),
# the group 4 mostly, 0-12 projects mostly (or up to 0x100, or a negative count), their categories mostly the group's, their ids mostly
# ones with a definition and a record, their research groups and the group's highest about each other, its current project none, among
# them or past them. Cat0-Cat3 (0x4ca630-0x4ca690): the lab as AllDone's, its projects' categories 0-5 mostly. Left (0x503ac0): the
# dearest project's index +0x1464 0, among the projects or the current one, the current one's mCost about 0.1 and its mResearchDone.
# Levels (0x503b80): the categories 0-7, 0-12 projects mostly (or up to 0x100, or a negative count) of the category mostly, mostly done,
# their research groups 0-8 about the group's highest (0-8 or 0xff), the thresholds at 0x785540 0-101 or any; the rest advisor.py's PostId
# set-up (the world [0x80239c] its time's, the advisor the table entry of its +0x1da71c), the messages 0x85-0x88's records sometimes
# not yet posted; the stack message's unwritten bytes masked in the slots and the copy at +0x16d0 (as headquarters.py).
# Efforts (0x503330): see efforts_case(). Init (0x502d60): see init_case(). FlagOn / FlagOff / Sync (0x504090 / 0x504140 / 0x5041f0): the
# flag +0x1468 0, 1 or any, the game object's word +0x1da742 0 or not; the message centre [0x788d3c] made, agent B's unported 0x40fb10 a
# thiscall recorder 21 (driver.cpp's kObjectEnv) logging the message's 12 bytes. Message (0x503d40): see message_case(). Progress
# (0x504630): see progress_case().
# usage: research.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Effort,ObjId,Count,NotDone,Ctor,Next,Fill,AllDone,Flag,Choose,ChooseBack,Cat0,Cat1,Cat2,Cat3,Left,Levels,Efforts,Init,FlagOn,FlagOff,Sync,Message)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdtree  # noqa: E402
from advisor import ADVISOR, NOW, queue  # noqa: E402
from thing_ctors import CENTRE_RANGES, centre  # noqa: E402
from thingenv import park  # noqa: E402
from fncmp import HEAP, LOG, LOG_BYTES, Bench, Call, Mask, RAM, _cdecl_log, _recorder, blob_bytes, fb, heap, log_region, put, set_return, vtable  # noqa: E402



def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


MESSAGE_RANGES = [(0x409cd0, 0x409cf0), (0x40f5e0, 0x40f610), (0x40ff80, 0x4104c0), (0x410610, 0x410730), (0x412e60, 0x412f00),
                  (0x4d3d10, 0x4d3d90), (0x4e1910, 0x4e1930), (0x503b80, 0x503ff0), (0x504580, 0x504640), (0x50b350, 0x50b370), (0x516b00, 0x516c10),
                  (0x5194d0, 0x519520), (0x550ca0, 0x550d90), (0x59a940, 0x59aa80), (0x59abc0, 0x59ae30), (0x59b590, 0x59b630), (0x59bea0, 0x59c0b0),
                  (0x59c500, 0x59c580), (0x5aef50, 0x5af0f0), (0x5b0c40, 0x5b0cc0), (0x5b1680, 0x5b16c0), (0x5b2480, 0x5b2540), (0x5b2fd0, 0x5b3000),
                  (0x5b3110, 0x5b3140)]   # (Message's: tools/ranges.py 503d40's but the hooked)
INIT_RANGES = [(0x502d60, 0x502dc0), (0x519510, 0x51954d), (0x4c9960, 0x4c9a20), (0x401030, 0x401260), (0x401650, 0x401710), (0x516130, 0x5161c0)]
B = Bench(merged(INIT_RANGES + CENTRE_RANGES + [(0x503850, 0x503861), (0x5039f0, 0x503a20), (0x503b30, 0x503b79), (0x504040, 0x504081),
           (0x504750, 0x50475e), (0x504580, 0x504627), (0x504630, 0x50473e), (0x40f580, 0x40f5a2), (0x5031a0, 0x503325), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f),
           (0x4d3d10, 0x4d3d80), (0x5da3c0, 0x5da3c1), (0x4ca6b0, 0x4ca733), (0x4d3d80, 0x4d3df0), (0x519550, 0x51958d), (0x503ff0, 0x504037),
           (0x5034f0, 0x503693), (0x5036a0, 0x503844), (0x412e90, 0x412eea), (0x4d4b70, 0x4d4b84), (0x5af780, 0x5af7f0), (0x410ac0, 0x410b20),
           (0x47d460, 0x47d4b0), (0x5b30b0, 0x5b3110), (0x409410, 0x40947a), (0x4ca630, 0x4ca6a5), (0x503ac0, 0x503b26),
           (0x503b80, 0x503d3c), (0x503330, 0x503426), (0x4e1910, 0x4e1930), (0x5194d0, 0x51950d), (0x409cd0, 0x409cdd), (0x59b590, 0x59b611), (0x59a940, 0x59aa63),
           (0x59abc0, 0x59ae1c), (0x59c2b0, 0x59c680), (0x59bf20, 0x59c093), (0x550ca0, 0x550d90), (0x5d1750, 0x5d1751), (0x41a980, 0x41a986),
           (0x41a990, 0x41a9a7), (0x5997b0, 0x5997f8), (0x429fe0, 0x42a010), (0x44b2e0, 0x44b3d0), (0x474070, 0x4740e0), (0x5442e0, 0x544340),
           (0x5989c0, 0x598ae0), (0x5ed580, 0x5ed5b0), (0x67a400, 0x6902c0), (0x504090, 0x504132), (0x504140, 0x5041e2), (0x5041f0, 0x504209),
           (0x40f630, 0x40f650), (0x415e00, 0x415e80)] + MESSAGE_RANGES),
          hooks={0x40fb10: _recorder(21, 1), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67a7d0: _cdecl_log,
                 0x4db090: _recorder(11, 4), 0x481b60: _recorder(12, 1)})   # (as tree_finds.py's; the last two as endthing.py's)
LAB = RAM
CALLS = {'Effort': 0x503850, 'ObjId': 0x5039f0, 'Count': 0x503b30, 'NotDone': 0x504040, 'Ctor': 0x504750, 'Next': 0x504580, 'Fill': 0x5031a0, 'AllDone': 0x4ca6b0,
         'Flag': 0x503ff0, 'Choose': 0x5034f0, 'ChooseBack': 0x5036a0, 'Cat0': 0x4ca630, 'Cat1': 0x4ca650, 'Cat2': 0x4ca670, 'Cat3': 0x4ca690, 'Left': 0x503ac0,
         'Levels': 0x503b80, 'Efforts': 0x503330, 'Init': 0x502d60, 'FlagOn': 0x504090, 'FlagOff': 0x504140, 'Sync': 0x5041f0,
         'Message': 0x503d40, 'Progress': 0x504630}
TABLE, ITEMS, DEFS = RAM + 0x2000, RAM + 0x4000, 0x7890a0   # (the game's object control table, the object definitions and their list)
IDS = [1, 2, 3, 4, 5, 9]
SAVE, THEAD, TNIL, TNODES = RAM + 0x9000, RAM + 0x9100, RAM + 0x9200, RAM + 0xa000   # (the global save info and its tree of keys)
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


def save_keys(r, regions):
    """the global save info [0xf7c58c] whose key tree (+0x28) holds 1-8 of the ids 0-11"""
    n = r.choice([1, 2, 3, 5, 8])
    nodes, root = stdtree.build(r, n)
    keys = sorted(nodes)
    ids = dict(zip(keys, sorted(r.sample(range(12), n))))   # (the order kept)
    addr = {k: TNODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: TNIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * n)
    for k in keys:
        base = addr[k] - TNODES
        put(blob, base, 'III', ptr(nodes[k][0]), THEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        put(blob, base + 0xc, 'II', ids[k], 0)
        put(blob, base + 0x10, 'I', nodes[k][3])
    regions.update({THEAD: struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) + bytes(0x20), TNIL: put(bytearray(0x40), 0x10, 'I', 1),
                    TNODES: blob, 0xf7c970: struct.pack('<I', TNIL), SAVE: put(bytearray(r.bytes(0x40)), 0x28, 'II', 0, THEAD),
                    0xf7c58c: struct.pack('<I', SAVE)})


def choose_case(r, op, lab):
    """Choose / ChooseBack (the module comment)"""
    regions = {LAB: lab}
    defs(r, regions)
    lst, items, table = regions[DEFS], regions[ITEMS], regions[TABLE]
    have = [struct.unpack_from('<H', items, 0x600 * i)[0] for i in range(struct.unpack_from('<i', lst, 0x528)[0])]
    known = [struct.unpack_from('<H', table, 0x20 * k)[0] for k in range(struct.unpack_from('<i', table, 0x12c0)[0])]
    good = [i for i in have if i in known] or IDS
    for i in range(6):
        put(items, 0x600 * i + 0x284, 'H', r.choice([0, 0, 0, r.choice(good)]))
        table[0x20 * i + 0x10] = r.choice([0, 1, 1, 0xff])
    save_keys(r, regions)
    group = r.choice([4, 4, 4, 0, 1, 2, 3])
    n = r.choice([0, 1, 2, 3, 5, 8, 12, 12, r.randrange(0x101), -1, -5])
    for i in range(max(0, min(n, 0x100))):
        p = 0xc + i * 20
        put(lab, p, 'H', r.choice(good) if r.random() < 0.97 else r.choice(IDS))
        put(lab, p + 4, 'I', group if r.random() < 0.7 else r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
        lab[p + 8] = r.choice([0, 1, 1, 2, 3, 0xff])
    put(lab, 0x145c, 'i', n)
    g = 0x140c + group * 16
    put(lab, g + 8, 'i', r.choice([-1, -1, 0, 1, r.randrange(max(n, 1)), r.randrange(max(n, 1)), n, r.randrange(0x100)]))
    lab[g + 0xc] = r.choice([0, 1, 2, 3, 3, 0xff])
    return Call(CALLS[op], [LAB, group], regions, conv='t')


def levels_case(r, lab):
    c = queue(r, 'PostId')
    regions = c.regions
    for code in (0x85, 0x86, 0x87, 0x88, 0x89):   # (0x89: the record a category 4 would post, were it not refused)
        if r.random() < 0.6:
            put(regions[0x76e300], 0x38 * code + 4, 'I', code)
        if r.random() < 0.6:
            put(regions[ADVISOR], 0xe0 + 0x10 * code, 'I', 0)
    for k in [k for k in regions if 0x7cfb90 <= k < 0x7cfb90 + 0x1000]:
        del regions[k]
    h = r.randrange(8)
    regions[0x7cfb90] = put(bytearray(r.bytes(20 * 8)), 20 * h, 'I', ADVISOR)
    world = bytearray(r.bytes(0x40))
    world[0:4] = regions[NOW][0:4]
    regions[NOW] = put(world, 0x10, 'H', h)
    cat = r.choice([0, 1, 2, 3, 0, 1, 2, 3, 4, 4, 4, 5, 6, 7])
    thresholds = bytearray(r.bytes(0x400))
    for k in range(9):
        put(thresholds, 4 * k, 'I', r.choice([0, 1, 10, 25, 50, 50, 75, 99, 100, 100, 101, r.getrandbits(32)]))
    regions[0x785540] = thresholds
    lab[0x1418 + cat * 16] = r.choice([0, 0, 1, 2, 3, 5, 6, 6, 7, 7, 8, 0xff, r.getrandbits(8)])
    n = r.choice([0, 1, 2, 3, 5, 8, 12, 12, 12, r.randrange(0x101), -1, -5])
    done = r.random()
    for i in range(max(0, min(n, 0x100))):
        p = 0xc + i * 20
        put(lab, p + 4, 'I', cat if r.random() < 0.7 else r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
        lab[p + 8] = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 0xff])
        if r.random() < done:
            put(lab, p + 0xc, 'ff', 1.0, r.choice([1.0, 1.0, 2.0, 1e30]))
    put(lab, 0x145c, 'i', n)
    regions[LAB] = lab
    offsets = sorted([0x16d0 + k for k in (2, 3, 0x17)] + [0x14 + 0x18 * s + k for s in range(8) for k in (2, 3, 0x17)])
    mask = Mask(ADVISOR, offsets, "the stack message's bytes +2, +3, +0x17, never written (0x503b80), copied into a slot or +0x16d0")
    return Call(0x503b80, [LAB, cat], regions, conv='t', masks=[mask])


def efforts_case(r, lab):
    """Efforts (0x503330): Levels' set-up, then the settings at 0x785568 about 100, the projects' categories 0-4 (mostly) and research
    groups and each group's highest about each other, mostly done, the groups' current projects and mWorkLoad any."""
    call = levels_case(r, lab)
    regions = call.regions
    for k in range(5):
        put(regions[0x785540], 0x28 + 4 * k, 'I', r.choice([0, 1, 7, 50, 99, 100, 101, 0x80000000, r.getrandbits(32)]))
        lab[0x1418 + k * 16] = r.choice([0, 1, 2, 3, 5, 6, 7, 8, 0xff])
    n = struct.unpack_from('<i', lab, 0x145c)[0]
    done = r.random()
    for i in range(max(0, min(n, 0x100))):
        p = 0xc + i * 20
        put(lab, p + 4, 'I', r.choice([0, 1, 2, 3, 4, 4, r.choice([5, r.getrandbits(32)])]) if r.random() < 0.6 else
            struct.unpack_from('<I', lab, p + 4)[0])
        if r.random() < done:
            put(lab, p + 0xc, 'ff', 1.0, r.choice([1.0, 2.0, float('nan')]))
        elif r.random() < 0.5:
            put(lab, p + 0xc, 'ff', r.choice([2.0, float('nan')]), r.choice([1.0, float('nan')]))
    put(lab, 0x1460, 'I', r.choice([0, 1, 100, 250, 0x10000, r.getrandbits(32)]))
    regions[LAB] = lab
    return Call(0x503330, [LAB], regions, conv='t', masks=call.masks)


def init_case(r, lab):
    """Init (0x502d60): defs()'s definitions and table (its word +0x12e4, the game object's +0x1da720, one of the slots), thingenv's things
    (0-6 of the kinds 8, 4, 7, 3 or 1, their +3 0 mostly) for the staff count, the settings at 0x785568 (the efforts) and 0x785580 (the
    output) about their limits, the level thresholds at 0x785540 all above 100 (no level rises: ApplyEfforts posts nothing; Levels and
    Efforts cover that), the groups' highest levels any, and thing_ctors.py's message centre (made, its maps 0-4 keys; or not yet) over
    the heap."""
    regions = {LAB: lab}
    things, _, _ = park(r, kinds=(8, 8, 8, 4, 7, 3, 1), defs=False, pool=RAM + 0x30000)
    regions.update(things)
    defs(r, regions)   # (after park: [0x80239c] the table's)
    put(regions[TABLE], 0x1da720 - 0x1d943c, 'H', r.randrange(12))
    settings = bytearray(r.bytes(0x400))
    for k in range(0x100):
        put(settings, 4 * k, 'I', r.choice([101, 200, 0x7fffffff, 0xffffffff]))
    for k in range(5):
        put(settings, 0x28 + 4 * k, 'I', r.choice([0, 1, 7, 50, 99, 100, 101, 0x80000000, r.getrandbits(32)]))
    put(settings, 0x40, 'I', r.choice([0, 1, 100, 250, 0x10000, r.getrandbits(32)]))
    regions[0x785540] = settings
    for g in range(5):
        lab[0x1418 + g * 16] = r.choice([0, 1, 2, 3, 5, 7, 8, 0xff])
    centre(r, regions)
    regions[HEAP] = heap(0x2000)   # (the centre's node, or the centre itself and its node)
    return Call(0x502d60, [LAB], regions, conv='t')


def progress_case(r):
    """Progress (0x504630): a project of defs()'s table's ids mostly (one missing faults in both), mCost 0-1000 (or NaN, an infinity),
    mResearchDone about it, the amount about the gap (or NaN, negative); the message centre made, 0x40fb10 recorder 21 logging the
    message's bytes but its unwritten +0xa / +0xb."""
    regions = {}
    defs(r, regions)
    table = regions[TABLE]
    known = [struct.unpack_from('<H', table, 0x20 * k)[0] for k in range(struct.unpack_from('<i', table, 0x12c0)[0])]
    project = bytearray(r.bytes(20))
    put(project, 0, 'H', r.choice(known) if r.random() < 0.97 else r.choice(IDS))
    cost = r.choice([0.0, 1.0, 10.0, 250.0, 1000.0, r.uniform(0, 1000), float('nan'), float('inf')])
    done = r.choice([0.0, cost, cost - 1.0, cost * 0.5, cost + 1.0, r.uniform(0, 1000), float('nan')])
    gap = cost - done
    amount = r.choice([0.0, gap, gap, gap + 0.5, gap - 0.5, gap * 0.5, 1.0, -1.0, float('nan'), r.uniform(-10, 1000)])
    cost, done, amount = fb(cost), fb(done), fb(amount)
    if r.random() < 0.15:   # (past the float range in the register, against an infinite cost)
        cost, done, amount = r.choice([fb(float('inf')), fb(3e38)]), fb(3e38), fb(3e38)
    nan = lambda: r.choice([0x7fc00000, 0xffc00000, 0x7f800001, 0x7fa00000, 0xff812345, 0x7fffffff])  # noqa: E731
    if r.random() < 0.1:   # (two NaNs: the x87 keeps the larger significand)
        done, amount = nan(), nan()
    put(project, 0xc, 'II', cost, done)
    log = log_region()
    blob_bytes(log, 21, 0, 0xf3ff)
    regions.update({LAB: project, 0x788d3c: struct.pack('<I', RAM + 0x3800), LOG: log})
    return Call(0x504630, [LAB, amount], regions, conv='t')


def flag_case(r, op):
    lab = bytearray(r.bytes(0x1480))
    lab[0x1468] = r.choice([0, 1, 0, 1, r.getrandbits(8)])
    world = put(bytearray(r.bytes(4)), 2, 'H', r.choice([0, 0, 1, 0x100, r.getrandbits(16)]))
    log = log_region()
    blob_bytes(log, 21, 0, 0xfff)
    regions = {LAB: lab, RAM + 0x3000: world, 0x80239c: struct.pack('<I', RAM + 0x3000 + 2 - 0x1da742), 0x788d3c: struct.pack('<I', RAM + 0x3800), LOG: log}
    return Call(CALLS[op], [LAB], regions, conv='t', ret='v')


MSG, MSG_VT, CENTRE, OBJECT, NAME = RAM + 0x7000, RAM + 0x7100, RAM + 0x7140, RAM + 0x7200, RAM + 0x7300
INFO, THEME, GHEAD, GNIL, GNODES, GOBJS = RAM + 0x7400, RAM + 0x7480, RAM + 0x7600, RAM + 0x7640, RAM + 0x7700, RAM + 0x7800
WORLD2 = RAM + 0x40000 - 0x1d943c   # (Message's world: the control table at RAM + 0x40000, the advisor's time and index right after it)


def wide(s):
    return s.encode('utf-16-le') + b'\0\0'


def info_tree(r, regions):
    """the global save info [0xf7c58c]: its current theme +0x48 (or none) with mAllResearchCompleted +0xb9 any; its themes' map (head +0x3c,
    a red-black tree of stdtree.py's of 0-3 nodes on the nil [0xf7c968]) whose mapped objects (+0x20) have +0xb9 set mostly"""
    n = r.choice([0, 1, 2, 3])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: GNODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: GNIL if k is None else addr[k]  # noqa: E731
    blob, objs = bytearray(r.bytes(0x40 * max(n, 1))), bytearray(r.bytes(0x100 * max(n, 1)))
    for i, k in enumerate(keys):
        base = addr[k] - GNODES
        put(blob, base, 'III', ptr(nodes[k][0]), GHEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        put(blob, base + 0x20, 'II', GOBJS + 0x100 * i, nodes[k][3])
        objs[0x100 * i + 0xb9] = r.choice([1, 1, 1, 1, 0, 0x80])
    head = struct.pack('<III', addr[keys[0]] if keys else GHEAD, addr[root] if keys else GNIL, addr[keys[-1]] if keys else GHEAD) + bytes(0x20)
    info = put(bytearray(r.bytes(0x50)), 0x3c, 'I', GHEAD)
    put(info, 0x48, 'I', r.choice([THEME, THEME, 0]))
    regions.update({GHEAD: head, GNIL: put(bytearray(0x40), 0x24, 'I', 1), GNODES: blob, GOBJS: objs, 0xf7c968: struct.pack('<I', GNIL),
                    INFO: info, THEME: r.bytes(0x100), 0xf7c58c: struct.pack('<I', INFO)})


def message_case(r, lab):
    """Message (0x503d40): Levels' set-up (the category the message's +0xc), its world moved to WORLD2 so that defs()' control table lies
    under it too (the definitions at least 1, named at +0x390 "End" (one now and then) or "Shop"); the message's type (its vtable's slot 0,
    recorder k 0) 0x11 mostly, its id one of the definitions' mostly (theirs +0x17c 4 half the time: no next project); the projects all
    done half the time; the message centre made (agent B's unported 0x40fb10 recorder 21, the message's 12 bytes logged); the global save
    info as info_tree() has it; the mode setting 0xfb3b7c 0-3 under its guard 0xfb34ac (read from 0x7a1a8c's bits when clear); the "End"
    object as endthing.py's (0x4db090 recorder 11 answering an object, 0x481b60 recorder 12) over the heap."""
    call = levels_case(r, lab)
    regions, cat = call.regions, call.args[1]
    now = regions.pop(NOW)
    tmp = {}
    defs(r, tmp)
    lst, items, table = tmp[DEFS], tmp[ITEMS], tmp[TABLE]
    n = max(1, struct.unpack_from('<i', lst, 0x528)[0])
    put(lst, 0x528, 'i', n)
    newt = WORLD2 + 0x1d943c
    put(table, 0x12c8, 'I', struct.unpack_from('<I', table, 0x12c8)[0] - TABLE + newt)
    end = r.choice([None, None, r.randrange(n)])
    for i in range(6):
        nm = wide('End' if i == end else 'Shop')
        items[0x600 * i + 0x390:0x600 * i + 0x390 + len(nm)] = nm
    ids = [struct.unpack_from('<H', items, 0x600 * i)[0] for i in range(n)]
    mid = r.choice(ids) if r.random() < 0.95 else r.choice(IDS)
    if r.random() < 0.5:
        for i in range(6):
            if struct.unpack_from('<H', items, 0x600 * i)[0] == mid:
                put(items, 0x600 * i + 0x17c, 'i', 4)
    if r.random() < 0.5:
        for i in range(max(0, min(struct.unpack_from('<i', lab, 0x145c)[0], 0x100))):
            put(lab, 0xc + i * 20 + 0xc, 'ff', 1.0, r.choice([1.0, 2.0]))
    regions.update({DEFS: lst, ITEMS: items, newt: table[:0x12d0], newt + 0x12d0: now, 0x80239c: struct.pack('<I', WORLD2), LAB: lab})
    if r.random() < 0.7:   # (the advisor message 0xa7's record found and not yet posted, as Levels' for its codes)
        put(regions[0x76e300], 0x38 * 0xa7 + 4, 'I', 0xa7)
        put(regions[ADVISOR], 0xe0 + 0x10 * 0xa7, 'I', 0)
    log = regions[LOG]
    set_return(log, 0, r.choice([0x11] * 8 + [0x10, 0x12, 0xe]))
    blob_bytes(log, 21, 0, 0xfff)
    set_return(log, 11, OBJECT)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 3 << 16)   # (the cell and the id: words)
    msg = put(bytearray(r.bytes(0x10)), 0, 'I', MSG_VT)
    put(msg, 8, 'H', mid)
    put(msg, 0xc, 'I', cat)
    info_tree(r, regions)
    wstr = put(bytearray(r.bytes(0x14)), 8, 'I', NAME)
    regions.update({MSG: msg, MSG_VT: vtable({}, 1), 0x788d3c: struct.pack('<I', CENTRE), CENTRE: r.bytes(0x40), OBJECT: r.bytes(0x40),
                    0x7cf9c0: wstr, NAME: wide('End'), 0x7cf4f4: r.bytes(4), 0xfb34ac: bytes([r.choice([1, 1, 0, 3])]),
                    0xfb3b7c: struct.pack('<I', r.choice([0, 2, 0, 2, 1, 3])), HEAP: heap()})
    return Call(0x503d40, [LAB, MSG], regions, conv='t', masks=call.masks)


def case(r):
    op = r.deal(OPS)
    if op in ('FlagOn', 'FlagOff', 'Sync'):
        return flag_case(r, op)
    if op == 'Message':
        return message_case(r, bytearray(r.bytes(0x1800)))
    if op == 'Progress':
        return progress_case(r)
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
    if op == 'Levels':
        return levels_case(r, lab)
    if op == 'Efforts':
        return efforts_case(r, lab)
    if op == 'Init':
        return init_case(r, lab)
    if op in ('Choose', 'ChooseBack'):
        return choose_case(r, op, lab)
    if op in ('Cat0', 'Cat1', 'Cat2', 'Cat3'):   # (the lab as AllDone's: the game object's word +0x1da722 picks its entry at 0x7cfb90)
        k = r.randrange(4)
        regions = {LAB: lab, TABLE: put(bytearray(r.bytes(0x1400)), 0x1da722 - 0x1d943c, 'H', k),
                   0x80239c: struct.pack('<I', TABLE - 0x1d943c), 0x7cfb90: put(bytearray(r.bytes(80)), 20 * k, 'I', LAB)}
        return Call(CALLS[op], [], regions, conv='c', ret='a')
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
    if op == 'Left':
        current = struct.unpack_from('<i', lab, 0x140c + group * 16 + 8)[0]
        put(lab, 0x1464, 'i', r.choice([0, r.randrange(0x100), r.randrange(0x100), max(current, 0)]))
        if current >= 0 and r.random() < 0.3:
            put(lab, 0xc + current * 20 + 0xc, 'f', r.choice([0.1, 0.10000001, 0.0999999, 1.0, 1.0000001, 0.5]))
        return Call(0x503ac0, [LAB, group], {LAB: lab}, conv='t', ret='a')
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
