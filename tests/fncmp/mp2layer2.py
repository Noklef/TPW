# The MP2 decoder's layer II dequantisation (src/game/core/mp2layer2) against the originals: the float form (0x6ccc20) and the fixed-point one
# (0x6cd220) over a decoder of 0 to 3 channels (mostly 1 or 2) with sblimit and jsbound in range (now and then past 32 or below 0), a frame of
# random bits read from a random position, allocation tables of 4, 8 or 16 entries per subband (samples read one by one: d < 0, or grouped in
# threes: d 3, 5 or 9 with codes of 5, 7 or 10 bits), allocations mostly within them, and the factor and grouping tables filled with random
# values (floats now and then NaN, infinite or denormal, wild cases any bits).
# usage: mp2layer2.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'float': 0x6ccc20, 'fixed': 0x6cd220}
B = Bench([(0x6ccc20, 0x6cd218), (0x6cd220, 0x6cde4d)])
DECODER, FRAME, BITS, OUT, SCALE, TABLE = RAM, RAM + 0x3000, RAM + 0x3800, RAM + 0x4000, RAM + 0x4800, RAM + 0x5000
FACTORS, FIXED = 0x70ce08, 0x70e908          # 27 rows of 64 each (float, then s32)
GROUPS = 0x70af28                            # the 3-, 5- and 9-level grouping tables (from the pointers at 0x782e00)
GROUPS_END = 0x70b108 + 729 * 3 + 0x300      # (codes past a table's end read on into the next)
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
GROUPED = {3: 5, 5: 7, 9: 10}                # d: the code's bits


def table(r):
    """Allocation tables for 32 subbands: each 1 << step entries, its first (step, 0), the rest (bits, d)."""
    out, steps = bytearray(), []
    for _ in range(32):
        step = r.choice([2, 3, 4, 4])
        steps.append(step)
        out += struct.pack('<hh', step, 0)
        for _ in range((1 << step) - 1):
            if r.random() < 0.3:
                d = r.choice(list(GROUPED))
                k = GROUPED[d]
            else:
                k = r.randrange(2, 17)
                d = -((1 << (k - 1)) - 1)
            if r.wild and r.random() < 0.004:
                k, d = r.randrange(0, 24), r.choice([d, r.randrange(-0x8000, 0)])
            out += struct.pack('<hh', k, d)
    return out, steps


def case(r):
    name = r.deal(OPS)
    sblimit = r.choice([32, 30, 27, 8, 12, r.randrange(0, 33)])
    jsbound = r.choice([sblimit, sblimit, 4, 8, 16, r.randrange(0, sblimit + 1)])
    jsbound = min(jsbound, sblimit)
    if r.random() < 0.04:
        sblimit, jsbound = r.choice([(33, 8), (-2, -4), (0, 0), (4, 6)])
    channels = r.choice([1, 2, 2, 2, 0, 3]) if r.random() < 0.1 else r.choice([1, 2, 2])
    decoder = bytearray(r.bytes(0x2460))
    struct.pack_into('<2i', decoder, 0x2418, sblimit, jsbound)
    struct.pack_into('<2I', decoder, 0x2428, FRAME, r.randrange(0, 64))
    struct.pack_into('<i', decoder, 0x244c, channels)
    tab, steps = table(r)
    bits = bytearray()   # (as the subbands read them: one per channel below jsbound, two for each joint one)
    per = max(channels, 0) if name == 'float' else 2 if channels == 2 else 1
    for i in range(max(sblimit, 0)):
        for _ in range((per if i < jsbound else 2) if i < 32 else 0):
            bits.append(0 if r.random() < 0.25 else r.randrange(1, 1 << steps[i]))
    if r.wild:
        for i in range(len(bits)):
            if r.random() < 0.004:
                bits[i] = r.getrandbits(8)
    scale = bytearray(r.choice([r.randrange(0, 63), r.randrange(0, 63), r.getrandbits(8) if r.wild else 0]) for _ in range(0x200))
    if name == 'float':
        rows = b''.join(struct.pack('<I', r.f(r.choice([1.0, 1e-3, 2.0]))) for _ in range(27 * 64 + 0x80))
        regions = {FACTORS - 0x100: r.bytes(0x100) + rows}
    else:
        rows = b''.join(struct.pack('<I', r.choice([r.getrandbits(16), r.getrandbits(20), r.getrandbits(32)])) for _ in range(27 * 64 + 0x80))
        regions = {FIXED - 0x100: r.bytes(0x100) + rows}
    regions.update({DECODER: decoder, FRAME: r.bytes(0x500), BITS: bits + r.bytes(0x40), OUT: r.bytes(0x500), SCALE: scale, TABLE: tab + r.bytes(0x40),
                    GROUPS: bytearray(r.choice([r.randrange(27), r.randrange(27), r.getrandbits(8)]) for _ in range(GROUPS_END - GROUPS))})
    return Call(CALLS[name], [DECODER, BITS, OUT, r.randrange(0, 4), SCALE, TABLE], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
