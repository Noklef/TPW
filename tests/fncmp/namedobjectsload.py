# The start-up's data loaded (src/game/core/namedobjects: 0x407f20 NamedObjects_LoadData) against the original, the Strings and the heap real: the base folder (cdecl k 3), Quickload_Locate (cdecl k 4), the scape's objects (cdecl k 5), the sky (k 6), the texture list (k 7) and the balance files' load (k 8) recorders.
# The folder with its backslash or without, the two Strings; the render flag, the count about 0, +0x38 set or not; the allocation of the list failing now and then; either load failing.
# usage: namedobjectsload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, global_heap, heap, log_region, put, set_return, strings  # noqa: E402
from texdirs import merged  # noqa: E402

BASE, LOCATE, SCAPE, SKY, LIST, LOAD = 3, 4, 5, 6, 7, 8
B = Bench(merged([(0x407f20, 0x4080d4), (0x5f5fc0, 0x5f7f00), (0x5c5480, 0x5c5510), (0x5c5520, 0x5c5530), (0x424cf0, 0x424d10), (0x424d30, 0x424db0), (0x67a400, 0x690000), (0x5da3c0, 0x5da3c1)]),
          hooks={0x40ef40: _recorder(BASE, 0, cdecl=True), 0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x550e00: _recorder(SCAPE, 1, cdecl=True), 0x5852b0: _recorder(SKY, 1), 0x418c30: _recorder(LIST, 0), 0x401650: _recorder(LOAD, 2)})
SELF, FOLDER, LOADS = RAM, RAM + 0x100, RAM + 0x200


def string(regions, at, text):
    regions[at] = put(bytearray(0x14), 0, 'IIIII', 0x703090, len(text), at + 0x20, len(text) + 1, 8) + bytes(0xc) + text + bytes(0x10)


def case(r):
    log = log_region()
    folder = r.choice([b'C:\\tp', b'C:\\tp\\', b'', b'x'])
    regions = {LOG: log, HEAP: heap(0x8000, r.choice([0] * 12 + [2, 3, 4, 5])), **global_heap(), SELF: put(bytearray(r.bytes(0x40)), 0x38, 'I', r.choice([0, 0, r.getrandbits(32)])), FOLDER: folder + b'\0',
               LOADS: struct.pack('<40I', *[r.choice([1, 1, 1, 0x100, 0]) for _ in range(40)]), 0x8bcbc8: struct.pack('<I', r.getrandbits(32) | 0x400000 if r.random() < 0.7 else r.getrandbits(32) & ~0x400000),
               0x785724: struct.pack('<i', r.choice([1, 2, 0x7fffffff, 0, -1]))}
    string(regions, 0xf80be0, b'\\')                                       # (the global backslash)
    string(regions, 0x786bd8, r.choice([b'balance.txt', b'', b'data\\a']))
    string(regions, 0x7868c8, r.choice([b'b.txt', b'', b'data\\b']))
    set_return(log, BASE, FOLDER)
    set_return(log, LOCATE, r.getrandbits(32))
    answers(log, LOAD, LOADS)
    strings(log, LOCATE, 1)
    strings(log, LOAD, 1)
    return Call(0x407f20, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
