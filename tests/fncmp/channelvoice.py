# A sound channel's voices (src/game/core/channelvoice) against the originals: voices of 0x8c bytes (a recorder vtable: slot 11 the
# release's value; +4 a sound (a recorder vtable, slot 2) or null, +0x2c an id, +0x74 the critical section), the imports
# (Enter/LeaveCriticalSection, InterlockedIncrement/Decrement) recorders; the count at 0xfb2058. The pool (+0 capacity, +4 an array of
# 0-4 voices or null, +8 the count in use (0 now and then, the first voice still released), +0xc a channel whose source's slot 10 (a
# recorder) writes a status through its out (negative now and then) and returns the out; the shift at 0x782db8 any byte). The pool's
# count from the settings' +0x40 (below, at or above the capacity). SoundChannel_ReleaseVoice (soundchannel, 0x6b7e60) is benched here
# ('releasevoice'): a voice or null. ChannelVoice_SetFlag: the byte on over random upper bytes, any mask. The pool's constructor: 0-3
# voices, the heap failing now and then (the array, a voice's buffer, the events, their handles). The sound commands' functors' runs
# (soundchannel, 0x6b8d60-0x6b8eb0): a handle as find's at the functor's +4, the channel's pool of the voices, the source's slots. The refill: slot 7 (the read, its out
# logged as 0) answering 1 or else, Reset / SetEvent recorders. The pool's release of each voice (0x6ba390): as the release-all's pool. The scaled rate (0x6bb860) and the id's high part (0x6bb8d0): see case().
# The pool's destructor (0x6ba3e0): as the release-all's pool, its +0x10 voice events (a recorder vtable, the stop event's handle at their
# count, the flags' bit 1 clear) and the voices' slot 0 (the array's delete) a recorder.
# The channel's release (0x6b76d0) and the base's and channel's destructors (0x6b7410, 0x6b9c00) over a channel whose +0x14 is that pool
# or null, +0x18 a source (a recorder vtable: slot 13's out a local, logged as 0) or null, +0x1c a synthesiser holder without a
# synthesiser or null, +0x20 an EAX buffer (its three COM objects recorders or null) or null; the device singleton made (its two COM
# objects recorders or null); the copier 0xfb2018 and the string's block any (freed to the heap).
# usage: channelvoice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, echo, fb, fill, heap, import_table, log_region, recorder, set_return, strings, vtable  # noqa: E402

# name: (address, convention, returns)
CALLS = {'reset': (0x6c5060, 'c', 'v'), 'inc': (0x6c5070, 't', 'a'), 'dec': (0x6c5080, 't', 'a'), 'sound': (0x6bb7a0, 't', 'a'),
         'release': (0x6bb7c0, 't', 'b'), 'releaseall': (0x6babc0, 't', 'v'), 'setcount': (0x6bac30, 't', 'v'),
         'releasevoice': (0x6b7e60, 't', 'a'), 'setflag': (0x6b8330, 't', 'v'),
         'apply': (0x6bb3e0, 't', 'a'), 'ctor': (0x6bb270, 't', 'a'), 'new': (0x6baf50, 't', 'a'),
         'poolctor': (0x6ba230, 't', 'a'), 'refill': (0x6bb590, 't', 'a'), 'scaled': (0x6bb860, 't', 'a'), 'sethigh': (0x6bb8d0, 't', 'v'),
         'releasevoices': (0x6b7eb0, 't', 'v'), 'find': (0x6baa60, 't', 'a'), 'findout': (0x6baad0, 't', 'a'), 'holds': (0x6bab50, 't', 'a'),
         'fn2d0': (0x6b8d60, 't', 'a'), 'fn5b8': (0x6b8df0, 't', 'a'), 'fn5c0': (0x6b8e60, 't', 'a'), 'fn5b0': (0x6b8dd0, 't', 'a'),
         'fn268': (0x6b8e40, 't', 'a'), 'fn270': (0x6b8eb0, 't', 'a'), 'releaseeach': (0x6ba390, 't', 'a'), 'pooldtor': (0x6ba3e0, 't', 'v'),
         'chanrelease': (0x6b76d0, 't', 'v'), 'basedtor': (0x6b7410, 't', 'v'), 'chandtor': (0x6b9c00, 't', 'v')}
RANGES = [(0x6c5060, 0x6c506b), (0x6c5070, 0x6c5078), (0x6c5080, 0x6c5088), (0x6bb7a0, 0x6bb7bd), (0x6bb7c0, 0x6bb856), (0x6babc0, 0x6bac04),
          (0x6bac30, 0x6bac5c), (0x6b7e60, 0x6b7ea5), (0x6b8330, 0x6b8358),
          (0x6bb3e0, 0x6bb468), (0x6bb270, 0x6bb311), (0x6baf50, 0x6bafc9), (0x67acf0, 0x67ada0),
          (0x6ba230, 0x6ba2b5), (0x6bb590, 0x6bb5d2), (0x6c6120, 0x6c615d), (0x6bb860, 0x6bb8c9),
          (0x6bb8d0, 0x6bb922), (0x6baa60, 0x6baaa2), (0x6baad0, 0x6bab18), (0x6bab50, 0x6bab86), (0x6b7eb0, 0x6b7ef6),
          (0x6b8d60, 0x6b8ec6), (0x6ba390, 0x6ba3d2), (0x6ba3e0, 0x6ba455), (0x6c6160, 0x6c6179),
          (0x6b76d0, 0x6b774e), (0x6b7410, 0x6b7469), (0x6b9c00, 0x6b9c4d), (0x5f61e0, 0x5f61f3), (0x6b7840, 0x6b7864), (0x6b78e0, 0x6b7904),
          (0x6c4630, 0x6c4670), (0x6c4410, 0x6c4550), (0x6c47e0, 0x6c4800), (0x6c48b0, 0x6c4900)]
B = Bench(RANGES)
VOICES, VVT, SND, SVT, POOL, CHAN, SRC, SRCVT, SETS, OUT = (RAM + 0x400 * i for i in range(10))
EVENTS, EVT, HANDLES = RAM + 0x3800, RAM + 0x3900, RAM + 0x3a00   # (the pool destructor's voice events)
CH2, CO, COVT, EAXB2, HOST, SRC2, SRC2VT = (RAM + 0x3c00 + 0x80 * i for i in range(7))   # (the channel's release and destructors)
COUNT, SHIFT = 0xfb2058, 0x782db8
REC, PAIR = RAM + 0x2800, RAM + 0x2c00   # (the apply's record and its pair)
MODS = RAM + 0x2e00   # (the scaled rate's four modifiers)
FUNCTOR = RAM + 0x3000   # (the commands' stack functors: +4 a voice's handle)
IMPORTS = {'InterlockedIncrement': (40, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (41, 0x6fd12c, 1, 0, 0),
           'EnterCriticalSection': (42, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (43, 0x6fd130, 1, 0, 0),
           'CreateEventA': (44, 0x6fd0d4, 4, 0, 0), 'InitializeCriticalSection': (45, 0x6fd144, 1, 0, 0),
           'ResetEvent': (46, 0x6fd168, 1, 0, 0), 'SetEvent': (47, 0x6fd124, 1, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def tie(r):
    """The scaled rate's rate and modifier where float and double rounding part: the exact 56-bit product rate * m (m the modifier's
    24-bit significand, odd) has bit 31 set, bits 30-3 clear and 1-3 in bits 2-0 below its even 24-bit top. The x87's one rounding to 24
    bits goes up (past the halfway point); a double product's rounding to 53 bits first drops bits 2-0 onto the tie, which goes to the
    even top. The modifier's exponent keeps the result in fistp's range (2^25-2^26, or 2^24-2^25)."""
    while True:
        m = r.randrange(1 << 23, 1 << 24) | 1
        n = ((1 << 31) + r.randrange(1, 4)) * pow(m, -1, 1 << 32) % (1 << 32)
        p = n * m
        if p >> 55 == 1 and not (p >> 32) & 1:
            return n, ((127 - r.choice([7, 8])) << 23) | (m & 0x7fffff)


def voices(r, n):
    out = bytearray()
    for _ in range(n):
        v = bytearray(r.bytes(0x8c))
        struct.pack_into('<II', v, 0, VVT, r.choice([SND, SND, 0]))
        struct.pack_into('<I', v, 0x2c, r.choice([r.getrandbits(32), r.getrandbits(8)]))
        out += v
    return out


def common(r, log, n):
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 2, r.getrandbits(32))
    for k in (40, 41, 44, 46, 47):
        set_return(log, k, r.getrandbits(32))
    strings(log, 10, 0, out=1)   # (the source's out: the voice argument's own stack slot)
    strings(log, 7, 0, out=4)   # (the refill's read: its out a local)
    set_return(log, 7, r.choice([0, 1, 1, 2, r.getrandbits(32)]))
    fill(log, 10, 0, b''.join(struct.pack('<I', r.choice([0, 1, 0x80000000, 0xffffffff, r.getrandbits(32)])) for _ in range(4)), records=4)
    echo(log, 10, 0)   # (slot 10 returns its out)
    chan = bytearray(r.bytes(0x40))
    struct.pack_into('<I', chan, 0x18, SRC)
    srcvt = vtable({10: 2})   # (the source's slots 11, 12, 14 and 15 log as k 31, 30, 29 and 28, apart from the voices')
    for slot, k, args in ((11, 31, 2), (12, 30, 2), (14, 29, 1), (15, 28, 1)):
        struct.pack_into('<I', srcvt, 4 * slot, recorder(k, args))
    return {VOICES: voices(r, n), VVT: vtable({11: 1, 7: 4, 12: 1}), SND: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({}), CHAN: chan,
            SRC: struct.pack('<I', SRCVT) + r.bytes(0xc), SRCVT: srcvt, COUNT: r.bytes(4), SHIFT: bytes([r.getrandbits(8)]) + r.bytes(3),
            OUT: r.bytes(4), IMPORT_TABLE: import_table(log, IMPORTS), LOG: log}


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    n = r.choice([0, 1, 2, 3, 4])
    regions = common(r, log, max(n, 1))
    if name in ('find', 'findout', 'holds'):   # (a handle whose index (past the shift's low 5 bits) is one of the voices'; slot 12 answering)
        shift = regions[SHIFT][0] & 31
        handle = ((r.randrange(max(n, 1)) << shift) | r.getrandbits(shift)) & 0xffffffff
        set_return(log, 12, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        regions[POOL] = struct.pack('<II', r.getrandbits(32), VOICES) + r.bytes(8)
        regions[SETS] = struct.pack('<I', handle)
        return Call(address, [POOL, SETS] + ([] if name == 'find' else [OUT]), regions, conv=conv, ret=ret)
    if name.startswith('fn'):   # (the functor's handle as find's, slot 12 answering; the pool's voices, their count any of theirs)
        shift = regions[SHIFT][0] & 31
        handle = ((r.randrange(max(n, 1)) << shift) | r.getrandbits(shift)) & 0xffffffff
        set_return(log, 12, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        strings(log, 12, 0, out=1)   # (the voice's slot 12: the handle a local)
        struct.pack_into('<I', regions[CHAN], 0x14, POOL)
        regions[POOL] = struct.pack('<4I', r.getrandbits(32), VOICES, r.choice([n, n, 0, r.randint(0, n)]), CHAN)
        regions[FUNCTOR] = struct.pack('<II', r.getrandbits(32), handle)
        return Call(address, [FUNCTOR, OUT, CHAN], regions, conv=conv, ret=ret)
    if name == 'releasevoices':   # (the channel's pool of the voices, the count theirs or fewer, 0 now and then; no array now and then)
        struct.pack_into('<I', regions[CHAN], 0x14, POOL)
        regions[POOL] = struct.pack('<4I', r.getrandbits(32), r.choice([VOICES] * 6 + [0]), r.choice([n, n, 0, r.randint(0, n)]), CHAN)
        return Call(address, [CHAN], regions, conv=conv, ret=ret)
    if name == 'releasevoice':
        return Call(address, [CHAN, OUT, r.choice([VOICES] * 6 + [0]), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'poolctor':   # (0-3 voices, the heap failing now and then: the array, a voice's buffer, the events or their handles)
        count = r.choice([0, 1, 2, 3])
        regions[HEAP] = heap(0xa000, fail=r.choice([0, 0, 0, 1, 2, count + 2, count + 3]))
        regions[POOL] = r.bytes(0x14)
        return Call(address, [POOL, CHAN, count], regions, conv=conv, ret=ret)
    if name == 'refill':   # (the buffer +0x68 and the events +0x6c / *+0x70 any; slot 7 answering 1 (the end) or not)
        struct.pack_into('<I', regions[VOICES], 0x70, OUT)
        return Call(address, [VOICES], regions, conv=conv, ret=ret)
    if name in ('ctor', 'new'):   # (the heap failing now and then: the array's block, or a voice's buffer; 0-3 voices)
        regions[HEAP] = heap(0xa000, fail=r.choice([0, 0, 0, 1, 2, 3]))
        if name == 'ctor':
            return Call(address, [VOICES], regions, conv=conv, ret=ret)
        return Call(address, [OUT, r.choice([0, 1, 2, 3])], regions, conv=conv, ret=ret)
    if name == 'apply':   # (the voice's flag 2 or not; the record's flags 2, 0x400, 0x40000 and others)
        struct.pack_into('<I', regions[VOICES], 0x10, r.choice([0, 2, 0xfffffffd, r.getrandbits(32)]))
        rec = bytearray(r.bytes(0xa0))
        flags = r.getrandbits(32) if r.random() < 0.2 else r.choice([0, 2]) | r.choice([0, 0x400]) | r.choice([0, 0x40000]) | r.getrandbits(32) & 0x1080
        struct.pack_into('<I', rec, 0x14, flags)
        struct.pack_into('<I', rec, 0x94, PAIR)
        regions.update({REC: rec, PAIR: r.bytes(8)})
        return Call(address, [VOICES, REC, OUT], regions, conv=conv, ret=ret)
    if name == 'setflag':   # (the byte on over random upper bytes)
        return Call(address, [VOICES, r.choice([0, 1, 0x80, 0x100, 0xffff7f00, r.getrandbits(32)]), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'reset':
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'scaled':   # (the rate any; the mask 0, bits 0-3 any way or any; the modifiers ordinary, 1.0, 0, infinities, NaNs, any bits)
        v = bytearray(regions[VOICES][:0x8c] or r.bytes(0x8c))
        struct.pack_into('<III', v, 0x18, r.choice([0, 1, 22050, 44100, 0x7fffffff, 0xffffffff, r.getrandbits(32), r.getrandbits(16)]),
                         r.getrandbits(32), r.getrandbits(32))
        struct.pack_into('<I', v, 0x24, r.choice([0, r.randrange(16), r.randrange(16), r.randrange(16), r.getrandbits(32)]))
        struct.pack_into('<I', v, 0x34, MODS)
        regions[VOICES] = v
        regions[MODS] = struct.pack('<4I', *[r.choice([fb(r.f()), fb(r.f()), 0x3f800000, 0, 0x80000000, 0x7f800000, 0xff800000, 0x7fc00000,
                                                       0x7f800001, 0xffc12345, 0x00000001, r.getrandbits(32)]) for _ in range(4)])
        if r.random() < 0.3:   # (a rate and modifier 0 whose product sits just past a 24-bit tie: see tie())
            rate, mod = tie(r)
            struct.pack_into('<I', v, 0x18, rate)
            struct.pack_into('<I', v, 0x24, r.choice([1, 1, 3, 0xf, r.getrandbits(32) | 1]))
            regions[MODS] = struct.pack('<4I', mod, 0x3f800000, 0x3f800000, 0x3f800000)   # (the others 1.0)
        return Call(address, [VOICES], regions, conv=conv, ret=ret)
    if name == 'sethigh':   # (the guard set or not, the mask kept any; the shift any byte)
        regions[0xfb1f18] = bytes([r.choice([0, 1, r.getrandbits(8)])]) + r.bytes(3) + struct.pack('<I', r.getrandbits(32))
        return Call(address, [OUT, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name in ('inc', 'dec'):
        return Call(address, [r.choice([COUNT, OUT])], regions, conv=conv, ret=ret)
    if name in ('sound', 'release'):
        return Call(address, [VOICES] + ([r.getrandbits(32)] if name == 'release' else []), regions, conv=conv, ret=ret)
    pool = struct.pack('<IIII', r.choice([0, 1, 2, 4, 8, r.getrandbits(32)]), r.choice([VOICES] * 6 + [0]), n, CHAN)
    sets = bytearray(r.bytes(0x50))
    struct.pack_into('<I', sets, 0x40, r.choice([0, 1, 2, 3, 4, 8, r.getrandbits(32)]))
    regions.update({POOL: pool, SETS: sets})
    if name in ('pooldtor', 'chanrelease', 'basedtor', 'chandtor'):   # (+0x10 the voice events at EVENTS (their slot 0 a recorder of one argument, as the voices'; the stop event the handle at
        # their count; their flags' bit 1 clear: else a spin in both), the voices' slot 0 too (the array's delete, 3))
        events = bytearray(r.bytes(0x18))
        struct.pack_into('<4I', events, 0, EVT, r.getrandbits(32), r.getrandbits(32), r.randrange(4))
        struct.pack_into('<II', events, 0x10, HANDLES, r.getrandbits(32) & ~2)
        regions.update({POOL: pool + struct.pack('<I', EVENTS), EVENTS: events, EVT: vtable({0: 1}), HANDLES: r.bytes(0x10),
                        VVT: vtable({0: 1, 11: 1, 7: 4, 12: 1})})
    if name in ('chanrelease', 'basedtor', 'chandtor'):
        ch = bytearray(r.bytes(0x68))
        struct.pack_into('<5I', ch, 0x10, r.getrandbits(32), r.choice([POOL, POOL, 0]), r.choice([SRC2, SRC2, 0]), r.choice([HOST, 0]),
                         r.choice([EAXB2, 0]))
        co = lambda: r.choice([CO, CO, 0])
        strings(log, 13, 0, out=1)
        regions.update({CH2: ch, CO: struct.pack('<I', COVT) + r.bytes(0xc), COVT: vtable({}, com={2: 1}),
                        EAXB2: struct.pack('<3I', co(), co(), co()), HOST: r.bytes(4) + struct.pack('<I', 0),
                        SRC2: struct.pack('<I', SRC2VT) + r.bytes(0xc), SRC2VT: vtable({13: 1, 0: 1}), HEAP: heap(0x100),
                        0xfb2018: r.bytes(4), 0xfb2028: bytes([r.getrandbits(8) | 1]) + r.bytes(7) + struct.pack('<2I', co(), co()) + r.bytes(8)})
        return Call(address, [CH2], regions, conv=conv, ret=ret)
    return Call(address, [POOL] + ([SETS] if name == 'setcount' else []), regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
