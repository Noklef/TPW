# The GUI library's registry of named windows (src/game/ui/windowreg: 0x6588b6, 0x659a58, 0x659b52, 0x659bc9, 0x659c42) against the
# originals: a list at 0xfaa610 of 0-5 nodes (or none) whose hashes are mostly those of a few short names, their windows two objects with
# a recorder vtable (slot 0x20 answering a value now and then) or 0; names of 0-15 characters (high bytes too) from the same few or random;
# the heap now and then failing the node's allocation; Clear's word 0, 1, or set only in the high half.
# usage: windowreg.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Hash,Register,Clear,FindName,FindHash)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Hash': 0x6588b6, 'Register': 0x659a58, 'Clear': 0x659b52, 'FindName': 0x659bc9, 'FindHash': 0x659c42}
B = Bench([(0x6588b6, 0x6588ff), (0x659a58, 0x659cc0), (0x659ce0, 0x659d10), (0x67ae00, 0x67af10), (0x67ff70, 0x680000)])
NODES, WINDOWS, VTABLE, NAME = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
WINS = [WINDOWS, WINDOWS + 0x40]
WORDS = [b'a', b'ok', b'main', b'Dialog12', b'\xe9t\xe9', b'twelve_chars', b'thirteen_char']


def hash_(text, mul):
    h = 0
    for c in text:
        h = (((c - 256 if c > 127 else c) ^ h) * mul) & 0xffffffff
    return h


def name(r):
    if r.random() < 0.7:
        return r.choice(WORDS)
    return bytes(r.randrange(1, 256) for _ in range(r.choice([0, 1, 3, r.randrange(16)])))


def case(r):
    op = r.deal(OPS)
    log = log_region()
    if r.random() < 0.7:
        set_return(log, 8, r.getrandbits(32))
    n = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x18 * 5))
    for i in range(n):
        put(nodes, 0x18 * i, 'I', r.choice(WINS + WINS + [0]))
        put(nodes, 0x18 * i + 0x10, 'I', hash_(r.choice(WORDS), 0x2f) if r.random() < 0.8 else r.getrandbits(32))
        put(nodes, 0x18 * i + 0x14, 'I', NODES + 0x18 * (i + 1) if i + 1 < n else 0)
    regions = {LOG: log, VTABLE: vtable({0: 1}), NODES: nodes, 0xfaa610: struct.pack('<I', NODES if n else 0),
               WINDOWS: b''.join(struct.pack('<I', VTABLE) + r.bytes(0x3c) for _ in WINS), NAME: name(r) + b'\0' + r.bytes(0x10)}
    if op == 'Hash':
        mul = r.choice([0x2f, 0x2f, 1, 0, 0xffffffff, r.getrandbits(32)])
        return Call(CALLS[op], [NAME, mul], regions, ret='a')
    if op == 'Register':
        regions[HEAP] = heap(0x100, r.choice([0] * 8 + [1]))
        return Call(CALLS[op], [r.choice(WINS * 4 + [0]), NAME], regions, ret='w')
    if op == 'Clear':
        regions[HEAP] = heap(0x100)
        return Call(CALLS[op], [r.choice([0, 1, 1, 0x10000, r.getrandbits(32)])], regions)
    if op == 'FindName':
        return Call(CALLS[op], [NAME], regions, ret='a')
    return Call(CALLS[op], [r.choice([hash_(r.choice(WORDS), 0x2f)] * 3 + [r.getrandbits(32)])], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
