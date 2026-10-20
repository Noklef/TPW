# The GUI library's list window (vtable 0x705950: src/game/ui/window705950) against the originals. SelectAt / TrackAt / ClickAt /
# PostChosen (0x65b93f, 0x65bac2, 0x65ba1e, 0x65ba83): the window's selection, row count, first row, row height (0 now and then, negative,
# edges) and rows shown small or edges, the inner top small, flag 0x80 of +0x48 either way, the rectangle +0x2c a GUI one, the point in it,
# on a row or anywhere (upper halves random), the parent set or 0, [0xfaa5d4] 0 or 1, window.py's queues. Scrolled (0x65b8dc): children
# by +0x5c / +0x54 with ids 1 or others (or none), their slot 8 a recorder answering the first row shown, near it or anything. Proc
# (0x65bb6b): its five messages and some only the default handler takes (slots 4 of the window, post 4, the manager's slot 10, a click
# posted), the point a or b as above, the inner rectangle +0x138 a GUI one, the buttons held +0x11c and the press table [0xfaa598].
# usage: window705950.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SelectAt,TrackAt,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES, queues  # noqa: E402

CALLS = {'SelectAt': 0x65b93f, 'TrackAt': 0x65bac2, 'PostChosen': 0x65ba83, 'ClickAt': 0x65ba1e, 'Scrolled': 0x65b8dc, 'Proc': 0x65bb6b}
B = Bench(RANGES + [(0x65b8dc, 0x65bb10), (0x65bb3f, 0x65bc24), (0x65f6d1, 0x65fba8), (0x65dcaf, 0x65dd18), (0x680050, 0x68005b)])
LIST, PARENT, BAR, OTHER, VT = RAM + 0xa000, RAM + 0xa200, RAM + 0xa400, RAM + 0xa500, RAM + 0xa600
MANAGER, MVT = RAM + 0xa700, RAM + 0xa800
PROC_MESSAGES = [0x10003, 0x10003, 0x13, 0x11006, 0x11006, 0x11007, 0x800, 0x800, 3, 5, 0x10002, 0x10006, 0x12, 0x14, 0x801, 0x11005]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VT: vtable({2: 2})}
    win = bytearray(r.bytes(0x160))
    win[4] = r.choice([0, 0, 0, 1])
    put(win, 0x48, 'I', r.getrandbits(32) & ~0x80 | r.choice([0, 0x80]))
    put(win, 0x60, 'I', r.choice([PARENT] * 5 + [0]))
    left, top = r.randrange(-30, 20), r.randrange(-30, 20)
    right, bottom = left + r.choice([0, 1, 40, 100]), top + r.choice([0, 1, 40, 100])
    put(win, 0x2c, 'Ihhhh', 0x705920, left, top, right, bottom)
    inner = r.choice([top, top + 2, 0, r.randrange(-0x8000, 0x8000)])
    put(win, 0x13e, 'h', inner)
    height = r.choice([0, 1, 8, 8, 12, 12, 20, -5, 0x7fff, -0x8000])
    put(win, 0x154, 'h', height)
    put(win, 0x156, 'h', r.choice([0, 1, 4, 8, 8, -1, 0x7fff]))
    put(win, 0x14c, 'i', r.choice([0, 1, 3, 10, 10, 100, -1, r.randrange(-0x80000000, 0x80000000)]))
    put(win, 0x150, 'i', r.choice([0, 0, 1, 5, -3, r.randrange(-0x80000000, 0x80000000)]))
    put(win, 0x148, 'i', r.choice([-1, -1, 0, 1, 2, 5, r.randrange(-0x80000000, 0x80000000)]))
    regions[LIST] = win
    regions[PARENT] = bytes([0, 0, 0, 0, r.choice([0, 0, 0, 1])]) + r.bytes(0x10)
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1]))
    queues(r, regions)
    message = r.choice(PROC_MESSAGES) if op == 'Proc' else 0
    if op == 'Proc':
        put(win, 0, 'I', VT)
        put(win, 0x138, 'I', 0x705920)   # (OffsetInner pushes its words with the registers' upper halves: a GUI rectangle reads words)
        put(win, 0x11c, 'I', r.choice([0, 0, 5, 1]))
        regions[0xfaa598] = r.bytes(0x24)
        regions[0x77c48c] = struct.pack('<I', MANAGER)
        regions[MANAGER] = struct.pack('<I', MVT)
        regions[MVT] = vtable({8: 2})
    if op == 'Scrolled' or message == 0x800:
        put(win, 0x5c, 'I', r.choice([BAR, BAR, BAR, OTHER, 0]))
        regions[BAR] = put(put(put(bytearray(r.bytes(0x90)), 0, 'I', VT), 0x8c, 'I', r.choice([1, 1, 2, 0x10001])), 0x54, 'I', r.choice([0, OTHER]))
        regions[OTHER] = put(put(put(bytearray(r.bytes(0x90)), 0, 'I', VT), 0x8c, 'I', r.choice([1, 3])), 0x54, 'I', 0)
        first = struct.unpack_from('<I', win, 0x150)[0]
        set_return(log, 8, r.choice([first, first, first + 1, 0, r.getrandbits(32)]))
        if op == 'Scrolled':
            return Call(CALLS[op], [LIST], regions, conv='t', ret='v')
    if op == 'PostChosen':
        return Call(CALLS[op], [LIST], regions, conv='t', ret='v')
    x = r.choice([left, right - 1, r.randrange(left - 5, right + 5), r.randrange(-0x8000, 0x8000)]) & 0xffff
    if r.random() < 0.7 and height:
        y = inner + height * r.choice([0, 0, 0, -1, 1, 2, r.randrange(10)]) + r.choice([0, 1, abs(height) - 1])
    else:
        y = r.choice([top, bottom - 1, r.randrange(top - 5, bottom + 5), r.randrange(-0x8000, 0x8000)])
    if op == 'Proc':
        point = (x << 16 | y & 0xffff) if r.random() < 0.9 else r.getrandbits(32)
        a, b = r.getrandbits(32), r.getrandbits(32)
        if message == 0x10003:
            a = point
        elif message == 0x11006:
            b = point
        elif message == 0x10006:
            a, b = r.choice([0, 1, 2]), point
        return Call(CALLS[op], [LIST, message, a, b], regions, ret='a')
    return Call(CALLS[op], [LIST, x | r.getrandbits(16) << 16, (y & 0xffff) | r.getrandbits(16) << 16], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
