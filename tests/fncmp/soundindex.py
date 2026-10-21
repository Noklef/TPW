# The sound resource and its stream and mapped indexes (src/game/core/soundindex) against the originals. The object at SELF (random bytes):
# its vtable a recorder for slot 4 (k 20), its source (+0x10) an object whose slots 0 and 1 are recorders (k 10, 11) or null, its StreamBase
# (+0x14 for the stream index, +0x18 for the mapped one) a recorder vtable (k 1 close, 2 read (buffer, count) filling the buffer with a count,
# 4 seek, 7 map answering MAPPED or null) whose source is null or a RefObject at REF (StreamBase_Dtor's Release runs: InterlockedDecrement a
# recorder answering 0 or more, the RefObject's slots 9 and 0 recorders). The counts small, or ones whose size wraps (count * 4 past 32 bits);
# the offsets' block TABLE (any index from 0 to past the count); the blocks from the heap region, freed blocks any pointer.
# usage: soundindex.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, heap, log_region, recorder, set_return  # noqa: E402

B = Bench([(0x6d4910, 0x6d4964), (0x6c5450, 0x6c5497), (0x6c54e0, 0x6c54fc), (0x6d46e0, 0x6d474f), (0x6d4760, 0x6d4785), (0x6d4790, 0x6d479e),
           (0x6d47a0, 0x6d47ea), (0x6c55c0, 0x6c562c), (0x6c55a0, 0x6c55bc), (0x6d47f0, 0x6d4856), (0x6d4860, 0x6d4885), (0x6c5520, 0x6c558c),
           (0x6c5500, 0x6c551c), (0x6338b0, 0x6338c0), (0x5f71e0, 0x5f7208), (0x618d30, 0x618d72), (0x67a400, 0x690000)])
SELF, OUT, SOURCE, SELF_VT, INNER_VT, REF, REF_VT, MAPPED, TABLE, RECORD = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500,
                                                                          RAM + 0x600, RAM + 0x700, RAM + 0x810, RAM + 0x900)
DECREMENT = 0x6fd12c   # the import table's InterlockedDecrement
CALLS = {'ctor': 0x6d4910, 'dtor': 0x6c5450, 'deldtor': 0x6c54e0, 'load': 0x6d46e0, 'unload': 0x6d4760, 'offset': 0x6d4790, 'record': 0x6d47a0,
         'sdtor': 0x6c55c0, 'sdeldtor': 0x6c55a0, 'map': 0x6d47f0, 'unmap': 0x6d4860, 'mdtor': 0x6c5520, 'mdeldtor': 0x6c5500}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def table(slots):
    """A vtable of 32 slots: {slot: recorder}, the rest null."""
    return struct.pack('<32I', *[slots.get(i, 0) for i in range(32)])


def count(r):
    return r.choice([r.randrange(0, 0x10), r.randrange(0, 0x10), r.randrange(0, 0x400), 0, 0x40000000, 0x40000001, 0xc0000002])


def env(r, at):
    """The log, the object (its stream at `at`) and the regions: the source, the vtables, the RefObject and the import."""
    log = log_region()
    me = bytearray(r.bytes(0x40))
    struct.pack_into('<I', me, 0, SELF_VT)
    struct.pack_into('<I', me, 0x10, SOURCE if r.random() < 0.8 else 0)
    struct.pack_into('<II', me, at, INNER_VT, REF if r.random() < 0.6 else 0)
    ref = bytearray(r.bytes(0x10))
    struct.pack_into('<II', ref, 0, REF_VT, 0)
    set_return(log, 14, r.choice([0, 0, 1, 2, 0xffffffff]))
    regions = {SELF: me, OUT: r.bytes(4), SOURCE: struct.pack('<I', SOURCE + 4) + table({0: recorder(10, 0), 1: recorder(11, 0)}),
               SELF_VT: table({4: recorder(20, 0)}), INNER_VT: table({1: recorder(1, 0), 2: recorder(2, 2), 4: recorder(4, 2), 7: recorder(7, 0)}),
               REF: ref, REF_VT: table({0: recorder(13, 1), 9: recorder(12, 0)}), DECREMENT: struct.pack('<I', recorder(14, 1, True)),
               LOG: log, HEAP: heap()}
    return log, me, regions


def case(r):
    name = r.deal(OPS)
    mapped = name in ('map', 'unmap', 'mdtor', 'mdeldtor')
    log, me, regions = env(r, 0x18 if mapped else 0x14)
    n = count(r)
    if name == 'ctor':
        return Call(0x6d4910, [SELF, SOURCE], regions, conv='t', ret='a')
    if name in ('dtor', 'sdtor', 'mdtor'):
        return Call(CALLS[name], [SELF], regions, conv='t', ret='v')
    if name in ('deldtor', 'sdeldtor', 'mdeldtor'):
        return Call(CALLS[name], [SELF, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'load':
        set_return(log, 4, r.choice([0, 0, 0, 0xffffffff, 5]))
        set_return(log, 2, r.choice([4, 4, 4, 0, 8, 0xffffffff]))
        fill(log, 2, 0, struct.pack('<I', n))
        return Call(0x6d46e0, [SELF, OUT, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    if name in ('unload', 'unmap'):
        struct.pack_into('<I', me, 0x14 if mapped else 8, r.choice([0, r.getrandbits(32), HEAP + 0x10]))
        return Call(CALLS[name], [SELF], regions, conv='t', ret='v')
    if name == 'map':
        struct.pack_into('<I', me, 0xc, r.getrandbits(32))
        set_return(log, 7, MAPPED if r.random() < 0.8 else 0)
        regions[MAPPED] = struct.pack('<I', n) + r.bytes(0x1c)
        return Call(0x6d47f0, [SELF, OUT, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    n = r.randrange(0, 0x30)
    struct.pack_into('<II', me, 8, TABLE, n)
    regions[TABLE - 0x10] = r.bytes(0xe0)
    i = r.choice([r.randrange(0, n + 3), r.randrange(1, n + 1) if n else 1, r.getrandbits(32), 0])
    if name == 'offset':
        i = r.randrange(0, n + 1)
        return Call(0x6d4790, [SELF, i], regions, conv='t', ret='a')
    regions[RECORD] = struct.pack('<I', RECORD + 0x10) + r.bytes(0x40)
    return Call(0x6d47a0, [SELF, OUT, RECORD, i], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
