# The free list take of src/game/core/voices (0x520d60) against the original: a table of up to 12 records with random links (negative ones too) and a head word either
# side of zero, the 0x140-byte records for the index; the spawn (0x5224f0): a reset or not, entries live or not, counts by the quarter of the age (divisors 0 and -1 too),
# spreads either side of zero, free lists that run out or not, random seeds and a table of sines; the reset (0x51fb70) with the sound system on or off and 0 to 12 records.
# usage: voices.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Take,Return,Scale,Start,Spawn,Reset)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Take': 0x520d60, 'Return': 0x521890, 'Scale': 0x521d60, 'Start': 0x522360, 'Spawn': 0x5224f0, 'Reset': 0x51fb70}
B = Bench([(0x520d60, 0x520e00), (0x521890, 0x521940), (0x521d60, 0x521e60), (0x522360, 0x5224f0), (0x5224f0, 0x5226fa), (0x51fb70, 0x51fd16)])
NODES = RAM + 0x1000


def spawn(r):
    """0x5224f0: entry `index` of the 0x140-byte table at 0x80cefc, the free list of 0x34-byte records (from [0x80cef8]) Take hands out, the sines at [0x80ceec]."""
    index = r.randrange(4)
    entries = bytearray(r.bytes(0x140 * 4))
    e = 0x140 * index
    put(entries, e, 'h', r.choice([1, 1, 1, 2, 0, -1]))
    put(entries, e + 0xcc, 'i', r.choice([0, 0x100, 0x1000, 0x7f, -0x200, r.randrange(-0x4000, 0x4000)]))
    put(entries, e + 0x50, 'h', r.choice([0, 0x10, 0x80, -0x80, r.randrange(-0x800, 0x800)]))
    d = r.choice([0, 4, 4, 0x100, 1, -1, r.randrange(1, 0x1000)])
    put(entries, e + 0xd4, 'i', d)
    put(entries, e + 0x20, 'i', r.choice([0, d // 4, d // 2, 3 * d // 4, d, -d, 0x20000000, r.getrandbits(32) - (1 << 31)]))
    for off in (0x68, 0x69, 0x6a, 0x6b):
        entries[e + off] = r.choice([0, 1, 2, 3, 0xff, 0xfe, 0x80])
    put(entries, e + 0x78, 'i', r.choice([0, 1, 2, 3, 0x40, -1, -4, r.randrange(-0x100, 0x100)]))
    put(entries, e + 2, 'h', r.choice([-1, 0, 3, r.randrange(12)]))
    nodes = bytearray(r.bytes(0x34 * 12))
    free = r.sample(range(12), r.choice([0, 1, 3, 6, 12]))
    for a, b in zip(free, free[1:] + [-1]):
        put(nodes, 0x34 * a + 0x30, 'h', b)
    sines = b''.join(struct.pack('<i', r.randrange(-0x10000, 0x10001)) for _ in range(0x200))
    regions = {0x80cefc: entries, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: struct.pack('<h', free[0] if free else -1),
               0x80ceec: struct.pack('<I', RAM + 0x8000), RAM + 0x8000: sines, 0x80cee0: r.bytes(4), 0x80ced0: r.bytes(4)}
    return Call(0x5224f0, [index, r.choice([0, 0, 0, 1, 0x100])], regions, ret='v')


def reset(r):
    """0x51fb70: the globals from 0x80ced0 to 0x816d40 in one region (the entries and instances among them), the records just after a guard (0 records write before them)."""
    state = bytearray(r.bytes(0x816d44 - 0x80ced0))
    put(state, 0x816d40 - 0x80ced0, 'I', r.choice([1, 1, 1, 0, 0x100]))
    count = r.choice([0, 1, 2, 3, 5, 12])
    put(state, 0x80cedc - 0x80ced0, 'I', count)
    put(state, 0x80cef8 - 0x80ced0, 'I', NODES)
    return Call(0x51fb70, [], {0x80ced0: state, NODES - 0x40: r.bytes(0x40 + 0x34 * 12)}, ret='v')


def case(r):
    name = r.choice(OPS)
    if name == 'Reset':
        return reset(r)
    if name == 'Spawn':
        return spawn(r)
    if name == 'Start':
        link = lambda: r.choice([-1, 0, 1, 2, 5, 11, r.randrange(12)])  # noqa: E731
        inst = bytearray(r.bytes(0x68 * 12))
        for k in range(12):
            put(inst, 0x68 * k + 0x60, 'hh', link(), link())
        templates = bytearray(r.bytes(0x68 * 20))
        for k in range(20):
            templates[0x68 * k + 0x44] = r.choice([0, 0, 1, 7])
        regions = {0x8164fc: inst, 0x80c6b0: templates, 0x816d40: struct.pack('<I', r.choice([1, 1, 1, 0, 5])), 0x80ced8: struct.pack('<I', r.choice([0, 0, 1])),
                   0x816d1e: struct.pack('<hhhh', link(), link(), link(), link()), 0x80ceea: struct.pack('<H', r.choice([0, 1, 0xffff, r.getrandbits(16)]))}
        voice = r.choice([0, 1, 5, 19, 20, -1, 0x14, r.randrange(20)])
        return Call(CALLS[name], [voice & 0xffffffff] + [r.choice([0, 15, 16, -16 & 0xffffffff, 0x12345, r.getrandbits(32)]) for _ in range(3)], regions, ret='a')
    if name == 'Scale':
        index = r.randrange(4)
        records = bytearray(r.bytes(0x140 * 4))
        for off in (0x68, 0x69, 0x6a, 0x6b):
            records[0x140 * index + off] = r.choice([0, 1, 2, 0x7f, 0x80, 0xff, 0x40, r.getrandbits(8)])
        put(records, 0x140 * index + 0x64, 'h', r.choice([0, 1, 2, 100, -1, 0x7fff, r.randrange(-50, 3000)]))
        put(records, 0x140 * index + 0x60, 'i', r.choice([0, 1, 2, 100, -1, 0x7fffffff, r.randrange(-50, 100000)]))
        factor = r.choice([0, 1, 512, 1024, 1023, 2000, 0x100000, -1 & 0xffffffff, r.getrandbits(12), r.getrandbits(32)])
        return Call(CALLS[name], [index], {0x80cefc: records, 0x80cef0: struct.pack('<I', factor)}, ret='v')
    if name == 'Return':
        nodes = bytearray(r.bytes(0x34 * 12))
        link = lambda: r.choice([-1, 0, 1, 2, 5, 11, r.randrange(12)])  # noqa: E731
        for k in range(12):
            put(nodes, 0x34 * k + 0x30, 'hH', link(), r.choice([0xffff, 0, 3, r.randrange(12)]))
        index = r.randrange(4)
        record = r.choice([-1, 0, 1, 5, 11, r.randrange(12)])
        records = bytearray(r.bytes(0x140 * 4))
        put(records, 0x140 * index + 2, 'h', record if r.random() < 0.6 else r.choice([-1, 3, 7]))
        regions = {0x80cf00: records, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: put(bytearray(r.bytes(4)), 0, 'h', r.choice([-1, 0, 4, r.randrange(12)]))}
        return Call(CALLS[name], [record & 0xffffffff, index], regions, ret='v' if name == 'Return' else 'a')
    nodes = bytearray(r.bytes(0x34 * 12))
    link = lambda: r.choice([-1, 0, 1, 2, 5, 11, r.randrange(12), 0x7fff & r.getrandbits(4)])  # noqa: E731
    for k in range(12):
        put(nodes, 0x34 * k + 0x30, 'hH', link(), r.choice([0xffff, 0, 3, r.randrange(12)]))
    index = r.randrange(4)
    records = bytearray(r.bytes(0x140 * 4))
    put(records, 0x140 * index + 2, 'h', r.choice([-1, 0, 1, 7, r.randrange(12), -5]))
    head = r.choice([-1, 0, 1, 5, 11, r.randrange(12)])
    regions = {0x80cf00: records, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: put(bytearray(r.bytes(4)), 0, 'h', head)}
    return Call(CALLS[name], [index], regions, ret='v' if name == 'Return' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
