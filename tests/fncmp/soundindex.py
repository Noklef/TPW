# The sound resource and its stream and mapped indexes (src/game/core/soundindex) against the originals. The object at SELF (random bytes):
# its vtable a recorder for slot 4 (k 20), its source (+0x10) an object whose slots 0 and 1 are recorders (k 10, 11) or null, its StreamBase
# (+0x14 for the stream index, +0x18 for the mapped one) a recorder vtable (k 1 close, 2 read (buffer, count) filling the buffer with a count,
# 4 seek, 7 map answering MAPPED or null) whose source is null or a RefObject at REF (StreamBase_Dtor's Release runs: InterlockedDecrement a
# recorder answering 0 or more, the RefObject's slots 9 and 0 recorders). The counts small, or ones whose size wraps (count * 4 past 32 bits);
# the offsets' block TABLE (any index from 0 to past the count); the blocks from the heap region, freed blocks any pointer.
# SoundResource_New (0x6c53c0) and SoundChannel_NewResource (0x6b8160: the source the channel) as the create; the ref's constructor
# (0x6c8b40) over random bytes; its destructors (0x6c8b80, 0x6c8b60: the resource at REF, its slot 0 a recorder; flags 0-3 or any); its forwards (0x6c8bd0-0x6c8c60, and 0x6bada0 through a handle: the index 0 or not, the reference or null; the resource null now and
# then, its slots recorders; slots 6 and 15 answer their out (the argument's own stack slot), which they fill or not). SoundChannel_LoadBank (0x6b9310): see loadbank().
# usage: soundindex.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, blob, blob_bytes, echo, fill, heap, log_region, recorder, set_return, strings  # noqa: E402

B = Bench([(0x6d4910, 0x6d4964), (0x6c5450, 0x6c5497), (0x6c54e0, 0x6c54fc), (0x6d46e0, 0x6d474f), (0x6d4760, 0x6d4785), (0x6d4790, 0x6d479e),
           (0x6d47a0, 0x6d47ea), (0x6c55c0, 0x6c562c), (0x6c55a0, 0x6c55bc), (0x6d47f0, 0x6d4856), (0x6d4860, 0x6d4885), (0x6c5520, 0x6c558c),
           (0x6c5500, 0x6c551c), (0x6338b0, 0x6338c0), (0x5f71e0, 0x5f7208), (0x618d30, 0x618d72), (0x67a400, 0x690000),
           (0x6c5240, 0x6c5440), (0x5f71b0, 0x5f71e0), (0x5f7208, 0x5f72a0), (0x618d20, 0x618d30), (0x4036a0, 0x4036d0), (0x6d49f0, 0x6d4a30),
           (0x6d4aa0, 0x6d4b10), (0x6d44a0, 0x6d4522), (0x6c8b40, 0x6c8b5b), (0x6c8b60, 0x6c8bc8), (0x6c8bd0, 0x6c8c8b), (0x6b8160, 0x6b8176),
           (0x6b9310, 0x6b9652), (0x6bead0, 0x6beb03), (0x632230, 0x632290), (0x6b9260, 0x6b9280), (0x6c58d0, 0x6c58e2), (0x6c58f0, 0x6c59f4),
           (0x6c5a00, 0x6c5aca), (0x5f5fc0, 0x5f71b0), (0x5f72a0, 0x5f7f00), (0x6bada0, 0x6badc4)])
SELF, OUT, SOURCE, SELF_VT, INNER_VT, REF, REF_VT, MAPPED, TABLE, RECORD = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500,
                                                                          RAM + 0x600, RAM + 0x700, RAM + 0x810, RAM + 0x900)
HANDLE = RAM + 0xe40   # (0x6bada0's handle: an index, the reference SELF)
DECREMENT = 0x6fd12c   # the import table's InterlockedDecrement
CALLS = {'ctor': 0x6d4910, 'dtor': 0x6c5450, 'deldtor': 0x6c54e0, 'load': 0x6d46e0, 'unload': 0x6d4760, 'offset': 0x6d4790, 'record': 0x6d47a0,
         'sdtor': 0x6c55c0, 'sdeldtor': 0x6c55a0, 'map': 0x6d47f0, 'unmap': 0x6d4860, 'mdtor': 0x6c5520, 'mdeldtor': 0x6c5500,
         'create': 0x6c5240, 'new': 0x6c53c0, 'refctor': 0x6c8b40, 'refdtor': 0x6c8b80, 'refdel': 0x6c8b60, 'ref11': 0x6c8bd0, 'ref13': 0x6c8be0, 'ref12': 0x6c8bf0, 'refoffset': 0x6c8c00,
         'ref15': 0x6c8c20, 'refread': 0x6c8c60, 'handleread': 0x6bada0, 'channelnew': 0x6b8160, 'loadbank': 0x6b9310}
INCREMENT = 0x6fd0c0   # the import table's InterlockedIncrement
DESC, STREAM, OWNER, SYNTH, API = RAM + 0xa00, RAM + 0xa40, RAM + 0xa80, RAM + 0xac0, RAM + 0xb00
BANK, OPENER_VT, WIDE, NAME, STATUS1, STATUS3, DMAP, BLOCK0, BLOCK1 = (RAM + 0xc00, RAM + 0xc80, RAM + 0xd00, RAM + 0xd80, RAM + 0xe00, RAM + 0xe10,
                                                                     RAM + 0xe20, RAM + 0x1000, RAM + 0x2000)
CATEGORY = 0xfb1f28   # the category singleton (SoundCategory_Get's), its guard byte at + 0xd0
PATH_CHARS = b'ab.\\.x'
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


def create(r, mode=None):
    """SoundResource_Create (0x6c5240): a descriptor of any type (mostly 1-3) whose StreamBase at STREAM shares the RefObject REF (null now
    and then: the AddRef crashes), its slots 1 (read, filling the count), 3 (seek) and 6 (map) recorders; the owner null, or its synthesiser
    (+4) null or one whose bank load reaches its API's slot 11 (a cdecl recorder answering not 0: the load answers -1); the heap's first or
    second block failing now and then. The new object loads through the game's vtables (driver.cpp's kResolvedVtables)."""
    log, me, regions = env(r, 0x14)
    n = count(r)
    regions[OUT] = r.bytes(8)
    regions[REF_VT] = table({0: recorder(13, 1), 9: recorder(12, 0), 1: recorder(21, 2), 3: recorder(23, 2), 6: recorder(26, 2)})
    regions[INCREMENT] = struct.pack('<I', recorder(15, 1, True))
    set_return(log, 23, r.choice([0, 0, 0, 0xffffffff, 5]))
    set_return(log, 21, r.choice([4, 4, 4, 0, 8, 0xffffffff]))
    fill(log, 21, 0, struct.pack('<I', n))
    set_return(log, 26, MAPPED if r.random() < 0.8 else 0)
    regions[MAPPED] = struct.pack('<I', n) + r.bytes(0x1c)
    regions[STREAM] = struct.pack('<II', r.getrandbits(32), REF if r.random() < 0.95 else 0)
    regions[DESC] = struct.pack('<4I', r.choice([1, 1, 2, 2, 3, 3, 0, 4, r.getrandbits(32)]), r.getrandbits(32), STREAM, r.getrandbits(32))
    # the owner's synthesiser (its bank's load: SfMan_LoadBank's file structure (0x80, the path) logged through blob(), the pair a local,
    # the word +0xc; the API's answer not 0, so the presets are not counted)
    regions[OWNER] = struct.pack('<II', r.getrandbits(32), SYNTH if r.random() < 0.7 else 0)
    synth = bytearray(r.bytes(0x20))
    struct.pack_into('<I', synth, 8, API if r.random() < 0.9 else 0)
    synth[0x14] |= 1 if r.random() < 0.9 else 0
    regions[SYNTH], regions[API] = synth, table({11: recorder(30, 3, cdecl=True)})
    strings(log, 30, 0, out=2)
    blob(log, 30, 2, 3)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 30, 1 << 16)
    set_return(log, 30, r.choice([1, 0xffffffff, r.getrandbits(32) | 1]))
    regions[HEAP] = heap(fail=r.choice([0] * 8 + [1, 2]))
    if mode == 'new':   # (0x6c53c0: the same through its argument slots, then the ref object from the heap, its block failing now and then)
        if r.random() < 0.3:
            regions[HEAP] = heap(fail=r.choice([2, 3, 4]))
        return Call(0x6c53c0, [r.choice([OWNER, OWNER, 0]), DESC, SOURCE], regions, conv='s', ret='a')
    if mode in ('channelnew', 'loadbank'):   # (0x6b8160: the channel the source, its +0x1c the owner; its vtable moved to + 0x40)
        regions[SOURCE] = struct.pack('<I', SOURCE + 0x40) + r.bytes(0x18) + struct.pack('<I', r.choice([OWNER, OWNER, 0])) + r.bytes(0x20) + regions[SOURCE][4:]
        if mode == 'loadbank':
            return loadbank(r, log, regions)
        return Call(0x6b8160, [SOURCE, DESC], regions, conv='t', ret='a')
    return Call(0x6c5240, [OUT, OUT + 4, r.choice([OWNER, OWNER, 0]), DESC, SOURCE], regions, conv='s', ret='a')


def chars(r, n, width):
    """n units of PATH_CHARS (width 1 or 2 bytes, the high bytes any of them too) and a terminator, in a block of the string's capacity."""
    cap = (n & ~7) + 8
    block = bytearray(r.choice(PATH_CHARS) for _ in range(width * (cap + 1)))
    block[width * n:width * n + width] = bytes(width)
    return cap, block


def loadbank(r, log, regions):
    """SoundChannel_LoadBank (0x6b9310) on the channel at SOURCE (create()'s, its +0x28 random): the category singleton made already (its
    guard's bit 0 set), deque B's begin iterator anywhere in BLOCK0 (the next block BLOCK1), its count small; the entries BANK or null. The
    bank's resource +8 null mostly, its WString +0xc of PATH_CHARS units (the path takes its bytes), its flags +0x20 mostly 1, 2 or 4, its
    opener +0x24 a recorder vtable: slot 1 (k 16, SoundBank_Open's: soundbank.py's) and slot 3 (k 3, the stream's open: its source REF or
    null) answer pointers to random statuses; slot 4 (k 17) fills the name (a String of PATH_CHARS, so the SoundFont's cut at "." varies)
    now and then. The resources load as create()'s (the stream the local one, type 3 a SoundFont through the owner)."""
    j = r.choice([0, 0x3fe, 0x3ff, r.randrange(0x400)])
    category = bytearray(r.bytes(0xd4))
    struct.pack_into('<4I', category, 0x6c, BLOCK0, BLOCK0 + 0x1000, BLOCK0 + 4 * j, DMAP)
    struct.pack_into('<I', category, 0x94, r.choice([0, 1, 2, 3, 5, r.randrange(0x400)]))
    category[0xd0] = r.choice([1, r.getrandbits(8) | 1])
    regions[CATEGORY] = category
    regions[DMAP] = struct.pack('<II', BLOCK0, BLOCK1)
    for block in (BLOCK0, BLOCK1):
        regions[block] = struct.pack('<1024I', *[r.choice([BANK] * 6 + [0]) for _ in range(1024)])
    bank = bytearray(r.bytes(0x40))
    n = r.choice([0, 1, 2, 3, 5, 8, 9, 16])
    cap, regions[WIDE] = chars(r, n, 2)
    struct.pack_into('<I5I', bank, 8, 0 if r.random() < 0.9 else r.getrandbits(32), 0x7030c8, n, WIDE, cap, 8)
    struct.pack_into('<II', bank, 0x20, r.choice([1, 2, 4, 4, 0, 3, 6, r.getrandbits(32)]), OPENER_VT)
    regions[BANK] = bank
    regions[OPENER_VT] = table({1: recorder(16, 3), 3: recorder(3, 5), 4: recorder(17, 3)})
    set_return(log, 16, STATUS1)
    strings(log, 16, 2, out=1)                   # (soundbank.py's: the status 0, the path hashed, the info's empty text)
    blob_bytes(log, 16, 2, 0, inner=(0x20, 1))
    set_return(log, 3, STATUS3)
    strings(log, 3, 4, out=3)                    # the status and the stream (on the stack) 0, the path hashed
    fill(log, 3, 1, struct.pack('<I', REF if r.random() < 0.9 else 0), which=1, offset=4)
    strings(log, 17, 0, out=7)                   # the status, the name and the empty string: on the stack
    if r.random() < 0.7:
        m = r.choice([0, 1, 2, 3, 5, 8, 9])
        cap, regions[NAME] = chars(r, m, 1)
        fill(log, 17, 1, struct.pack('<5I', 0x703090, m, NAME, cap, 8), which=2)
    for at in (STATUS1, STATUS3):
        regions[at] = struct.pack('<i', r.choice([-1, -0x80000000, 0, 0, 1, 0x7fffffff]))
    return Call(0x6b9310, [SOURCE, r.choice([0, 1, 2, 3, r.randrange(8), r.getrandbits(32)])], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    if name in ('create', 'new', 'channelnew', 'loadbank'):
        return create(r, name)
    if name in ('ref11', 'ref12', 'ref13', 'refoffset', 'ref15', 'refread', 'handleread'):
        log = log_region()
        for k in (9, 11, 12, 13):
            set_return(log, k, r.getrandbits(32))
        for k, which in ((6, 0), (15, 1)):
            echo(log, k, 0)
            strings(log, k, 0, out=1)
            if r.random() < 0.7:
                fill(log, k, 0, struct.pack('<I', r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])), which=which)
        regions = {SELF: struct.pack('<II', r.getrandbits(32), r.getrandbits(32)) + struct.pack('<II', r.choice([REF] * 9 + [0]), r.getrandbits(32)),
                   REF: struct.pack('<I', REF_VT) + r.bytes(0xc), REF_VT: table({6: recorder(6, 3), 9: recorder(9, 1), 11: recorder(11, 0), 12: recorder(12, 0),
                                                                                13: recorder(13, 0), 15: recorder(15, 2)}), LOG: log, OUT: r.bytes(4)}
        if name == 'handleread':   # (the index 0, small or any; the reference SELF or null)
            regions[HANDLE] = struct.pack('<II', r.choice([0, 1, 0x100, r.getrandbits(32), r.getrandbits(32)]), r.choice([SELF] * 4 + [0]))
            return Call(0x6bada0, [r.getrandbits(32), HANDLE], regions, conv='s', ret='a')
        args = {'refoffset': [r.getrandbits(32)], 'ref15': [OUT, r.getrandbits(32)], 'refread': [r.getrandbits(32), r.getrandbits(32)]}.get(name, [])
        return Call(CALLS[name], [SELF] + args, regions, conv='t', ret='a')
    if name in ('refdtor', 'refdel'):
        regions = {SELF: struct.pack('<II', r.getrandbits(32), r.getrandbits(32)) + struct.pack('<II', REF, r.getrandbits(32)),
                   REF: struct.pack('<I', REF_VT) + r.bytes(0xc), REF_VT: table({0: recorder(0, 0)}), LOG: log_region(), HEAP: heap(0x40)}
        if name == 'refdel':
            return Call(0x6c8b60, [SELF, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(0x6c8b80, [SELF], regions, conv='t')
    if name == 'refctor':
        return Call(0x6c8b40, [SELF, r.getrandbits(32)], {SELF: r.bytes(0x10)}, conv='t', ret='a')
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
