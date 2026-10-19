# The GUI library's 0x705b20 window (src/game/ui/window705b20: 0x66b47d, 0x66ae83) against the originals. SetRange: any window bytes, the
# range +0x138 / +0x13c now and then the one given, the step +0x144 0, 1, -1, the sign bit or any; from / to about each other (equal, one
# apart, the sign bit) or any. SetEnabled: window.py's tree and hover (Window_UpdateHover), the root the window, the others windows of
# the vtables 0x705980, 0x705a50 (+0x98 / +0x9c recorders of slot 3 or 0, their events masked to the look word +4) and 0x705b20, flag 2 of
# each +0x48 either way, the handlers +0x114 fncmp's logger or 0, [0xfaa5d4] set or not; enabled 0, 1 or any (upper bytes random).
# usage: window705b20.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetRange,SetEnabled)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob_bytes, log_region, put, vtable  # noqa: E402
from window import RANGES as WINDOW_RANGES, hover, tree  # noqa: E402

CALLS = {'SetRange': 0x66b47d, 'SetEnabled': 0x66ae83}
B = Bench(sorted(set(WINDOW_RANGES + [(0x66b47d, 0x66b508), (0x66ae83, 0x66af19), (0x65d56b, 0x65d57c), (0x65da8d, 0x65db25),
                                      (0x668a23, 0x668aad), (0x668820, 0x6688b3)])))
OBJ98, OBJ9C, VT98, VT9C = RAM + 0xa000, RAM + 0xa040, RAM + 0xa080, RAM + 0xa100


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
    if r.deal(OPS) == 'SetEnabled':
        return set_enabled(r)
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
