# The voice-coded sound samples (src/game/core/voicesample) and the samples' shared slots (src/game/core/soundsample) against the
# originals: a sample at SAMPLE whose vtable is a recorder (slot 17 the channels: 1-3 mostly, any byte in --wild; slot 19's answer any,
# slot 6 the sample rate), a recorder next stream now and then. The decode / read over a decoder of two channels (as voicecodec.py's
# frame: its samples floats up to 1, 30000, 100000 or past the int range, special ones now and then), a memory reader over random bits
# (the end pointer where the frame leaves the reader (a probe of the original) or one either side, the start, within the bits, past them, 0 or ~0; the samples the clamp's edges and halves now and then), the frame played 0x1b0 (decoded first), 0x1af or any below, bytes up to 0x400; the
# read's end time (+0x1c) set now and then (timeGetTime a recorder, the clock disabled; the end time now, a ms either side or any). The opens: a reader already there or a new one
# (Mem_Alloc failing now and then; the buffered reader refilled from the sub-stream's slot 1, a recorder), +0x34 any. The close's
# decoder / reader any or 0 (freed in the heap stand-in). The end time's flag 0 mostly, the format byte any, the rate 11025-44100 or any.
# The destructors: the source at +0x24 an object whose slot 0 is a recorder, or none; the sub-stream's stream source (+0x50) a RefObject
# (InterlockedDecrement answering 1, 0 or -1; its slots recorders) or none, CloseHandle a recorder; the deletes' pools (0xfb20bc-0xfb20cc)
# with a free list or none (Enter / LeaveCriticalSection recorders), flags any. The sample stream's constructor (0x6c6e70: a record's format,
# the source's slot 1 a recorder; the real vtable 0x70ac50's slot 15) and its deleting destructors (0x6c70c0, 0x6c70a0), flags any.
# The voice, file and MP2 samples' constructors (as it, +4 the data's length any, +0x1b bit 2 now and then; the stream a StreamBase over
# REF: InterlockedIncrement and CreateSemaphoreA recorders; the start any) and the file / MP2 samples' destructors (as the others; the
# file sample's window at +0x2c, its source at +0x3c; the pools 0xfb20b0 and 0xfb26d8-0xfb26eb).
# The memory copier's install (0x6c3f70: the MMX switch 0xfb2704 any, the block failing now and then) and the MMX copy (0x6c3fc0: runs
# within one buffer, overlapping either way by a few bytes now and then; counts about the 32-byte blocks). The sound base's constructor
# (0x6b7390) that installs it, on the sample's bytes. The sound objects' new channels (0x6b8410, 0x6b9db0: a channel or an EAX one at +0x24, Mem_Alloc
# failing now and then).
# usage: voicesample.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, heap, log_region, recorder, set_return, vtable  # noqa: E402

# name: (address, returns)
CALLS = {'decode': (0x6c7cf0, 'a'), 'read': (0x6c7c20, 'a'), 'query': (0x6c7bc0, 'a'), 'restart': (0x6c7ce0, 'v'),
         'close': (0x6c7be0, 'v'), 'open': (0x6c7c80, 'v'), 'sopen': (0x6c80c0, 'v'), 'srestart': (0x6c8120, 'v'),
         'schedule': (0x6c7010, 'b'), 'drained': (0x6c7070, 'a'), 'describe': (0x6bb1d0, 'v'), 'sdtor': (0x6c6ef0, 'v'),
         'dtor': (0x6c8330, 'v'), 'sdtor2': (0x6c8000, 'v'), 'ddtor': (0x6c7ba0, 'a'), 'sddtor': (0x6c7fe0, 'a'),
         'sctor': (0x6c6e70, 'a'), 'sdel': (0x6c70c0, 'a'), 'basedel': (0x6c70a0, 'a'),
         'memread': (0x6c7250, 'a'), 'fileread': (0x6c7890, 'a'), 'filerewind': (0x6c7920, 'a'),
         'vctor': (0x6c7b20, 'a'), 'vsctor': (0x6c7f70, 'a'), 'fctor': (0x6c7780, 'a'), 'mctor': (0x6c82c0, 'a'), 'msctor': (0x6c87c0, 'a'),
         'fdtor': (0x6c7820, 'v'), 'fddtor': (0x6c7800, 'a'), 'mddtor': (0x6c8310, 'a'), 'msdtor': (0x6c8850, 'v'), 'msddtor': (0x6c8830, 'a'),
         'copier': (0x6c3f70, 'a'), 'mmxcopy': (0x6c3fc0, 'v'), 'basector': (0x6b7390, 'a'), 'chanctor': (0x6b8ed0, 'a'),
         'newchan': (0x6b8410, 'a'), 'neweaxchan': (0x6b9db0, 'a')}
CTORS = ('vctor', 'vsctor', 'fctor', 'mctor', 'msctor')
DTORS = ('sdtor', 'dtor', 'sdtor2', 'ddtor', 'sddtor', 'sdel', 'fdtor', 'fddtor', 'mddtor', 'msdtor', 'msddtor')
B = Bench([(0x6c7bc0, 0x6c7dfb), (0x6c80c0, 0x6c8138), (0x6c7010, 0x6c7082), (0x6bb1d0, 0x6bb1dd), (0x6c6230, 0x6c625a),
           (0x6c62c0, 0x6c62d8), (0x6d4ec0, 0x6d4ed9), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa),
           (0x6d4ee0, 0x6d4f2a), (0x6d4fa0, 0x6d4fdc), (0x6d4f30, 0x6d4f99), (0x6d4fe0, 0x6d503a), (0x6d5200, 0x6d5254),
           (0x6d5040, 0x6d51ed), (0x6d5260, 0x6d5341), (0x6d5350, 0x6d5443), (0x6d61c0, 0x6d6285),
           (0x6d57e0, 0x6d61ba), (0x6d5450, 0x6d57d2), (0x6d6290, 0x6d62bd), (0x6c6ef0, 0x6c6f37), (0x6c8330, 0x6c8335),
           (0x6c8000, 0x6c806a), (0x6c7ba0, 0x6c7bc0), (0x6c7fe0, 0x6c7ffe), (0x6c7ad0, 0x6c7b15), (0x6c7f20, 0x6c7f65),
           (0x5f71e0, 0x5f7220), (0x618d30, 0x618d90), (0x6307e0, 0x630800),
           (0x6c6e70, 0x6c6eed), (0x6c70a0, 0x6c70dc), (0x6c7090, 0x6c7097), (0x6c6f40, 0x6c6f87), (0x6c6f90, 0x6c6fee),
           (0x6c7250, 0x6c72e9), (0x6c7890, 0x6c7911), (0x6c7920, 0x6c7925),
           (0x6c7b20, 0x6c7b9e), (0x6c7f70, 0x6c7fd7), (0x6c7780, 0x6c77f2), (0x6c7800, 0x6c788a), (0x6c82c0, 0x6c8301), (0x6c8310, 0x6c832e),
           (0x6c87c0, 0x6c882c), (0x6c8830, 0x6c88ba), (0x6d4d10, 0x6d4d72), (0x6d4d80, 0x6d4de2), (0x5f71b0, 0x5f71d1), (0x618d20, 0x618d2b),
           (0x6307c0, 0x6307d7), (0x6c7730, 0x6c7775), (0x6c8270, 0x6c82b5), (0x6c8770, 0x6c87b5), (0x6c3f70, 0x6c3fbf), (0x6c3fc0, 0x6c400f),
           (0x6b7390, 0x6b73ed), (0x6338b0, 0x6338c0), (0x6b8ed0, 0x6b8f5b), (0x5f61a0, 0x5f61f0), (0x6b8410, 0x6b8472), (0x6b9db0, 0x6b9e2b)])
SAMPLE, SVT, NEXT, SUBVT, FILEVT, OUT, RECORD, MEMREADER = (RAM + 0x100 * i for i in range(8))
# (the buffer ends at the regions' 64 KB chunk: a wild count's frames run off it there at once, in both, where at RAM + 0x800 the
# original wrote on through the decoder and the rest of the chunk, 0x7c00 samples and 37 frames decoded, past the emulator's 5M steps)
BUFFER, DECODER, BITS, BREADER = RAM + 0xf800, RAM + 0x1000, RAM + 0x3000, RAM + 0x4000
MEMORY_VTABLE, BUFFERED_VTABLE = 0x70adb8, 0x70ae28
CLOCK, TIME_GET_TIME, TIME_K = 0xfa71e0, 0x6fd3ec, 12
EDGES = [32767, 32768, -32768, -32769, 65535, 65536, 98303, 98304, -65536, -65537, 131071, 131072, -0.5, 0.5, 1.5, 2.5, -1.5]
SLOTS = {0: 1, 2: 0, 5: 1, 6: 0, 17: 0, 19: 2, 20: 1}
SOURCE, SOURCE_VT, REF, REF_VT = RAM + 0x7000, RAM + 0x7100, RAM + 0x7200, RAM + 0x7300
POOLS = 0xfb20b0   # the file sample pool's list and lock, 4 bytes, the voice sample pool's, 4 bytes, the voice stream pool's
MP2_POOLS = 0xfb26d8   # the MP2 memory sample pool's list and lock, 4 bytes, the MP2 streamed sample pool's
STREAM = RAM + 0x7600   # the constructors' stream (a StreamBase: its source REF)
FREE = RAM + 0x7400   # a block on a pool's free list
COPIER, COPIER_VT, COPIER_AT = RAM + 0x7500, RAM + 0x7510, 0xfb2018   # the memory copier and where it is kept
# import: (recorder slot, import table address, stack arguments)
IMPORTS = {'InterlockedDecrement': (34, 0x6fd12c, 1), 'CloseHandle': (33, 0x6fd13c, 1), 'EnterCriticalSection': (35, 0x6fd140, 1),
           'LeaveCriticalSection': (36, 0x6fd130, 1), 'CreateSemaphoreA': (37, 0x6fd16c, 4), 'InterlockedIncrement': (38, 0x6fd0c0, 1)}
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
        samples = bytearray(floats(r, 0x300, r.choice([1.0, 30000.0, 100000.0, 3e9])))
        if r.getrandbits(1):   # (the clamp's edges and halves: the played samples are floats 0x150-0x2ff)
            for i in range(0x150, 0x300):
                if r.random() < 0.25:
                    struct.pack_into('<f', samples, 4 * i, r.choice(EDGES))
        d += floats(r, 12, r.choice([1.0, 0.5])) + samples
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
    if name in ('copier', 'basector', 'chanctor', 'newchan', 'neweaxchan'):   # (chanctor: the channel's constructor over the base's, its string's block from the heap)
        regions[COPIER_AT] = r.bytes(4)
        regions[0xfb2704] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
        if name in ('newchan', 'neweaxchan'):   # (the sound object's new channel at +0x24: a block of the heap, or none when it fails)
            regions[SAMPLE] = r.bytes(0x30)
            return Call(address, [SAMPLE], regions, conv='t', ret=ret)
        regions[SAMPLE] = sample + (r.bytes(8) if name == 'chanctor' else b'')   # (the object itself compared: the constructor's fields; the channel's to +0x68)
        return Call(address, [SAMPLE], regions, conv='t', ret=ret)
    if name == 'mmxcopy':
        regions[BUFFER] = r.bytes(0x200)
        source = r.randrange(0x40, 0xc0)
        dest = source + r.choice([0, 1, -1, 4, -4, 7, -7, 8, -8, 9, 31, -33, r.randrange(-0x40, 0x40)]) if r.random() < 0.6 else r.randrange(0, 0x100)
        count = r.choice([0, 1, 31, 32, 33, 63, 64, 65, 96, r.randrange(0, 0x100)])
        return Call(address, [r.getrandbits(32), BUFFER + dest, BUFFER + source, count], regions, conv='t', ret=ret)
    if name in ('decode', 'read'):
        channels = r.choice([1, 1, 2, 2, 3])
        if r.wild and r.random() < 0.05:
            channels = r.getrandbits(32)
        set_return(log, 17, channels)
        set_return(log, 19, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        struct.pack_into('<I', sample, 0x30, r.choice([0x1b0, 0x1b0, 0x1af, r.randrange(0, 0x1b0)]))
        if name == 'read' and r.random() < 0.25:
            struct.pack_into('<I', sample, 0x1c, end or 1)
        regions[DECODER] = decoder(r)
        start = struct.unpack_from('<I', regions[DECODER], 0)[0]
        struct.pack_into('<I', sample, 0x3c, r.choice([start, start + 1, BITS + r.randrange(0, 0x100), BITS + 0x1000, 0, 0xffffffff]))
        regions[BITS] = r.bytes(0x1000)
        regions[BUFFER] = r.bytes(0x800)
        if channels & 0xff and r.random() < 0.3:   # (the end where the frame leaves the reader: a probe of the original over a copy)
            struct.pack_into('<I', sample, 0x30, 0x1b0)
            struct.pack_into('<I', sample, 0x3c, 0xffffffff)
            probe = {k: bytes(v) for k, v in regions.items()}
            probe[SAMPLE] = bytes(sample)
            if not isinstance(B.original(Call(0x6c7cf0, [SAMPLE, BUFFER, 2 * (channels & 0xff), OUT], probe, conv='t', ret='a')), str):
                left = B.emu.u32(DECODER)
                struct.pack_into('<I', sample, 0x3c, r.choice([left, left, left + 1, left - 1]) & 0xffffffff)
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
    elif name == 'sctor':   # (the record a format: its word +0x18 a rate or any, +0x1a 16 bits or not, +0x1b bit 0; the source's slot 1)
        rec = bytearray(r.bytes(0x48))
        struct.pack_into('<H', rec, 0x18, r.choice([0x5622, 0xac44, 0x2b11, r.getrandbits(16)]))
        rec[0x1a] = r.choice([0x10, 8, r.getrandbits(8)])
        rec[0x1b] = r.choice([0, 1, r.getrandbits(8)])
        regions[RECORD] = rec
        regions[SOURCE] = struct.pack('<I', SOURCE_VT) + r.bytes(0xc)
        regions[SOURCE_VT] = struct.pack('<2I', recorder(30, 0), recorder(29, 0))
        struct.pack_into('<I', sample, 0x1c, r.getrandbits(32))   # (the end time: the constructor's to clear)
        args = [RECORD, SOURCE]
    elif name == 'memread':   # (+0x10 the data, +0x14 the position, +0x18 the length; the copier's slot 0 and slot 19 recorders)
        if r.random() < 0.25:
            struct.pack_into('<I', sample, 0x1c, end or 1)
        length = r.choice([0, 1, 0x100, r.randrange(0, 0x10000), r.getrandbits(32)])
        pos = r.choice([0, length, length - 1, r.randrange(0, length + 1), r.getrandbits(32)]) & 0xffffffff
        struct.pack_into('<3I', sample, 0x10, r.getrandbits(32), pos, length)
        set_return(log, 19, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        regions[COPIER] = struct.pack('<I', COPIER_VT) + r.bytes(0xc)
        regions[COPIER_VT] = struct.pack('<I', recorder(25, 3))
        regions[COPIER_AT] = struct.pack('<I', COPIER)
        left = (length - pos) & 0xffffffff
        args = [BUFFER, r.choice([0, 1, left, left + 1, left - 1, r.getrandbits(32)]) & 0xffffffff, OUT, r.getrandbits(32)]
    elif name == 'fileread':   # (the window at +0x2c: SUBVT's slot 1 a recorder, its position +0x30; +0x18 the length)
        if r.random() < 0.25:
            struct.pack_into('<I', sample, 0x1c, end or 1)
        length = r.choice([0, 1, 0x100, r.randrange(0, 0x10000), r.getrandbits(32)])
        pos = r.choice([0, length, length - 1, r.randrange(0, length + 1), r.getrandbits(32)]) & 0xffffffff
        struct.pack_into('<I', sample, 0x18, length)
        struct.pack_into('<II', sample, 0x2c, SUBVT, pos)
        set_return(log, 19, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        left = (length - pos) & 0xffffffff
        args = [BUFFER, r.choice([0, 1, left, left + 1, left - 1, r.getrandbits(32)]) & 0xffffffff, OUT, r.getrandbits(32)]
    elif name == 'filerewind':   # (the window's stream at +0x38: FILEVT's slot 4 the seek)
        struct.pack_into('<I', sample, 0x38, FILEVT)
        set_return(log, 4, r.getrandbits(32))
    elif name == 'basedel':
        args = [r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])]
    elif name in CTORS:   # (the record a format as sctor's, +4 the data's length, +0x1b bit 2 now and then; the stream's source REF)
        rec = bytearray(r.bytes(0x48))
        struct.pack_into('<H', rec, 0x18, r.choice([0x5622, 0xac44, 0x2b11, r.getrandbits(16)]))
        rec[0x1a] = r.choice([0x10, 8, r.getrandbits(8)])
        rec[0x1b] = r.choice([0, 1, 4, 5, r.getrandbits(8)])
        struct.pack_into('<I', rec, 4, r.choice([0, 0x100, r.getrandbits(32)]))
        regions[RECORD] = rec
        regions[SOURCE] = struct.pack('<I', SOURCE_VT) + r.bytes(0xc)
        regions[SOURCE_VT] = struct.pack('<2I', recorder(30, 0), recorder(29, 0))
        struct.pack_into('<I', sample, 0x1c, r.getrandbits(32))
        regions[STREAM] = struct.pack('<II', r.getrandbits(32), REF) + r.bytes(8)
        regions[REF] = struct.pack('<3I', REF_VT, 0, r.getrandbits(32))
        for k, slot, n in IMPORTS.values():
            regions[slot] = struct.pack('<I', recorder(k, n, True))
        set_return(log, 37, r.getrandbits(32))
        args = {'vctor': [RECORD, SOURCE], 'mctor': [RECORD, SOURCE], 'vsctor': [STREAM, RECORD, SOURCE]}.get(
            name, [STREAM, RECORD, r.choice([0, 0x100, r.getrandbits(32)]), SOURCE])
    elif name in DTORS:   # (the window on the stream at +0x2c (the file sample) or +0x40: its stream's source at +0x3c / +0x50)
        struct.pack_into('<I', sample, 0x24, r.choice([SOURCE, SOURCE, 0]))
        regions[SOURCE] = struct.pack('<I', SOURCE_VT) + r.bytes(0xc)
        regions[SOURCE_VT] = struct.pack('<I', recorder(30, 0))
        struct.pack_into('<II', sample, 0x3c if name in ('fdtor', 'fddtor') else 0x50, r.choice([REF, REF, 0]), r.getrandbits(32))
        regions[REF] = struct.pack('<3I', REF_VT, 0, r.getrandbits(32))
        # (slot 0 the deleting destructor, one argument: k 27, a plain recorder's k below 32, where k 32 was recorder(0, 2)'s stub)
        regions[REF_VT] = struct.pack('<16I', *[recorder(27, 1) if k == 0 else recorder(31, 0) for k in range(16)])
        set_return(log, 34, r.choice([1, 0, 0xffffffff, 2]))
        for k, slot, n in IMPORTS.values():
            regions[slot] = struct.pack('<I', recorder(k, n, True))
        regions[POOLS] = struct.pack('<8I', r.choice([0, FREE]), r.getrandbits(32), r.getrandbits(32), r.choice([0, FREE]), r.getrandbits(32), 0,
                                     r.choice([0, FREE]), r.getrandbits(32))
        regions[MP2_POOLS] = struct.pack('<5I', r.choice([0, FREE]), r.getrandbits(32), r.getrandbits(32), r.choice([0, FREE]), r.getrandbits(32))
        regions[FREE] = r.bytes(0x60)
        if name in ('ddtor', 'sddtor', 'sdel', 'fddtor', 'mddtor', 'msddtor'):
            args = [r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])]
    regions[SAMPLE] = sample
    return Call(address, [SAMPLE] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
