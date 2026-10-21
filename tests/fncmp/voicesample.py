# The voice-coded sound samples (src/game/core/voicesample) and the samples' shared slots (src/game/core/soundsample) against the
# originals: a sample at SAMPLE whose vtable is a recorder (slot 17 the channels: 1-3 mostly, any byte in --wild; slot 19's answer any,
# slot 6 the sample rate), a recorder next stream now and then. The decode / read over a decoder of two channels (as voicecodec.py's
# frame: its samples floats up to 1, 30000, 100000 or past the int range, special ones now and then), a memory reader over random bits
# (the end pointer within them, past them, 0 or ~0), the frame played 0x1b0 (decoded first), 0x1af or any below, bytes up to 0x400; the
# read's end time (+0x1c) set now and then (timeGetTime a recorder, the clock disabled; the end time now, a ms either side or any). The opens: a reader already there or a new one
# (Mem_Alloc failing now and then; the buffered reader refilled from the sub-stream's slot 1, a recorder), +0x34 any. The close's
# decoder / reader any or 0 (freed in the heap stand-in). The end time's flag 0 mostly, the format byte any, the rate 11025-44100 or any.
# usage: voicesample.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, heap, log_region, recorder, set_return, vtable  # noqa: E402

# name: (address, returns)
CALLS = {'decode': (0x6c7cf0, 'a'), 'read': (0x6c7c20, 'a'), 'query': (0x6c7bc0, 'a'), 'restart': (0x6c7ce0, 'v'),
         'close': (0x6c7be0, 'v'), 'open': (0x6c7c80, 'v'), 'sopen': (0x6c80c0, 'v'), 'srestart': (0x6c8120, 'v'),
         'schedule': (0x6c7010, 'b'), 'drained': (0x6c7070, 'a'), 'describe': (0x6bb1d0, 'v')}
B = Bench([(0x6c7bc0, 0x6c7dfb), (0x6c80c0, 0x6c8138), (0x6c7010, 0x6c7082), (0x6bb1d0, 0x6bb1dd), (0x6c6230, 0x6c625a),
           (0x6c62c0, 0x6c62d8), (0x6d4ec0, 0x6d4ed9), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa),
           (0x6d4ee0, 0x6d4f2a), (0x6d4fa0, 0x6d4fdc), (0x6d4f30, 0x6d4f99), (0x6d4fe0, 0x6d503a), (0x6d5200, 0x6d5254),
           (0x6d5040, 0x6d51ed), (0x6d5260, 0x6d5341), (0x6d5350, 0x6d5443), (0x6d61c0, 0x6d6285),
           (0x6d57e0, 0x6d61ba), (0x6d5450, 0x6d57d2), (0x6d6290, 0x6d62bd)])
SAMPLE, SVT, NEXT, SUBVT, FILEVT, OUT, RECORD, MEMREADER = (RAM + 0x100 * i for i in range(8))
BUFFER, DECODER, BITS, BREADER = RAM + 0x800, RAM + 0x1000, RAM + 0x3000, RAM + 0x4000
MEMORY_VTABLE, BUFFERED_VTABLE = 0x70adb8, 0x70ae28
CLOCK, TIME_GET_TIME, TIME_K = 0xfa71e0, 0x6fd3ec, 12
SLOTS = {0: 1, 2: 0, 5: 1, 6: 0, 17: 0, 19: 2, 20: 1}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def floats(r, n, scale):
    special = r.choice([0, 0, 0.02, 0.2])
    return struct.pack('<%dI' % n, *[r.f(scale, special) for _ in range(n)])


def decoder(r):
    """A decoder as voicecodec.py's frame: the state over BITS, +0xc / +0x10, two channels."""
    d = bytearray(struct.pack('<3I', BITS + r.randrange(0, 0x40), r.getrandbits(32), r.choice([r.randrange(8, 33)] * 6 + [r.randrange(0, 8)])))
    d += r.bytes(4) + struct.pack('<2I', r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 1, 0x100, r.getrandbits(32)]))
    for _ in range(2):
        d += struct.pack('<I', r.choice([r.randrange(0, 0x48), r.getrandbits(32)])) + floats(r, 64, r.choice([1.0, 100.0]))
        d += floats(r, 12, r.choice([1.0, 0.5])) + floats(r, 0x300, r.choice([1.0, 30000.0, 100000.0, 3e9]))
    return d


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    sample = bytearray(r.bytes(0x60))
    struct.pack_into('<II', sample, 0, SVT, r.choice([NEXT, NEXT, 0]))
    struct.pack_into('<I', sample, 0x2c, MEMREADER)
    struct.pack_into('<I', sample, 0x1c, 0)
    struct.pack_into('<I', sample, 0x38, DECODER)
    struct.pack_into('<I', sample, 0x40, SUBVT)
    struct.pack_into('<I', sample, 0x4c, FILEVT)
    for k in (5, 6, 19, 20):
        if r.getrandbits(1):
            set_return(log, k, r.getrandbits(32))
    now = r.getrandbits(32)   # (timeGetTime's: Time_GetMs answers it with the clock disabled)
    set_return(log, TIME_K, now)
    end = r.choice([now, now, (now + 1) & 0xffffffff, (now - 1) & 0xffffffff, r.getrandbits(32)])
    regions = {SVT: vtable(SLOTS), NEXT: struct.pack('<I', SVT) + r.bytes(0x10), SUBVT: vtable({1: 2}), FILEVT: vtable({4: 2}),
               OUT: r.bytes(4), RECORD: r.bytes(0x48), MEMREADER: struct.pack('<2I', MEMORY_VTABLE, 0), LOG: log,
               CLOCK: bytes(0x20), TIME_GET_TIME: struct.pack('<I', recorder(TIME_K, 0, True)), HEAP: heap(0x2100, fail=r.choice([0, 0, 0, 1]))}
    args = []
    if name in ('decode', 'read'):
        channels = r.choice([1, 1, 2, 2, 3])
        if r.wild and r.random() < 0.05:
            channels = r.getrandbits(32)
        set_return(log, 17, channels)
        set_return(log, 19, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        struct.pack_into('<I', sample, 0x30, r.choice([0x1b0, 0x1b0, 0x1af, r.randrange(0, 0x1b0)]))
        struct.pack_into('<I', sample, 0x3c, r.choice([BITS + r.randrange(0, 0x100), BITS + r.randrange(0, 0x100), BITS + 0x1000, 0, 0xffffffff]))
        if name == 'read' and r.random() < 0.25:
            struct.pack_into('<I', sample, 0x1c, end or 1)
        regions[DECODER] = decoder(r)
        regions[BITS] = r.bytes(0x1000)
        regions[BUFFER] = r.bytes(0x800)
        count = r.choice([0, 1, 2, 3, r.randrange(4, 0x40), r.randrange(0x40, 0x400), r.randrange(0x40, 0x400)])
        if r.wild and r.random() < 0.05:
            count = r.getrandbits(32)
        args = [BUFFER, count, OUT] + ([r.getrandbits(32)] if name == 'read' else [])
    elif name == 'query':
        args = [OUT, RECORD]
    elif name == 'close':
        struct.pack_into('<II', sample, 0x2c, r.choice([0, r.getrandbits(32)]), r.getrandbits(32))
        struct.pack_into('<I', sample, 0x38, r.choice([0, r.getrandbits(32)]))
    elif name in ('open', 'sopen'):
        struct.pack_into('<I', sample, 0x34, r.choice([0, 1, r.getrandbits(32)]))
        regions[DECODER] = r.bytes(0x1a7c)
        if name == 'open':
            struct.pack_into('<I', sample, 0x2c, r.choice([0, MEMREADER]))
            regions[BITS] = r.bytes(0x40)
            args = [BITS + r.randrange(0, 0x10)]
        else:
            reader = bytearray(r.bytes(0x2010))
            struct.pack_into('<I', reader, 0, BUFFERED_VTABLE)
            struct.pack_into('<I', reader, 0x2008, SAMPLE + 0x40)
            regions[BREADER] = reader
            struct.pack_into('<I', sample, 0x2c, r.choice([0, 0, BREADER]))
            fill(log, 1, 0, r.bytes(0x70))
            args = [r.getrandbits(32)]
    elif name == 'schedule':
        sample[0x20] = r.choice([0, 2, 0x40, 0x42, r.getrandbits(8)])
        set_return(log, 6, r.choice([11025, 22050, 44100, 44100, 1, r.getrandbits(32) | 1]))
        args = [r.choice([0, 0, 0x100, 1, r.getrandbits(32)]), r.choice([0, 1, r.randrange(2, 0x10000), r.getrandbits(32)])]
    elif name == 'drained':
        struct.pack_into('<I', sample, 0x1c, end)
    elif name == 'describe':
        args = [RECORD]
    regions[SAMPLE] = sample
    return Call(address, [SAMPLE] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
