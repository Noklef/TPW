# A sound mixer's voice (src/game/core/soundvoice) against the originals: a voice at VOICE (its vtable recorders: slot 7 the squared distance,
# a float in st0, slot 8 a start; +4 a definition of 0-5 variations (now and then many) whose weights' sums reach the random number or not,
# +8 the current one (samples 0-5, rising thresholds), +0x4c a parameter block (a box around the listener, ints) or null, the heap
# failing now and then; +0x24 a sound or null), the sound system [0xfb1f24] a recorder vtable whose slot 4 gets the stack functor, logged
# by its vtable and sound and the record's value bytes (+0x28; +0xc-+0x17 for the listener's record; the record's other set fields fall
# outside the recorder's 15-byte window), its +0x44-+0x47 category bytes, +0x60 the listener (floats about the box, equal to its bounds
# now and then). The game's random number any; a command whose slot 0 answers 1-3 or else; timeGetTime a recorder (the clock disabled).
# The byte +0x44 sent (100 now and then): slot 4 fills the record's +0x18 through the functor (its +8), any bits. The switch voice's
# branch pick: a node at NODE (its sample count 0 now and then, 0-8 branches at ENTRIES, each a variation in BRANCHES of weight 1 or more
# and a byte range around small values), the voice's +8 that node or null (the definition DEF2's first variation then the node), the
# value small, 0xff or any (the parameter block's byte +4 for the switch). The sequence voice's next: +8 null, on any variation up to
# past the last, or off the stride; flag 4 either way. The constructors run Class70a7e0_Ctor. The linked voices: the parameter block's
# +0x1c a parent voice at PARENT (its own block at PARENT_PARAMS); the linked switch's pick on the parent's node (the switch's setup moved
# there, the value its block's byte +4); the init's slots 14, 15 and 19 answering any (0 now and then). The rolls: the current variation
# (null now and then) with byte ranges in order, equal, reversed or about the sign's edge; slot 16 answering AL nonzero or not, its local
# (logged as 0) filled with a percentage or any. The record's level and the linked voice's record (RECORD, its +0x30 left out of the
# comparison with flag 8: the address of the original's local): flags 4, 8, 0x80 now and then, the system's +0x28 bits 0, 1, 3, 7 and
# 0x43, its slots 10 and 11 floats, the radius about 2.0 now and then, the voice's slot 18 answering 0 now and then, +0xc 0 and the
# system null now and then. The value +0x20 and its record (flag 1 either way); the levels' record (flag 0x400 either way); the position's
# (floats or any, flag 8 either way, the scale 1, 0.5 or 2 now and then).
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
         'cull': (0x6bca70, 'v'), 'command': (0x6bcab0, 'a'), 'follow': (0x6bd5f0, 'v'), 'byte44': (0x6bcb80, 'a'),
         'branch': (0x6be450, 'b'), 'switch': (0x6be5e0, 'b'), 'next': (0x6be6d0, 'b'), 'swctor': (0x6be660, 'a'), 'seqctor': (0x6be730, 'a'),
         'flags': (0x6bbe90, 'v'), 'init': (0x6bbf60, 'a'),
         'setparam': (0x6bbb80, 'v'), 'swsetparam': (0x6be580, 'v'), 'swcreate': (0x6be610, 'a'), 'seqcreate': (0x6be680, 'a'),
         'linkctor': (0x6c3a00, 'a'), 'linkswctor': (0x6c3de0, 'a'), 'linkinit': (0x6c3a80, 'b'), 'linkfollow': (0x6c3c70, 'v'),
         'linkbranch': (0x6c3e00, 'b'), 'rollvol': (0x6bc090, 'a'), 'rollpitch': (0x6bc170, 'a'), 'level': (0x6bc3c0, 'a'),
         'rollword': (0x6bc2d0, 'a'), 'value20': (0x6bba90, 'v'), 'levels': (0x6bbbe0, 'v'), 'position': (0x6bbce0, 'a'),
         'fillrec': (0x6c3ae0, 'a')}
LINKED_OPS = ('linkinit', 'linkfollow', 'linkbranch')
B = Bench([(0x6b7150, 0x6b7190), (0x6bc900, 0x6bcb56), (0x6c5130, 0x6c5150), (0x6bbf00, 0x6bbf56), (0x6bc650, 0x6bc674), (0x6bc680, 0x6bc806),
           (0x6bc810, 0x6bc8f0), (0x6bd5f0, 0x6bd781), (0x6bd7a0, 0x6bd7ac), (0x6bb220, 0x6bb237), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa),
           (0x6bcb80, 0x6bcce1), (0x6c50d0, 0x6c50e8), (0x6be450, 0x6be573), (0x6be5e0, 0x6be608), (0x6be6d0, 0x6be727), (0x6be660, 0x6be672),
           (0x6be730, 0x6be742), (0x6bb950, 0x6bb98e), (0x6bbe90, 0x6bbef9), (0x6bbf60, 0x6bc034),
           (0x6bbb80, 0x6bbbde), (0x6be580, 0x6be5d3), (0x6be610, 0x6be657), (0x6be680, 0x6be6c7),
           (0x6c3a00, 0x6c3a80), (0x6c3de0, 0x6c3df9), (0x6c3a80, 0x6c3ad4), (0x6c3c70, 0x6c3d7e), (0x6c3e00, 0x6c3f63),
           (0x6b9260, 0x6b9280), (0x6bc090, 0x6bc240), (0x6bc3c0, 0x6bc410), (0x6c3ae0, 0x6c3c70), (0x6c5110, 0x6c5130), (0x6c5150, 0x6c51a0),
           (0x6c51d0, 0x6c5230), (0x6bc2d0, 0x6bc3bb), (0x6bba90, 0x6bbb72), (0x6bbbe0, 0x6bbcdd), (0x6bbce0, 0x6bbe3d), (0x6bb240, 0x6bb265)])
VOICE, OUT, SYS, SVT, VVT, DEF, VARS, SAMPLES, PARAMS, LISTENER, CMD, CVT = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500,
                                                                            RAM + 0x600, RAM + 0xa00, RAM + 0xb00, RAM + 0xb40, RAM + 0xb80, RAM + 0xbc0)
SYSTEM, RANDOM, CLOCK, TIME_GET_TIME = 0xfb1f24, 0xfb1f20, 0xfa71e0, 0x6fd3ec
NODE, ENTRIES, BRANCHES, DEF2 = RAM + 0xc00, RAM + 0xc40, RAM + 0xc80, RAM + 0xe00
OWNER, OVT, OINFO, POS, MEM, INFO = RAM + 0xe40, RAM + 0xe80, RAM + 0xf00, RAM + 0xf20, RAM + 0xf40, RAM + 0xfa0
PARENT, PARENT_PARAMS = RAM + 0x1000, RAM + 0x1080
RECORD = RAM + 0x1100
ROLL_OPS = ('rollvol', 'rollpitch', 'level', 'fillrec', 'rollword', 'levels')
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


def branch(r, voice, params, regions, seed):
    """The switch voice's node, its branches and the definition whose first variation it is; the voice's +8 and +4, its parameter block.
    Returns the value; now and then the matching branches' running sum meets the random number exactly (their sum above it)."""
    n = r.choice([0, 1, 2, 3, 4, 6, 8, 8])
    node = bytearray(r.bytes(0x40))
    struct.pack_into('<HHI', node, 0, r.choice([0, 0, 1, r.getrandbits(16)]), r.getrandbits(16), n)
    struct.pack_into('<I', node, 0x26, ENTRIES)
    entries, branches = bytearray(r.bytes(0x40)), bytearray(r.bytes(0x2a * 8))
    for i in range(8):
        lo = r.randrange(0, 0x20)
        hi = r.randrange(lo, 0x40) if r.random() < 0.9 else r.getrandbits(8)
        struct.pack_into('<I', entries, 8 * i, BRANCHES + 0x2a * i)
        entries[8 * i + 6], entries[8 * i + 7] = lo, hi
        struct.pack_into('<I', branches, 0x2a * i + 0x1e, r.choice([r.randrange(1, 100), r.randrange(1, 0x10000), r.getrandbits(28) | 1]))
        for at in (0x16, 0x1a):
            struct.pack_into('<H', branches, 0x2a * i + at, r.choice([0, 0, r.getrandbits(16)]))
    value = r.choice([r.randrange(0, 0x40), r.randrange(0, 0x40), 0, 0xff, r.getrandbits(8), r.getrandbits(32)])
    matching = [i for i in range(n) if entries[8 * i + 6] <= value <= entries[8 * i + 7]]
    s = (seed * 0x19660d + 0x3c6ef35f) & 0xffffffff
    target = (s >> 13 | s << 19) & 0xffffffff
    if len(matching) >= 2 and target < 0xffff0000 and r.random() < 0.4:
        k = r.randrange(1, len(matching))
        cut = sorted(r.randrange(target + 1) for _ in range(k - 1))
        weights = [b - a for a, b in zip([0] + cut, cut + [target])] + [r.randrange(1, 0x8000) for _ in matching[k:]]
        weights[0] = weights[0] or 1   # (a zero sum before the target would leave it unreached; the total stays above the target)
        for i, w in zip(matching, weights):
            struct.pack_into('<I', branches, 0x2a * i + 0x1e, w)
    current = r.choice([NODE, NODE, NODE, 0])
    struct.pack_into('<II', voice, 4, DEF2 if not current or r.random() < 0.7 else 0, current)
    struct.pack_into('<I', voice, 0x4c, PARAMS)
    definition = bytearray(r.bytes(0x20))
    struct.pack_into('<I', definition, 8, NODE)
    regions.update({NODE: node, ENTRIES: entries, BRANCHES: branches, DEF2: definition})
    return value


def rolls(r, name, address, ret, log, voice, variations, system, regions):
    var = r.choice([VARS, VARS + 0x2a, VARS, 0 if name in ('rollvol', 'rollpitch', 'levels') else VARS])
    struct.pack_into('<I', voice, 8, var)
    for o in (0, 0x2a):   # (the ranges: in order, equal, reversed, about the sign's edge)
        for at in (0xc, 0xe):
            a = r.choice([r.getrandbits(8), 0, 0x7f, 0x80, 0xff, r.randrange(0x70, 0x90)])
            variations[o + at] = a
            variations[o + at + 1] = r.choice([a, r.getrandbits(8), (a + r.randrange(1, 8)) & 0xff, (a - r.randrange(1, 8)) & 0xff])
        a = r.choice([r.getrandbits(16), 0, 0, 0x7fff, 0x8000, 0xffff, r.randrange(0x7ff0, 0x8010)])   # (the words +0x10 / +0x12)
        struct.pack_into('<HH', variations, o + 0x10, a, r.choice([a, 0, r.getrandbits(16), (a + r.randrange(1, 8)) & 0xffff,
                                                                (a - r.randrange(1, 8)) & 0xffff]))
        if r.random() < 0.3:
            struct.pack_into('<I', variations, o + 0x22, r.choice([fb(2.0), fb(r.uniform(0, 4)), fb(2.0000002), fb(1.9999999)]))
    set_return(log, 16, r.choice([0, 1, 1, 0x100, 0xff, r.getrandbits(32)]))
    strings(log, 16, 0, out=2)   # (the percentage: a local)
    fill(log, 16, 1, struct.pack('<I', r.choice([0, 50, 100, r.randrange(101), 0xffffffff, r.getrandbits(32)])))
    voice[0x30] = r.choice([0, 4, 8, 0xc, 0x80, 0x84, r.getrandbits(8)])
    struct.pack_into('<I', system, 0x28, r.choice([0, 1, 2, 3, 0x83, 0x8b, 0xc8, 0x0b, 0x43, 0x80, r.getrandbits(32)]))
    if r.random() < 0.3:
        struct.pack_into('<I', system, 0x2c, r.choice([fb(2.0), fb(1.0), fb(r.uniform(0, 4))]))
        struct.pack_into('<I', voice, 0x40, r.choice([fb(1.0), fb(2.0), fb(r.uniform(0, 2))]))
    set_float_return(log, 10, r.choice([r.uniform(0, 1), r.uniform(-100, 100), 0.0]))
    set_float_return(log, 11, r.choice([r.uniform(0, 1), r.uniform(-100, 100), 0.0]))
    if name in ('rollvol', 'rollpitch', 'rollword'):
        return Call(address, [VOICE], regions, conv='t', ret=ret)
    if name == 'levels':   # (flag 0x400 either way: the voice's byte +0x31 any)
        regions[SYSTEM] = struct.pack('<I', r.choice([SYS] * 9 + [0]))
        blob_bytes(log, 4, 1, 0xff, inner=r.choice([(8, 0x7000, 8), (8, 0xf00 if voice[0x31] & 4 else 0x7f00, 28)]))   # (flags; pitch, volume set)
        return Call(address, [VOICE], regions, conv='t', ret=ret)
    regions[RECORD] = r.bytes(0xa0)
    if name == 'level':
        return Call(address, [VOICE, RECORD], regions, conv='t', ret=ret)
    struct.pack_into('<I', voice, 0xc, r.choice([0, 1, r.getrandbits(32), r.getrandbits(32)]))
    struct.pack_into('<I', voice, 0x4c, PARAMS)
    regions[SYSTEM] = struct.pack('<I', r.choice([SYS] * 9 + [0]))
    set_return(log, 18, r.choice([0, 1, r.getrandbits(32), r.getrandbits(32)]))
    ignore = [(RECORD + 0x30, 4)] if voice[0x30] & 8 else []
    return Call(address, [VOICE, r.getrandbits(32), RECORD], regions, conv='t', ret=ret, ignore=ignore)


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    if name in ('follow', 'linkfollow'):
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0xfff, 12))   # the functor: its vtable, the sound; the record's +0xc-+0x17
    elif name == 'byte44':
        hundred = r.random() < 0.5
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0xf000, 28) if hundred else (8, 0xfff, 12))   # the functor; the record's +0x28 (set for 100)
        fill(log, 4, 1, struct.pack('<I', r.choice([0, 2, 3, 0xfffffffd, r.getrandbits(32)])), offset=0x18, through=8)   # or +0xc-+0x17; +0x18
    elif name in ('value20', 'position'):   # the record's flags +0x14 and the value +0x2c; the position's +0xc-+0x17 and its pointer +0x30 (the levels': in rolls())
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0xf << 36 | 0x7000, 8) if name == 'value20' else (8, 0xf << 36 | 0xfff, 12))
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
    struct.pack_into('<I', voice, 0x4c, r.choice([0, PARAMS, PARAMS] if name not in ('follow', 'audible') + LINKED_OPS else [PARAMS]))
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
    regions = {VOICE: voice, OUT: r.bytes(4), SYS: system, SVT: vtable({4: 2, 10: 1}), VVT: vtable({7: 0, 8: 1, 14: 1, 15: 1, 16: 2, 18: 1}), DEF: d, VARS: variations,
               SAMPLES: samples, PARAMS: params, LISTENER: listener, CMD: struct.pack('<I', CVT) + r.bytes(0xc),
               CVT: struct.pack('<I', recorder(CMD_K, 0)), SYSTEM: struct.pack('<I', SYS), RANDOM: struct.pack('<I', seed), CLOCK: bytes(0x20),
               TIME_GET_TIME: struct.pack('<I', recorder(TIME_K, 0, True)), LOG: log, HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))}
    if name in LINKED_OPS or name in ('linkctor', 'linkswctor', 'fillrec'):
        parent, parent_params = bytearray(r.bytes(0x50)), bytearray(r.bytes(0x20))
        struct.pack_into('<I', parent, 0x4c, PARENT_PARAMS)
        struct.pack_into('<I', params, 0x1c, PARENT)
        regions.update({PARENT: parent, PARENT_PARAMS: parent_params})
    if name in ROLL_OPS:
        return rolls(r, name, address, ret, log, voice, variations, system, regions)
    if name in ('linkctor', 'linkswctor'):
        regions[HEAP] = heap(0x40, fail=r.choice([0] * 9 + [1]))
        return Call(address, [VOICE, r.choice([PARENT, PARENT, 0, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'linkinit':
        struct.pack_into('<I', d, 0x10, r.getrandbits(32))
        for k, choices in ((14, [0, 0x100, 1, r.getrandbits(32)]), (15, [0, SAMPLES, r.getrandbits(32)]), (19, [0, r.getrandbits(32)])):
            set_return(log, k, r.choice(choices))
        return Call(address, [VOICE, DEF, r.getrandbits(32)], regions, conv='t', ret=ret)
    if name == 'linkfollow':
        struct.pack_into('<I', voice, 0x24, r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
        struct.pack_into('<I', voice, 0x30, r.getrandbits(32))
        return Call(address, [VOICE], regions, conv='t', ret=ret)
    if name == 'linkbranch':
        value = branch(r, voice, params, regions, seed)
        parent = regions[PARENT]
        struct.pack_into('<I', parent, 8, struct.unpack_from('<I', voice, 8)[0])
        struct.pack_into('<I', parent, 0x30, r.getrandbits(32))
        struct.pack_into('<I', voice, 8, r.getrandbits(32))   # (overwritten by the parent's)
        regions[PARENT_PARAMS][4] = value & 0xff
        params[4] = r.getrandbits(8)
        return Call(address, [VOICE, r.getrandbits(32)], regions, conv='t', ret=ret)
    if name in ('branch', 'switch'):
        value = branch(r, voice, params, regions, seed)
        params[4] = value & 0xff
        return Call(address, [VOICE, value if name == 'branch' else r.getrandbits(32)], regions, conv='t', ret=ret)
    if name == 'next':
        n = struct.unpack_from('<I', d, 4)[0]
        struct.pack_into('<I', voice, 8, r.choice([0, VARS + 0x2a * r.randrange(n + 2), VARS + 0x2a * max(n - 1, 0), VARS - 0x54, VARS + r.randrange(1, 0x2a)]))
        struct.pack_into('<I', voice, 0x30, r.getrandbits(32))
        return Call(address, [VOICE, r.getrandbits(32)], regions, conv='t', ret=ret)
    if name in ('setparam', 'swsetparam'):   # (a key that one of the parameter block's bytes 0-3 has, now and then two of them)
        params[:4] = bytes(r.choice([0, 1, 2, 3, r.getrandbits(8)]) for _ in range(4))
        params[4] = r.choice([0, 0x7f, r.getrandbits(8)])
        key = r.choice([params[r.randrange(4)], params[r.randrange(4)], r.getrandbits(8)]) | r.choice([0, r.getrandbits(32) & ~0xff])
        struct.pack_into('<I', voice, 0x30, r.getrandbits(32))
        set_return(log, 14, r.choice([0, 0x100, 1, r.getrandbits(32)]))
        struct.pack_into('<I', voice, 8, r.choice([VARS, VARS + 0x2a, 0]))   # (no current variation: +0xc left)
        regions[HEAP] = heap(0x40, fail=r.choice([0] * 7 + [1]))
        return Call(address, [VOICE, key, r.getrandbits(32)], regions, conv='t', ret=ret)
    if name in ('flags', 'init', 'swcreate', 'seqcreate'):
        struct.pack_into('<I', voice, 4, DEF)
        struct.pack_into('<H', d, 0x12, r.choice([0, 0, r.getrandbits(16), r.getrandbits(8) << 8] if name != 'swcreate' else [0] + [r.getrandbits(16) | 1] * 9))
        scale = r.choice([0, 0x80000000, 0x7fc00000, 0xffc00000, 0x7f800001, fb(r.uniform(0, 10)), fb(r.uniform(-10, 0)), r.f(), 1])
        if name in ('swcreate', 'seqcreate'):
            info = bytearray(r.bytes(0x1c))
            struct.pack_into('<I', info, 0x18, scale)
            regions.update({OWNER: struct.pack('<II', OVT, r.getrandbits(32)) + struct.pack('<I', OINFO), OVT: vtable({}), OINFO: r.bytes(0x10),
                            MEM: r.bytes(0x50), INFO: info, HEAP: heap(0x40, fail=r.choice([0] * 9 + [1]))})
            return Call(address, [OWNER, r.choice([MEM] * 9 + [0]), DEF, INFO], regions, conv='c', ret=ret)
        if name == 'flags':
            return Call(address, [VOICE, scale], regions, conv='t', ret=ret)
        current = r.choice([VARS, VARS + 0x2a, 0])
        struct.pack_into('<II', voice, 8, current, r.choice([0, 0, SAMPLES + 0x10 * r.randrange(8)]))
        set_return(log, 14, r.choice([0, 0x100, 0xffffff00] + ([1, r.getrandbits(32)] if current else [])))
        set_return(log, 15, r.choice([0, SAMPLES + 0x10 * r.randrange(8)]))
        info = bytearray(r.bytes(0x10))
        regions.update({OWNER: struct.pack('<II', OVT, r.getrandbits(32)) + struct.pack('<I', OINFO), OVT: vtable({}), OINFO: info,
                        POS: r.bytes(12)})
        return Call(address, [VOICE, OUT, OWNER, DEF, POS, scale], regions, conv='t', ret=ret)
    if name == 'pending':
        return Call(address, [VOICE, OUT, r.choice([0, 1, 0xffff, 0x10000, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'value20':
        voice[0x30] = r.getrandbits(8)
        return Call(address, [VOICE, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'position':
        voice[0x30] = r.getrandbits(8)
        position = r.bytes(12) if r.random() < 0.2 else struct.pack('<3f', *(r.uniform(-1000, 1000) for _ in range(3)))
        regions[POS] = position
        if r.random() < 0.3:
            struct.pack_into('<I', system, 0x2c, r.choice([fb(1.0), fb(0.5), fb(2.0)]))
        return Call(address, [VOICE, OUT, POS], regions, conv='t', ret=ret)
    if name in ('sample', 'variation'):
        return Call(address, [VOICE, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'command':
        return Call(address, [VOICE, OUT, CMD], regions, conv='t', ret=ret)
    return Call(address, [VOICE], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
