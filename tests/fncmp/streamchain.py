# A sample's filter chain (src/game/core/soundstream: SoundStream_BuildChain 0x6ba460) against the original: a record (+0x14 flags with bits
# 14, 6 and 21 any way; +0x44 the loops 0-5 or any; +0x54 the start ms; +0x58 the silence ms, 0 now and then), a format (its rate word,
# bits and channels bytes; its loop +0x20 / +0x24 each 0 or not) and the stream in front of which the filters go: a recorder (slot 6 the
# rate, 12 the seek, 14 the length, 16 the bits and 17 the channels (AL) answering what a sample's would) or null now and then. The four
# pools (the seeking stream's 0x70abf8, the looping stream's, the plain filter's and the silence stream's) with an empty free list or one
# block of RAM; the rest from the heap. The new streams run through their real vtables (the driver's kResolvedVtables).
# usage: streamchain.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x6ba460, 0x6ba606), (0x6c61e0, 0x6c6340), (0x6c6340, 0x6c6495), (0x6c68a0, 0x6c69f5), (0x6c6a00, 0x6c6a3d), (0x6c6bd0, 0x6c6d25),
           (0x6c6d30, 0x6c6e4b), (0x6c6600, 0x6c6725), (0x6c6730, 0x6c67c5), (0x6c6f40, 0x6c6f87), (0x6c6f90, 0x6c6fee), (0x6bb0d0, 0x6bb170),
           (0x6c7090, 0x6c7097), (0x5dc650, 0x5dc656)])
RECORD, FORMAT, STREAM, SVT, BLOCKS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000
POOLS = [(0xfb208c, 0x10), (0xfb2080, 0x18), (0xfb2068, 0x10), (0xfb2074, 0x28)]   # (list global, the lock after it; the next's offset)
IMPORTS = {'EnterCriticalSection': (20, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (21, 0x6fd130, 1, 0, 0)}


def case(r):
    r.deal(['chain'])
    log = log_region()
    set_return(log, 6, r.choice([22050, 44100, 11025, r.getrandbits(32)]))
    set_return(log, 14, r.choice([0, 0x10000, r.randrange(1, 0x100000), r.getrandbits(32)]))
    set_return(log, 16, r.choice([8, 0x10, 0x10, r.getrandbits(32)]))
    set_return(log, 17, r.choice([1, 2, 2, r.getrandbits(32)]))
    rec = bytearray(r.bytes(0x60))
    flags = r.getrandbits(32) & ~0x204040
    for bit in (0x4000, 0x40, 0x200000):
        if r.random() < 0.6:
            flags |= bit
    struct.pack_into('<I', rec, 0x14, flags)
    struct.pack_into('<I', rec, 0x44, r.choice([0, 1, 2, 3, 5, r.getrandbits(32)]))
    struct.pack_into('<I', rec, 0x54, r.choice([0, 100, 1500, r.randrange(100000), r.getrandbits(32)]))
    struct.pack_into('<I', rec, 0x58, r.choice([0, 50, 1000, r.randrange(10000), r.getrandbits(32)]))
    fmt = bytearray(r.bytes(0x30))
    struct.pack_into('<H', fmt, 0x18, r.choice([0x5622, 0xac44, 0x2b11, r.getrandbits(16)]))
    fmt[0x1a] = r.choice([0x10, 8, r.getrandbits(8)])
    fmt[0x1b] = r.choice([0, 1, r.getrandbits(8)])
    struct.pack_into('<II', fmt, 0x20, r.choice([0, r.getrandbits(32)]), r.choice([0, r.getrandbits(32)]))
    regions = {RECORD: rec, FORMAT: fmt, STREAM: struct.pack('<I', SVT) + r.bytes(0x2c), SVT: vtable({6: 0, 12: 1, 14: 0, 16: 0, 17: 0}),
               HEAP: heap(0x400), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}
    for i, (at, nxt) in enumerate(POOLS):
        block = BLOCKS + 0x100 * i
        if r.random() < 0.3:
            b = bytearray(r.bytes(0x60))
            struct.pack_into('<I', b, nxt, 0)
            regions[block] = b
            regions[at] = struct.pack('<II', block, r.getrandbits(32))
        else:
            regions[at] = struct.pack('<II', 0, r.getrandbits(32))
    stream = STREAM if r.random() < 0.9 else 0
    return Call(0x6ba460, [RECORD, FORMAT, stream], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
