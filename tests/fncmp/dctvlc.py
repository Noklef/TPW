# The video library's coefficient codes (src/game/gfx/dctvlc: 0x676c80 the tables, 0x677c00 a block) against the originals.
# Build: the function reads only the exe's static code lists, so every case builds the same tables: the cases differ in the random
# bytes the tables start from (any entry left unwritten, or written out of place, shows) around 0xfbc440-0xfbd560 and the flag 0xfaac68.
# Block: made-up tables (the first-level one's 0x100-0x17f the end marker, as built; else direct entries of skip 1-9 with runs mostly
# 0-3, the markers 0xf / 0x1f / 0x2f / 0x3f with random upper bits; the second-level ones skip 1-16, any run), a zigzag of random
# offsets 0-0xfc, a random quantiser and coefficients; the bit reader holding 16-32 valid bits (the rest clear) over random words. The
# stream is decoded here first: a code whose run would take the coefficient index past 63 (the listing reads past the zigzag) is
# replaced by the end code (bits 10) at its place, so each block ends inside its 64 coefficients.
# Skip / Code: the reader's count 0-32 (now and then any: unsigned below 16 reads a word), the bits and words random, Skip's n 0-32 (now
# and then any), Code's table 0xfbd44c random with skips 0-20. Fill: the DC random, the pitch 8-0x40 dwords. Begin: the built flag
# 0xfaac68 clear in a sixth of cases (the tables then built), the scales 0xfbc24c-0xfbc34b and the rest random, quality 0-31 mostly.
# usage: dctvlc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Build,Block,Skip,Code,Fill,Begin)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Build': 0x676c80, 'Block': 0x677c00, 'Skip': 0x6770b0, 'Code': 0x677110, 'Fill': 0x677140, 'Begin': 0x676bc0}
B = Bench([(0x676c80, 0x676e3a), (0x677c00, 0x677dad), (0x6770b0, 0x677105), (0x677110, 0x677134), (0x677140, 0x677197), (0x676bc0, 0x676c77)])
COEFFS, ZIGZAG, FIRST, SECOND, THIRD, QUANT, READER = 0xfbc14c, 0xfbc34c, 0xfbc44c, 0xfbcc4c, 0xfbd04c, 0xfbd54c, 0xfbd64c
WORDS = RAM
NWORDS = 0x100


def direct(r, top):
    run = r.randrange(4) if r.random() < 0.9 else r.randrange(64)
    return (r.getrandbits(10) << 22 | run << 16 | r.randrange(1, top + 1)) & 0xffffffff


def tables(r):
    first = []
    for i in range(0x200):
        if 0x100 <= i < 0x180:
            first.append(0x3f)
        elif r.random() < 0.6:
            first.append(direct(r, 9))
        else:
            first.append(r.getrandbits(24) << 8 | r.choice([0xf, 0x1f, 0x2f, 0x3f]))
    return first, [direct(r, 16) for _ in range(0x100)], [direct(r, 16) for _ in range(0x100)]


def value(bits, at, n):
    v = 0
    for b in bits[at:at + n]:
        v = v << 1 | b
    return v


def stream(r, first, second, third):
    """The bits (the reader's then the words'), with the codes that would run past 63 replaced by the end code."""
    bits = [r.getrandbits(1) for _ in range(32 + 16 * NWORDS)]
    pos, k = 8, 1
    while True:
        e = first[value(bits, pos, 9)]
        kind = e & 0xff
        prefix = 0
        if (kind if kind < 0x80 else kind - 0x100) > 9:
            if kind >= 0x30:
                break
            prefix = 6 if kind >= 0x10 else 9
            if kind >= 0x20:
                e = value(bits, pos + 6, 16) << 16 | 0x10
            else:
                e = (third if kind >= 0x10 else second)[value(bits, pos + prefix, 8)]
        if k + (e >> 16 & 0x3f) > 63:
            bits[pos], bits[pos + 1] = 1, 0
            break
        k += (e >> 16 & 0x3f) + 1
        pos += prefix + (e & 0xff)
    return bits


def block_case(r):
    first, second, third = tables(r)
    bits = stream(r, first, second, third)
    count = r.randrange(16, 33)
    held = value(bits, 0, count) << (32 - count)
    rest = bits[count:]
    words = b''.join(struct.pack('<H', value(rest, i * 16, 16)) for i in range(NWORDS))
    zigzag = struct.pack('<64I', *[4 * r.randrange(64) for _ in range(64)])
    quant = struct.pack('<64I', *[r.choice([1, 2, 16, r.randrange(0x100), r.getrandbits(32)]) for _ in range(64)])
    regions = {COEFFS: r.bytes(0x100), ZIGZAG: zigzag, FIRST: struct.pack('<512I', *first), SECOND: struct.pack('<256I', *second),
               THIRD: struct.pack('<256I', *third), QUANT: quant, READER: struct.pack('<3I', WORDS, held, count), WORDS: words}
    return Call(CALLS['Block'], [], regions, ret='a')


def reader(r):
    count = r.randrange(33) if r.random() < 0.9 else r.getrandbits(32)
    return {READER: struct.pack('<3I', WORDS, r.getrandbits(32), count), WORDS: r.bytes(8)}


def case(r):
    op = r.deal(OPS)
    if op == 'Block':
        return block_case(r)
    if op == 'Skip':
        return Call(CALLS[op], [r.randrange(33) if r.random() < 0.9 else r.getrandbits(32)], reader(r), conv='f', ret='v')
    if op == 'Code':
        regions = reader(r)
        regions[0xfbd44c] = b''.join(struct.pack('<I', r.getrandbits(24) << 8 | r.randrange(21)) for _ in range(64))
        return Call(CALLS[op], [], regions, ret='a')
    if op == 'Fill':
        pitch = r.choice([8, 9, 16, 0x40])
        return Call(CALLS[op], [WORDS, pitch], {WORDS: r.bytes(32 * pitch), COEFFS: r.bytes(4)}, conv='f', ret='v')
    if op == 'Begin':
        flag = struct.pack('<I', 0 if r.random() < 1 / 6 else r.getrandbits(32) | 1)
        quality = r.randrange(32) if r.random() < 0.85 else r.getrandbits(32)
        regions = {0xfbc240: r.bytes(0x1418), 0xfaac60: r.bytes(8) + flag + r.bytes(4), WORDS: r.bytes(8), RAM + 0x100: r.bytes(4)}
        return Call(CALLS[op], [WORDS, RAM + 0x100, quality], regions, conv='f', ret='v')
    return Call(CALLS['Build'], [], {0xfbc400: r.bytes(0x1180), 0xfaac60: r.bytes(0x10)}, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
