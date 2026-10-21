# The GUI library's text label, the 0x705bf8 window (src/game/ui/window705bf8: 0x66d1f8, 0x66d386, 0x66d457, 0x66d56f, 0x66d617, 0x66d651,
# 0x66d678) against the originals. The window's bytes any, its edges +0x30-+0x36 small words (now and then the sign bit), the alignments
# +0x13c / +0x140 0-3 or any, the font +0x4c a recorder object (slot 1 no arguments, slot 2 three; their answers small words or any) or 0;
# texts of 0-12 wide characters ('\n' among them now and then) or 0 where the function checks. SetText: the room +0x148 about the text's
# length (or the sign bit), the block +0x138 one in RAM or 0, the heap failing now and then. GetText: out a buffer or 0, n 0-16.
# usage: window705bf8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Place,TextX,TextY,SetText,GetText,SetAlignX,SetAlignY)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Place': 0x66d1f8, 'TextX': 0x66d386, 'TextY': 0x66d457, 'SetText': 0x66d56f, 'GetText': 0x66d617, 'SetAlignX': 0x66d651,
         'SetAlignY': 0x66d678}
B = Bench([(0x66d1f8, 0x66d386), (0x66d386, 0x66d457), (0x66d457, 0x66d56f), (0x66d56f, 0x66d617), (0x66d617, 0x66d644),
           (0x66d651, 0x66d678), (0x66d678, 0x66d69f), (0x67acd0, 0x67acee), (0x67bf10, 0x67bf3d), (0x67b0d0, 0x67b10f)])
WINDOW, FONT, FONTVT, TEXT, BLOCK, OUT = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x400, RAM + 0x500, RAM + 0x600


def text(r):
    n = r.choice([0, 1, 2, 3, 5, 8, 12])
    return [r.choice([10, 10, 0x41, 0x62, 0x20, 0x7a, r.randrange(1, 0x10000)]) for _ in range(n)]


def wide(chars, size=0x40):
    return struct.pack('<%dH' % (len(chars) + 1), *chars, 0).ljust(size, b'\xcd')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    w = bytearray(r.bytes(0x150))
    put(w, 0x30, 'hhhh', *[r.choice([r.randrange(-50, 200)] * 6 + [-0x8000, 0x7fff]) for _ in range(4)])
    put(w, 0x13c, 'II', *[r.choice([0, 1, 1, 2, 2, 3, r.getrandbits(32)]) for _ in range(2)])
    regions[FONTVT] = vtable({1: 0, 2: 3})
    regions[FONT] = struct.pack('<I', FONTVT)
    put(w, 0x4c, 'I', FONT if r.random() < 0.85 else 0)
    for k in (1, 2):
        if r.random() < 0.8:
            set_return(log, k, r.choice([0, 1, 8, 12, 40, 0x7fff, 0x8000, r.randrange(0, 300)]) | r.choice([0, r.getrandbits(16) << 16]))
    chars = text(r)
    regions[TEXT] = wide(chars)
    regions[WINDOW] = w
    if op in ('SetAlignX', 'SetAlignY'):
        off = 0x13c if op == 'SetAlignX' else 0x140
        value = r.choice([struct.unpack_from('<I', w, off)[0], 0, 1, 2, r.getrandbits(32)])
        return Call(CALLS[op], [WINDOW, value], regions, conv='t', ret='v')
    if op == 'Place':
        put(w, 0x138, 'I', TEXT if r.random() < 0.85 else 0)
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='v')
    if op == 'TextX':
        return Call(CALLS[op], [WINDOW, TEXT if r.random() < 0.9 else 0], regions, conv='t', ret='w')
    if op == 'TextY':
        regions[OUT] = r.bytes(4)
        return Call(CALLS[op], [WINDOW, TEXT if r.random() < 0.9 else 0, r.choice([OUT, 0])], regions, conv='t', ret='w')
    if op == 'GetText':
        put(w, 0x138, 'I', BLOCK)
        regions[BLOCK] = wide(text(r))
        regions[OUT] = r.bytes(0x40)
        return Call(CALLS[op], [WINDOW, r.choice([OUT, OUT, 0]), r.randrange(0, 17)], regions, conv='t', ret='v')
    # SetText: the room about the length; the old block in RAM or 0 (only freed: Mem_Free hashes the pointer)
    put(w, 0x148, 'i', r.choice([len(chars), len(chars) - 1, len(chars) + 1, 0, -1, -0x80000000, r.randrange(0, 20)]))
    put(w, 0x138, 'I', r.choice([BLOCK, BLOCK, 0]))
    regions[BLOCK] = r.bytes(0x40)
    regions[HEAP] = heap(0x400, r.choice([0] * 6 + [1]))
    return Call(CALLS[op], [WINDOW, TEXT], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
