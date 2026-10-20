# The GUI library's window scripts (src/game/ui/windowscript: 0x65fbf0, 0x65fc18, 0x65fc38, 0x65fcc5, 0x65fd58, 0x65fd0e) against the
# originals. The readers: a script of 0x200 random words from RAM + 0x100, the read position [0xfaa620] in it or up to 0x80 words before it;
# the words read to an output block or (now and then) into the script itself at the read position.
# Run (0x65fd58) / Build (0x65fd0e): windowfactory.py's CreateChild set-up (window.py's tree, the heap, the timers, the focus), the window
# run on WINDOW2 (0x180 random bytes: a recorder vtable whose slot 3, the kind, answers 4, 7 or another; no children or the tree's root;
# its own rectangle +8 and +0x2c real GUI rectangles, the hit area +0x44 its +8 or none, +0x98 / +0x9c none, a parent from the tree or
# none, flag 0x400 either way) and a script generated for it: 0-6 opcodes a window (children to depth 2, each child's script following,
# its kind known so that the kind-bound opcodes 0xa / 0xb / 0xd get their operands only where the original reads them; 9 on the window
# only, with its child of id 1 tracked), ended by 5 or an opcode past 0x12 (a few words of 5 after it). Hashes for 1 / 2: a registry
# [0xfaa610] of one node (its window a recorder, slot 8 answering 0) or none, the hash its or random. Not given (game code the driver
# cannot run natively): flags windowfactory.py leaves out, opcode 4's rectangle on a window that already has one (its deleting destructor
# 0x665eb0 is not ported), children under 8's button (its handler 0x66bb9b). Build: the position anywhere (it is reset), the handler 0
# or random.
# usage: windowscript.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ReadDword,ReadWord,ReadRect,ReadPair,Run,Build)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402
from window import TREE, VTABLE, WINDOW2  # noqa: E402
from windowfactory import FACTORY, ctor_args, no_handlers, timer_env  # noqa: E402

CALLS = {'ReadDword': 0x65fbf0, 'ReadWord': 0x65fc18, 'ReadRect': 0x65fc38, 'ReadPair': 0x65fcc5, 'Run': 0x65fd58, 'Build': 0x65fd0e}
RANGES = (FACTORY + [(0x65fbf0, 0x66076e), (0x659c42, 0x659cc0), (0x6588b6, 0x658900), (0x659d43, 0x659da0), (0x659dac, 0x659df5),
                      (0x65bb10, 0x65bb4f), (0x65c979, 0x65c9cd), (0x65cfff, 0x65d333), (0x65d7d0, 0x65d832), (0x6616b0, 0x661710),
                      (0x661830, 0x661880), (0x663554, 0x6636c2), (0x6651ef, 0x66522e), (0x66c054, 0x66c0a6), (0x66c1a6, 0x66c203),
                      (0x66c4a1, 0x66c578), (0x66cad3, 0x66cb79), (0x66bff7, 0x66c01a), (0x66c760, 0x66c7dc), (0x65a9d0, 0x65aa1b),
                      (0x66c568, 0x66c5a4), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0)]
           + [(a, a + 0x10) for a in (0x65c2c8, 0x65ad33, 0x6628e4, 0x6662a7, 0x668aad, 0x66ae73, 0x66c84a, 0x66cac3, 0x66d1e8, 0x66d875,
                                      0x66da3d, 0x66a5b4, 0x66a271)])   # (the kinds, slot 3)
B = Bench(RANGES)
SCRIPT, OUT = RAM + 0x100, RAM + 0x1000
WORDS, REG, REGWIN = RAM + 0xa000, RAM + 0xb000, RAM + 0xb100
KINDS = list(range(1, 14))
FLAGS = [0, 0x10, 0x40, 0x80, 0x100, 0x101, 0x1000, 0x1010, 0x1040, 0x1050, 0x90, 0xd0, 0x50, 0x1001]


def coord(r):
    return r.choice([r.randrange(-40, 60) & 0xffff] * 6 + [0, 0x7fff, 0x8000, 0xffff])


def dword(r, w, value):
    w += [value & 0xffff, value >> 16]


class Gen:
    def __init__(self, r, root_kind, reg_hash):
        self.r, self.root_kind, self.reg_hash, self.has1 = r, root_kind, reg_hash, False
        self.w = []

    def rect(self):
        self.w += [coord(self.r) for _ in range(4)]

    def child(self, kind, depth, plain=False):
        self.rect()
        self.window(kind, depth + 1, plain)

    def window(self, kind, depth, plain=False):
        r, w = self.r, self.w
        root = depth == 0
        has_rect = False
        for _ in range(r.choice([0, 1, 2, 3, 4, 6] if depth < 2 else [0, 1, 2])):
            ops = [1, 2, 3, 4, 0xa, 0x11, 0x12]
            if depth < 2 and not plain:
                ops += [0, 6, 7, 8, 0xc, 0xe, 0xf, 0x10]
            if root and not plain:
                ops += [9, 9]
            if kind == 7:
                ops += [0xb]
            if kind == 11:
                ops += [0xd]
            op = r.choice(ops)
            if op == 4 and has_rect:
                continue
            w.append(op)
            if op == 0:
                k = r.choice(KINDS)
                flags = no_handlers(k, r.choice(FLAGS))
                ident = r.choice([0, 1, 2, 3, 0x10, r.getrandbits(32)])
                w.append(k)
                dword(r, w, flags)
                dword(r, w, ident)
                if root and ident == 1:
                    self.has1 = True
                self.child(k, depth)
            elif op in (1, 2):
                dword(r, w, self.reg_hash if r.random() < 0.6 else r.getrandbits(32))
            elif op == 3:
                self.rect()
            elif op == 4:
                sub = r.choice([1, 2, 3, 4, 4, 0, 5, 0xffff])
                w.append(sub)
                if sub == 1:
                    self.rect()
                    has_rect = True
                elif sub == 2:
                    w += [coord(r), coord(r), r.choice([0, 1, 5, 20, 0xffff, r.getrandbits(16)])]
                elif sub == 3:
                    self.rect()
                elif sub == 4:
                    n = r.choice([0, 1, 2, 3, 4, 6])
                    w.append(n)
                    w += [coord(r) for _ in range(2 * n)]
            elif op in (6, 0xe):
                if root:
                    self.has1 = True
                self.child(2, depth)
            elif op in (7, 0xf, 0xc):
                if op == 0xc:
                    w.append(r.choice([0, 1, 0xfff0, 0xfff1, 0x7fff, r.getrandbits(16)]))
                    if root and w[-1] == 0xfff1:
                        self.has1 = True
                self.child(2, depth)
            elif op == 8:
                self.child(2, depth, plain=True)
            elif op == 0x10:
                w.append(r.choice([0, 1, 0xffff, r.getrandbits(16)]))
                self.child(6, depth)
            elif op == 9:
                if kind in (4, 7) and not self.has1:
                    self.has1 = True
                    self.child(3, depth)
            elif op == 0xa:
                if kind in (4, 7):
                    self.rect()
            elif op == 0xb:
                n = r.choice([0, 1, 2, 3, 5])
                w.append(n)
                w += [coord(r) for _ in range(2 * n)]
            elif op == 0xd:
                w += [coord(r), coord(r)]
            elif op in (0x11, 0x12):
                dword(r, w, r.getrandbits(32))
        w.append(r.choice([5] * 8 + [0x13, 0xffff, 0x7fff]))


def run_case(r, op):
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    args = ctor_args(r, regions)
    timer_env(r, regions, log)
    regions[HEAP] = heap(0x3000, r.choice([0] * 30 + [1, 2, 3, 5]))
    regions[0xfaa5ec] = struct.pack('<I', r.choice([args[0], 0, 0]))
    win = regions[WINDOW2] = bytearray(regions[WINDOW2])
    kind = r.choice([4, 4, 7, 7, 1, 2, 11])
    set_return(log, 3, kind)
    put(win, 0, 'I', VTABLE)
    put(win, 8, 'I', 0x705920)
    put(win, 0x2c, 'I', 0x705920)
    put(win, 0x44, 'I', r.choice([WINDOW2 + 8, 0]))
    put(win, 0x48, 'I', r.choice([1, 0x401, 0x400, 0]) | (r.getrandbits(32) & 0x0ffff800 if r.random() < 0.3 else 0))
    put(win, 0x5c, 'I', r.choice([0, TREE]))
    put(win, 0x60, 'I', r.choice([args[0], 0]))
    put(win, 0x98, 'I', 0)
    put(win, 0x9c, 'I', 0)
    if win[0x5c:0x60] == struct.pack('<I', TREE):
        put(regions[TREE], 0x54, 'I', 0)
        put(regions[TREE], 0x8c, 'I', r.choice([1, 2, r.getrandbits(32)]))
    reg_hash = r.getrandbits(32)
    if r.random() < 0.6:
        regions[REG] = struct.pack('<I12sII', REGWIN, b'name', reg_hash, 0)
        regions[REGWIN] = struct.pack('<I', VTABLE)
        set_return(log, 8, 0)
        regions[0xfaa610] = struct.pack('<I', REG)
    else:
        regions[0xfaa610] = struct.pack('<I', 0)
    g = Gen(r, kind, reg_hash)
    g.has1 = win[0x5c:0x60] == struct.pack('<I', TREE) and regions[TREE][0x8c:0x90] == struct.pack('<I', 1)
    pos = 0 if op == 'Build' else r.choice([0, 1, 7, r.randrange(0x40)])
    g.w = [r.getrandbits(16) for _ in range(pos)]
    g.window(kind, 0)
    words = g.w + [5] * 8
    regions[WORDS] = struct.pack('<%dH' % len(words), *words)
    regions[0xfaa620] = struct.pack('<I', pos if op == 'Run' else r.getrandbits(32))
    if op == 'Run':
        return Call(CALLS[op], [WINDOW2, WORDS], regions, conv='t', ret='a')
    return Call(CALLS[op], [WINDOW2, WORDS, r.choice([0, 0x66bb9b, r.getrandbits(32)])], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op in ('Run', 'Build'):
        return run_case(r, op)
    pos = r.choice([0, 1, 2, 3, r.randrange(0x1f8), r.randrange(0x1f8), 0x1f8])
    if r.random() < 0.2:   # a little before the script (EAX's upper half 0xffff for ReadWord)
        pos = -r.randrange(1, 0x81) & 0xffffffff
    regions = {RAM: r.bytes(0x100 + 0x400 + 0x10), OUT: r.bytes(0x20), 0xfaa620: struct.pack('<I', pos)}
    if op in ('ReadDword', 'ReadWord'):
        return Call(CALLS[op], [SCRIPT], regions, ret='a')
    n = 4 if op == 'ReadRect' else 2
    outs = [OUT + 2 * r.randrange(16) for _ in range(n)]
    if r.random() < 0.2:   # into the script, over words still to be read
        outs = [(SCRIPT + 2 * pos + 2 * r.randrange(4)) & 0xffffffff for _ in range(n)]
    return Call(CALLS[op], [SCRIPT] + outs, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
