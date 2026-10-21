# The GUI library's kind 12 window (vtable 0x705af0, a slider going by its fields) against the originals (src/game/ui/window705af0:
# 0x66a5c4, 0x66a68d, 0x66a6db, 0x66a71c). The window's range +0x138 / +0x13c and value +0x140 small (-3 to 12), equal or crossed now and
# then, or any.
# SetRange: the low and high pointers each a dword (small or any) or 0. SetStep: a dword or 0. Distance: another window of the same kind,
# both with a recorder vtable whose slot 3 answers its own value (fncmp.set_return: the two the same or not).
# SetValue: the new value in or near the range, equal to +0x140 now and then, the byte +0x154 0, 1 or any; the objects +0x98 / +0x9c
# recorders (slot 3: k 3 and 22) or 0, each logging its event as the hash of its dword +0 (the rest is stack garbage); the parent +0x60
# a window or 0 and window.py's message queues (Window_PostMessage).
# usage: window705af0.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetRange,SetStep,Distance,SetValue)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import queues  # noqa: E402

CALLS = {'SetRange': 0x66a5c4, 'SetStep': 0x66a68d, 'Distance': 0x66a6db, 'SetValue': 0x66a71c}
B = Bench([(0x658ccc, 0x658daf), (0x659710, 0x659740), (0x661525, 0x6615a6), (0x6697e3, 0x6698b4), (0x66a5c4, 0x66a7e1)])
WINDOW, OTHER, OBJ98, OBJ9C, VT98, VT9C = RAM, RAM + 0x180, RAM + 0x300, RAM + 0x340, RAM + 0x380, RAM + 0x400
PARENT, VTW, VTO, PTRS = RAM + 0x480, RAM + 0x500, RAM + 0x580, RAM + 0x600


def small(r):
    return r.choice([r.randrange(-3, 13)] * 4 + [0x7fffffff, -0x80000000, r.getrandbits(32)]) & 0xffffffff


def window(r):
    w = bytearray(r.bytes(0x160))
    lo = small(r)
    hi = r.choice([lo, small(r), small(r)])
    put(w, 0x138, 'II', lo, hi)
    put(w, 0x140, 'I', r.choice([lo, hi, small(r)]))
    return w


def case(r):
    op = r.deal(OPS)
    log = log_region()
    w = window(r)
    regions = {LOG: log, WINDOW: w}
    if op in ('SetRange', 'SetStep'):
        regions[PTRS] = put(bytearray(8), 0, 'II', small(r), small(r))
        args = [r.choice([PTRS, PTRS, 0]), r.choice([PTRS + 4, PTRS + 4, 0])]
        return Call(CALLS[op], [WINDOW] + args[:2 if op == 'SetRange' else 1], regions, conv='t', ret='v')
    if op == 'Distance':
        o = window(r)
        regions[OTHER], regions[VTW], regions[VTO] = o, vtable({}, size=8), vtable({}, size=8)
        put(regions[VTO], 0xc, 'I', recorder(22, 0))
        put(w, 0, 'I', VTW)
        put(o, 0, 'I', VTO)
        mine = r.getrandbits(32)
        set_return(log, 3, mine)
        set_return(log, 22, mine if r.random() < 0.7 else r.getrandbits(32))
        return Call(CALLS[op], [WINDOW, OTHER], regions, conv='t', ret='a')
    vt9c = vtable({}, size=8)
    put(vt9c, 0xc, 'I', recorder(22, 2))
    regions.update({VT98: vtable({3: 2}, size=8), VT9C: vt9c, OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98),
                    OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT9C), PARENT: r.bytes(0x120)})
    for off, obj in ((0x98, OBJ98), (0x9c, OBJ9C)):
        put(w, off, 'I', obj if r.random() < 0.8 else 0)
    put(w, 0x60, 'I', r.choice([PARENT, PARENT, 0]))
    w[0x154] = r.choice([0, 0, 1, r.getrandbits(8)])
    blob_bytes(log, 3, 1, 0xf)
    blob_bytes(log, 22, 1, 0xf)
    queues(r, regions)
    value = w[0x140] | w[0x141] << 8 | w[0x142] << 16 | w[0x143] << 24 if r.random() < 0.3 else small(r)
    return Call(CALLS[op], [WINDOW, value], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
