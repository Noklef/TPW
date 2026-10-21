# A sound mixer's voice (src/game/core/soundvoice) against the originals: a voice at VOICE (its vtable recorders: slot 7 the squared distance,
# a float in st0, slot 8 a start; +4 a definition of 0-5 variations (now and then many) whose weights' sums reach the random number or not,
# +8 the current one (samples 0-5, rising thresholds), +0x4c a parameter block (a box around the listener, ints) or null, the heap
# failing now and then; +0x24 a sound or null), the sound system [0xfb1f24] a recorder vtable whose slot 4 gets the stack functor, logged
# by its vtable and sound and the record's value bytes (+0x28; +0xc-+0x17 for the listener's record; the record's other set fields fall
# outside the recorder's 15-byte window), its +0x44-+0x47 category bytes, +0x60 the listener (floats about the box, equal to its bounds
# now and then). The game's random number any; a command whose slot 0 answers 1-3 or else; timeGetTime a recorder (the clock disabled).
# The byte +0x44 sent (100 now and then): slot 4 fills the record's +0x18 through the functor (its +8), any bits.
# usage: soundvoice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (HEAP, LOG, RAM, Bench, Call, blob_bytes, fb, fill, heap, log_region, recorder, set_float_return, set_return, strings,  # noqa: E402
                   vtable)

# name: (address, returns)
CALLS = {'ensure': (0x6b7150, 'v'), 'pending': (0x6bc900, 'a'), 'apply': (0x6bbf00, 'v'), 'radius': (0x6bc650, 'd'),
         'sample': (0x6bc680, 'a'), 'category': (0x6bc760, 'b'), 'variation': (0x6bc810, 'b'), 'audible': (0x6bca10, 'b'),
         'cull': (0x6bca70, 'v'), 'command': (0x6bcab0, 'a'), 'follow': (0x6bd5f0, 'v'), 'byte44': (0x6bcb80, 'a')}
B = Bench([(0x6b7150, 0x6b7190), (0x6bc900, 0x6bcb56), (0x6c5130, 0x6c5150), (0x6bbf00, 0x6bbf56), (0x6bc650, 0x6bc674), (0x6bc680, 0x6bc806),
           (0x6bc810, 0x6bc8f0), (0x6bd5f0, 0x6bd781), (0x6bd7a0, 0x6bd7ac), (0x6bb220, 0x6bb237), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa),
           (0x6bcb80, 0x6bcce1), (0x6c50d0, 0x6c50e8)])
VOICE, OUT, SYS, SVT, VVT, DEF, VARS, SAMPLES, PARAMS, LISTENER, CMD, CVT = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500,
                                                                            RAM + 0x600, RAM + 0xa00, RAM + 0xb00, RAM + 0xb40, RAM + 0xb80, RAM + 0xbc0)
SYSTEM, RANDOM, CLOCK, TIME_GET_TIME = 0xfb1f24, 0xfb1f20, 0xfa71e0, 0x6fd3ec
CMD_K, TIME_K = 9, 12
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def picked(seed):
    """The random number the picks compare: the seed advanced, rotated right 13, its high word."""
    seed = (seed * 0x19660d + 0x3c6ef35f) & 0xffffffff
    return ((seed >> 13 | seed << 19) & 0xffffffff) >> 16


def thresholds(r, n, target):
    out = sorted(r.randrange(0x10000) for _ in range(n))
    if r.random() < 0.7 and n:
        out[-1] = 0xffff
    if r.random() < 0.4:   # one of them the random number itself, those before it below
        i = r.randrange(n)
        out = [r.randrange(target) if target else 0 for _ in range(i)] + [target] + [r.randrange(target, 0x10000) for _ in range(n - i - 1)]
        out[:i] = sorted(out[:i])
    return out


def definition(r, target):
    n = r.choice([0, 1, 1, 2, 2, 3, 4, 5, 5, r.randrange(6, 20)])
    d = bytearray(r.bytes(0x20))
    struct.pack_into('<II', d, 4, n, VARS)
    weights = [r.randrange(0x20000 // max(n, 1)) for _ in range(n)]
    if r.random() < 0.3:
        weights = [r.getrandbits(32) for _ in range(n)]
    elif r.random() < 0.4 and n:   # the running sum reaching the random number exactly at one of them
        i = r.randrange(n)
        before = [r.randrange(target // max(i, 1) + 1) for _ in range(i)]
        if sum(before) <= target:
            weights[:i + 1] = before + [target - sum(before)]
    variations = bytearray(r.bytes(0x2a * max(n, 2)))
    for i in range(max(n, 2)):
        o = 0x2a * i
        samples = r.choice([0, 1, 1, 2, 3, 4, 5, r.randrange(6, 8)])
        struct.pack_into('<HH', variations, o, samples, r.getrandbits(16))
        struct.pack_into('<I', variations, o + 8, SAMPLES)
        for at in (0x16, 0x1a):
            struct.pack_into('<H', variations, o + at, r.choice([0, 0, r.getrandbits(16)]))
        if i < n:
            struct.pack_into('<I', variations, o + 0x1e, weights[i])
        struct.pack_into('<I', variations, o + 0x22, r.choice([r.f(), fb(r.uniform(0, 100)), fb(r.uniform(0, 100)), fb(float(r.randrange(100)))]))
    return d, variations


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    if name == 'follow':
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0xfff, 12))   # the functor: its vtable, the sound; the record's +0xc-+0x17
    elif name == 'byte44':
        hundred = r.random() < 0.5
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0xf000, 28) if hundred else (8, 0xfff, 12))   # the functor; the record's +0x28 (set for 100)
        fill(log, 4, 1, struct.pack('<I', r.choice([0, 2, 3, 0xfffffffd, r.getrandbits(32)])), offset=0x18, through=8)   # or +0xc-+0x17; +0x18
    elif name != 'command':
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0x7000, 28) if name == 'pending' else None)   # (the stop's functor: two dwords)
    if name != 'pending':
        strings(log, 4, 0, out=1)   # (the out is the caller's local)
    set_return(log, 4, r.getrandbits(32))
    set_float_return(log, 7, r.choice([r.uniform(0, 10000), r.uniform(0, 100) ** 2, float(r.randrange(100) ** 2), 0.0]) if r.random() < 0.85
                     else struct.unpack('<f', struct.pack('<I', r.f()))[0])
    set_return(log, CMD_K, r.choice([1, 2, 3, 0, r.getrandbits(32)]))
    set_return(log, TIME_K, r.getrandbits(32))
    seed = r.getrandbits(32)
    target = picked(seed)
    d, variations = definition(r, target)
    voice = bytearray(r.bytes(0x50))
    struct.pack_into('<I', voice, 0, VVT)
    struct.pack_into('<I', voice, 4, r.choice([DEF] * 6 + [0]))
    struct.pack_into('<I', voice, 8, VARS + 0x2a * r.randrange(2) if name != 'radius' or r.random() < 0.7 else 0)
    struct.pack_into('<I', voice, 0x4c, r.choice([0, PARAMS, PARAMS] if name not in ('follow', 'audible') else [PARAMS]))
    struct.pack_into('<I', voice, 0x24, r.choice([0, r.getrandbits(32) | 1, r.getrandbits(16) + 1 << 16]))   # (a sound whose low word is 0 too)
    struct.pack_into('<I', voice, 0x3c, r.choice([0, r.getrandbits(32)]))
    struct.pack_into('<I', voice, 0x40, r.f() if r.random() < 0.3 else fb(r.uniform(0, 10)))
    voice[0x45], voice[0x46] = r.choice([0, 1, 2, r.getrandbits(8)]), r.choice([0, 1, 2, r.getrandbits(8)])
    voice[0x47] = r.choice([0, r.getrandbits(4), r.getrandbits(8)])
    if name == 'byte44':
        voice[0x44] = 0x64 if hundred else r.choice([0x64, 0, r.getrandbits(8)])
    params = bytearray(r.bytes(0x20))
    lo_x, lo_z = r.randrange(-100, 100), r.randrange(-100, 100)
    box = [lo_x, lo_z + r.randrange(0, 100), lo_x + r.randrange(0, 100), lo_z]   # +8 x min, +0xc z max, +0x10 x max, +0x14 z min
    if r.random() < 0.15:
        box = [r.getrandbits(32) - (1 << 31) for _ in range(4)]
    struct.pack_into('<4i', params, 8, *box)
    listener = bytearray(12)
    for i, (lo, hi) in ((0, (box[0], box[2])), (2, (box[3], box[1]))):
        v = r.choice([fb(float(lo)), fb(float(hi)), fb(r.uniform(min(lo, hi) - 50, max(lo, hi) + 50)), r.f(), r.choice([0x7fc00000, 0xffc00000])])
        struct.pack_into('<I', listener, 4 * i, v)
    struct.pack_into('<I', listener, 4, r.f())
    system = bytearray(r.bytes(0x70))
    struct.pack_into('<I', system, 0, SVT)
    struct.pack_into('<I', system, 0x28, r.choice([0, 0x80, 0x40, 0xc0, 0x81, 0x02, r.getrandbits(32)]))   # (0x43 and 0x80: the record's 0x820)
    struct.pack_into('<I', system, 0x2c, r.f() if r.random() < 0.3 else fb(r.uniform(0, 100)))
    for i in range(4):
        system[0x44 + i] = r.choice([0, 1, r.getrandbits(8)])
    struct.pack_into('<I', system, 0x60, LISTENER)
    samples = bytearray(r.bytes(0x80))
    for i, t in enumerate(thresholds(r, 8, target)):
        struct.pack_into('<I', samples, 0x10 * i + 4, t if r.random() < 0.95 else r.getrandbits(32))
    if name == 'audible' and r.random() < 0.4:   # a whole radius whose square is the distance
        radius = r.randrange(100)
        struct.pack_into('<I', variations, struct.unpack_from('<I', voice, 8)[0] - VARS + 0x22, fb(float(radius)))
        struct.pack_into('<I', system, 0x2c, fb(float(radius)))
        struct.pack_into('<I', voice, 0x40, fb(1.0))
        set_float_return(log, 7, float(radius * radius))
    regions = {VOICE: voice, OUT: r.bytes(4), SYS: system, SVT: vtable({4: 2}), VVT: vtable({7: 0, 8: 1}), DEF: d, VARS: variations,
               SAMPLES: samples, PARAMS: params, LISTENER: listener, CMD: struct.pack('<I', CVT) + r.bytes(0xc),
               CVT: struct.pack('<I', recorder(CMD_K, 0)), SYSTEM: struct.pack('<I', SYS), RANDOM: struct.pack('<I', seed), CLOCK: bytes(0x20),
               TIME_GET_TIME: struct.pack('<I', recorder(TIME_K, 0, True)), LOG: log, HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))}
    if name == 'pending':
        return Call(address, [VOICE, OUT, r.choice([0, 1, 0xffff, 0x10000, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name in ('sample', 'variation'):
        return Call(address, [VOICE, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'command':
        return Call(address, [VOICE, OUT, CMD], regions, conv='t', ret=ret)
    return Call(address, [VOICE], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
