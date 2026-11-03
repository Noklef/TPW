# One action of the coaster's action replay (src/game/coaster/arreplay: 0x4045b0) against the original: each kind with its actions or another, the stream (a sequenced read, slot 2 of its vtable) whole or cut short.
# Recorders (driver.cpp's kArReplayEnv): the read 2, then 3-17 the calls. Words read into a dword leave stack garbage above them, so those arguments are logged as low words (the byte one as its low byte).
# usage: arreplay.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, blob_bytes, log_region, put, seq_read, strings, vtable  # noqa: E402

HOOKS = {0x4479a0: _recorder(3, 0, cdecl=True), 0x447a10: _recorder(4, 0, cdecl=True), 0x447d30: _recorder(5, 0, cdecl=True), 0x447d90: _recorder(6, 0, cdecl=True), 0x524790: _recorder(7, 4, cdecl=True), 0x524960: _recorder(8, 4, cdecl=True),
         0x404d80: _recorder(9, 2), 0x404f70: _recorder(10, 4), 0x52f880: _recorder(11, 2, cdecl=True), 0x444190: _recorder(12, 2, cdecl=True), 0x444220: _recorder(13, 2, cdecl=True), 0x4442f0: _recorder(14, 3, cdecl=True),
         0x447b60: _recorder(15, 2, cdecl=True), 0x447e30: _recorder(16, 2, cdecl=True), 0x447bb0: _recorder(17, 3, cdecl=True)}
B = Bench([(0x4045b0, 0x404d80), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
SELF, VT, DATA = RAM, RAM + 0x800, RAM + 0x1000
ACTIONS = {0: [0xf, 0x14], 1: [0x11], 2: [0x28, 0x2d], 3: [5], 4: [0x39, 0x3a], 5: [0x3b], 6: [0x3f, 0x45], 7: [0x3c], 8: [0x37, 0x38, 0x40, 0x41], 9: [0x20], 10: [0x21], 11: [0xe7]}
SIZES = {0: [1, 1], 1: [2], 2: [1, 1, 2], 3: [2, 1], 4: [4, 2], 5: [4, 4, 2], 6: [2, 2], 7: [2, 2, 2], 8: [], 9: [1], 10: [2, 1], 11: [0x80]}


def word_args(log, k, mask):
    struct.pack_into('<I', log, LOG_BYTES + 4 * k, mask)


def case(r):
    log = log_region()
    kind = r.deal(list(range(12)))
    action = r.choice(ACTIONS[kind]) if r.random() < 0.8 else r.randrange(0x100)
    table = list(struct.unpack('<256I', r.bytes(0x400)))
    table[action] = kind if r.random() < 0.97 else r.choice([12, 0xffffffff])
    self_ = bytearray(r.bytes(0x34)) + struct.pack('<256I', *table)
    put(self_, 0xc, 'I', VT)
    data = bytearray([action])
    for size in SIZES[kind]:
        data += r.bytes(size)
    n = 0
    if kind in (9, 10):
        n = r.choice([0, 1, 2, 7, 8, 0x20, 0x40])
        data[-1] = n
        data += r.bytes(n * 2)
    whole = len(data)
    if r.random() < 0.2:
        data = data[:r.randrange(0, whole)]
    data = bytes(data)
    seq_read(log, 2, DATA, len(data), buffer=0, count=1)
    strings(log, 2, 0, out=1)   # (the buffer's a stack address)
    word_args(log, 7, 1 << 18)
    word_args(log, 8, 1 << 18)
    word_args(log, 9, 1 << 17)
    word_args(log, 10, 1 << 17 | 1 << 2)
    blob(log, 10, 3, (1 << (n * 2 // 4)) - 1)
    blob_bytes(log, 11, 0, 3)
    word_args(log, 15, 1 << 16 | 1 << 17)
    word_args(log, 17, 1 << 16 | 1 << 17 | 1 << 18)
    regions = {SELF: self_, VT: vtable({2: 2}), DATA: data + b'\0', LOG: log}
    return Call(0x4045b0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
