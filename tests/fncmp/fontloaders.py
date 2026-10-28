# The text library's font loaders (src/game/files/fontloaders) against the originals, on charsets.py's stream and index:
#   BitmapLoad (0x6b0340): charsets.py's load_case with the 8-byte header ("BFMB" mostly; bytes 4 and 6-7 any), the read answering all
#   or a fixed count; the heap failing now and then.
#   FontLoad (0x6b0680): the stream's size (slot 5) 0 to 0x100 (now and then huge: the block's size wraps), the read answering all of
#   it or not, its first 0x40 bytes written ("F4FB" mostly); the index charsets.py's ResIndex (key 1's record of kind 0 or not).
#   FontCtor (0x6b0480): random bytes, any data pointer, charsets.py's index. BaseCtor (0x6b4a10): random bytes.
# usage: fontloaders.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=BitmapLoad,FontLoad,FontCtor,BaseCtor)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import charsets  # noqa: E402
from charsets import LOADER, OUT, RESULT, STREAM, THIS, VT_STREAM  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6b0340, 0x6b042a), (0x6b0480, 0x6b050b), (0x6b0680, 0x6b0709), (0x6b4a10, 0x6b4a22), (0x6af760, 0x6af770), (0x6af960, 0x6af9c9),
           (0x6afa30, 0x6afa6f), (0x6afc60, 0x6afc8d), (0x6afc90, 0x6afd36)])
CALLS = {'BitmapLoad': 0x6b0340, 'FontLoad': 0x6b0680, 'FontCtor': 0x6b0480, 'BaseCtor': 0x6b4a10}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
charsets.MAGIC['BitmapLoad'] = [0x424d4642]   # "BFMB"
charsets.HEADER['BitmapLoad'] = 8


def font_load(r, regions):
    log = regions[LOG]
    size = r.choice([0, 3, 4, 8, 0x10, 0x20, 0x40, r.randrange(0x101), r.randrange(0x101)]) if r.random() < 0.97 else \
        r.choice([0xffffffff, 0xfffffff0, 0xffffffe8])
    set_return(log, 5, size)
    data = bytearray(r.bytes(min(size, 0x40)))
    if len(data) >= 4 and r.random() < 0.85:
        struct.pack_into('<I', data, 0, 0x42463446)   # "F4FB"
    if data:
        fill(log, 2, 0, bytes(data))
    strings(log, 2, 0, out=1)   # (the buffer: the new block)
    if r.random() < 0.75:
        echo(log, 2, 1)
    else:
        set_return(log, 2, r.choice([size, size, 0, size - 1, 4, r.getrandbits(32)]))
    regions[STREAM] = struct.pack('<I', VT_STREAM) + r.bytes(0xc)
    regions[VT_STREAM] = vtable({2: 2})
    regions[LOADER] = r.bytes(4)
    regions[OUT] = r.bytes(4)
    return [RESULT, STREAM, OUT, r.getrandbits(32), charsets.index(r, regions)]


def case(r):
    op = r.deal(OPS)
    regions = {LOG: log_region(), HEAP: heap(0x2000, fail=1 if r.random() < 0.08 else 0)}
    this = THIS
    if op == 'BitmapLoad':
        stack = charsets.load_case(r, op, regions)
        this = LOADER
    elif op == 'FontLoad':
        stack = font_load(r, regions)
        this = LOADER
    elif op == 'FontCtor':
        stack = [r.getrandbits(32), charsets.index(r, regions)]
    else:
        stack = []
    regions[THIS] = r.bytes(0x20)
    regions[RESULT] = r.bytes(8)
    return Call(CALLS[op], [this] + stack, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
