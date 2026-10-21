# The sound mixer (src/game/core/soundmixer: 0x6b6120-0x6b6ba0) against the originals: a mixer at MIX whose voices (0-4, recorder vtables)
# are entries of one page at PAGE (100 entries of 0x50 bytes from +0xc; the page list at +0x1c: that page, then a second node now and then,
# or none), chained into the active (+0x24) and free (+0x20) chains in a random order; ids at +0x2c whose low 16 bits name the entry or not;
# flags with bit 10 or not. The worker's byte +0x34 is always set (the destructor spins on it). The imports are recorders (InterlockedIncrement
# writes a count through its argument). VoicePage_New: counts small, 100 or wrapping, out null now and then, the heap failing now and then.
# usage: soundmixer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, fill, heap, import_table, log_region, strings, vtable  # noqa: E402

# name: (address, convention, returns)
CALLS = {'dtor': (0x6b6170, 't', 'v'), 'deldtor': (0x6b6120, 't', 'a'), 'touch': (0x6b6260, 't', 'a'), 'retire': (0x6b6440, 't', 'v'),
         'each': (0x6b6550, 't', 'v'), 'find': (0x6b6830, 't', 'a'), 'stamp': (0x6b6990, 't', 'a'), 'page': (0x6b6ab0, 'c', 'a'),
         'clear': (0x6b6ba0, 't', 'a')}
B = Bench([(0x6b6120, 0x6b62a1), (0x6b6440, 0x6b65a0), (0x6b6830, 0x6b68c3), (0x6b6990, 0x6b69ab), (0x6b6a50, 0x6b6bb4), (0x6b9da0, 0x6b9db0)])
MIX, VVT, OBJ, OVT, LEVELS, NODE2, STATUS, OUTP = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x510
PAGE = RAM + 0x10000
IMPORTS = {'InterlockedExchange': (40, 0x6fd200, 2, 0, 0), 'EnterCriticalSection': (41, 0x6fd140, 1, 0, 0),
           'LeaveCriticalSection': (42, 0x6fd130, 1, 0, 0), 'DeleteCriticalSection': (43, 0x6fd0d0, 1, 0, 0),
           'InterlockedIncrement': (44, 0x6fd0c0, 1, 0, 1)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def chain(order, mixer, at, page):
    """Links the voices (page entry indexes) in order into the chain whose head is the mixer's dword at `at`."""
    addrs = [PAGE + 0xc + 0x50 * i for i in order]
    struct.pack_into('<I', mixer, at, addrs[0] if addrs else 0)
    for j, a in enumerate(addrs):
        struct.pack_into('<II', page, a - PAGE + 0x34, addrs[j + 1] if j + 1 < len(addrs) else 0, addrs[j - 1] if j else 0)


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    blob_bytes(log, 6, 1, 0xff)   # the functor (on the stack): its vtable and the voice's +0x24
    strings(log, 6, 0, out=1)     # the out: the argument's own stack slot
    iat = import_table(log, IMPORTS)
    fill(log, 44, 0, struct.pack('<I', r.choice([1, 2, 0x10000, r.getrandbits(32)])))
    n = r.choice([1, 2, 3, 4] if name == 'retire' else [0, 1, 2, 3, 4])
    page = bytearray(r.bytes(0xc + 0x50 * 5)) + bytearray(0x2000)
    for i in range(n):
        e = 0xc + 0x50 * i
        struct.pack_into('<I', page, e, VVT)
        struct.pack_into('<I', page, e + 0x2c, r.choice([i, i | r.getrandbits(16) << 16, r.getrandbits(32), i + 1]))
        struct.pack_into('<I', page, e + 0x30, r.getrandbits(32) & ~0x400 | r.choice([0, 0x400]))
    order = list(range(n))
    r.shuffle(order)
    k = r.randrange(0, n + 1)
    mixer = bytearray(r.bytes(0x50))
    chain(order[:k], mixer, 0x24, page)
    chain(order[k:], mixer, 0x20, page)
    struct.pack_into('<I', page, 8, r.choice([0, 0, NODE2]))
    struct.pack_into('<I', mixer, 0x1c, r.choice([PAGE, PAGE, PAGE, 0]))
    struct.pack_into('<I', mixer, 0x3c, r.choice([OBJ, 0]))
    struct.pack_into('<II', mixer, 0x40, r.getrandbits(32), LEVELS)
    mixer[0x34] = r.randrange(1, 256)
    regions = {MIX: mixer, PAGE: page, VVT: vtable({3: 0, 6: 2, 0xb: 0, 0xc: 1}), OBJ: struct.pack('<I', OVT), OVT: vtable({2: 1}),
               LEVELS: r.bytes(0x90), NODE2: struct.pack('<III', r.getrandbits(32), r.getrandbits(32), 0), STATUS: r.bytes(8),
               IMPORT_TABLE: iat, LOG: log, HEAP: heap(0x1000, fail=r.choice([0] * 9 + [1]))}
    voices = [PAGE + 0xc + 0x50 * i for i in range(n)]
    if name == 'page':
        count = r.choice([0, 1, 2, 3, 100, 0x33333334, 0x33333335, r.getrandbits(32)])
        return Call(address, [STATUS, r.choice([OUTP, OUTP, OUTP, 0]), count], regions, conv='c', ret='a')
    if name == 'deldtor':
        args = [r.choice([0, 1, r.getrandbits(32)])]
    elif name == 'retire':   # an active voice (one of the free ones when none is)
        active = [PAGE + 0xc + 0x50 * i for i in order[:k]]
        args = [r.choice(active or voices)]
    elif name == 'each':
        args = [r.choice([0, 0, 1, r.getrandbits(32)])]
    elif name == 'find':
        args = [r.choice([0, 1, 2, 3, 4, r.getrandbits(32)] + [struct.unpack_from('<I', page, 0xc + 0x50 * i + 0x2c)[0] for i in range(n)] * 2)]
    elif name == 'stamp':
        args = [r.choice(voices + [MIX + 4])] if voices else [MIX + 4]
    else:
        args = []
    return Call(address, [MIX] + args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
