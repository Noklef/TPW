# A GUI window's construction and destruction (src/game/ui/window, ui/window705a50) against the originals.
# Ctor (0x65bcad): a random window built with a parent from window.py's tree (its +0x60 chain and word +0xe0 near 0x100) or none, flags 0x400
# and 1 set or not, words edges or near (upper halves random); the root the new window, the tree's or none, the queues as window.py's.
# ForgetGlobals / DropModal / Destroying (0x6592b6, 0x65e672, 0x65c2d8): the globals the window (or one in its tree) or others (upper
# halves too); Destroying on a tree as HitTest's, the handlers fncmp's logger or 0, the hook [0xfaa5fc] set or not.
# Dtor / DeletingDtor (0x65c025, 0x661680): a window (going or not) with its objects +0x98 / +0x9c recorders (their counts near letting go)
# or 0, its timer flags set or not over a timer list [0xfaa634] with its and others' nodes, its rectangle +0x44 its own, a recorder or 0, a
# parent and siblings or none, 0-3 real child windows (vtable 0x705980: their own deleting destructors unlink them), the manager [0x77c48c]
# a recorder, the globals, queues and timed table (window.py's timers()), the hover's root a tree's or none; now and then an object's counts
# both -0x8000 (its release's 16-bit result 0xffff, the 32-bit one not -1). Dtor705a50 / DeletingDtor705a50 (0x6689bc, 0x6693a0): the
# same, with the text row +0x148 or 0; the children of either window class (the driver resolves both vtables). Dtor7059f0 (0x66274e): the
# same, 0x180 bytes, no items (the count +0x150 0: Clear is window7059f0.py's), the column array +0x16c a recorder object (slot 4) or 0,
# the row array +0x148 of 0-3 recorder objects (slot 2) or 0s (the rows shown +0x172 now and then -1) or none, the items +0x144 any or
# 0 (only freed: Mem_Free hashes the pointer), the highlight +0x174 a counted recorder object (its counts near letting go) or 0.
# Dtor705a20 (0x6661bc): the same, 0x170 bytes, the buffer +0x138 any or 0 (flag 0x40 of +0x48 either way), the timer +0x164 set or 0
# (one of the timer nodes now and then the window's 0x75e88), the caret +0x15c a counted recorder object or 0. Dtor705bf8 (0x66d1a6) and
# Del705bf8 (0x66d720, flags any): the same, the text +0x138 any or 0 (only freed).
# usage: windowlife.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402
from window import (CHILDREN, MANAGER, MISC, PARENT, RANGES, RECT, ROWS, TNODES, VTABLE, WINDOW, WINDOW2, point, queues, timers,  # noqa: E402
                    tree)

CALLS = {'Ctor': 0x65bcad, 'ForgetGlobals': 0x6592b6, 'DropModal': 0x65e672, 'Destroying': 0x65c2d8,
         'Dtor': 0x65c025, 'DeletingDtor': 0x661680, 'Dtor705a50': 0x6689bc, 'DeletingDtor705a50': 0x6693a0,
         'Dtor7059f0': 0x66274e, 'Dtor705a20': 0x6661bc, 'Dtor705bf8': 0x66d1a6, 'Del705bf8': 0x66d720}
B = Bench(RANGES + [(0x66274e, 0x6628e4), (0x6649d5, 0x664b68), (0x6661bc, 0x6662a7), (0x66d1a6, 0x66d1e8), (0x66d720, 0x66d74e)])
COLS7, ROWS7, OBJS7 = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200


def destructor(r, regions, op):
    regions[HEAP] = heap()
    wins = tree(r, regions)
    x, y = point(r, regions, wins)
    regions[0xfaa540] = struct.pack('<II', x, y)
    win = bytearray(r.bytes({'Dtor7059f0': 0x180, 'Dtor705a20': 0x170}.get(op, 0x150)))
    win[4] = 0 if r.random() < 0.7 else 1
    put(win, 0x148, 'I', r.choice([ROWS, ROWS, 0]))
    regions[ROWS] = r.bytes(0x40)
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
    regions[0x77c48c] = struct.pack('<I', MANAGER)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    regions[VTABLE] = vtable({0: 1, 5: 1})
    for k, off in enumerate((0x98, 0x9c)):
        impl = bytearray(r.bytes(0x20))
        put(impl, 0, 'I', VTABLE)
        edge = r.random() < 0.05   # both counts -0x8000 after the release
        put(impl, 0x10, 'H', 0x8000 if edge else r.choice([0, 1, 1, 2, 0xffff]))
        put(impl, 0x12, 'H', 0x8001 if edge else r.choice([0, 1, 1, 2, 3]))
        regions[MISC + 0x40 * k] = impl
        put(win, off, 'I', r.choice([MISC + 0x40 * k, MISC + 0x40 * k, 0]))
    for i in range(4):
        put(win, 0x120 + 4 * i, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    nodes = []   # the timer list: the window's ids and others, linked both ways
    for i in range(r.choice([0, 1, 3, 5])):
        node = bytearray(r.bytes(0x20))
        put(node, 0, 'I', r.choice([WINDOW2, WINDOW2, wins[0]]))
        put(node, 0xc, 'I', r.choice([0x80080, 0x80081, 0x80082, 0x80083, 0x10]))
        put(node, 0x18, 'I', 0)
        put(node, 0x1c, 'I', TNODES + 0x20 * (i - 1) if i else 0)
        if i:
            put(nodes[-1], 0x18, 'I', TNODES + 0x20 * i)
        nodes.append(node)
    if op == 'Dtor705a20' and nodes and r.random() < 0.7:
        put(r.choice(nodes), 0xc, 'I', 0x75e88)
    if nodes:
        regions[TNODES] = b''.join(nodes)
    regions[0xfaa634] = struct.pack('<I', TNODES if nodes and r.random() < 0.9 else 0)
    put(win, 0x44, 'I', r.choice([WINDOW2 + 8] * 3 + [RECT, 0]))
    regions[RECT] = struct.pack('<I', RECT + 0x40) + r.bytes(0xc)
    regions[RECT + 0x40] = vtable({4: 1})
    parent = r.choice([PARENT, 0])
    put(win, 0x60, 'I', parent)
    put(win, 0x58, 'I', r.choice([PARENT + 0x100, 0]))
    put(win, 0x54, 'I', r.choice([PARENT + 0x200, 0]))
    for k in range(3):
        regions[PARENT + 0x100 * k] = r.bytes(0x70)
    n = r.choice([0, 0, 1, 2, 3])
    kids = [CHILDREN + 0x150 * i for i in range(n)]
    put(win, 0x5c, 'I', kids[0] if n else 0)
    for i, a in enumerate(kids):
        kid = bytearray(r.bytes(0x150))
        put(kid, 0, 'I', r.choice([0x705980, 0x705a50]))
        put(kid, 0x148, 'I', r.choice([ROWS + 0x10 * (i + 1), 0]))
        kid[4] = r.choice([0, 1])
        put(kid, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
        put(kid, 0x60, 'I', WINDOW2)
        put(kid, 0x58, 'I', kids[i - 1] if i else 0)
        put(kid, 0x54, 'I', kids[i + 1] if i + 1 < n else 0)
        put(kid, 0x5c, 'I', 0)
        put(kid, 0x98, 'I', 0)
        put(kid, 0x9c, 'I', 0)
        for j in range(4):
            put(kid, 0x120 + 4 * j, 'I', 0)
        put(kid, 0x44, 'I', a + 8)
        regions[a] = kid
    if op == 'Dtor7059f0':
        put(win, 0x150, 'i', 0)
        put(win, 0x16c, 'I', r.choice([COLS7, COLS7, 0]))
        regions[COLS7] = struct.pack('<I', COLS7 + 0x40) + r.bytes(0x20)
        regions[COLS7 + 0x40] = vtable({4: 1})
        rows = r.choice([0, 1, 2, 3, -1])
        put(win, 0x172, 'h', rows)
        put(win, 0x148, 'I', r.choice([ROWS7, ROWS7, 0]))
        regions[ROWS7] = b''.join(struct.pack('<I', r.choice([OBJS7 + 0x20 * i, OBJS7 + 0x20 * i, 0])) for i in range(4))
        for i in range(4):
            regions[OBJS7 + 0x20 * i] = struct.pack('<I', OBJS7 + 0x100)
        regions[OBJS7 + 0x100] = vtable({2: 1})
        put(win, 0x144, 'I', r.choice([0, r.getrandbits(32)]))
        hl = bytearray(r.bytes(0x20))
        put(hl, 0, 'I', VTABLE)
        put(hl, 0x10, 'HH', r.choice([0, 1, 1, 2]), r.choice([0, 1, 1, 2, 3]))
        regions[OBJS7 + 0x180] = hl
        put(win, 0x174, 'I', r.choice([OBJS7 + 0x180, OBJS7 + 0x180, 0]))
    if op == 'Dtor705a20':
        put(win, 0x138, 'I', r.choice([0, r.getrandbits(32)]))
        put(win, 0x48, 'I', struct.unpack_from('<I', win, 0x48)[0] & ~0x40 | r.choice([0, 0x40]))
        put(win, 0x164, 'I', r.choice([0, 1, r.getrandbits(32)]))
        caret = bytearray(r.bytes(0x20))
        put(caret, 0, 'I', VTABLE)
        put(caret, 0x10, 'HH', r.choice([0, 1, 1, 2]), r.choice([0, 1, 1, 2, 3]))
        regions[OBJS7 + 0x1c0] = caret
        put(win, 0x15c, 'I', r.choice([OBJS7 + 0x1c0, OBJS7 + 0x1c0, 0]))
    if op in ('Dtor705bf8', 'Del705bf8'):
        put(win, 0x138, 'I', r.choice([0, r.getrandbits(32)]))
    regions[WINDOW2] = win
    timers(r, regions, WINDOW2)

    def some():
        # (a wild one in the first 64 KB, never mapped: one elsewhere the native driver can read where the emulator faults)
        return r.choice([WINDOW2, WINDOW2, wins[0], 0, r.getrandbits(16)] + kids)
    regions[0xfaa578] = struct.pack('<II', r.getrandbits(32), some())
    regions[0xfaa588] = struct.pack('<3I', some(), some(), some())
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0, 0]), some(), some(), r.getrandbits(32), r.choice([0, 0, 0, some()]))
    regions[0xfaa5ec] = struct.pack('<I', some())
    if op.startswith('Dtor'):
        return Call(CALLS[op], [WINDOW2], regions, conv='t', ret='a')
    return Call(CALLS[op], [WINDOW2, r.choice([0, 1, 1, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'Ctor':
        wins = tree(r, regions)
        for i, w in enumerate(wins):
            put(regions[w], 0x60, 'I', r.choice([wins[i - 1], 0]) if i else 0)
            put(regions[w], 0xe0, 'H', r.choice([0, 1, 0xfe, 0xff, 0x100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        regions[WINDOW2] = r.bytes(0x140)
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
        return Call(CALLS[op], [WINDOW2, r.choice(wins + [0, 0]), flags, r.getrandbits(32)] + words, regions, conv='t', ret='a')
    if op in ('ForgetGlobals', 'DropModal', 'Destroying'):
        wins = tree(r, regions) if op == 'Destroying' else [WINDOW]
        if op != 'Destroying':
            regions[WINDOW] = r.bytes(0x20)
        else:
            for w in wins:
                put(regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))

        def some():
            return r.choice(wins + [wins[0], 0, r.getrandbits(32), (wins[0] & 0xffff) | 0x12340000])
        regions[0xfaa578] = struct.pack('<II', r.getrandbits(32), some())
        regions[0xfaa588] = struct.pack('<3I', some(), some(), some())
        regions[0xfaa5dc] = struct.pack('<4I', some(), some(), r.getrandbits(32), some())
        regions[0xfaa5ec] = struct.pack('<I', some())
        if op == 'ForgetGlobals':
            return Call(CALLS[op], [r.choice([WINDOW] * 5 + [0])], regions, ret='a')
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)], regions, conv='t', ret='a')
    return destructor(r, regions, op)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
