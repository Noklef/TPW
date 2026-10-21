# The GUI library's 0x705b20 window (src/game/ui/window705b20: 0x66b47d, 0x66ae83) against the originals. SetRange: any window bytes, the
# range +0x138 / +0x13c now and then the one given, the step +0x144 0, 1, -1, the sign bit or any; from / to about each other (equal, one
# apart, the sign bit) or any. SetEnabled: window.py's tree and hover (Window_UpdateHover), the root the window, the others windows of
# the vtables 0x705980, 0x705a50 (+0x98 / +0x9c recorders of slot 3 or 0, their events masked to the look word +4) and 0x705b20, flag 2 of
# each +0x48 either way, the handlers +0x114 fncmp's logger or 0, [0xfaa5d4] set or not; enabled 0, 1 or any (upper bytes random).
# SetPos (0x66b088): see set_pos(); the modes the callers pass (0-3, -1, -2), the value near the range or any.
# usage: window705b20.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetRange,SetEnabled,SetPos)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
from window import RANGES as WINDOW_RANGES, hover, tree  # noqa: E402

CALLS = {'SetRange': 0x66b47d, 'SetEnabled': 0x66ae83, 'SetPos': 0x66b088}
B = Bench(sorted(set(WINDOW_RANGES + [(0x66b47d, 0x66b508), (0x66ae83, 0x66af19), (0x65d56b, 0x65d57c), (0x65da8d, 0x65db25),
                                      (0x668a23, 0x668aad), (0x668820, 0x6688b3),
                                      (0x66b088, 0x66b438), (0x661525, 0x661596), (0x65c9bd, 0x65cd95), (0x661700, 0x66176a),
                                      (0x659e3e, 0x659e83), (0x659d43, 0x659d90)])))
OBJ98, OBJ9C, VT98, VT9C = RAM + 0xa000, RAM + 0xa040, RAM + 0xa080, RAM + 0xa100
BAR, BAR98, BAR9C, BARVT98, BARVT9C = RAM + 0xa200, RAM + 0xa380, RAM + 0xa3c0, RAM + 0xa400, RAM + 0xa480


def set_pos(r):
    """windowtree.py's rect_env tree, its root the thumb (id 3, else now and then another id) and the bar's only child (no
    thumb faults in both: Window_SetRect reads null + 8); the bar's own objects +0x98 /
    +0x9c recorders of slot 3 (k 24 / 25, their event's dword +0 hashed) or 0, the parent +0x60 one of the tree's windows or 0; the range
    small (empty now and then, the ends either way round), the position in it or past it, the step and page small of either sign or edges."""
    from windowtree import rect_env
    log = log_region()
    regions = {LOG: log}
    wins = rect_env(r, regions, log)
    put(regions[wins[0]], 0x8c, 'I', 3 if r.random() < 0.97 else r.choice([0, 2, 4, 0x10003]))
    bar = bytearray(r.bytes(0x150))
    put(bar, 0x5c, 'I', wins[0] if r.random() < 0.99 else 0)
    put(bar, 0x60, 'I', r.choice(wins + [0]))
    put(bar, 0x30, 'hhhh', *[r.randrange(-40, 120) for _ in range(4)])
    for off, k, obj, vt in ((0x98, 24, BAR98, BARVT98), (0x9c, 25, BAR9C, BARVT9C)):
        regions[vt] = put(vtable({}, size=8), 0xc, 'I', recorder(k, 2))
        regions[obj] = struct.pack('<I', vt)
        put(bar, off, 'I', obj if r.random() < 0.8 else 0)
        blob_bytes(log, k, 1, 0xf)
    lo = r.randrange(-50, 100)
    hi = lo + r.choice([0, 0, 1, 2, 10, 100, -5, r.randrange(-300, 300)])
    if r.random() < 0.08:
        lo, hi = r.choice([0, 0x80000000, 0x7fffffff, r.getrandbits(32)]), r.choice([0, 0x7fffffff, 0x80000000, r.getrandbits(32)])
    pos = r.choice([lo, hi, (lo + hi) // 2, lo - 3, hi + 3, r.randrange(-100, 200)]) & 0xffffffff
    put(bar, 0x138, 'III', hi & 0xffffffff, lo & 0xffffffff, pos)
    put(bar, 0x144, 'II', r.choice([1, 1, -1, 2, -3, 0, 10, 0x7fffffff, 0x80000000]) & 0xffffffff, r.choice([1, 5, 20, -4, 0, 1000]) & 0xffffffff)
    put(bar, 0x14c, 'H', r.choice([1, 2, 3, 5, 6, 0x8002, r.getrandbits(16)]))
    regions[BAR] = bar
    mode = r.choice([0, 1, 2, 3, 0xffffffff, 0xffffffff, 0xfffffffe])
    value = r.choice([lo, hi, pos, lo + 1, hi - 1, r.randrange(-200, 300), r.getrandbits(32)]) & 0xffffffff
    return Call(CALLS['SetPos'], [BAR, mode, value], regions, conv='t', ret='a')


def set_enabled(r):
    log = log_region()
    regions = {LOG: log}
    wins = tree(r, regions)
    hover(r, regions, wins)
    regions[VT98], regions[VT9C] = vtable({3: 2}, size=8), vtable({3: 2}, size=8)
    regions[OBJ98], regions[OBJ9C] = struct.pack('<I', VT98), struct.pack('<I', VT9C)
    blob_bytes(log, 3, 1, 0x30)
    for i, w in enumerate(wins):
        buf = regions[w] = regions[w] + bytearray(r.bytes(0x40))
        put(buf, 0, 'I', 0x705b20 if i == 0 else r.choice([0x705980, 0x705a50, 0x705a50, 0x705b20]))
        buf[0x48] = (buf[0x48] & 0xfd) | r.choice([0, 2])
        put(buf, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
        put(buf, 0x98, 'I', r.choice([OBJ98, 0]))
        put(buf, 0x9c, 'I', r.choice([OBJ9C, 0]))
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1, 0x80000000]))
    enabled = r.choice([0, 0, 1, 1, 0x8000, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
    return Call(CALLS['SetEnabled'], [wins[0], enabled], regions, conv='t', ret='w')


def case(r):
    op = r.deal(OPS)
    if op == 'SetEnabled':
        return set_enabled(r)
    if op == 'SetPos':
        return set_pos(r)
    w = bytearray(r.bytes(0x160))
    a = r.choice([0, 1, 0x3ff, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
    b = r.choice([a, (a + 1) & 0xffffffff, (a - 1) & 0xffffffff, 0, 0x3ff, 0x80000000, 0x7fffffff, r.getrandbits(32)])
    if r.random() < 0.5:
        a, b = b, a
    put(w, 0x144, 'I', r.choice([0, 1, 2, 0xffffffff, 0xfffffffe, 0x80000000, 0x7fffffff, r.getrandbits(32)]))
    if r.random() < 0.25:
        put(w, 0x138, 'II', b, a)
    elif r.random() < 0.2:
        put(w, 0x138, 'I', b)
    elif r.random() < 0.2:
        put(w, 0x13c, 'I', a)
    return Call(CALLS['SetRange'], [RAM, a, b], {RAM: w}, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
