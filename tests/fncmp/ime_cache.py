# The IME's 0x24-byte entries (four dwords, a vector<wstring> at +0x10, a dword at +0x20) assigned (src/game/core/buffers: 0x479490, and
# std::copy of them 0x47d780; their copy constructor 0x47dba0 and a vector of them's insert(position, n, x) 0x47bce0; the vector's own operator= and insert(position, n, x) in core/stdwstring: 0x47ac10, 0x47c970) and the current one cached (core/ime: 0x47a9b0)
# against the originals: the vectors null, smaller, within
# the capacity or past it (a new block); assigned onto itself; the cache marked or not, matching the current entry's first dword or not.
# usage: ime_cache.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# (with the wide string's and the vectors' helpers: assign, _Construct, _Ucopy, the copy constructor, the release of ranges and of one,
# the vector sizes, the counted buffer reset, max_size, the traits' move, _Grow, _Copy, _Split)
B = Bench([(0x479490, 0x4796a8), (0x47d780, 0x47d9cc), (0x47a9b0, 0x47ab92), (0x47a960, 0x47a975), (0x47b5b0, 0x47b5fc),
           (0x47db20, 0x47db74), (0x47ae40, 0x47ae53), (0x47b300, 0x47b313), (0x405280, 0x405390), (0x479150, 0x4791b0),
           (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67acd0, 0x67ad00), (0x47ae60, 0x47b060),
           (0x47b600, 0x47b750), (0x479230, 0x479256), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47ac10, 0x47ae3d),
           (0x47c970, 0x47cc0c), (0x47dba0, 0x47dd47), (0x47bce0, 0x47bfd7), (0x4791c0, 0x479221)])
ENTS = RAM            # entries: the object +0x40, x +0x80; std::copy's sources +0x100 on, destinations +0x200 on; the IME's list +0x300 on
ARRS = RAM + 0x400    # a vector's strings: 0x80 (eight) each
IME = RAM + 0xc00     # the IME (0x190 bytes)
BLOCKS = RAM + 0x1000  # a string's block: 0x40 each, a pad byte, the count byte, then up to 0x1d characters + NUL
SLOT = 0x40


def chars(r, n):
    return b''.join(r.choice([r.randrange(1, 0x100), r.randrange(1, 0x10000)]).to_bytes(2, 'little') for _ in range(n))


def string(r, blocks, k):
    """A string at block slot k: null, or a counted block with characters, terminated. Returns (ptr, len, res)."""
    if r.random() < 0.15:
        return 0, 0, 0
    n = r.choice([0, 1, r.randrange(12), r.randrange(10, 0x1d)])
    res = min(0x1d, r.choice([n, n + r.randrange(10), 0x1d]))
    at = k * SLOT
    blocks[at + 1] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    blocks[at + 2:at + 2 + 2 * n] = chars(r, n)
    blocks[at + 2 + 2 * n:at + 4 + 2 * n] = bytes(2)
    return BLOCKS + at + 2, n, res


class Mem:
    def __init__(self, r):
        self.r = r
        self.ram = bytearray(r.bytes(0x1000))
        self.blocks = bytearray(r.bytes(0x1000))
        self.arrays = 0
        self.slots = 0

    def vector(self):
        """A vector of 0-5 strings in the next array (some room past them, or none), or null. Returns (first, last, end)."""
        r = self.r
        if r.random() < 0.12:
            return 0, 0, 0
        m = r.choice([0, 1, 2, 3, 5])
        cap = min(8, m + r.choice([0, 0, 1, 3]))
        first = ARRS + 0x80 * self.arrays
        self.arrays += 1
        for i in range(m):
            put(self.ram, first - RAM + 0x10 * i, '4I', r.getrandbits(32), *string(r, self.blocks, self.slots))
            self.slots += 1
        return first, first + 0x10 * m, first + 0x10 * cap

    def entry(self, at, first=None):
        """An entry at at: random dwords (the first one first when given) and a vector."""
        r = self.r
        put(self.ram, at - RAM, '5I', *[r.getrandbits(32) for _ in range(5)])
        if first is not None:
            put(self.ram, at - RAM, 'I', first)
        put(self.ram, at - RAM + 0x14, '3I', *self.vector())

    def regions(self):
        return {RAM: self.ram, BLOCKS: self.blocks, HEAP: heap()}


def case(r):
    op = r.deal(OPS)
    m = Mem(r)
    if op == 'Assign':   # onto x, or onto itself
        m.entry(ENTS + 0x40)
        m.entry(ENTS + 0x80)
        return Call(0x479490, [ENTS + 0x40, r.choice([ENTS + 0x80] * 4 + [ENTS + 0x40])], m.regions(), conv='t', ret='a')
    if op == 'VecAssign':   # the entries' vectors alone
        m.entry(ENTS + 0x40)
        m.entry(ENTS + 0x80)
        return Call(0x47ac10, [ENTS + 0x50, r.choice([ENTS + 0x90] * 4 + [ENTS + 0x50])], m.regions(), conv='t', ret='a')
    if op == 'Insert':   # n copies of x (a string of its own, or one of the vector's) at a position from first to last
        m.entry(ENTS + 0x40)
        first, last = struct.unpack_from('<2I', m.ram, 0x54)
        put(m.ram, 0xe0, '4I', r.getrandbits(32), *string(r, m.blocks, m.slots))
        position = first + 0x10 * r.randrange((last - first) // 0x10 + 1) if first else 0
        x = r.choice([ENTS + 0xe0] * 3 + ([first + 0x10 * r.randrange((last - first) // 0x10)] if last != first else []))
        n = r.choice([0, 1, 1, 2, 3, r.randrange(4, 9)])
        return Call(0x47c970, [ENTS + 0x50, position, n, x], m.regions(), conv='t', ret='v')
    if op == 'CtorCopy':   # x's copy made over random bytes
        m.entry(ENTS + 0x80)
        return Call(0x47dba0, [ENTS + 0x40, ENTS + 0x80], m.regions(), conv='t', ret='a')
    if op == 'EntInsert':   # a vector (at +0x30) of 0-5 entries from +0x100 (room for up to 8) or null; n copies of x (an entry of its own or one of them)
        k = r.choice([0, 1, 2, 3, 5])
        cap = 0 if r.random() < 0.1 else min(8, k + r.choice([0, 0, 1, 2, 4]))
        k = min(k, cap)
        first = ENTS + 0x100 if cap else 0
        for i in range(k):
            m.entry(first + 0x24 * i)
        put(m.ram, 0x30, '4I', r.getrandbits(32), first, first + 0x24 * k, first + 0x24 * cap)
        m.entry(ENTS + 0x80)
        position = first + 0x24 * r.randrange(k + 1)
        x = r.choice([ENTS + 0x80] * 3 + ([first + 0x24 * r.randrange(k)] if k else []))
        n = r.choice([0, 1, 1, 2, 3, r.randrange(4, 7)])
        return Call(0x47bce0, [ENTS + 0x30, position, n, x], m.regions(), conv='t', ret='v')
    if op == 'Copy':   # 0-3 entries to the destinations (or onto themselves)
        k = r.choice([0, 1, 2, 3, 3])
        for i in range(3):
            m.entry(ENTS + 0x100 + 0x24 * i)
            m.entry(ENTS + 0x200 + 0x24 * i)
        return Call(0x47d780, [ENTS + 0x100, ENTS + 0x100 + 0x24 * k, r.choice([ENTS + 0x200] * 3 + [ENTS + 0x100])], m.regions(), ret='a')
    # Cache: the current of three entries (or none) over the cache, marked or not, its first dword the same or not
    key = r.getrandbits(32)
    m.entry(IME + 0x160, key)
    for i in range(3):
        m.entry(ENTS + 0x300 + 0x24 * i, r.choice([key, key, None]))
    put(m.ram, IME - RAM + 0x3c, 'I', ENTS + 0x300)
    put(m.ram, IME - RAM + 0x54, 'i', r.choice([-1, 0, 1, 2]))
    put(m.ram, IME - RAM + 0x184, 'B', r.choice([0, 0, 1, r.randrange(0x100)]))
    put(m.ram, IME - RAM + 0x188, 'I', r.choice([0, 0, r.getrandbits(32)]))
    return Call(0x47a9b0, [IME], m.regions(), conv='t', ret='a')


KINDS = ['Assign', 'Copy', 'Cache', 'VecAssign', 'Insert', 'CtorCopy', 'EntInsert']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
