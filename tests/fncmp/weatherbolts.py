# The lightning's list of bolts (src/game/render/weatherfx: 0x57ee30, 0x580320) against the original, the bolt's drawing, the screen quad, the ticks and rand recorders, __ftol real: bolts running out or kept, flashing or not, a drawing failing; a new bolt with the list full, its ends the same, huge or NaN.
# usage: weatherbolts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fb, log_region, put, set_return  # noqa: E402

BOLT, QUAD, TICKS, RAND = 1, 2, 3, 4
B = Bench([(0x57ee30, 0x57efcb), (0x580320, 0x58046c), (0x67a830, 0x67a860)],
          hooks={0x57ec50: _recorder(BOLT, 2), 0x57ed50: _recorder(QUAD, 0, cdecl=True), 0x591210: _recorder(TICKS, 0, cdecl=True), 0x67b5c0: _recorder(RAND, 0, cdecl=True)})
FX, LIST, TIMES, LOOK, DRAWN = RAM, RAM + 0x3000, RAM + 0x9d90, RAM + 0xa190, RAM + 0xb000
OPS = ['draw', 'draw', 'draw', 'quad fails', 'draw fails', 'add', 'add', 'add', 'full', 'same ends', 'huge', 'nan']


def point(r, op):
    """Three coordinates as dwords."""
    if op == 'huge':
        return [fb(r.choice([1, -1]) * 10.0 ** r.randrange(15, 39)) for _ in range(3)]
    p = [fb(r.uniform(-500, 500)) for _ in range(3)]
    if op == 'nan':
        p[r.randrange(0, 3)] = r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    return p


def case(r):
    op = r.deal(OPS)
    log = log_region()
    count = 0 if op == 'draw' and r.random() < 0.1 else r.randrange(1, 17)
    bolts = bytearray(r.bytes(0x310))
    for i in range(16):
        put(bolts, 8 + 0x30 * i + 0x24, 'i', r.choice([r.randrange(1, 2000), r.randrange(1, 40), r.randrange(-40, 1), r.randrange(0, 1 << 31)]))
        if r.random() < 0.5:
            put(bolts, 8 + 0x30 * i + 0x2c, 'I', 0)
    set_return(log, QUAD, 0 if op == 'quad fails' else r.choice([1, 1, 0x100, r.getrandbits(32) | 1]))
    set_return(log, TICKS, r.getrandbits(32))
    set_return(log, RAND, r.getrandbits(32))
    answers(log, BOLT, DRAWN)
    drawn = [r.choice([1, 1, 2, 0x100]) for _ in range(40)]
    if op == 'draw fails':
        drawn[r.randrange(0, 4)] = 0
    adding = op in ('add', 'full', 'same ends', 'huge', 'nan')
    if adding:
        count = 16 + r.randrange(0, 2) if op == 'full' else r.randrange(0, 16)
    regions = {LOG: log, FX: put(bytearray(r.bytes(0x10)), 4, 'I', r.deal([0, 0, 1, 0x100]) if op != 'quad fails' else 1), LIST: put(bolts, 0x308, 'I', count), TIMES: put(bytearray(r.bytes(0x20)), 0xc, 'I', r.deal([0, 1, 16, 33, 100, 500, 1000])),
               LOOK: r.bytes(0x20), DRAWN: struct.pack('<40I', *drawn)}
    if not adding:
        return Call(0x57ee30, [FX], regions, conv='t', ret='a')
    start = point(r, op)
    end = start if op == 'same ends' else point(r, op)
    return Call(0x580320, [FX] + start + end + [r.getrandbits(32), r.deal([0, 0, 1, 0x100])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
