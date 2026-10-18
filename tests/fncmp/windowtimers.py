# The GUI library's window timers (src/game/ui/windowtimers: 0x661faa, 0x6620e2, 0x662306, 0x662500) against the originals: a list of
# 0-6 nodes (linked both ways, now and then a previous link missing), their windows among two and 0 and their ids among the window
# timers' 0x80080-0x80083 and others (repeats included), the head [0xfaa634] the first node, a later one or 0; the window and id asked
# for from the nodes' or others, the delete's flags with bit 0 set or not.
# usage: windowtimers.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unlink,Find,Delete,Kill)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'Unlink': 0x661faa, 'Find': 0x6620e2, 'Delete': 0x662500, 'Kill': 0x662306}
B = Bench([(0x661f9b, 0x661fe5), (0x6620e2, 0x66213e), (0x662306, 0x662384), (0x662500, 0x66252c)])
NODES = RAM
WINDOWS = [RAM + 0x1000, RAM + 0x1200, 0]
IDS = [0x80080, 0x80081, 0x80082, 0x80083, 1, 0x10]


def case(r):
    op = r.deal(OPS)
    n = r.choice([0, 1, 1, 2, 3, 4, 6]) if op == 'Kill' else r.choice([1, 2, 3, 4, 6])
    nodes = [NODES + 0x20 * i for i in range(n)]
    buf = bytearray(r.bytes(0x20 * n))
    for i in range(n):
        put(buf, 0x20 * i, 'I', r.choice(WINDOWS[:2] * 3 + [0, r.getrandbits(32)]))
        put(buf, 0x20 * i + 0xc, 'I', r.choice(IDS))
        put(buf, 0x20 * i + 0x18, 'I', nodes[i + 1] if i + 1 < n else 0)
        put(buf, 0x20 * i + 0x1c, 'I', nodes[i - 1] if i and r.random() < 0.9 else 0)
    regions = {NODES: buf} if n else {}
    regions[0xfaa634] = struct.pack('<I', r.choice([nodes[0]] * 6 + nodes + [0]) if n else 0)
    window = r.choice(WINDOWS + [r.getrandbits(32)])
    ident = r.choice(IDS + [r.getrandbits(32)])
    if r.random() < 0.5 and n:   # one that is there
        k = r.randrange(n)
        window, ident = struct.unpack_from('<I', buf, 0x20 * k)[0], struct.unpack_from('<I', buf, 0x20 * k + 0xc)[0]
    if op == 'Kill':
        return Call(CALLS[op], [window, ident], regions, ret='a')
    node = r.choice(nodes)
    if op == 'Find':
        return Call(CALLS[op], [node, window, ident], regions, conv='t', ret='a')
    if op == 'Delete':
        return Call(CALLS[op], [node, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(CALLS[op], [node], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
