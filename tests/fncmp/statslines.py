# The statistics graphs' lines (src/game/gfx/statsgraph: 0x480460, 0x480570, 0x49aa30, 0x4a4750) against the originals, statsgraph.py's game and CircBufU32s (its float one for 0x480570 and +0x20f10; the head now and then under -1: nothing drawn). 0x47fe90 (agent B's, unported) recorder 24 (kStatsGraphEnv): the points 0x7b4bf8's bytes compared, its stack
# (the colour's dword logged, its fifth byte with the garbage above it not).
# usage: statslines.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=PlotU32,PlotFloat,Lines,Lines2)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import statsgraph  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, strings  # noqa: E402

CALLS = {'PlotU32': 0x480460, 'PlotFloat': 0x480570, 'Lines': 0x49aa30, 'Lines2': 0x4a4750}
B = Bench([(0x480460, 0x480568), (0x480570, 0x480693), (0x49aa30, 0x49ab7b), (0x4a4750, 0x4a4827), (0x519510, 0x519550), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860)], hooks={0x47fe90: _recorder(24, 5)})
GAUGE, GAME, RECORD, LINE, COLOUR = RAM, RAM + 0x100, RAM + 0x10000, RAM + 0x50000, RAM + 0x50400


def line(r, floats=False):
    buf = bytearray(statsgraph.circbuf_f(r) if floats else statsgraph.circbuf(r))
    if r.random() < 0.06:
        put(buf, 0x244, 'i', r.choice([-1, -2, -3]))
        buf[0x24c] = 0
    return bytes(buf)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, GAUGE: r.bytes(0x20), 0x7b4bf8: r.bytes(0x250)}
    strings(log, 24, 0, out=1 << 4)
    set_return(log, 24, r.getrandbits(32))
    if op.startswith('Plot'):
        regions[LINE] = line(r, op == 'PlotFloat')
        regions[COLOUR] = r.bytes(8)
        regions[0x6ff0a4] = struct.pack('<f', r.choice([0.0, 1.5, -2.5, 100.0, 3e9, float('nan')]))
        kind = r.choice([0, 1, 2, 3, r.getrandbits(32)])
        return Call(CALLS[op], [GAUGE, LINE, COLOUR, r.choice([1, 1, 1, 2, 3, 0, -1 & 0xffffffff]), kind], regions, cw=0x7f, ret='v')
    index = r.randrange(4)
    table = bytearray(r.bytes(0x50))
    put(table, 20 * index, 'I', RECORD)
    regions.update({GAME + 0x1da720: struct.pack('<H', index) + r.bytes(2), 0x7cf83c: struct.pack('<I', GAME), 0x7cfb90: bytes(table)})
    flags = bytes([r.choice([0, 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 3, r.getrandbits(8)])]) + r.bytes(3)
    if op == 'Lines':
        regions[0x751fa0] = flags
        regions[0x7ca2ec] = struct.pack('<I', r.choice([0, 1, 2, 3]))
        regions[0x7ca298] = r.bytes(0x54)
        for offset in statsgraph.LINES:
            regions[RECORD + offset] = line(r)
    else:
        regions[0x753c48] = flags
        regions[0x7cb39c] = struct.pack('<I', r.choice([0, 1, 2, 3]))
        regions[0x7cb308] = r.bytes(8)
        regions[0x7cb368] = r.bytes(0x34)
        for offset in statsgraph.LINES2:
            regions[RECORD + offset] = line(r, offset == 0x20f10)
    return Call(CALLS[op], [GAUGE], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
