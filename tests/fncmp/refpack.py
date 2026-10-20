# RefPack decompression (src/game/files/refpack: 0x641f10 Decode, 0x642110 DecodedSize, stdcall) against the originals: well-formed streams of
# every opcode (2-, 3- and 4-byte matches with 0-3 literals before, 4-112 literal runs, the stop with 0-3 last literals), matches reaching up to
# 0x100 bytes before dst (prefilled), the plain 0x10fb header or one with flag 0x01 (a compressed size first), a null source; DecodedSize on
# headers of either kind and random bytes.
# usage: refpack.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Decode,Size)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x641f10, 0x642100), (0x642110, 0x642147), (0x6419b0, 0x6419d4)])
SRC, BEFORE, SLACK = RAM, RAM + 0x2000, 0x100
DST = BEFORE + SLACK
CALLS = {'Decode': 0x641f10, 'Size': 0x642110}


def stream(r):
    """A RefPack body and the size it decodes to."""
    out, n = bytearray(), 0
    for _ in range(r.choice([0, 1, 2, 4, 8, 16])):
        k = r.choice('2344ll')
        lit = r.randrange(4)
        if k == 'l':
            m = r.randrange(0x1c)
            out += bytes([0xe0 + m]) + r.bytes(4 * m + 4)
            n += 4 * m + 4
            continue
        n += lit
        if k == '2':
            length, far = r.randrange(3, 11), 0x400
        elif k == '3':
            length, far = r.randrange(4, 68), 0x4000
        else:
            length, far = r.choice([r.randrange(5, 40), r.randrange(5, 1029)]), 0x20000
        off = r.randrange(1, min(far, n + SLACK) + 1)
        o = off - 1
        if k == '2':
            out += bytes([(o >> 3) & 0x60 | (length - 3) << 2 | lit, o & 0xff])
        elif k == '3':
            out += bytes([0x80 | (length - 4), lit << 6 | o >> 8, o & 0xff])
        else:
            L = length - 5
            out += bytes([0xc0 | (o >> 12) & 0x10 | (L >> 6) & 0x0c | lit, (o >> 8) & 0xff, o & 0xff, L & 0xff])
        out += r.bytes(lit)
        n += length
    tail = r.randrange(4)
    out += bytes([0xfc + tail]) + r.bytes(tail)
    return out, n + tail


def header(r, size):
    if r.random() < 0.3:   # flag 0x01: the compressed size, then the decoded one
        return bytes([r.choice([0x11, 0x01, 0x91]), 0xfb]) + r.bytes(3) + struct.pack('>I', size)[1:]
    return bytes([r.choice([0x10, 0x10, 0x90, 0x12]), 0xfb]) + struct.pack('>I', size)[1:]


def case(r):
    op = r.deal(OPS)
    if op == 'Size':
        k = r.random()
        src = header(r, r.getrandbits(24)) if k < 0.6 else bytes([0x10, 0xfb]) + r.bytes(6) if k < 0.8 else r.bytes(8)
        return Call(0x642110, [SRC], {SRC: src + r.bytes(4)}, conv='s', ret='a')
    body, n = stream(r)
    size = n if r.random() < 0.8 else r.getrandbits(24)
    src = header(r, size) + body
    regions = {SRC: src + r.bytes(4), BEFORE: r.bytes(SLACK + n + 8)}
    return Call(0x641f10, [DST, SRC if r.random() < 0.95 else 0, r.getrandbits(32)], regions, conv='s', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
