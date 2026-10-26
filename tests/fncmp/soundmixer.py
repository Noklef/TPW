# The sound mixer (src/game/core/soundmixer: 0x6b6120-0x6b6ba0) against the originals: a mixer at MIX whose voices (0-4, recorder vtables)
# are entries of one page at PAGE (100 entries of 0x50 bytes from +0xc; the page list at +0x1c: that page, then a second node now and then,
# or none), chained into the active (+0x24) and free (+0x20) chains in a random order; ids at +0x2c whose low 16 bits name the entry or not;
# flags with bit 10 or not. The worker's byte +0x34 is always set (the destructor spins on it). The imports are recorders (InterlockedIncrement
# writes a count through its argument). # VoicePage_New: counts small, 100 or wrapping, out null now and then, the heap failing now and then. The update (0x6b62b0): the voices'
# flags steering each path (retired, past their end or not by the time timeGetTime answers with the clock disabled, slot 6, categories
# on or off, audible or not by slot 7's float in st0, following the listener within a box), the loudest kept in the array at +0x44
# (room for 1-6), the sound system [0xfb1f24] a recorder vtable (its functors logged by vtable and sound).
# The page added (0x6b69b0): a whole page of 100 from a heap of 0x2100 bytes (failing now and then) after the page list.
# usage: soundmixer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, fb, fill, heap, import_table, log_region,  # noqa: E402
                   set_float_return, set_return, strings, vtable)

# name: (address, convention, returns)
CALLS = {'dtor': (0x6b6170, 't', 'v'), 'deldtor': (0x6b6120, 't', 'a'), 'touch': (0x6b6260, 't', 'a'), 'retire': (0x6b6440, 't', 'v'),
         'each': (0x6b6550, 't', 'v'), 'find': (0x6b6830, 't', 'a'), 'stamp': (0x6b6990, 't', 'a'), 'page': (0x6b6ab0, 'c', 'a'),
         'clear': (0x6b6ba0, 't', 'a'), 'params': (0x6b64b0, 't', 'a'), 'pending': (0x6b6510, 't', 'a'), 'send': (0x6b65a0, 't', 'a'),
         'update': (0x6b62b0, 't', 'v'), 'addpage': (0x6b69b0, 't', 'a')}
B = Bench([(0x6b6120, 0x6b62a1), (0x6b6440, 0x6b65a0), (0x6b6830, 0x6b68c3), (0x6b6990, 0x6b69ab), (0x6b6a50, 0x6b6bb4), (0x6b9da0, 0x6b9db0),
           (0x6b65a0, 0x6b65e0), (0x6b7150, 0x6b7190), (0x6bc900, 0x6bca10), (0x6c5130, 0x6c5150), (0x6b62b0, 0x6b6396), (0x6bcb60, 0x6bcb7e),
           (0x6b6bc0, 0x6b6c27), (0x6bc650, 0x6bc674), (0x6bc760, 0x6bc806), (0x6bca10, 0x6bcab0), (0x6bd5f0, 0x6bd781), (0x6bb220, 0x6bb237),
           (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa), (0x6b69b0, 0x6b6a4e)])
MIX, VVT, OBJ, OVT, LEVELS, NODE2, STATUS, OUTP = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x510
PAGE = RAM + 0x10000
PARAMS, SRC, SYS, SVT = RAM + 0x600, RAM + 0x640, RAM + 0x680, RAM + 0x700
SYSTEM = 0xfb1f24   # the sound system (SoundVoice_SetPending's)
DEF, VAR, LISTENER = RAM + 0x780, RAM + 0x7c0, RAM + 0x800
CLOCK, TIME_K = 0xfa71e0, 12   # (timeGetTime: IMPORTS)
IMPORTS = {'InterlockedExchange': (40, 0x6fd200, 2, 0, 0), 'EnterCriticalSection': (41, 0x6fd140, 1, 0, 0),
           'LeaveCriticalSection': (42, 0x6fd130, 1, 0, 0), 'DeleteCriticalSection': (43, 0x6fd0d0, 1, 0, 0),
           'InterlockedIncrement': (44, 0x6fd0c0, 1, 0, 1), 'timeGetTime': (12, 0x6fd3ec, 0, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def chain(order, mixer, at, page):
    """Links the voices (page entry indexes) in order into the chain whose head is the mixer's dword at `at`."""
    addrs = [PAGE + 0xc + 0x50 * i for i in order]
    struct.pack_into('<I', mixer, at, addrs[0] if addrs else 0)
    for j, a in enumerate(addrs):
        struct.pack_into('<II', page, a - PAGE + 0x34, addrs[j + 1] if j + 1 < len(addrs) else 0, addrs[j - 1] if j else 0)


def update_regions(r, log, mixer, page, n):
    """The update's voices and mixer: flags steering the paths, ends, categories; slot 7's distance; the loudest's room."""
    now = r.getrandbits(32)
    set_return(log, TIME_K, now)
    set_float_return(log, 7, r.choice([r.uniform(0, 400), 0.0, 1e30]))
    strings(log, 5, 0, out=1)   # (slot 5's out, a local)
    blob_bytes(log, 4, 1, 0xff)   # the sound system's functors: vtable, sound (the stop's has no record)
    strings(log, 4, 0, out=1)
    for i in range(n):
        e = 0xc + 0x50 * i
        flags = r.getrandbits(32) & ~0x5152 | r.choice([0, 0, 0, 0x10]) | r.choice([0, 0, 0, 0x100]) | r.choice([0, 0, 0, 0x40]) | r.choice([0, 2])
        flags |= r.choice([0, 0x1000]) | r.choice([0, 0x400]) | r.choice([0, 0x4000])
        struct.pack_into('<II', page, e + 4, r.choice([DEF, 0]), r.choice([VAR, VAR, 0]))
        struct.pack_into('<I', page, e + 0x1c, (now - r.randrange(0, 200)) & 0xffffffff)
        struct.pack_into('<I', page, e + 0x30, flags)
        struct.pack_into('<I', page, e + 0x3c, r.randrange(0, 200))
        struct.pack_into('<I', page, e + 0x40, fb(r.uniform(0, 20)))
        page[e + 0x47] = r.choice([0, r.getrandbits(4)])
        if flags & 0x1000:
            struct.pack_into('<I', page, e + 0x4c, PARAMS)
    struct.pack_into('<I', mixer, 0x4c, r.randrange(1, 7))
    mixer[0x50] = r.choice([0, 1])


def update_extra(r):
    params = bytearray(r.bytes(0x20))
    struct.pack_into('<4i', params, 8, -10, 10, 10, -10)
    system = bytearray(r.bytes(0x70))
    struct.pack_into('<I', system, 0, SVT)
    struct.pack_into('<I', system, 0x2c, fb(r.uniform(0, 20)))
    for i in range(4):
        system[0x44 + i] = r.choice([0, 1, 1])
    struct.pack_into('<I', system, 0x60, LISTENER)
    listener = struct.pack('<3I', *(fb(r.uniform(-20, 20)) for _ in range(3)))
    var = bytearray(r.bytes(0x2a))
    struct.pack_into('<I', var, 0x22, fb(r.uniform(0, 20)))
    return {PARAMS: params, SYSTEM: struct.pack('<I', SYS), SYS: system, SVT: vtable({4: 2}), DEF: r.bytes(0x20), VAR: var,
            LISTENER: listener, CLOCK: bytes(0x20)}


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    blob_bytes(log, 6, 1, 0xff)   # the functor (on the stack): its vtable and the voice's +0x24
    strings(log, 6, 0, out=1)     # the out: the argument's own stack slot
    blob_bytes(log, 4, 1, 0xff, inner=(8, 0x7000, 28))   # the sound system's functor: vtable, sound; the record's +0x28-+0x2a
    set_return(log, 4, r.getrandbits(32))
    iat = import_table(log, IMPORTS)
    fill(log, 44, 0, struct.pack('<I', r.choice([1, 2, 0x10000, r.getrandbits(32)])))
    n = r.choice([1, 2, 3, 4] if name == 'retire' else [0, 1, 2, 3, 4])
    page = bytearray(r.bytes(0xc + 0x50 * 5)) + bytearray(0x2000)
    for i in range(n):
        e = 0xc + 0x50 * i
        struct.pack_into('<I', page, e, VVT)
        struct.pack_into('<I', page, e + 0x2c, r.choice([i, i | r.getrandbits(16) << 16, r.getrandbits(32), i + 1]))
        struct.pack_into('<I', page, e + 0x30, r.getrandbits(32) & ~0x400 | r.choice([0, 0x400]))
        struct.pack_into('<I', page, e + 0x4c, r.choice([0, PARAMS, PARAMS]))   # (a parameter block or none: SoundVoice_EnsureParams)
        struct.pack_into('<I', page, e + 0x24, r.choice([0, r.getrandbits(32) | 1]))
    order = list(range(n))
    r.shuffle(order)
    k = r.randrange(0, n + 1)
    mixer = bytearray(r.bytes(0x54))
    chain(order[:k], mixer, 0x24, page)
    chain(order[k:], mixer, 0x20, page)
    struct.pack_into('<I', page, 8, r.choice([0, 0, NODE2]))
    struct.pack_into('<I', mixer, 0x1c, r.choice([PAGE, PAGE, PAGE, 0]))
    struct.pack_into('<I', mixer, 0x3c, r.choice([OBJ, 0]))
    struct.pack_into('<II', mixer, 0x40, r.getrandbits(32), LEVELS)
    mixer[0x34] = r.randrange(1, 256)
    if name == 'update':
        update_regions(r, log, mixer, page, n)
    regions = {MIX: mixer, PAGE: page, VVT: vtable({3: 0, 5: 2, 6: 2, 8: 1, 0xb: 0, 0xc: 1}), OBJ: struct.pack('<I', OVT), OVT: vtable({2: 1}),
               LEVELS: r.bytes(0x90), NODE2: struct.pack('<III', r.getrandbits(32), r.getrandbits(32), 0), STATUS: r.bytes(8),
               IMPORT_TABLE: iat, LOG: log, HEAP: heap(0x1000, fail=r.choice([0] * 9 + [1]))}
    if name == 'update':
        regions.update(update_extra(r))
    if name in ('params', 'pending'):
        regions.update({PARAMS: r.bytes(0x20), SRC: r.bytes(0x10), SYSTEM: struct.pack('<I', SYS), SYS: struct.pack('<I', SVT), SVT: vtable({4: 2})})
    voices = [PAGE + 0xc + 0x50 * i for i in range(n)]
    if name == 'page':
        count = r.choice([0, 1, 2, 3, 100, 0x33333334, 0x33333335, r.getrandbits(32)])
        return Call(address, [STATUS, r.choice([OUTP, OUTP, OUTP, 0]), count], regions, conv='c', ret='a')
    if name == 'addpage':   # (a whole page from the heap, failing now and then; the page list: none, one page or two)
        regions[HEAP] = heap(0x2100, fail=r.choice([0] * 6 + [1]))
        args = [STATUS]
    elif name == 'deldtor':
        args = [r.choice([0, 1, r.getrandbits(32)])]
    elif name == 'retire':   # an active voice (one of the free ones when none is)
        active = [PAGE + 0xc + 0x50 * i for i in order[:k]]
        args = [r.choice(active or voices)]
    elif name == 'each':
        args = [r.choice([0, 0, 1, r.getrandbits(32)])]
    elif name in ('find', 'params', 'pending', 'send'):
        args = [STATUS] * (name != 'find') + [r.choice([0, 1, 2, 3, 4, r.getrandbits(32)] + [struct.unpack_from('<I', page, 0xc + 0x50 * i + 0x2c)[0] for i in range(n)] * 2)]
        args += {'params': [SRC], 'pending': [r.choice([0, 1, 0xffff, r.getrandbits(32)])], 'send': [r.getrandbits(32)]}.get(name, [])
    elif name == 'stamp':
        args = [r.choice(voices + [MIX + 4])] if voices else [MIX + 4]
    else:
        args = []
    return Call(address, [MIX] + args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
