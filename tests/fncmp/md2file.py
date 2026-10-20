# A .md2 model file read and made ready (src/game/model/md2file: 0x46d6d0) against the original. The file through vfs.py's mounts and memory
# source (its size as the stream gives it: the file's mostly, now and then a few bytes off), its bytes a made-up model: the header (magic and
# version mostly right, sub-version mostly 0xcb, flags any with bit 0 mostly set) and each table the loader walks laid out after it at a file
# offset (or 0 where the loader tests for it), 0-3 records each (materials, the +0x7c and +0xac records, geometry nodes with 0-3 groups, other
# nodes); the motion block (there or not, mostly as flags bit 1 asks) with its records, its +0x30 pairs and 0-3 tracks (bit 14 of their +4
# either way, their sub-blocks there or not); every offset the loader only moves any (0, inside the file, random). Flags 0-7; no name now
# and then.
# usage: md2file.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, global_heap, heap, log_region, put, set_return  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged(vfs.RANGES + [(0x46d6d0, 0x46dcef), (0x5c4d50, 0x5c5020), (0x45b9d0, 0x45ba30), (0x67a400, 0x690000)]))


class Model:
    def __init__(self, r):
        self.r = r
        self.data = bytearray(r.bytes(0xb4))

    def add(self, size):   # a block of random bytes at the end (4-aligned); its offset
        while len(self.data) % 4:
            self.data.append(self.r.getrandbits(8))
        at = len(self.data)
        self.data += self.r.bytes(size)
        return at

    def leaf(self, at, offset):   # an offset the loader only moves
        r = self.r
        put(self.data, at + offset, 'I', r.choice([0, 0, r.randrange(1, max(len(self.data), 2)), r.getrandbits(32)]))

    def table(self, at, offset, count, size, leaves, fill=None, sometimes=False):   # count records of size at a new offset (or 0)
        if sometimes and self.r.random() < 0.25:
            put(self.data, at + offset, 'I', 0)
            return None
        t = self.add(count * size)
        put(self.data, at + offset, 'I', t)
        for i in range(count):
            for o in leaves:
                self.leaf(t + size * i, o)
            if fill:
                fill(t + size * i)
        return t


def count(r):
    return r.choice([0, 1, 1, 2, 3])


def motion(m, at):
    r = m.r
    put(m.data, at + 0xc, 'H', count(r))
    m.table(at, 0x20, struct.unpack_from('<H', m.data, at + 0xc)[0], 0x10, (8, 0xc), sometimes=True)
    for o in (0x24, 0x28, 0x38, 0x3c):
        m.leaf(at, o)
    put(m.data, at + 0x18, 'H', count(r))
    m.table(at, 0x30, struct.unpack_from('<H', m.data, at + 0x18)[0], 8, (), sometimes=True)
    put(m.data, at + 0x12, 'H', count(r))

    def track(e):
        put(m.data, e + 4, 'I', r.getrandbits(32) & ~0x4000 | r.choice([0, 0x4000]))
        m.table(e, 0x24, 1, 0x10, (0xc,), sometimes=True)
        if r.random() < 0.25:
            put(m.data, e + 0x28, 'I', 0)
        elif struct.unpack_from('<I', m.data, e + 4)[0] & 0x4000:
            m.table(e, 0x28, 1, 0x10, (0, 8))
        else:
            def keys(s):
                put(m.data, s + 2, 'H', count(r))
                m.table(s, 0xc, struct.unpack_from('<H', m.data, s + 2)[0], 0x14, (4, 8, 0xc), sometimes=True)
            m.table(e, 0x28, 1, 0x14, (0x10,), fill=keys)
        m.table(e, 0x2c, 1, 0x14, (4, 0xc, 0x10), sometimes=True)
    m.table(at, 0x2c, struct.unpack_from('<H', m.data, at + 0x12)[0], 0x40, (0x18, 0x20, 0x1c, 0x30, 0x34), fill=track, sometimes=True)


def model(r, flags):
    m = Model(r)
    d = m.data
    put(d, 0, 'III', 0x1cd15d46 if r.random() < 0.95 else r.getrandbits(32), r.choice([0xdd] * 8 + [0xdc, 0xde, 0]),
        r.choice([0xcb] * 6 + [0xca, 0xcc, 0x100]))
    put(d, 0x30, 'I', r.getrandbits(32) & ~1 | (1 if r.random() < 0.85 else 0))
    geo, other = count(r), count(r)
    put(d, 0x36, 'H', count(r))
    put(d, 0x40, 'HHH', count(r), geo + other, geo)
    put(d, 0x48, 'H', count(r))
    for o in range(0x4c, 0x7c, 4):
        m.leaf(0, o)
    m.table(0, 0x54, struct.unpack_from('<H', d, 0x36)[0], 0x10, (0xc,),
            fill=lambda t: put(m.data, t + 4, 'I', r.choice([0, r.getrandbits(32)])))
    m.table(0, 0x6c, 1, 0x30, (0x28, 0x2c), sometimes=True)   # (both moved even when 0)

    def node(t):
        put(m.data, t + 0x5a, 'H', count(r))
        m.table(t, 0x6c, struct.unpack_from('<H', m.data, t + 0x5a)[0], 0x10, (0,), sometimes=True)
    m.table(0, 0x70, geo, 0xa0, (4, 8, 0xc, 0x60, 0x64, 0x68, 0x70, 0x94, 0x54), fill=node)
    m.table(0, 0x74, other, 0x58, (4, 8, 0xc, 0x54))
    m.table(0, 0x7c, struct.unpack_from('<H', d, 0x48)[0], 0x14, (0xc, 0x10), sometimes=True)
    m.table(0, 0xac, struct.unpack_from('<H', d, 0x40)[0], 0x10, (8, 0xc))
    if (r.random() < 0.85) != bool(flags & 2):
        put(m.data, 0x98, 'I', m.add(0x40))
        motion(m, struct.unpack_from('<I', m.data, 0x98)[0])
    else:
        put(m.data, 0x98, 'I', 0)
    return bytes(m.data)


def case(r):
    flags = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 0, 4])
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    data = model(r, flags)
    vfs.memory_source(r, log, regions, data)
    set_return(log, 8, r.choice([len(data)] * 8 + [len(data) - 4, len(data) + 3]))
    regions[vfs.NAME] = vfs.name_under(r, root) + b'.md2\0' + r.bytes(7)
    return Call(0x46d6d0, [vfs.NAME if r.random() < 0.97 else 0, flags], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
