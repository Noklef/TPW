# The track rides' module helpers (src/game/world/trackrides: 0x543490, 0x54bbb0, 0x549b50,
# 0x549d30, 0x5452a0, 0x54ab40) against the originals: writes and reads that fit or run short, pools absent, full or
# with a free record anywhere, handles good, stale, of other rides or 0, with the module loaded
# or not and links absent or flagged either way.
# usage: trackrides.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Write,Read,Free,Linked,Field5c,PopNode)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put, record_stream  # noqa: E402

B = Bench([(0x5452a0, 0x545307), (0x54ab40, 0x54abc3), (0x543490, 0x5434d0), (0x54bbb0, 0x54bbd6), (0x549b50, 0x549b71), (0x549d30, 0x549da6),
           (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80), (0x544160, 0x5441f0), (0x5441f0, 0x544290), (0x544c00, 0x544c80), (0x545100, 0x545180), (0x54abd0, 0x54ac70), (0x5da3c0, 0x5da3c1)])
STREAM, SINK, BUF, POOL, LINKS, GLOBALS = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x1000, RAM + 0x800, 0x877b58
CALLS = ['Write', 'Read', 'Free', 'Linked', 'Field5c', 'PopNode', 'Worn', 'Dur', 'PushNode', 'ReadN', 'ReadD']


def case(r):
    op = r.choice(OPS)
    globals_ = bytearray(r.bytes(0x40))
    if op in ('ReadN', 'ReadD'):  # a chunk: its stream (+4), the position (+8) and the end (+0x10); the stream holds `left` more bytes
        count = r.choice([0, 1, 3, 4, 5, 12, r.randrange(0x40)])
        left = r.choice([count, count, 0, 2, r.randrange(0x40), 0x100])
        chunk = bytearray(r.bytes(0x20))
        pos = r.choice([0, 1, 100])
        put(chunk, 4, 'II', STREAM, pos)
        put(chunk, 0x10, 'I', pos + r.choice([0, 1, 3, 4, 5, count, count + 3, 0x40]))
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), BUF: bytearray(r.bytes(0x100)), RAM: chunk}
        if op == 'ReadD':
            return Call(0x5441f0, [RAM], regions, ret='a')
        return Call(0x544160, [RAM, BUF, count], regions, ret='a')
    if op in ('Write', 'Read'):
        count = r.choice([0, 1, 4, 12, r.randrange(0x40)])
        left = r.choice([count, count, 0, r.randrange(0x40), 0x100])
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), BUF: r.bytes(0x100)}
        if op == 'Write':
            regions[RAM] = put(bytearray(r.bytes(0x20)), 4, 'I', STREAM)
            return Call(0x543490, [RAM, BUF, count], regions, ret='a')
        regions[RAM] = r.bytes(0x20)
        return Call(0x54bbb0, [RAM, STREAM, BUF, count, r.choice([RAM + 8, 0])], regions, ret='a')
    if op == 'Free':
        pool = bytearray(r.bytes(0x100 * 0xac))
        for i in range(0x100):
            pool[i * 0xac] |= 1
        if r.random() < 0.8:
            for _ in range(r.randrange(1, 4)):
                pool[r.choice([0, 1, 0xff, r.randrange(0x100)]) * 0xac] &= 0xfe
        put(globals_, 0x10, 'I', r.choice([POOL, POOL, 0]))
        return Call(0x549b50, [], {GLOBALS: globals_, POOL: pool}, ret='a')
    rides = bytearray(r.bytes(0x100 * 0xd0))
    if op in ('Worn', 'Dur', 'PushNode'):  # the handle of a ride (or a stale one, or 0 with the module loaded); the free node list at 0x877b8c
        nodes = bytearray(r.bytes(0x40))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        serial = r.choice([0, 1, r.getrandbits(24)])
        put(rides, index * 0xd0, 'I', serial)
        put(rides, index * 0xd0 + 0xc8, 'I', r.choice([LINKS + 0x20, 0, r.getrandbits(32)]))
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.85 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        put(globals_, 0x34, 'I', r.choice([LINKS, LINKS, 0]))
        handle = r.choice([index | (serial << 8) & 0xffffffff] * 4 + [index | (serial ^ 1) << 8 & 0xffffffff, r.getrandbits(32), 1])
        if op == 'Worn':
            return Call(0x544c00, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides}, ret='v')
        return Call(0x545100 if op == 'Dur' else 0x54abd0, [handle & 0xffffffff, r.getrandbits(32)], {GLOBALS: globals_, POOL: rides, LINKS: nodes}, ret='v')
    if op == 'PopNode':
        put(globals_, 0x34, 'I', r.getrandbits(32))
        nodes = bytearray(r.bytes(0x40))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        serial = r.choice([0, 1, r.getrandbits(24)])
        put(rides, index * 0xd0, 'I', serial)
        put(rides, index * 0xd0 + 0xc8, 'I', r.choice([LINKS, LINKS, 0]))
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.85 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        handle = r.choice([index | (serial << 8) & 0xffffffff] * 4 + [index | (serial ^ 1) << 8 & 0xffffffff, r.getrandbits(32), 0])
        return Call(0x54ab40, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: nodes}, ret='a')
    links = bytearray(r.bytes(0x40))
    put(links, 0, 'I', r.choice([0x400000, 0x400001, 0, r.getrandbits(32)]))
    index = r.choice([0, 1, 0xff, r.randrange(0x100)])
    serial = r.choice([0, 1, r.getrandbits(24), r.getrandbits(32)])
    put(rides, index * 0xd0, 'I', serial)
    put(rides, index * 0xd0 + 0x58, 'I', r.choice([LINKS, LINKS, 0]))
    loaded = r.random() < 0.8
    put(globals_, 0, 'I', 0x4a454647 if loaded else r.getrandbits(32))
    put(globals_, 8, 'I', POOL)
    handle = r.choice([index | (serial << 8) & 0xffffffff] * 4 + [index | (serial ^ 1) << 8 & 0xffffffff, r.getrandbits(32)])
    if not loaded and handle == 0:
        handle = 1  # the original reads address 0x58
    if op == 'Field5c':
        return Call(0x5452a0, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: links}, ret='a')
    return Call(0x549d30, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: links}, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
