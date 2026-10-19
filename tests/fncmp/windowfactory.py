# The GUI window kinds' constructors (each over Window_Ctor, 0x65bcad) against the originals. Each: windowlife.py's Ctor set-up (a random
# window built with a parent from window.py's tree (its +0x60 chain and word +0xe0 near 0x100) or none, flags 0x400 and 1 set or not, words
# edges or near (upper halves random); the root the new window, the tree's or none, the queues as window.py's), the window's bytes past
# Window_Ctor's random too. Ctor705a20 (0x665fcd) and its SaveRect705a20 (0x665fb0): also the heap (an allocation now and then failing),
# the window timers' pool [0xfaa630] of 0-4 nodes (free or not; the count theirs or 0) and list [0xfaa634] (0-2 nodes or none), the GUI
# clock [0x77c490] a recorder, the focus [0xfaa5ec] the tree's, the new window or none and the byte 0xfaa56c set or not. CreateChild (the
# factory 0x65dd7b): Ctor705a20's set-up, the parent one of the tree's, the kind 1-13 or outside, the flags the bits the kinds read
# (0x10, 0x40, 0x80, 0x100, 0x1000) in their combinations or ctor_args's; an allocation (the first to the fourth) now and then failing,
# which faults in both (sic). Not given: flag 0x1000 to kinds 4 and 7 (their scroll bar's disable calls its handler 0x66ba22, and flag 0
# with 0x80 to kind 3 (its arrows' showing would call theirs, 0x668f9c): game code the driver cannot run natively (both end in the
# unported default handler 0x65f6d1), so those branches are not compared here. Layout705950 / Layout7059f0 (0x65aac0, 0x662530): random
# windows. Ctor705b20 / Ctor705950 / Ctor7059f0 (kinds 3, 4, 7: 0x66acf9, 0x65ab64, 0x662562): the constructors' set-up with CreateChild's
# heap and flags (the same left out). Layout705b20 (0x66ac00): a random window, the bits 2 / 4 of its word +0x14c either way. AddArrow1 /
# AddArrow2 / AddThumb (0x66b557, 0x66b604, 0x66b6b1): the parent one of the tree's (0x180 bytes; +0x14c's bit 2 either way; flag 0 of
# +0x48 clear, as above), CreateChild's heap.
# usage: windowfactory.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor705a50,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES, TREE, VTABLE, WINDOW2, point, queues, tree  # noqa: E402

# the handlers the kinds 2, 3, 4, 5, 7 and the thumb set (0x668f9c, 0x66ba22, 0x65bb6b, 0x66857d, 0x665c35, 0x66bb9b) and what they call
# (tools/ranges.py), for the messages a new window gets at once (0x11 from Window_SetShown, 0x17 from the enables)
HANDLERS = [(0x658ccc, 0x658e04), (0x65968e, 0x6596c0), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65bac2, 0x65bb20), (0x65bb6b, 0x65bc40),
            (0x65c707, 0x65c788), (0x65cc04, 0x65cda5), (0x65cf3d, 0x65cf6e), (0x65d4a0, 0x65d4db), (0x65d55a, 0x65d57b), (0x65d57c, 0x65d59d),
            (0x65d7bc, 0x65d832), (0x65dd18, 0x65dd6f), (0x65e549, 0x65e5ab), (0x65f150, 0x65f17b), (0x65f6d1, 0x65fc00), (0x661770, 0x6617e0),
            (0x665c35, 0x665e90), (0x66857d, 0x6687c0), (0x668abd, 0x668ae3), (0x668bec, 0x668d6f), (0x668dd9, 0x668f45), (0x668f9c, 0x6693b0),
            (0x66af19, 0x66b479), (0x66b771, 0x66bfa0)]
CALLS = {'Ctor705a50': 0x6688b3, 'RadioGroupCtor': 0x66a1e8, 'Ctor705c58': 0x66d9e0, 'Ctor705b98': 0x66c7e0,
         'Ctor705c28': 0x66d750, 'Ctor705bf8': 0x66d0c0, 'Ctor705bc8': 0x66c970, 'Ctor705af0': 0x66a4c0, 'Ctor705a20': 0x665fcd,
         'SaveRect705a20': 0x665fb0, 'CreateChild': 0x65dd7b, 'Layout705950': 0x65aac0, 'Layout7059f0': 0x662530,
         'Ctor705b20': 0x66acf9, 'Ctor705950': 0x65ab64, 'Ctor7059f0': 0x662562, 'Layout705b20': 0x66ac00, 'AddArrow1': 0x66b557,
         'AddArrow2': 0x66b604, 'AddThumb': 0x66b6b1}
B = Bench(RANGES + [(0x6688b3, 0x6689bc), (0x66a1e8, 0x66a255), (0x66d9e0, 0x66da21), (0x66c7e0, 0x66c82e), (0x65e526, 0x65e549),
                       (0x66d750, 0x66d82e), (0x66d0c0, 0x66d1b6), (0x66c970, 0x66cab7), (0x66a4c0, 0x66a5a8), (0x665fb0, 0x665fcd),
                       (0x665fcd, 0x6661bc), (0x661ece, 0x661f2b), (0x661f2b, 0x661f9b), (0x66220d, 0x6622d2), (0x65968e, 0x6596a2),
                       (0x65dd7b, 0x65e4f2), (0x65aac0, 0x65ab64), (0x662530, 0x662562), (0x66acf9, 0x66ae57), (0x65ab64, 0x65acce),
                       (0x662562, 0x66274e), (0x66ac00, 0x66acf9), (0x66b557, 0x66b771), (0x66b47d, 0x66b508), (0x66ae83, 0x66af19),
                       (0x65da8d, 0x65db25), (0x668a23, 0x668aad), (0x668820, 0x6688b3), (0x65d56b, 0x65d57c)] + HANDLERS)
POOL, TLIST, CLOCK, CLOCKVT = RAM + 0x9000, RAM + 0x9800, RAM + 0x9c00, RAM + 0x9d00


def ctor_args(r, regions):
    """windowlife.py's Ctor set-up for a window at WINDOW2 of 0x180 random bytes; returns the arguments after `this`"""
    wins = tree(r, regions)
    for i, w in enumerate(wins):
        put(regions[w], 0x60, 'I', r.choice([wins[i - 1], 0]) if i else 0)
        put(regions[w], 0xe0, 'H', r.choice([0, 1, 0xfe, 0xff, 0x100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
    regions[WINDOW2] = r.bytes(0x180)
    queues(r, regions)
    words = []
    for _ in range(4):
        w = r.choice([r.randrange(-40, 60) & 0xffff] * 4 + [0, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)])
        words.append(w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
    x, y = point(r, regions, wins)
    if r.random() < 0.4:
        x, y = words[0] + r.randrange(-2, 30), words[1] + r.randrange(-2, 30)
    regions[0xfaa540] = struct.pack('<II', x & 0xffffffff, y & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], WINDOW2, WINDOW2, 0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                    r.getrandbits(32), r.choice([0] * 6 + [1]))
    flags = r.choice([1, 0x401, 0x400, 0, 0x10000001, r.getrandbits(32)]) | (r.getrandbits(32) & 0x0ffff800 if r.random() < 0.3 else 0)
    return [r.choice(wins + [0, 0]), flags, r.getrandbits(32)] + words


def timer_env(r, regions, log):
    """the heap, the window timers' pool and list, the GUI clock, the focus and the byte 0xfaa56c (Ctor705a20's)"""
    regions[HEAP] = heap(0x1000, r.choice([0] * 5 + [1]))
    n = r.choice([0, 1, 2, 4])
    pool = bytearray(r.bytes(0x20 * n))
    for i in range(n):
        put(pool, 0x20 * i + 0x14, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    if n:
        regions[POOL] = pool
    regions[0xfaa630] = struct.pack('<I', POOL)
    regions[0xfaa62c] = struct.pack('<i', r.choice([n, n, n, 0]))
    k = r.choice([0, 1, 2])
    nodes = bytearray(r.bytes(0x20 * k))
    for i in range(k):
        put(nodes, 0x20 * i + 0x18, 'I', TLIST + 0x20 * (i + 1) if i + 1 < k else 0)
    if k:
        regions[TLIST] = nodes
    regions[0xfaa634] = struct.pack('<I', TLIST if k else 0)
    set_return(log, 0, r.getrandbits(32))
    regions[CLOCKVT] = vtable({})
    regions[CLOCK] = struct.pack('<I', CLOCKVT)
    regions[0x77c490] = struct.pack('<I', CLOCK)
    regions[0xfaa56c] = bytes([r.choice([0, 0, 1, r.getrandbits(8)])])


def no_handlers(kind, flags):
    """flags without the branches that would call a new window's handler natively (see above)"""
    if kind in (4, 7):
        return flags & ~0x1000
    return flags & ~1 if kind == 3 and flags & 0x80 else flags


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op in ('SaveRect705a20', 'Layout705950', 'Layout7059f0', 'Layout705b20'):
        regions[WINDOW2] = r.bytes(0x180)
        if op == 'Layout705b20':
            regions[WINDOW2] = put(bytearray(regions[WINDOW2]), 0x14c, 'H', r.choice([0, 1, 2, 4, 6, 0xffff, r.getrandbits(16)]))
        return Call(CALLS[op], [WINDOW2], regions, conv='t', ret='v')
    args = ctor_args(r, regions)
    if op == 'CreateChild':
        timer_env(r, regions, log)
        regions[HEAP] = heap(0x2000, r.choice([0] * 20 + [1, 2, 3, 4]))
        regions[0xfaa5ec] = struct.pack('<I', r.choice([args[0], 0, 0]))
        kind = r.choice(list(range(1, 14)) * 3 + [0, 14, 0xffffffff, r.getrandbits(32)])
        flags = r.choice([0, 0x10, 0x40, 0x80, 0x100, 0x101, 0x1000, 0x1010, 0x1040, 0x1050, 0x90, 0xd0, 0x50, 0x1001, args[1]])
        flags = no_handlers(kind, flags)
        return Call(CALLS[op], [args[0] or TREE, kind, flags] + args[2:], regions, conv='t', ret='a')
    if op in ('Ctor705b20', 'Ctor705950', 'Ctor7059f0'):
        regions[HEAP] = heap(0x2000, r.choice([0] * 20 + [1, 2, 3, 4]))
        flags = r.choice([0, 0x10, 0x40, 0x80, 0x100, 0x101, 0x1000, 0x1010, 0x1040, 0x1050, 0x90, 0xd0, 0x50, 0x1001, args[1]])
        flags = no_handlers({'Ctor705b20': 3, 'Ctor705950': 4, 'Ctor7059f0': 7}[op], flags)
        return Call(CALLS[op], [WINDOW2, args[0], flags] + args[2:], regions, conv='t', ret='a')
    if op in ('AddArrow1', 'AddArrow2', 'AddThumb'):
        regions[HEAP] = heap(0x2000, r.choice([0] * 20 + [1, 2]))
        parent = args[0] or TREE
        buf = regions[parent] = bytearray(regions[parent]) + bytearray(r.bytes(0x180 - len(regions[parent])))
        put(buf, 0x14c, 'H', r.choice([0, 2, 1, 3, 0xffff, r.getrandbits(16)]))
        buf[0x48] &= 0xfe
        return Call(CALLS[op], [parent], regions, conv='t', ret='v')
    if op == 'Ctor705a20':
        timer_env(r, regions, log)
        regions[0xfaa5ec] = struct.pack('<I', r.choice([WINDOW2, args[0], 0, 0]))
    return Call(CALLS[op], [WINDOW2] + args, regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
