# The pluggable text driver (src/game/gfx/text: Text_Draw 0x5f8db0, Text_SetDriver 0x5f8ec0, Text_LineHeight 0x5f8f40) against the originals: the
# current text driver (0xfa76a0) a table of cdecl recorders (init, shutdown, measure, draw; init and shutdown now and then none), its context
# (0xfa76a4) any; measure fills its size, init its result (and returns it); the draw callback's advance its hash or small. Strings with newlines.
# The colour's palette index is logged as its byte (the original leaves the rest of its dword as the stack lies).
# Gap: the fonts' glyph callbacks (0x5f8e20-0x5f8ebf) and Draw_Sprite / Draw_SpriteShape (0x61b030, 0x61b050) are not in the listing, so the
# emulator cannot run them.
# usage: textdriver.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Draw,SetDriver,LineHeight)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, Bench, Call, RAM, echo, fill, log_region, recorder, set_return, strings  # noqa: E402

B = Bench([(0x5f8db0, 0x5f8e20), (0x5f8ec0, 0x5f8f71)])
DRV, OLD, TEXT, OUT = RAM, RAM + 0x20, RAM + 0x100, RAM + 0x180
GLOBALS = 0xfa76a0     # the text driver, its context
INIT, SHUTDOWN, MEASURE, DRAW, OLD_SHUTDOWN = 1, 2, 3, 4, 5
CALLS = {'Draw': 0x5f8db0, 'SetDriver': 0x5f8ec0, 'LineHeight': 0x5f8f40}


def driver(r, init, shutdown):
    return struct.pack('<4I', recorder(init, 3, cdecl=True) if r.random() < 0.8 else 0, recorder(shutdown, 3, cdecl=True) if r.random() < 0.8 else 0,
                       recorder(MEASURE, 3, cdecl=True), recorder(DRAW, 6, cdecl=True))


def colour(r):
    return [r.getrandbits(32), r.getrandbits(32)]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * DRAW, 1 << 5)
    strings(log, MEASURE, 0, out=0b10)
    if r.random() < 0.9:
        fill(log, MEASURE, 1, struct.pack('<2I', r.choice([0, 8, 12, r.getrandbits(32)]), r.choice([0, 8, 12, r.getrandbits(32)])), 0)
    strings(log, INIT, 0, out=0b1)
    fill(log, INIT, 0, struct.pack('<i', r.choice([0, 0, 1, -1, -0x80000000])), 1)
    echo(log, INIT, 0)
    if r.random() < 0.5:
        set_return(log, DRAW, r.choice([0, 6, 8, -4, r.getrandbits(32)]))
    current = r.choice([DRV, DRV, DRV, OLD, 0]) if op == 'SetDriver' else DRV
    regions = {GLOBALS: struct.pack('<II', current, r.getrandbits(32)), DRV: driver(r, INIT, SHUTDOWN), OLD: driver(r, INIT, OLD_SHUTDOWN), LOG: log,
               OUT: r.bytes(4)}
    address = CALLS[op]
    x, y = r.choice([0, 10, -5 & 0xffffffff, r.getrandbits(32)]), r.choice([0, 10, -5 & 0xffffffff, r.getrandbits(32)])
    if op == 'Draw':
        n = r.choice([0, 1, 5, 12, 20])
        regions[TEXT] = bytes(r.choice([0x0a, 0x20, 0x41, 0x7a, r.randrange(1, 256)]) for _ in range(n)) + b'\0'
        return Call(address, [x, y, TEXT] + colour(r), regions)
    if op == 'SetDriver':
        return Call(address, [OUT, r.choice([DRV, OLD]), r.getrandbits(32)], regions, ret='a')
    return Call(address, [r.choice([0x20, r.randrange(256), r.getrandbits(32)])], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
