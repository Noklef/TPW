# The movie player's frame headers and chunk helpers (src/game/gfx/movieplayer: 0x670860 the header by kind, 0x670930 the palette
# header, 0x670ab0 the size at 8, 0x677930 the palette's pixels, 0x677640 the bit fields, 0x6776c0 the four-colour blocks) against the
# originals. The player and its settings random bytes, the settings' skip flag mostly clear, the kind 0-8 (now
# and then any), the chunk mostly there; the palette header's count 0-0x100 (its first entry any), the brightness mostly 100 (else
# 0-255 or any: products wrapping), the palette data random. Pixels: the channel bits mostly 4-8 (now and then any) and shifts 0-31
# (now and then any), the flags random. Unpack: the count 0-40, the field bits 1-32 (now and then 0), the data random. Expand:
# the count -1-6 (now and then -0x80000000), the records random.
# Place: the view's sizes random (small or any), the settings' flags random (each bit about half the time).
# Draw (0x6779e0): depth 8 half the time (else 16 / 24 / 32 / any), the flags random, height 0-6, width 0-40 (often a multiple of 8),
# pitch from a little under to well over the row (not always a multiple of 4), the source and built pixels random.
# Indices (0x67862f): count -5-40 (now and then -0x7ffffffe: the signed tests' overflow), bits 1-16 mostly (else 0-32 or any: the pairs
# wrapping in their dword), the source random. Blocks (0x6786d8): across 0-8 and rows 0-4 (now and then -1), the pitch the row's bytes
# (now and then 4 or 8 more), split 0-8, indices mostly 0 to split + 16 (else any small), offsets -0x100-0x100 by 4 (the previous frame
# mapped 0x400 bytes either side), the codebook random. SizeFlags (0x670900): the player and data random.
# Size3 (0x6708e0): the same. ByKind (0x670890): Header's player and chunk, the kind read +0x3a8 0-8 (now and then any).
# Number (0x66ee80): a cursor at a tag, its count 0-4 mostly (else 5-8 or any byte), the bytes random (high bits set about half).
# usage: movieplayer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Header,Palette,Size,Pixels,Unpack,Expand,Place,Draw,Indices,
# Blocks,SizeFlags,Size3,ByKind,Number)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Header': 0x670860, 'Palette': 0x670930, 'Size': 0x670ab0, 'Pixels': 0x677930, 'Unpack': 0x677640, 'Expand': 0x6776c0, 'Place': 0x6711a0, 'Draw': 0x6779e0, 'Indices': 0x67862f,
         'Blocks': 0x6786d8, 'SizeFlags': 0x670900, 'Size3': 0x6708e0, 'ByKind': 0x670890, 'Number': 0x66ee80}
B = Bench([(0x670860, 0x67092e), (0x670930, 0x670aa2), (0x670ab0, 0x670ac5), (0x677930, 0x6779dc), (0x677640, 0x6776b8),
           (0x6776c0, 0x6777ce), (0x6711a0, 0x67122f), (0x6779e0, 0x677bf3), (0x676478, 0x6764bc), (0x67862f, 0x6786d8),
           (0x6786d8, 0x67878c), (0x670900, 0x67092e), (0x66ee80, 0x66eee9)])
PLAYER, SETTINGS, DATA = RAM, RAM + 0x1000, RAM + 0x2000


def settings(r):
    s = bytearray(r.bytes(0xb4 + 3 * 0x100 + 0x10))
    s[0x78:0x7c] = struct.pack('<I', r.choice([100] * 4 + [0, 50, 200, 255, r.randrange(0x100), r.getrandbits(32)]))
    s[0x88:0x8c] = struct.pack('<I', 0 if r.random() < 0.85 else r.choice([1, r.getrandbits(32)]))
    return bytes(s)


def data(r):
    d = bytearray(r.bytes(0x10 + 3 * 0x100))
    d[6:8] = struct.pack('<H', r.choice([0, 1, 2, 3, 0x10, 0x100, r.randrange(0x101)]))
    return bytes(d)


def pixels_case(r):
    s = bytearray(r.bytes(0x7c0))
    for at in (0x2c, 0x30, 0x34):
        s[at:at + 4] = struct.pack('<I', r.randrange(4, 9) if r.random() < 0.9 else r.getrandbits(32))
    for at in (0x38, 0x3c, 0x40):
        s[at:at + 4] = struct.pack('<I', r.randrange(32) if r.random() < 0.9 else r.getrandbits(32))
    return Call(CALLS['Pixels'], [PLAYER], {PLAYER: struct.pack('<I', SETTINGS), SETTINGS: bytes(s)}, conv='t', ret='v')


def place_case(r):
    view = struct.pack('<I', SETTINGS) + b''.join(struct.pack('<I', r.choice([r.randrange(0x400), r.getrandbits(32)])) for _ in range(6))
    s = bytearray(r.bytes(0x80))
    s[4:8] = struct.pack('<I', r.getrandbits(32))
    return Call(CALLS['Place'], [PLAYER], {PLAYER: view + r.bytes(8), SETTINGS: bytes(s)}, conv='t', ret='v')


def draw_case(r):
    source, out = RAM + 0x4000, RAM + 0x8000
    s = bytearray(r.bytes(0x7b4))
    s[0x48:0x4c] = struct.pack('<I', 8 if r.random() < 0.5 else r.choice([16, 24, 32, r.getrandbits(32)]))
    height = r.choice([0, 1, 2, 3, 6])
    width = r.choice([0, 8, 16, 24, 40, r.randrange(41)])
    if s[0x48] == 8 and s[4] & 0x40 and r.random() < 0.9:  # (MoviePixels_Double runs on past a count of 0 or short of 4s)
        height, width = r.choice([1, 2, 3, 6]), r.choice([8, 16, 24, 40])
    s[0x60:0x68] = struct.pack('<II', height, width)
    pitch = width * r.choice([1, 2, 4]) + r.choice([0, 0, 4, 1, 2, 3, 0x20])
    player = bytearray(r.bytes(0x650))
    player[0:4] = struct.pack('<I', SETTINGS)
    player[0x648:0x64c] = struct.pack('<I', source)
    regions = {PLAYER: bytes(player), SETTINGS: bytes(s), source: r.bytes(height * (width + 8) + 8),
               out: r.bytes(height * max(pitch, width * 4 + 0x20) + 2 * height * width + 0x40)}
    return Call(CALLS['Draw'], [PLAYER, out, r.getrandbits(32), pitch], regions, conv='f', ret='v')


def indices_case(r):
    count = r.randrange(-5, 41) if r.random() < 0.97 else -0x7ffffffe
    bits = r.randrange(1, 17) if r.random() < 0.8 else r.randrange(33) if r.random() < 0.9 else r.getrandbits(32)
    size = max(count, 0) * (bits & 0x3f) // 8 + 0x10
    return Call(CALLS['Indices'], [SETTINGS, DATA, count & 0xffffffff, bits], {SETTINGS: r.bytes(0xa4), DATA: r.bytes(size)}, ret='v')


def blocks_case(r):
    across, rows = (r.randrange(9), r.randrange(5)) if r.random() < 0.95 else (r.choice([-1, 2]), r.choice([-1, 1]))
    pitch = 4 * max(across, 1) + r.choice([0, 0, 0, 4, 8])
    split = r.randrange(9)
    n = max(across, 1) * max(rows, 1)
    indices = [r.randrange(split + 16) if r.random() < 0.95 else r.randrange(-2, 0x20) for _ in range(n)]
    offsets = [4 * r.randrange(-0x40, 0x41) for _ in range(9)]
    size = 4 * pitch * max(rows, 1) + 0x10
    previous, out, table, book, at = RAM + 0x3000, RAM + 0x6000, RAM + 0x100, RAM + 0x200, RAM + 0x40
    regions = {at: struct.pack('<%di' % n, *indices), table - 8: struct.pack('<11i', 0, 0, *offsets), book: r.bytes(0x300),
               previous - 0x400: r.bytes(size + 0x800), out: r.bytes(size)}
    return Call(CALLS['Blocks'], [at, previous, out, split, table, book + 0x80, pitch, across & 0xffffffff, rows & 0xffffffff], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Indices':
        return indices_case(r)
    if op == 'Blocks':
        return blocks_case(r)
    if op == 'Number':
        count = r.choice([0, 1, 1, 2, 2, 3, 3, 4, r.randrange(5, 9), r.getrandbits(8)])
        body = bytearray(r.bytes(2 + count + 4))
        body[1] = count
        return Call(CALLS[op], [PLAYER], {PLAYER: struct.pack('<I', DATA), DATA: bytes(body)}, conv='t', ret='a')
    if op in ('SizeFlags', 'Size3'):
        regions = {PLAYER: struct.pack('<I', SETTINGS) + r.bytes(0x4c), SETTINGS: r.bytes(0x70), DATA: r.bytes(8)}
        return Call(CALLS[op], [PLAYER, DATA], regions, conv='f', ret='v')
    if op == 'Draw':
        return draw_case(r)
    if op == 'Place':
        return place_case(r)
    if op == 'Pixels':
        return pixels_case(r)
    if op == 'Unpack':
        count = r.randrange(41)
        bits = r.randrange(1, 33) if r.random() < 0.95 else 0   # (not past 32: the fields stay in the data)
        return Call(CALLS[op], [SETTINGS, DATA, count, bits], {SETTINGS: r.bytes(0xc0), DATA: r.bytes(0xa8)}, conv='f', ret='v')
    if op == 'Expand':
        count = r.choice([-1, 0, 1, 2, 3, 6, -0x80000000]) & 0xffffffff
        return Call(CALLS[op], [DATA, SETTINGS, count], {SETTINGS: r.bytes(0x70), DATA: r.bytes(0x30)}, conv='f', ret='v')
    player = bytearray(r.bytes(0x640))
    player[0:4] = struct.pack('<I', SETTINGS)
    player[0x1f4:0x1f8] = struct.pack('<I', r.randrange(9) if r.random() < 0.9 else r.getrandbits(32))
    if op == 'ByKind':
        player[0x3a8:0x3ac] = struct.pack('<I', r.randrange(9) if r.random() < 0.9 else r.getrandbits(32))
    player[0x1e8:0x1ec] = struct.pack('<I', DATA if r.random() < 0.9 else 0)
    chunk = bytearray(data(r))
    if (op == 'Header' and player[0x1f4] != 1) or (op == 'ByKind' and player[0x3a8] != 1):   # (byte 7 the kind 4 header's flags: any, for Header only)
        chunk[7] = r.getrandbits(8)
    regions = {PLAYER: bytes(player), SETTINGS: settings(r), DATA - 0x10: r.bytes(0x10) + bytes(chunk)}
    if op == 'Header':
        return Call(CALLS[op], [PLAYER], regions, conv='t', ret='v')
    return Call(CALLS[op], [PLAYER, DATA], regions, conv='f', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
