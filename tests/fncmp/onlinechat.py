# The online chat's flags under its lock (src/game/online/chat: 0x4e6130, 0x4e6150) against the originals: a chat of random bytes, its
# critical section (+0x48) entered and left through recording imports.
# Ctor (0x4e5260, the constructor): a chat of random bytes; its critical sections made through a recording import; stringmapsrecord.py's
# trees' nil nodes made already (any counter) or not; rand() the MSVC6 generator on a random seed (fncmp.RAND_SEED) through the CRT's swprintf; the global string
# 0x7a1500 null, empty or 1-0x7f characters in a block whose count byte is 0-2 or 0xfd-0xff (shared or copied), npos (0x6fe5ac) now and then
# small; mbtowc (0x687690, which swprintf's %08x goes through) the C locale's in Python; the four allocator bytes the original copies from an uninitialised stack byte (and the record's) masked.
# Queue (0x4e4fc0, stdcall: code and three unread dwords): the chat 0x7cddec at CHAT with its queue (+0xec8) of 16-byte entries in 0x1000-byte
# blocks (0-0x2ff entries from any slot of the first, the blocks in any order in a map with 0-2 slots after the end's; the end iterator at its
# block's end when full, or a count of 0), or none (made: the Ctor case's world, the new chat in the heap with its allocator bytes masked);
# the pairs' allocator bytes the original's copy constructor takes from the stack temporary's address masked in the freed vector blocks.
# Messages (0x4e61f0, stdcall: code, an unread dword, the vector): the same deque with a vector of 0-5 pairs of strings of any kind (null, shared, copied), or
# the codes -5 / -4 with dequeiter.py's command queue. The chat is there (0x46cf10 making it: mousetile.py's bench).
# usage: onlinechat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Get,Ctor)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dequeiter  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, RAND_SEED, Bench, Call, Mask, heap, import_table, log_region, put  # noqa: E402



def _mbtowc(e):   # cdecl (to, from, n) -> the C locale's mbtowc (0x687690; the original takes the locale lock through InterlockedIncrement,
    sp = e.r['ESP']   # which the driver's own swprintf (crt.cpp) never does): _woutput writes %x's digits through it
    to, frm, n = e.u32(sp), e.u32(sp + 4), e.u32(sp + 8)
    c = e.read(frm, 1)[0] if frm and n else None
    if c is not None and to:
        e.write(to, struct.pack('<H', c))
    e.r['EAX'] = 0 if not frm or not n or c == 0 else 1


B = Bench([(0x4e6130, 0x4e6150), (0x4e6150, 0x4e616f), (0x5c8b90, 0x5c8be0), (0x4e4fc0, 0x4e5220), (0x479230, 0x479270), (0x4e6410, 0x4e6570),
           (0x4f5560, 0x4f59a0), (0x4f6080, 0x4f60a0), (0x4f6e70, 0x4f6f00), (0x4f7480, 0x4f74b0), (0x6c1940, 0x6c1aa0),
           (0x405280, 0x405390), (0x45b380, 0x45b5f0), (0x45b680, 0x45b8e0), (0x47bad0, 0x47bc10), (0x4e5260, 0x4e54d0), (0x4f0010, 0x4f0100),
           (0x4f1400, 0x4f1530), (0x4f23f0, 0x4f2450), (0x4f4120, 0x4f41e0), (0x4f4b70, 0x4f4c30), (0x4f5080, 0x4f5140),
           (0x5ec210, 0x5ec230), (0x4e61f0, 0x4e640b), (0x46cf10, 0x46cf7d), (0x47f410, 0x47f61e), (0x632200, 0x632222), (0x6322e0, 0x632310), (0x4f7410, 0x4f7423), (0x4f6740, 0x4f67a4),
           (0x67a400, 0x6902c0), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={0x687690: _mbtowc, 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})   # (the STL lock, as stringmapsrecord.py's)
CALLS = {'Set': 0x4e6130, 'Get': 0x4e6150, 'Ctor': 0x4e5260, 'Queue': 0x4e4fc0, 'Messages': 0x4e61f0}
IMPORTS = {'EnterCriticalSection': (1, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (2, 0x6fd130, 1, 0, 0)}
CHAT, NILS, BLOCK, MAP, BLOCKS = RAM + 0x10000, RAM + 0x400, RAM + 0x800, RAM + 0x200, RAM + 0x20000
VEC, PAIRS, STRINGS = RAM + 0x300, RAM + 0x40000, RAM + 0x41000
TREES = [(0x7ced24, 0x7ced28), (0x7ced2c, 0x7ced30), (0x7ced34, 0x7ced38)]   # (stringmapsrecord.py's kinds D, C, E: nil global, map counter)
MASKED = [8, 0x18, 0xec8, 0xefc] + [0xe60 + k for k in (0, 0x10, 0x20, 0x21, 0x30, 0x31, 0x40, 0x41, 0x54)]


def ctor(r, chat=CHAT):
    log = log_region()
    regions = {CHAT: r.bytes(0xf0c), HEAP: heap(0x6000), LOG: log,
               IMPORT_TABLE: import_table(log, {'InitializeCriticalSection': (3, 0x6fd144, 1, 0, 0), **IMPORTS}), RAND_SEED: r.bytes(4)}
    glob = bytearray(r.bytes(0x20))
    for t, (nil_at, counter) in enumerate(TREES):
        nil = NILS + 0x40 * t
        regions[nil] = r.bytes(0x40)
        struct.pack_into('<II', glob, nil_at - 0x7ced20, r.choice([nil, nil, 0]), r.choice([0, 1, 2, 5, r.getrandbits(32)]))
    regions[0x7ced20] = glob
    n = r.choice([0, 1, 5, 31, 32, r.randrange(1, 0x80)])
    block = bytearray(r.bytes(0x100))
    block[0] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    block[1:1 + n] = bytes(r.randrange(1, 256) for _ in range(n))
    block[1 + n] = 0
    ptr = 0 if n == 0 and r.random() < 0.5 else BLOCK + 1
    regions[BLOCK] = block
    regions[0x7a1500] = struct.pack('<4I', r.getrandbits(32), ptr, n, r.choice([n, max(n, 31), n + r.randrange(40)]))
    if r.random() < 0.2:   # (npos small: the copy cut short)
        regions[0x6fe5ac] = put(bytearray(4), 0, 'I', r.choice([0, 1, 5, 0x20]))
    return regions, Mask(CHAT if chat == CHAT else HEAP, [chat - (CHAT if chat == CHAT else HEAP) + k for k in MASKED],
                         'the allocator bytes the original copies from uninitialised stack (sic)')


def queue(r):
    code = r.choice([0, 1, 0x2711, 0x100, r.getrandbits(8), r.getrandbits(32)])
    made = r.random() < 0.15
    regions, mask = ctor(r, HEAP + 0x90)   # (the new chat after the two vector blocks, 0x30 and 0x50 bytes)
    masks = [Mask(HEAP, [0x10, 0x20, 0x64, 0x74], "the copied pairs' allocator bytes: the stack temporary's address's low byte (sic)")]
    if made:
        masks.append(mask)
        regions[0x7cddec] = struct.pack('<I', 0)
        return Call(0x4e4fc0, [code] + [r.getrandbits(32) for _ in range(3)], regions, conv='s', ret='v', masks=masks)
    regions[0x7cddec] = struct.pack('<I', CHAT)
    deque(r, regions)
    return Call(0x4e4fc0, [code] + [r.getrandbits(32) for _ in range(3)], regions, conv='s', ret='v', masks=masks)


def deque(r, regions):
    """The chat's queue: see the top."""
    count = r.choice([0, 1, 2, 0xff, 0x100, 0x101, 0x1ff, r.randrange(0x300)])
    off = r.choice([0, 1, 0xff, 0x80, r.randrange(0x100)])
    blocks = (off + count) // 0x100 + 1
    first = r.randrange(3)
    slots = first + blocks + r.choice([0, 0, 1, 2])
    m = bytearray(r.bytes(4 * slots))
    place = r.sample(range(blocks + 1), blocks + 1)
    for i in range(blocks):
        struct.pack_into('<I', m, 4 * (first + i), BLOCKS + 0x1000 * place[i])

    def it(q):
        b = BLOCKS + 0x1000 * place[q // 0x100]
        return [b, b + 0x1000, b + 0x10 * (q % 0x100), MAP + 4 * (first + q // 0x100)]
    end = it(off + count)
    if count and (off + count) % 0x100 == 0 and r.random() < 0.7:   # (full: the end at its block's end)
        end = it(off + count - 1)
        end[2] = end[1]
    deq = bytearray(r.bytes(0x30))
    struct.pack_into('<4I4I3I', deq, 4, *it(off), *end, MAP, slots, count if count or r.random() < 0.5 else r.randrange(1, 9))
    chat = bytearray(regions[CHAT])
    chat[0xec8:0xef8] = deq
    regions.update({CHAT: chat, MAP: m, BLOCKS: r.bytes(0x1000 * (blocks + 1))})


def text(r, pool, off, width):
    """A string's ptr, length, reserve; its characters in the pool at off (the count byte before them any); its next free offset."""
    n = r.choice([0, 1, 5, 31, 40])
    if n == 0 and r.random() < 0.6:
        return 0, 0, r.choice([0, 31]), off
    pool[off] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    at = off + 1
    for k in range(n):
        pool[at + width * k:at + width * k + width] = r.randrange(1, 1 << (8 * width)).to_bytes(width, 'little')
    pool[at + width * n:at + width * n + width] = bytes(width)
    return STRINGS + at, n, r.choice([n, max(n, 31), n + r.randrange(40)]), at + width * (n + 1)


def messages(r):
    code = r.choice([-5, -5, -4, -4, -6, -3, 0, 1, r.getrandbits(31)])
    if code in (-5, -4):
        regions = {dequeiter.MAP: struct.pack('<5I', *(dequeiter.BLOCKS + 0x1000 * k for k in range(5))), dequeiter.BLOCKS: r.bytes(0x5000)}
        dequeiter.queue(r, regions)
    else:
        log = log_region()
        regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), HEAP: heap(0x6000)}
    regions.update({CHAT: r.bytes(0xf0c), 0x7cddec: struct.pack('<I', CHAT)})
    deque(r, regions)
    n = r.choice([0, 0, 1, 1, 2, 3, 5])
    pairs, pool, off = bytearray(r.bytes(0x24 * n + 4)), bytearray(r.bytes(0x400)), 0
    for i in range(n):
        for field, width in ((0, 1), (0x10, 2)):
            ptr, length, res, off = text(r, pool, off, width)
            put(pairs, 0x24 * i + field + 4, '3I', ptr, length, res)
    first = 0 if n == 0 and r.random() < 0.5 else PAIRS
    vec = put(bytearray(r.bytes(0x10)), 4, '2I', first, first and PAIRS + 0x24 * n)
    regions.update({VEC: vec, PAIRS: pairs, STRINGS: pool})
    return Call(0x4e61f0, [code & 0xffffffff, r.getrandbits(32), VEC], regions, conv='s', ret='v')


def case(r):
    op = r.choice(OPS)
    if op == 'Ctor':
        regions, mask = ctor(r)
        return Call(0x4e5260, [CHAT], regions, conv='t', ret='a', masks=[mask])
    if op == 'Queue':
        return queue(r)
    if op == 'Messages':
        return messages(r)
    log = log_region()
    regions = {RAM: r.bytes(0x100), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}
    return Call(CALLS[op], [RAM], regions, conv='t', ret='b' if op == 'Get' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
