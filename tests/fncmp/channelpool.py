# A sound channel's pool taking a voice and sending to one (src/game/core/channelvoice) against the originals, on channelvoice.py's fixture: the pool (+4 its
# voices, +8 the count, +0xc the channel), the voices' recorder vtable (slot 12 the find, 9 the send, 1 the stream handed over).
# - leave: the lock's section. both: a voice's sound and value. send / route: ChannelVoice_Send and the pool's. mix: the voice's wait (timing out now and then).
# - acquire: a record's flags on and off, its +8 a voice's +0x1c or not, the voices' values small so that a record's +0x2c lands above and below them; the
#   source's slot 17 answering 0 or not, voices with and without a sound.
# - by: with a handle (as channelvoice.py's find) or none.
# - play / notify: over the real ChannelVoices_CreateStream and NotifyStream_Create: the resource's slot 12 answering, the header's kind 0x40 (the
#   factory's path that makes no stream) or a length of 0, the notify's block from the heap (failing now and then).
# usage: channelpool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, RAM, Bench, Call, fill, heap, import_table, log_region, recorder, set_return, strings, vtable  # noqa: E402
import channelvoice as cv  # noqa: E402

CALLS = {'leave': (0x6b7380, 'v'), 'both': (0x6bb3b0, 'v'), 'send': (0x6bb320, 'a'), 'mix': (0x6bb510, 'a'), 'route': (0x6baa20, 'a'), 'acquire': (0x6bac60, 'a'),
         'by': (0x6baee0, 'a'), 'play': (0x6badd0, 'a'), 'notify': (0x6bae60, 'a')}
B = Bench(cv.RANGES + [(0x6b7380, 0x6b738a), (0x6baa20, 0x6baa5d), (0x6bac60, 0x6bad91), (0x6bada0, 0x6baf43), (0x6bb320, 0x6bb3dc), (0x6bb510, 0x6bb587), (0x6ba610, 0x6ba9f6),
           (0x41a830, 0x41a850), (0x6c8b40, 0x6c8eea), (0x6c5240, 0x6c5420)])
BASE = RAM + 0x8000
LOCK, DEST, COPIER, COPYVT, REF, RES, RESVT, HDR, STATUS, ANSWER, ANSVT, NEXT, NEXTVT = (BASE + 0x400 * i for i in range(13))
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
IMPORTS = dict(cv.IMPORTS, WaitForSingleObject=(48, 0x6fd128, 2, 0, 0))
SOURCE_K, ANSWER_K, NEXT_K, RES12_K = 27, 26, 25, 24   # (under 32: a plain recorder's k)


def pool_regions(r, log, n):
    """cv.common's voices (their +0x58 small) under a pool, the source's slot 17 a recorder."""
    regions = cv.common(r, log, max(n, 1))
    regions[IMPORT_TABLE] = import_table(log, IMPORTS)
    v = regions[cv.VOICES]
    for i in range(max(n, 1)):
        struct.pack_into('<I', v, 0x8c * i + 0x58, r.choice([r.randrange(8), r.randrange(8), 0xffffffff, r.getrandbits(32)]))
        struct.pack_into('<I', v, 0x8c * i + 0x1c, r.randrange(4))
    struct.pack_into('<I', regions[cv.SRCVT], 4 * 17, recorder(SOURCE_K, 1))
    set_return(log, SOURCE_K, r.choice([0, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<I', regions[cv.CHAN], 0x14, cv.POOL)
    regions[cv.POOL] = struct.pack('<4I', r.getrandbits(32), r.choice([cv.VOICES] * 12 + [0]), r.choice([n, n, n, 0, r.randint(0, n)]), cv.CHAN)
    return regions


def desc(r):
    d = bytearray(r.bytes(0xa0))
    struct.pack_into('<I', d, 8, r.randrange(4))
    struct.pack_into('<I', d, 0xc, r.getrandbits(32) & ~0x200 | r.choice([0, 0x200, 0x200]))
    d[0x14] = d[0x14] & ~4 | r.choice([0, 4, 4])
    struct.pack_into('<I', d, 0x2c, r.choice([0, r.randrange(8), r.randrange(8), 0xffffffff, r.getrandbits(32)]))
    return d


def handle_of(r, regions, n):
    shift = regions[cv.SHIFT][0] & 31
    return ((r.randrange(max(n, 1)) << shift) | r.getrandbits(shift)) & 0xffffffff


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    n = r.choice([1, 1, 2, 3, 4])
    regions = pool_regions(r, log, n)
    if name == 'leave':
        regions[LOCK] = struct.pack('<I', r.getrandbits(32))
        return Call(address, [LOCK], regions, conv='t', ret=ret)
    if name == 'both':
        regions[DEST] = r.bytes(8)
        return Call(address, [cv.VOICES, DEST, DEST + 4], regions, conv='t', ret=ret)
    if name in ('send', 'route'):   # (slot 12 answering 0 or a target: a recorder of its slot 9)
        strings(log, 12, 0, out=1)
        set_return(log, 12, r.choice([0, ANSWER, ANSWER]))
        vt = vtable({})
        struct.pack_into('<I', vt, 4 * 9, recorder(ANSWER_K, 2))
        regions.update({ANSWER: struct.pack('<I', ANSVT) + r.bytes(0xc), ANSVT: vt})
        h = handle_of(r, regions, n) if name == 'route' else r.getrandbits(32)
        return Call(address, ([cv.POOL] if name == 'route' else [cv.VOICES]) + [cv.OUT, h, r.getrandbits(32)], regions, conv='t', ret=ret)
    if name == 'mix':   # (the wait timing out or not; +0x30 set or not; the size any small, a copier recorder of three arguments)
        set_return(log, 48, r.choice([0, 0x102, 0x102, 1, 0x80]))
        v = regions[cv.VOICES]
        struct.pack_into('<II', v, 0x68, RAM + 0x7000, r.getrandbits(32))
        struct.pack_into('<II', v, 0x70, cv.OUT, 0)
        struct.pack_into('<I', v, 0x30, r.choice([0, 0, 1, r.getrandbits(32)]))
        regions.update({DEST: r.bytes(0x40), COPIER: struct.pack('<I', COPYVT) + r.bytes(4), COPYVT: vtable({0: 3}), 0xfb2018: struct.pack('<I', COPIER)})
        return Call(address, [cv.VOICES, DEST, r.choice([0, 1, 3, 4, 7, 0x20, 0x40])], regions, conv='t', ret=ret)
    if name == 'acquire':
        regions[cv.REC] = desc(r)
        return Call(address, [cv.POOL, cv.REC], regions, conv='t', ret=ret)
    if name == 'by':   # (a handle as find's, slot 12 answering; or none: the acquire; the count stepped)
        regions[cv.REC] = desc(r)
        set_return(log, 12, r.choice([0, 1, 0x100, r.getrandbits(32)]))
        strings(log, 12, 0, out=1)
        regions[DEST] = r.bytes(4)
        return Call(address, [cv.POOL, r.choice([0, 0, handle_of(r, regions, n) | 1]), DEST, cv.REC], regions, conv='t', ret=ret)
    # play / notify: the record's flags, its reference over a recorder resource (slot 12 answering; slot 6 the header's read, as soundlifecycle.py's make; the
    # header's kind 0x40 and its slot 15, so no sample is built), the voice's slot 12 / the acquire's voice; slot 1 of what AcquireBy answers
    rec = desc(r)
    struct.pack_into('<II', rec, 0, r.choice([0, 1, 2]), REF)
    flags = r.getrandbits(32) & ~0x2088 | r.choice([0, 0x88, 0x88, 0x88, 0x80, 8]) | r.choice([0, 0x2000, 0x2000, 0x2000])
    struct.pack_into('<I', rec, 0x14, flags & ~0x8800)
    struct.pack_into('<I', rec, 0x94, cv.PAIR)
    hdr = bytearray(r.bytes(0x28))
    struct.pack_into('<I', hdr, 4, r.choice([0, 0x40, 0x40]))
    hdr[0x1b] = 0x40
    resvt = vtable({6: 3, 15: 2})
    struct.pack_into('<I', resvt, 4 * 12, recorder(RES12_K, 0))
    regions.update({cv.REC: rec, REF: r.bytes(8) + struct.pack('<I', r.choice([RES, RES, RES, 0])) + r.bytes(0x10), RES: struct.pack('<I', RESVT) + r.bytes(0xc), RESVT: resvt, HDR: hdr,
                    STATUS: struct.pack('<II', r.choice([0, 0, 0, 0xffffffff]), r.choice([0, 0xffffffff])), cv.PAIR: r.bytes(8), HEAP: heap(0x400, fail=r.choice([0] * 8 + [1])), 0xfb26f0: bytes(0xc)})
    set_return(log, RES12_K, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    fill(log, 6, 1, struct.pack('<I', HDR))
    strings(log, 6, 0, out=3)
    strings(log, 15, 0, out=1)
    set_return(log, 6, STATUS)
    set_return(log, 15, STATUS + 4)
    regions[cv.VVT] = vtable({11: 1, 7: 4, 12: 1, 1: 1})   # (AcquireBy's answer: the acquire's voice, or the found voice's slot 12: a recorder object)
    regions[ANSWER] = struct.pack('<I', ANSVT) + r.bytes(0xc)
    ans = vtable({1: 1})
    struct.pack_into('<I', ans, 4, recorder(ANSWER_K, 1))
    regions[ANSVT] = ans
    nxt = vtable({})
    struct.pack_into('<I', nxt, 4 * 10, recorder(NEXT_K, 1))
    regions.update({NEXT: struct.pack('<I', NEXTVT) + r.bytes(0xc), NEXTVT: nxt})
    set_return(log, ANSWER_K, r.choice([0, NEXT, NEXT, NEXT]))
    set_return(log, 1, r.choice([0, NEXT, NEXT, NEXT]))   # (the voices' own slot 1, when the acquire's voice is the answer)
    set_return(log, NEXT_K, r.getrandbits(32))
    set_return(log, 12, r.choice([0, ANSWER, ANSWER, ANSWER]))
    strings(log, 12, 0, out=1)
    h = r.choice([0, 0, handle_of(r, regions, n) | 1])
    if name == 'play':
        return Call(address, [cv.POOL, h, cv.REC, cv.OUT], regions, conv='t', ret=ret)
    return Call(address, [cv.POOL, h, cv.REC], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
