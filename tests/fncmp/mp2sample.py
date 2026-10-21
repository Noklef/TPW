# The MP2 sound samples' opens (src/game/core/mp2sample) against the originals, over the real decoders (their vtables 0x7118e0 / 0x70af00
# resolved in the driver): a sample at SAMPLE (its slots 6, 15 and 20 recorders: slot 6 the rate, now and then the decoder's own as its
# header gives it, else any; slot 15's word logged by its low byte, the pushed register's high half being unset), open already now and
# then, its length kept or not; the data a header (mp2frame.py's: a known standard, layer, bitrate and rate mostly) and bytes, from the
# buffer or a stream (+0x40: slot 1 a sequenced read of it, slot 2 a seek recorder); the pools empty (the heap, failing now and then)
# or holding a block of RAM; half rate (0xfb2708) now and then.
# usage: mp2sample.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, heap, import_table, log_region, seq_read, set_return, strings,  # noqa: E402
                   vtable)

CALLS = {'buffer': 0x6c84e0, 'stream': 0x6c88c0}
B = Bench([(0x6c84e0, 0x6c8624), (0x6c88c0, 0x6c8a1b), (0x6c8f00, 0x6c920b), (0x6c9260, 0x6c9270), (0x6d18e0, 0x6d1aa4), (0x6cc740, 0x6cc7a1),
           (0x6ccb10, 0x6ccc13), (0x6cc7b0, 0x6cc9f8), (0x6cca00, 0x6ccb0e), (0x5d60c0, 0x5d60c3), (0x5da3c0, 0x5da3c1)])
SAMPLE, SVT, STREAM_VT, OUT, DATA, BLOCK = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x2000
GLOBALS, HALF = 0xfb270c, 0xfb2708   # the pools' lists and locks; half rate
RATES = [44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000]
IMPORTS = {'EnterCriticalSection': (40, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (41, 0x6fd130, 1, 0, 0)}
RATE_K, SET_RATE_K, AFTER_K, READ_K, SEEK_K = 6, 15, 20, 1, 2
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def header(r):
    """A header and the decoder's rate it gives (None: unknown)."""
    if r.random() < 0.08:
        return r.getrandbits(32), None
    standard, index = r.choice([(0x180000, 0), (0x100000, 3), (0, 6)]), r.randrange(3)
    h = 0xffe00000 | r.getrandbits(21)
    h = h & ~0x1efc00 | standard[0] | r.randrange(1, 4) << 17 | r.randrange(1, 15) << 12 | index << 10
    return h, RATES[standard[1] + index]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    half = r.choice([0, 0, 1, r.getrandbits(32)])
    h, hz = header(r)
    if hz is not None and half:
        hz //= 2
    rate = r.getrandbits(32)
    if r.random() < 0.6 and hz is not None:
        rate = hz
    elif r.random() < 0.5:
        rate = r.choice([r.randrange(0, 50000), r.choice(RATES), r.choice(RATES) // 2])
    set_return(log, RATE_K, rate)
    set_return(log, SEEK_K, r.getrandbits(32))
    struct.pack_into('<I', log, LOG_BYTES + 4 * SET_RATE_K, 1)
    data = struct.pack('>I', h) + r.bytes(r.choice([r.randrange(0, 0x40), r.randrange(0x40, 0x400)]))
    sample = bytearray(r.bytes(0x60))
    struct.pack_into('<I', sample, 0, SVT)
    struct.pack_into('<I', sample, 0x10, DATA)
    struct.pack_into('<I', sample, 0x18, r.choice([r.randrange(0, 1 << 22), r.getrandbits(32), len(data)]))
    struct.pack_into('<I', sample, 0x14, r.choice([0, r.randrange(0x10000), r.getrandbits(32)]))
    struct.pack_into('<I', sample, 0x30, r.choice([r.getrandbits(32), r.randrange(0x100000)]))
    struct.pack_into('<I', sample, 0x34, 0 if r.random() < 0.9 else r.getrandbits(32))
    struct.pack_into('<I', sample, 0x40, STREAM_VT)
    sample[0x2c] = r.choice([0, 1, r.getrandbits(8)])
    if name == 'stream':
        seq_read(log, READ_K, DATA, len(data), buffer=0, count=1)
        strings(log, READ_K, 0, out=1)
    globals_ = bytearray(0x10)
    for g in range(2):
        struct.pack_into('<II', globals_, 8 * g, BLOCK if r.random() < 0.4 else 0, r.getrandbits(32))
    block = bytearray(r.bytes(0x2c90))
    struct.pack_into('<I', block, 0x2404, 0)
    struct.pack_into('<I', block, 0x2478, 0)
    regions = {SAMPLE: sample, SVT: vtable({RATE_K: 0, SET_RATE_K: 1, AFTER_K: 0}), STREAM_VT: vtable({READ_K: 2, SEEK_K: 1}), OUT: r.bytes(4),
               DATA: data, BLOCK: block, GLOBALS: globals_, HALF: struct.pack('<I', half), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS),
               HEAP: heap(0x6000, fail=r.choice([0] * 12 + [1]))}
    return Call(CALLS[name], [SAMPLE, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
