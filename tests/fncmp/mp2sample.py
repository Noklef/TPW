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
from fncmp import (HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, echo, fill, heap, import_table, log_region, recorder, seq_read,  # noqa: E402
                   set_return, strings, vtable)

CALLS = {'buffer': 0x6c84e0, 'stream': 0x6c88c0, 'read': 0x6c83c0, 'query': 0x6c8340, 'close': 0x6c83a0, 'restart': 0x6c84a0,
         'srestart': 0x6c8b10, 'freedec': 0x6c8ae0}
SLOT_RETS = {'read': 'a', 'query': 'a', 'close': 'v', 'restart': 'v', 'srestart': 'v', 'freedec': 'v'}
B = Bench([(0x6c8340, 0x6c84d1), (0x6c8ae0, 0x6c8b2c), (0x6c7070, 0x6c7082), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa), (0x6c6230, 0x6c625a),
           (0x6d4ec0, 0x6d4ed9), (0x6c84e0, 0x6c8624), (0x6c88c0, 0x6c8a1b), (0x6c8f00, 0x6c920b), (0x6c9260, 0x6c9270), (0x6d18e0, 0x6d1aa4), (0x6cc740, 0x6cc7a1),
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


SVT2, NEXT, DVT, SUBVT, FILEVT, RECORD = (RAM + 0x3000 + 0x100 * i for i in range(6))
BUFFER, DECODER = RAM + 0x3800, RAM + 0x8000
CLOCK, TIME_GET_TIME, TIME_K, CLOSE_K, OPEN_K, FREE_K, FEED_K, END_K = 0xfa71e0, 0x6fd3ec, 12, 2, 21, 22, 23, 19


def slots_case(r, name):
    """The shared slots: the sample's recorder vtable (slot 21 the open writing a status through its out, negative now and then; slot 19
    answering 0, 1 or a zero low byte; 23 and 22 logged; 0 and 2 for SoundStream_Close, a next stream (+4) now and then); +0x1c set now
    and then (the clock: Time_GetMs through timeGetTime); the byte +0x3c any way; the decoder (+0x34; null now and then for the freeing)
    its byte +0x2442 0-2 or any, +0x2420 0x30 or not, its own recorder vtable; the position about the length, the bytes about what is
    left (up to 0x400); the record's +0x14 bits 6 and 11 any way; the sub-stream at +0x40 (its file's slot 4 the seek)."""
    log = log_region()
    sample = bytearray(r.bytes(0x60))
    struct.pack_into('<II', sample, 0, SVT2, r.choice([NEXT, 0, 0]))
    length = r.choice([0, r.randrange(1, 0x400), r.randrange(0x400, 0x10000), r.getrandbits(32)])
    pos = r.choice([0, length, length - 1, length + 1, length - r.randrange(0x400), r.randrange(0x10000), r.getrandbits(32)]) & 0xffffffff
    now = r.getrandbits(32)
    set_return(log, TIME_K, now)
    struct.pack_into('<4I', sample, 0x14, pos, r.getrandbits(32), r.getrandbits(32), 0 if r.random() < 0.75 else r.choice([now, now + 1, now - 1, 1]) & 0xffffffff or 1)
    struct.pack_into('<II', sample, 0x30, length, DECODER if r.random() < 0.9 or name in ('read', 'query', 'restart', 'srestart') else 0)
    sample[0x3c] = r.choice([0, 1, r.getrandbits(8)])
    struct.pack_into('<I', sample, 0x40, SUBVT)
    struct.pack_into('<I', sample, 0x4c, FILEVT)
    decoder = bytearray(r.bytes(0x2480))
    struct.pack_into('<I', decoder, 0, DVT)
    decoder[0x2442] = r.choice([0, 1, 1, 2, 2, r.getrandbits(8)])
    struct.pack_into('<I', decoder, 0x2420, r.choice([0x30, 0x30, 0x20, r.getrandbits(32)]))
    strings(log, OPEN_K, 0, out=1)
    fill(log, OPEN_K, 0, struct.pack('<I', r.choice([0, 1, 0x80000000, 0xffffffff, r.getrandbits(32)])))
    echo(log, OPEN_K, 0)
    set_return(log, END_K, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    record = bytearray(r.bytes(0x20))
    struct.pack_into('<I', record, 0x14, r.getrandbits(32) & ~0x840 | r.choice([0, 0x40, 0x800, 0x840]))
    regions = {SAMPLE: sample, SVT2: vtable({0: 1, CLOSE_K: 0, END_K: 2, OPEN_K: 1, FREE_K: 0, FEED_K: 2}), NEXT: struct.pack('<I', SVT2) + r.bytes(8),
               DECODER: decoder, DVT: vtable({0: 1, 2: 0}), SUBVT: vtable({}), FILEVT: vtable({4: 2}), RECORD: record, BUFFER: r.bytes(0x400),
               OUT: r.bytes(4), LOG: log, CLOCK: bytes(0x20), TIME_GET_TIME: struct.pack('<I', recorder(TIME_K, 0, True))}
    if name == 'read':
        bytes_ = r.choice([0, 1, r.randrange(0x400), 0x400, (length - pos) & 0xffffffff]) if (length - pos) & 0xffffffff <= 0x400 else r.choice([0, 1, r.randrange(0x401)])
        return Call(CALLS[name], [SAMPLE, BUFFER, min(bytes_, 0x400), OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'query':
        return Call(CALLS[name], [SAMPLE, OUT, RECORD], regions, conv='t', ret='a')
    return Call(CALLS[name], [SAMPLE], regions, conv='t', ret='v')


def case(r):
    name = r.deal(OPS)
    if name in SLOT_RETS:
        return slots_case(r, name)
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
