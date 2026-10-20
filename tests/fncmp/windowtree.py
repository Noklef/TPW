# A GUI window's tree (src/game/ui/window) against the originals. FindChild (0x65d7d0): a window with 0-6 children (ids from a few, repeats
# and misses) searched for one id. IsShown (0x65d773): a chain of 0-4 windows by +0x60. OffsetByParents (0x65c979): 0-4 parents by +0x60,
# some at addresses with other upper halves; the rectangle a recorder. HitTest / UpdateHover (0x65db25, 0x6589f9): a tree of 1-7 windows with
# real GUI rectangles (window.py's tree()), points near them or anywhere (upper halves random), the root, the hovered window and the queues set
# or not. NotifyShown / SetShown (0x65d90b, 0x65d9dd): window.py's shown_env. Offset (0x65c7fe): window.py's offset_env, moved by words 0 or
# not (upper halves random). SetEnabled (0x65da8d): shown_env's, flag 2 of each window's +0x48 either way; AX compared (only AX is set).
# usage: windowtree.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FindChild,IsShown,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, Bench, Call, log_region, put, vtable  # noqa: E402
from window import MISC, NODES, RANGES, TREE, VTABLE, WINDOW, chain, hover, offset_env, point, shown_env, tree, word  # noqa: E402

CALLS = {'FindChild': 0x65d7d0, 'IsShown': 0x65d773, 'OffsetByParents': 0x65c979, 'HitTest': 0x65db25, 'UpdateHover': 0x6589f9,
         'NotifyShown': 0x65d90b, 'SetShown': 0x65d9dd, 'Offset': 0x65c7fe, 'SetEnabled': 0x65da8d,
         'RaiseToTop': 0x65dbf0, 'OpenTree': 0x65c707, 'CloseTree': 0x65c778}
B = Bench(RANGES + [(0x65da8d, 0x65db25), (0x65dbf0, 0x65dcaf), (0x65c707, 0x65c7fe)])


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'FindChild':
        ids = [r.choice([0, 1, 5, 0x18, 0x1f, 0x100, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2, 3, 6]))]
        want = r.choice(ids + [r.choice([0, 5, 0x18, r.getrandbits(32)])]) if ids else r.choice([0, 5])
        win = bytearray(r.bytes(0x90))
        put(win, 0x5c, 'I', chain(r, regions, ids, NODES))
        regions[WINDOW] = win
        return Call(CALLS[op], [WINDOW, want], regions, conv='t', ret='a')
    if op == 'IsShown':   # a chain of 0-4 windows by +0x60, flag 0 of +0x48 mostly set (other bits random)
        n = r.randrange(0, 5)
        for i in range(n):
            w = bytearray(r.bytes(0x70))
            put(w, 0x48, 'I', r.getrandbits(32) | (1 if r.random() < 0.8 else 0) if r.random() < 0.9 else r.getrandbits(32) & ~1)
            put(w, 0x60, 'I', TREE + 0x80 * (i + 1) if i + 1 < n else 0)
            regions[TREE + 0x80 * i] = w
        return Call(CALLS[op], [TREE if n else 0], regions, conv='t', ret='w')
    if op == 'OffsetByParents':   # 0-4 parents by +0x60, some at addresses with other upper halves; the rectangle a recorder (slot 2: two)
        n = r.randrange(0, 5)
        parents = [r.choice([TREE + 0x80 * i, 0x31000000 + 0x10000 * r.randrange(1, 0x40) + 0x80 * i]) for i in range(n)]
        win = bytearray(r.bytes(0x70))
        put(win, 0x60, 'I', parents[0] if n else 0)
        regions[WINDOW] = win
        for i, a in enumerate(parents):
            w = bytearray(r.bytes(0x70))
            put(w, 0x60, 'I', parents[i + 1] if i + 1 < n else 0)
            regions[a] = w
        regions[MISC] = struct.pack('<I', VTABLE) + r.bytes(0xc)
        regions[VTABLE] = vtable({2: 2})
        return Call(CALLS[op], [WINDOW, MISC], regions, conv='t', ret='a')
    if op == 'HitTest':
        wins = tree(r, regions)
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)] + list(point(r, regions, wins)), regions, conv='t', ret='a')
    if op == 'UpdateHover':   # the root [0xfaa5d8] or none, [0xfaa5e8] mostly 0, the hovered window [0xfaa5e0] one of them or none
        hover(r, regions, tree(r, regions))
        return Call(CALLS[op], [], regions, ret='w')
    if op == 'Offset':   # see offset_env()
        wins = offset_env(r, regions, log)
        return Call(CALLS[op], [r.choice(wins), word(r), word(r)], regions, conv='t', ret='a')
    wins = shown_env(r, regions, log)   # NotifyShown / SetShown: EDX the window's upper half or random
    if op in ('RaiseToTop', 'OpenTree', 'CloseTree'):   # shown_env's, the previous links +0x58 set; the windows' vtable a recorder's (slot 0)
        for w in wins:
            prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
            while c:
                put(regions[c], 0x58, 'I', prev)
                prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
            put(regions[w], 0, 'I', VTABLE)
        put(regions[wins[0]], 0x58, 'I', 0)
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
        if op == 'RaiseToTop':
            return Call(CALLS[op], [r.choice(wins[1:] * 4 + wins[:1]) if len(wins) > 1 else wins[0]], regions, conv='t', ret='a')
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)], regions, conv='t', ret='w')
    window = r.choice(wins)
    if op == 'SetEnabled':
        for w in wins:
            regions[w][0x48] = (regions[w][0x48] & 0xfd) | r.choice([0, 2])
    shown = r.choice([0, 0, 1, 1, 2, 0x10000, 0xffff0000, 0x8000, r.getrandbits(32)])
    if op == 'NotifyShown':
        edx = r.choice([window & 0xffff0000, r.getrandbits(32)])
        return Call(CALLS[op], [window, edx, shown], regions, conv='f', ret='v')
    return Call(CALLS[op], [window, shown], regions, conv='t', ret='w' if op == 'SetEnabled' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
