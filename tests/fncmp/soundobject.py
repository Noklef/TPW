# The sound object's helpers (src/game/core/soundobject) against the originals: an object at OBJ whose mixer (+0x20, null now and then)
# has 0-4 voices (recorder vtables) as entries of a page at PAGE (a second page at NODE2 or none: ids 100-105 reach it), some chained
# active (+0x24); voices' +0xc an info block at INFO or null; the channel (+0x24, null now and then) a recorder vtable, its flag 0x400 set
# or not; the object's own vtable a recorder (slot 6). Ids: an entry's own, its low 16 bits with another stamp, or random. The stack functors and outs the recorders get are logged by
# their set bytes (blob_bytes) and as 0 (strings out). The path resolve (0x6b8690) over paths whose first three characters are drive
# letters, slashes of either kind and others, into a string whose slots 5, 6 and 8 are recorders.
# The functors sent to a voice (0x6b89b0, 0x6b8a10, 0x6b8a70): ids as the info's.
# usage: soundobject.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, import_table, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, stack arguments after out)
CALLS = {'broadcast': (0x6b85f0, 0), 'value': (0x6b8640, 1), 'info': (0x6b8b00, 2), 'send': (0x6b8930, 2), 'path': (0x6b8690, 0),
         'functor1': (0x6b89b0, 1), 'functor2': (0x6b8a10, 1), 'functor3': (0x6b8a70, 1)}
B = Bench([(0x6b6550, 0x6b65b0), (0x6b6830, 0x6b68e0), (0x6b6b40, 0x6b6bb0), (0x6b85f0, 0x6b8711), (0x6b8b00, 0x6b8b70), (0x6b8930, 0x6b8980),
           (0x6b65a0, 0x6b65e0), (0x6b89b0, 0x6b8a05), (0x6b8a10, 0x6b8a65), (0x6b8a70, 0x6b8ac5)])
OBJ, OUT, RESULT, CHAN, CVT, VVT, INFO, MIX, NODE2, OVT = (RAM + 0x100 * i for i in range(10))
PAGE = RAM + 0x1000
IMPORTS = {'EnterCriticalSection': (41, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (42, 0x6fd130, 1, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
ENTRY = 0x50
PARAMS = RAM + 0xa00
STR, STRVT, PATH = RAM + 0xb00, RAM + 0xb40, RAM + 0xc00   # (the path's string: slots 5 assign, 6 assign a string, 8 append)


def mixer_regions(r, regions, voices=False):
    """A mixer at MIX with up to 4 voices on the page at PAGE (entries 0-5), some chained active; returns the voices' ids. voices: each
    voice's +0x4c a parameter block at PARAMS or none, its +0x24 a sound or none (the soundcmd bench's voice commands)."""
    n = r.randrange(0, 5)
    page = bytearray(r.bytes(0xc + ENTRY * 6))
    ids = []
    for i in range(n):
        e = 0xc + ENTRY * i
        vid = r.choice([i, i | r.getrandbits(16) << 16, r.getrandbits(32)])
        ids.append(vid)
        struct.pack_into('<I', page, e, VVT)
        struct.pack_into('<I', page, e + 0xc, r.choice([INFO, INFO, 0]))
        struct.pack_into('<I', page, e + 0x2c, vid)
        if voices:
            struct.pack_into('<II', page, e + 0x48, r.getrandbits(32), r.choice([0, PARAMS, PARAMS]))
            struct.pack_into('<I', page, e + 0x24, r.choice([0, r.getrandbits(32) | 1]))
    active = [i for i in range(n) if r.random() < 0.7]
    r.shuffle(active)
    mixer = bytearray(r.bytes(0x50))
    addrs = [PAGE + 0xc + ENTRY * i for i in active]
    struct.pack_into('<I', mixer, 0x24, addrs[0] if addrs else 0)
    for j, a in enumerate(addrs):
        struct.pack_into('<I', page, a - PAGE + 0x34, addrs[j + 1] if j + 1 < len(addrs) else 0)
    struct.pack_into('<I', page, 8, r.choice([0, NODE2]))
    struct.pack_into('<I', mixer, 0x1c, r.choice([PAGE, PAGE, PAGE, 0]))
    node2 = bytearray(r.bytes(0xc + ENTRY * 6))
    struct.pack_into('<I', node2, 8, 0)
    for i in range(6):
        struct.pack_into('<II', node2, 0xc + ENTRY * i + 0xc, INFO, 100 + i)
    regions.update({MIX: mixer, PAGE: page, NODE2: node2, VVT: vtable({6: 2, 0xc: 1})})
    return ids


def object_regions(r, log, regions, slot6=True, voices=False):
    """The object at OBJ with its mixer and channel (and the log's masks) into regions; returns the voices' ids. slot6: the voices'
    functor calls (else slot 6 is the object's own)."""
    if slot6:   # (send: slot 6 is the object's own (out, id))
        blob_bytes(log, 6, 1, 0xff)   # the voices' functor (on the stack): its vtable and the voice's +0x24
        strings(log, 6, 0, out=1)     # their out: the argument's own stack slot
    blob_bytes(log, 4, 1, 0xf)    # the channel's functor: its vtable (the data unset)
    strings(log, 4, 0, out=1)     # its out (0x6b8640: the value's own stack slot)
    regions.update({IMPORT_TABLE: import_table(log, IMPORTS), LOG: log, OUT: r.bytes(4), RESULT: r.bytes(4), INFO: r.bytes(0x10)})
    ids = mixer_regions(r, regions, voices)
    obj = bytearray(r.bytes(0x30))
    struct.pack_into('<I', obj, 0, OVT)
    struct.pack_into('<II', obj, 0x20, MIX if r.random() < 0.85 else 0, CHAN if r.random() < 0.85 else 0)
    chan = bytearray(r.bytes(0x30))
    struct.pack_into('<I', chan, 0, CVT)
    struct.pack_into('<I', chan, 0x28, r.getrandbits(32) & ~0x400 | r.choice([0, 0x400]))
    regions.update({OBJ: obj, CHAN: chan, CVT: vtable({4: 2}), OVT: vtable({6: 2})})
    return ids


def case(r):
    name = r.deal(OPS)
    address, extra = CALLS[name]
    log = log_region()
    regions = {}
    ids = object_regions(r, log, regions, slot6=name not in ('send', 'path'))   # (send: slot 6 is the object's own (out, id); path: the string's)
    if name == 'path':   # (drive letters, slashes of either kind and others in the first three characters)
        path = bytes(r.choice(b'C:\\/a.') if r.random() < 0.9 else r.getrandbits(8) for _ in range(3)) + r.bytes(5) + b'\0'
        for k in (5, 6, 8):
            set_return(log, k, r.getrandbits(32))
        regions.update({STR: struct.pack('<I', STRVT) + r.bytes(0xc), STRVT: vtable({5: 1, 6: 1, 8: 1}), PATH: path})
        return Call(address, [OBJ, PATH, STR], regions, conv='t', ret='a')
    if name == 'value':
        args = [r.choice([0, 0, 1, r.getrandbits(32)])]
    elif name == 'info':
        args = [r.choice(ids * 3 + [0, 1, 2, 100, 101, 105, r.getrandbits(32)]), RESULT]
    elif name.startswith('functor'):
        args = [r.choice(ids * 3 + [0, 0, 1, 100, 103, r.getrandbits(32)])]
    elif name == 'send':
        args = [r.choice(ids * 3 + [0, 0, 1, 100, r.getrandbits(32)]), r.getrandbits(32)]
    else:
        args = []
    return Call(address, [OBJ, OUT] + args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
