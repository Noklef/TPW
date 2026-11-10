# RefPack decompression (src/game/files/refpack: 0x641f10 Decode, 0x642110 DecodedSize, stdcall) against the originals: well-formed streams of
# every opcode (2-, 3- and 4-byte matches with 0-3 literals before, 4-112 literal runs, the stop with 0-3 last literals), matches reaching up to
# 0x100 bytes before dst (prefilled), the plain 0x10fb header or one with flag 0x01 (a compressed size first), a null source; DecodedSize on
# headers of either kind and random bytes. Chunk (0x67850c, cdecl (src, dst, decode); EAX not compared: the original's POPAD restores the
# caller's): the same streams, decode mostly set (else 0: nothing decoded), a null source now and then.
# The encoder (0x641a50, thiscall (dst, src, len, workspace)) with WriteBigEndian (0x6419e0) and RefPack_MatchLength (0x641a20): inputs
# of random bytes, a small alphabet, runs (past 0x404: the longest match) and repeats from near and from past 0x400 / 0x4000 back, so
# every command form and literal run comes up. Its workspace's 0x10000 heads (256 KB) would cost a second a case as a region, so
# 'Encode' puts them in the game's .data at WORK (not compared: every head is set to -1 first, and each one read leads to a match the
# output shows) with the links that the input reaches as a region; 'EncodeFull' (one case in 20) has the whole workspace as a region,
# 'EncodeAlloc' none (the codec's slot 3 size from Mem_Alloc, freed after; the whole heap compared) or an allocation that fails.
# Not reached: inputs past 128 KB, where the window's lower bound starts to drop candidates. The largest inputs are just past 16 KB (for
# offsets past 0x4000): 17 KB was over the emulator's 5M steps a case.
# usage: refpack.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Decode,Size,Chunk)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, heap, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x641f10, 0x642100), (0x642110, 0x642147), (0x6419b0, 0x6419d4), (0x67850c, 0x67862f), (0x6419e0, 0x641a15), (0x641a20, 0x641a48),
           (0x641a50, 0x641f10)])
SRC, BEFORE, SLACK = RAM, RAM + 0x2000, 0x100
DST = BEFORE + SLACK
CALLS = {'Decode': 0x641f10, 'Size': 0x642110, 'Chunk': 0x67850c, 'Read': 0x6419b0, 'WriteBE': 0x6419e0, 'Match': 0x641a20, 'Encode': 0x641a50,
         'EncodeFull': 0x641a50, 'EncodeAlloc': 0x641a50}
WEIGHT = {'Encode': 6}
ESRC, EDST, CODEC, VT, FULL = RAM + 0x100000, RAM + 0x200000, RAM + 0x300000, RAM + 0x301000, RAM + 0x400000
WORK = 0x900000   # (game .data no case reads: the heads, see above)


def raw(r, n):
    return bytearray(r.getrandbits(8 * n).to_bytes(n, 'little')) if n else bytearray()


def plain(r, big):
    """An input of random pieces: noise, a small alphabet, runs, repeats of what came before."""
    out = bytearray()
    target = r.choice([0x4008, 0x4100]) if big else r.choice([0, 1, 2, 3, 5, 9, 40, 120, 400, 900, 1500])
    while len(out) < target:
        k = r.choice('nnarrp' if not big else 'nnnnnpf')
        n = r.choice([1, 2, 3, 4, 7, 20, 60]) if k != 'r' else r.choice([3, 5, 12, 70, 300, 1100, 1500])
        if k == 'n':
            out += raw(r, n)
        elif k == 'a':
            out += bytes(r.choice(b'abcd') for _ in range(n))
        elif k == 'r':
            out += bytes([r.getrandbits(8)]) * n
        elif out:
            far = len(out) if k == 'f' else min(len(out), r.choice([16, 0x400, 0x1000]))
            at = len(out) - r.randint(1, far)
            for i in range(r.choice([3, 4, 5, 10, 11, 0x43, 0x44, 200])):
                out.append(out[at + i])
    return out[:target] if big or r.random() < 0.5 else out


def encode(r, op):
    src = plain(r, r.random() < 0.08)
    n = len(src)
    regions = {ESRC: src + raw(r, 8), EDST: raw(r, n + n // 100 + 0x40)}
    if op == 'EncodeAlloc':
        log = log_region()
        set_return(log, 3, 0xc0000)
        regions.update({LOG: log, CODEC: struct.pack('<I', VT), VT: vtable({}), HEAP: heap(0xc0100) if r.random() < 0.7 else heap(0x100, fail=1)})
        return Call(0x641a50, [CODEC, EDST, ESRC, n, 0], regions, conv='t', ret='a')
    if op == 'EncodeFull':
        regions[FULL] = raw(r, 0xc0000)
        return Call(0x641a50, [CODEC, EDST, ESRC, n, FULL], regions, conv='t', ret='a')
    regions[WORK + 0x40000] = raw(r, 4 * n + 0x40)
    return Call(0x641a50, [CODEC, EDST, ESRC, n, WORK], regions, conv='t', ret='a')


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
    op = r.deal(DECK)
    if op.startswith('Encode'):
        return encode(r, op)
    if op == 'WriteBE':
        return Call(0x6419e0, [SRC, r.getrandbits(32), r.choice([0, 1, 2, 3, 4, 5, 6, 9])], {SRC: r.bytes(12)}, ret='v')
    if op == 'Match':
        a, same = r.bytes(64), r.choice([0, 1, 3, 5, 30, 64])
        b = a[:same] + r.bytes(64 - same)
        return Call(0x641a20, [SRC, SRC + 0x100, r.choice([0, 1, 2, 5, 30, 64])], {SRC: a, SRC + 0x100: b}, ret='a')
    if op == 'Read':
        return Call(0x6419b0, [SRC, r.choice([0, 1, 2, 3, 4, 5, 8])], {SRC: r.bytes(8)}, ret='a')
    if op == 'Size':
        k = r.random()
        src = header(r, r.getrandbits(24)) if k < 0.6 else bytes([0x10, 0xfb]) + r.bytes(6) if k < 0.8 else r.bytes(8)
        return Call(0x642110, [SRC], {SRC: src + r.bytes(4)}, conv='s', ret='a')
    body, n = stream(r)
    size = n if r.random() < 0.8 else r.getrandbits(24)
    src = header(r, size) + body
    regions = {SRC: src + r.bytes(4), BEFORE: r.bytes(SLACK + n + 8)}
    if op == 'Chunk':
        decode = r.choice([1, 1, 1, r.getrandbits(32), 0])
        return Call(0x67850c, [SRC if r.random() < 0.95 else 0, DST, decode], regions, ret='v')
    return Call(0x641f10, [DST, SRC if r.random() < 0.95 else 0, r.getrandbits(32)], regions, conv='s', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
DECK = [k for k in OPS for _ in range(WEIGHT.get(k, 1))]


if __name__ == '__main__':
    B.main(case)
