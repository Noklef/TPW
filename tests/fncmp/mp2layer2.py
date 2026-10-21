# The MP2 decoder's layer II dequantisation (src/game/core/mp2layer2) against the originals: the float form (0x6ccc20) and the fixed-point one
# (0x6cd220) over a decoder of 0 to 3 channels (mostly 1 or 2) with sblimit and jsbound in range (now and then past 32 or below 0), a frame of
# random bits read from a random position, allocation tables of 4, 8 or 16 entries per subband (samples read one by one: d < 0, or grouped in
# threes: d 3, 5 or 9 with codes of 5, 7 or 10 bits), allocations mostly within them, and the factor and grouping tables filled with random
# values (floats now and then NaN, infinite or denormal, wild cases any bits). The frame decoder (0x6cdf50) over a decoder of 1 or 2 channels
# (now and then 0) with a bitrate, sampling rate and mode that pick one of the game's allocation tables (lsf now and then: table 4), jsbound
# within sblimit and mono never joint stereo (else the original reads its stack unset), a frame of random bits, the synthesis's buffers,
# windows and the factor and grouping tables random, float or fixed point (0xfb2704), half rate (0xfb2708) now and then, the output none
# now and then (the fixed point through the transform 0x6d62c0).
# usage: mp2layer2.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402
import x87emu  # noqa: E402

CALLS = {'float': 0x6ccc20, 'fixed': 0x6cd220, 'frame': 0x6cdf50}
B = Bench([(0x6ccc20, 0x6cd218), (0x6cd220, 0x6cde4d), (0x6cdf50, 0x6ce594), (0x6c9270, 0x6c957d), (0x6c9580, 0x6ca574), (0x6ca580, 0x6cc735),
           (0x6d62c0, 0x6d6c4c)])
DECODER, FRAME, BITS, OUT, SCALE, TABLE = RAM, RAM + 0x3000, RAM + 0x3800, RAM + 0x4000, RAM + 0x4800, RAM + 0x5000
FACTORS, FIXED = 0x70ce08, 0x70e908          # 27 rows of 64 each (float, then s32)
GROUPS = 0x70af28                            # the 3-, 5- and 9-level grouping tables (from the pointers at 0x782e00)
GROUPS_END = 0x70b108 + 729 * 3 + 0x300      # (codes past a table's end read on into the next)
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
GROUPED = {3: 5, 5: 7, 9: 10}                # d: the code's bits
FIXED_POINT, HALF = 0xfb2704, 0xfb2708
PCM = RAM + 0x6000


def image(address, n):
    for va, d in x87emu.exe_sections():
        if va <= address < va + len(d):
            return bytes(d[address - va:address - va + n])


CHOICE = struct.unpack('<96I', image(0x7114f0, 0x180))   # the allocation table by 16 (2 sampling rate + 2 - channels) + bitrate
SUBBANDS = struct.unpack('<5i', image(0x711670, 0x14))   # each table's sblimit


def entry(r):
    """An allocation table's (bits, d): d < 0, samples read one by one; d 3, 5 or 9, codes into the grouping table d. Wild: any bits up to 23,
    but a grouped code's at most 21: its three bytes (at the table's pointer, 0x70af28 to 0x70b108, + 3 code) then stay in the game's data
    (below 0xfc5200), where 22 bits reach past it into memory only the driver has (the emulator faults where the C++ reads on)."""
    if r.random() < 0.3:
        d = r.choice(list(GROUPED))
        k = GROUPED[d]
    else:
        k = r.randrange(2, 17)
        d = -((1 << (k - 1)) - 1)
    if r.wild and r.random() < 0.004:
        d = r.choice([d, r.randrange(-0x8000, 0)])
        k = r.randrange(0, 22 if d > 0 else 24)
    return struct.pack('<hh', k, d)


def table(r):
    """Allocation tables for 32 subbands: each 1 << step entries, its first (step, 0), the rest (bits, d); then 16 more entries for the
    allocations past the last table's end (random bytes there would hand the original a group pointer and code width it never sees)."""
    out, steps = bytearray(), []
    for _ in range(32):
        step = r.choice([2, 3, 4, 4])
        steps.append(step)
        out += struct.pack('<hh', step, 0)
        for _ in range((1 << step) - 1):
            out += entry(r)
    for _ in range(16):
        out += entry(r)
    return out, steps


def tables(r):
    """0x70af28-0x710408: the grouping tables, the float window (from 0x70bc48), the fixed-point one (from 0x70c5c8), the float factors
    (0x70ce08) and the fixed-point ones (0x70e908)."""
    out = bytearray(r.choice([r.randrange(27), r.randrange(27), r.getrandbits(8)]) for _ in range(0x70bc48 - GROUPS))
    out += b''.join(struct.pack('<I', r.f(r.choice([0.5, 1.0]))) for _ in range((0x70c5c8 - 0x70bc48) // 4))
    out += b''.join(struct.pack('<h', int(r.gauss(0, r.choice([300, 3000]))) if r.random() < 0.9 else r.randrange(-0x8000, 0x8000))
                    for _ in range((0x70ce08 - 0x70c5c8) // 2))
    out += b''.join(struct.pack('<I', r.f(r.choice([1.0, 1e-3, 2.0]))) for _ in range(27 * 64))
    out += b''.join(struct.pack('<I', r.choice([r.getrandbits(16), r.getrandbits(20), r.getrandbits(32)])) for _ in range(27 * 64))
    return out


def case_frame(r):
    channels = r.choice([1, 2, 2]) if r.random() < 0.97 else 0
    lsf = r.random() < 0.1
    while True:
        bitrate, rate = r.randrange(1, 15), r.choice([0, 1, 2] if channels else [0, 1])   # (no channels: past the table at rate 2)
        table = 4 if lsf else CHOICE[((2 * rate + 2 - channels) << 4) + bitrate]
        if 0 <= table < 5:
            break
    sblimit = SUBBANDS[table]
    mode = r.choice([0, 1, 1, 2]) if channels == 2 else 3
    ext = r.randrange(4)
    while 4 * ext + 4 > sblimit:
        ext -= 1
    fixed = r.random() < 0.4
    decoder = bytearray(r.bytes(0x2460))
    if fixed:
        decoder[4:0x1204] = b''.join(struct.pack('<h', int(r.gauss(0, 2000))) for _ in range(0x900))
    else:
        decoder[4:0x1204] = b''.join(struct.pack('<I', r.f(r.choice([0.3, 1.0]))) for _ in range(0x480))
    decoder[0x243f], decoder[0x2445], decoder[0x2446], decoder[0x2448], decoder[0x2449] = lsf, bitrate, rate, mode, ext
    advance = r.choice([32 * max(channels, 1), 32, 64, r.randrange(0, 80)])
    struct.pack_into('<2I', decoder, 0x2428, FRAME, r.randrange(0, 64))
    struct.pack_into('<i', decoder, 0x244c, channels)
    struct.pack_into('<I', decoder, 0x2450, advance)
    out = 0 if r.random() < 0.05 else PCM
    regions = {DECODER: decoder, FRAME: r.bytes(0x2000), PCM: r.bytes(36 * 2 * 80 + 0x100), GROUPS: tables(r),
               FIXED_POINT: struct.pack('<I', r.choice([1, r.getrandbits(32)]) if fixed else 0),
               HALF: struct.pack('<I', r.choice([0, 0, 0, 1]))}
    return Call(CALLS['frame'], [DECODER, out], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    if name == 'frame':
        return case_frame(r)
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
    regions.update({DECODER: decoder, FRAME: r.bytes(0x500), BITS: bits + r.bytes(0x40), OUT: r.bytes(0x500), SCALE: scale, TABLE: tab,
                    GROUPS: bytearray(r.choice([r.randrange(27), r.randrange(27), r.getrandbits(8)]) for _ in range(GROUPS_END - GROUPS))})
    return Call(CALLS[name], [DECODER, BITS, OUT, SCALE, r.randrange(0, 4), TABLE], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
