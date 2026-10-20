# The GUI library's top depth (src/game/ui/guidepth: 0x659409, 0x659437; ui/window7059f0: 0x665bfb) against the originals. A tree of 1-7
# windows (0x160 bytes; the first the root [0xfaa5d8] or not), each with flags +0x48 bit 0 mostly set and bit 1 now and then, a depth word
# +0xe0 near the others' (or -1, 0x100, 0x8000), a GUI rectangle at +8 near the shape and small insets +0xf8-+0xfe, its kind (slot 3) one
# of two recorders (7 or another; their returns set per case) and, for kind 7, +0x148 0, a cell holding 0 or a cell holding one of the
# windows. The shape: none, a GUI rectangle or a circle near the windows. The three globals random to start. TopDepth's window: one of the
# tree or 0 (the root then, or -1 without one).
# usage: guidepth.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=TopDepth,TopDepthIn,Item148Depth)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, log_region, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'TopDepth': 0x659409, 'TopDepthIn': 0x659437, 'Item148Depth': 0x665bfb}
B = Bench([(0x659409, 0x65959a), (0x659730, 0x659760), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x659e83, 0x65a1d3), (0x65a3e9, 0x65a4a3),
           (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65d42e, 0x65d4b0), (0x65d547, 0x65d56a), (0x65d56b, 0x65d59d), (0x665bfb, 0x665c45),
           (0x66bff7, 0x66c01b), (0x66c01a, 0x66c7dc), (0x67acb0, 0x67acb1)])
WINS, VT_A, VT_B, CELLS, SHAPE = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1300


def windows(r, regions, log):
    n = r.choice([1, 2, 3, 4, 7])
    wins = [WINS + 0x160 * i for i in range(n)]
    bufs = [bytearray(r.bytes(0x160)) for _ in range(n)]
    base = r.randrange(-3, 10)
    regions[VT_A] = vtable({})
    regions[VT_B] = put(vtable({}), 0xc, 'I', recorder(23, 0))
    set_return(log, 3, r.choice([7, 1, 4, 11, 0x107]))
    set_return(log, 23, r.choice([7, 7, 2, 0]))
    for i, buf in enumerate(bufs):
        put(buf, 0, 'I', r.choice([VT_A, VT_B]))
        put(buf, 0x48, 'I', r.choice([1, 1, 1, 1, 3, 0, 2, 0x10001, r.getrandbits(32)]))
        put(buf, 0x54, 'I', 0)
        put(buf, 0x5c, 'I', 0)
        put(buf, 0xe0, 'h', r.choice([base + r.randrange(-2, 6)] * 6 + [-1, 0x100, -0x8000, 0x7fff]))
        x, y = r.randrange(-20, 40), r.randrange(-20, 40)
        struct.pack_into('<Ihhhh', buf, 8, 0x705920, x, y, x + r.randrange(0, 40), y + r.randrange(0, 40))
        struct.pack_into('<hhhh', buf, 0xf8, *[r.choice([0, 0, 1, 2, -1]) for _ in range(4)])
        k = r.random()
        put(buf, 0x148, 'I', 0 if k < 0.15 else CELLS + 4 * i)
        regions[CELLS + 4 * i] = struct.pack('<I', 0 if k < 0.3 else r.choice(wins))
    last = {}
    for i in range(1, n):
        parent = r.randrange(i)
        put(bufs[last[parent]] if parent in last else bufs[parent], 0x54 if parent in last else 0x5c, 'I', wins[i])
        last[parent] = i
    for w, buf in zip(wins, bufs):
        regions[w] = buf
    return wins


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = windows(r, regions, log)
    if op == 'Item148Depth':
        return Call(CALLS[op], [r.choice(wins)], regions, conv='t', ret='w')
    regions[0xfaa5d8] = struct.pack('<I', r.choice([wins[0]] * 4 + [r.choice(wins), 0]))
    regions[0xfaa570] = r.bytes(4)
    regions[0xfaa594] = struct.pack('<i', r.choice([0, -1, 5, 0x7fffffff, -0x80000000, r.getrandbits(31)]))
    regions[0xfaa5c8] = r.bytes(4)
    shape = r.choice([0, 0, SHAPE, SHAPE])
    x, y = r.randrange(-20, 60), r.randrange(-20, 60)
    if r.random() < 0.7:
        regions[SHAPE] = struct.pack('<Ihhhh', 0x705920, x, y, x + r.randrange(-2, 50), y + r.randrange(-2, 50))
    else:
        regions[SHAPE] = struct.pack('<Ihhh', 0x705b50, x, y, r.choice([1, 5, 20, 40])) + r.bytes(2)
    window = r.choice(wins + [wins[0], 0]) if op == 'TopDepth' else r.choice(wins + [wins[0]])
    return Call(CALLS[op], [window, shape], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
