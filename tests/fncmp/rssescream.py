# The RSSE's screams (src/game/rsse/scream: start 0x551130, volume 0x551290, move 0x5512d0) against the originals, over the sound globals
# (on 0x802bc8 and started 0x802bd4 each 0 or not, the manager 0x802bcc) as soundplay.py sets them: the manager's vtable +8 (the play
# command, hashed) and its engine's (+4) slots 3 (the parameter) and 5 (the position: three floats on the stack, hashed) recorders, their
# results now and then set (0 too: no sound). The scream 0 or not, the level and the volume's two inputs around their thresholds and
# overflowing, the position ints small, large or negative.
# usage: rssescream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=START,VOLUME,MOVE)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'START': 0x551130, 'VOLUME': 0x551290, 'MOVE': 0x5512d0}
B = Bench([(0x551130, 0x551284), (0x551290, 0x5512c8), (0x5512d0, 0x5512f4), (0x51bfc0, 0x51c048), (0x51bc40, 0x51bc70),
           (0x51c270, 0x51c2bc), (0x6b5b60, 0x6b5b75), (0x6b5b80, 0x6b5b9a), (0x5da3c0, 0x5da3c1)])
MANAGER, VT, ENGINE, EVT = RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x900


def coord(r):
    return r.choice([0, 1, -1, r.randrange(-2000, 2000), r.getrandbits(32) - (1 << 31), (1 << 24) + 1 + 2 * r.getrandbits(8)]) & 0xffffffff


def part(r):
    return r.choice([0, 1, 2, 99, 100, 101, 199, 200, 201, 250, -1, -2, -200, 0x7fffffff, r.randrange(-300, 300), r.getrandbits(32)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    blob(log, 2, 1, 0x7f)
    blob(log, 5, 1, 0x7)
    for k in (2, 3, 5):
        if r.random() < 0.6:
            set_return(log, k, r.choice([0, r.getrandbits(32)]))
    regions = {LOG: log, 0x802bc0: g, MANAGER: struct.pack('<II', VT, ENGINE) + r.bytes(8), VT: vtable({2: 2}, 4),
               ENGINE: struct.pack('<I', EVT) + r.bytes(0x30), EVT: vtable({3: 3, 5: 2}, 8), 0x803a24: r.bytes(4)}
    scream = r.choice([0, 0, r.getrandbits(32)] if op == 'START' else [0, r.getrandbits(32), r.getrandbits(32)])
    if op == 'START':
        level = r.choice([0, 1, 2, 3, 4, 7, 8, 9, 100, -1, -5, r.getrandbits(32)]) & 0xffffffff
        args = [scream, level, part(r), part(r), coord(r), coord(r), coord(r)]
    elif op == 'VOLUME':
        args = [scream, part(r), part(r)]
    else:
        args = [scream, coord(r), coord(r), coord(r)]
    return Call(CALLS[op], args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
