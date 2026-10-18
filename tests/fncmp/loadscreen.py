# The loading screen's close and atexit destructor (src/game/gfx/loadscreen: 0x5879c0, 0x586d70) against the originals: shown or not,
# timeGetTime (a recorder) 3000 ms or more past the time shown (less would wait forever: the recorder's answer does not move); the
# blocks at +0x10 / +0x14 set or not, their handles 0 or set and in the tracked table (as texturemem.py) or not.
# usage: loadscreen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Close,StaticDtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x5879c0, 0x587a64), (0x586d70, 0x586e17), (0x579b40, 0x579c13), (0x67acb0, 0x67acb1)])
CALLS = {'Close': 0x5879c0, 'StaticDtor': 0x586d70}
SCREEN, BLOCKS, COUNT, PAIRS, TIME = 0x8c4748, RAM + 0x100, 0x8bb214, 0x87b1f8, 40


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {HEAP: heap(), **global_heap(), LOG: log, 0x6fd3ec: struct.pack('<I', recorder(TIME, 0, True)), 0x8bcbc8: r.bytes(4)}
    now = r.getrandbits(32)
    set_return(log, TIME, now)
    globs = bytearray(r.bytes(0x48))   # (the static screen, the time shown at +0x38, the count of shows at +0x44)
    put(globs, 0x44, 'I', r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    put(globs, 0x38, 'I', (now - r.choice([3000, 3001, 5000, r.randrange(3000, 1 << 32)])) & 0xffffffff)
    screen = bytearray(r.bytes(0x18))
    blocks = bytearray(r.bytes(0x20))
    handles = []
    for k in range(2):
        h = r.choice([0, r.getrandbits(32)])
        put(blocks, 0x10 * k, 'I', h)
        handles.append(h)
        put(screen, 0x10 + 4 * k, 'I', BLOCKS + 0x10 * k if r.random() < 0.8 else 0)
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(handles + [r.getrandbits(32)])
        put(table, 8 * k, 'II', r.getrandbits(32), r.choice([b, b, (b + 4) & 0xffffffff]))
    regions.update({PAIRS: table, COUNT: struct.pack('<I', r.choice([0, 1, 2, 4, 8])), BLOCKS: blocks})
    if op == 'StaticDtor':
        globs[:0x18] = screen
        regions[SCREEN] = globs
        return Call(0x586d70, [], regions)
    regions[SCREEN] = globs
    regions[RAM] = screen
    return Call(0x5879c0, [RAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
