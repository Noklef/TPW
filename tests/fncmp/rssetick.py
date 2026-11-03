# The scripts' tick (src/game/rsse/rssetick: RSSE_Tick 0x5516b0) against the original. The VM (0x551cb0) runs as it is on a little code of its opcodes 1 (the lock set), 2 (the lock cleared, the slice over), 4 (a word
# fetched), its code's end killing the script; the tick's other calls are cdecl recorders (the driver's RsseTickEnv). The fixture: 1-4 scripts on the list, with their speed's receivers, children, and object nodes of
# kinds 1-11 on a ride's parts (some null).
# usage: rssetick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rsseexec  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, strings  # noqa: E402

CALLS = {0x474020: (29, 2), 0x55d950: (22, 2), 0x55d9b0: (19, 2), 0x43b2b0: (3, 2), 0x545180: (30, 2), 0x551b30: (9, 4), 0x51fe30: (21, 4), 0x51fd90: (29, 4), 0x51c270: (30, 4), 0x559060: (18, 2)}
HOOKS = dict(rsseexec.HOOKS)
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, (k, n) in CALLS.items()})
B = Bench([(0x5516b0, 0x551b06)] + rsseexec.RANGES, hooks=HOOKS)
SCRIPTS, OBJECT, RIDES, HOLDER, PARTS, CODE, VARS, NODES = RAM, RAM + 0x7c0, RAM + 0x800, RAM + 0x840, RAM + 0x900, RAM + 0xb00, RAM + 0xc00, RAM + 0xd00
THINGS = 0x7cfb90
SPEEDS = [0, 100, -100, 50, 1, -1, 32767, -32768]
OPS = ['Off', 'Empty'] + ['List'] * 6
CODE_WORDS = [0x80000002, 0x80000001, 0x80000004, 0x7fffffff, 0x80000002, 0x80000004, 5, 0x80000004, 6, 0x80000001, 0x80000002, 0x80000004, 7]


def value(r):
    return r.choice([0.0, 1.5, -2.25, 300.5, 30000.5, -70000.0, 3e9, -3e9, 2097152.5, r.randrange(-5000, 5000) / 8])


def script(r, i, count, ids, used):
    """Script i: its bytes, and the bytes of its nodes (numbered from used)."""
    ctx = bytearray(r.bytes(0x100))
    put(ctx, 0, 'I', SCRIPTS + 0x100 * (i + 1) if i + 1 < count else 0)
    put(ctx, 8, 'i', ids[i])
    put(ctx, 0xc, 'i', r.choice([0] * 12 + ids * 4 + [0x7ffffff]))   # (no such child: a fault in both)
    put(ctx, 0x18, 'I', r.choice([CODE, CODE, CODE, 0]))
    put(ctx, 0x1c, 'I', VARS)
    put(ctx, 0x3c, 'i', r.choice([0, 0, 0, 1, 4, 9, 11, -1, -10000]))
    put(ctx, 0x50, '2I', r.choice([len(CODE_WORDS), len(CODE_WORDS), 3, 6]), 8)
    put(ctx, 0x8c, 'i', r.choice([0, 3, 4, 16, -1]))
    put(ctx, 0x94, 'i', r.choice([0, 1, 2, 3, 6, -1, -5]))
    put(ctx, 0x9c, 'I', r.choice([0, 0, r.getrandbits(32)]))
    put(ctx, 0xac, 'H', r.choice([0, 0, 1, 2, 3]))
    put(ctx, 0xb8, 'B', r.choice([0, 0, 1, r.getrandbits(8)]))
    put(ctx, 0xc0, 'h', r.choice(SPEEDS))
    owner = r.choice([0, 1, 2, 3])
    put(ctx, 0xc8, 'i', owner)
    put(ctx, 0xe0, 'I', r.choice([0, 0, r.getrandbits(32)]))
    put(ctx, 0xe4, 'h', r.choice([0, 100, -100, 1000, r.getrandbits(15)]))
    nodes = bytearray()
    if owner and r.random() < 0.8:   # (with no owner a node's part faults in both)
        n = r.randrange(1, 4)
        put(ctx, 0xb0, 'I', NODES + 0x20 * used)
        for k in range(n):
            node = bytearray(r.bytes(0x20))
            put(node, 0, 'I', NODES + 0x20 * (used + k + 1) if k + 1 < n else 0)
            put(node, 8, 'i', r.choice([1, 1, 1, 2, 2, 2, 3, 4, 7, 10, 11, 0, -1]))
            put(node, 0x10, 'i', r.choice([0, 1, 5, 5, -1]))
            put(node, 0x14, 'I', r.randrange(0, 4))
            nodes += node
    else:
        put(ctx, 0xb0, 'I', 0)
    return ctx, nodes


def case(r):
    op = r.deal(OPS)
    log = log_region()
    tick = r.getrandbits(32)
    count = r.randrange(1, 5)
    ids = [(r.randrange(1, 1000) << 3) | r.choice([(tick + 1) & 7, r.randrange(8)]) for _ in range(count)]
    regions = {LOG: log,
               0x87919c: struct.pack('<7I', r.getrandbits(32), 0 if op == 'Off' else 1, tick, r.getrandbits(32), r.getrandbits(32), 0 if op == 'Empty' else SCRIPTS, r.getrandbits(32)),   # (lock, on, counter, -, -, head, answer)
               0x7a4610: struct.pack('<4I', 0, RIDES, RIDES + 0x10, RIDES),
               RIDES: b''.join(struct.pack('<3I', r.getrandbits(32), r.getrandbits(32), HOLDER) + r.bytes(4) for _ in range(2)),
               HOLDER: r.bytes(0x28) + struct.pack('<I', HOLDER + 0x30) + r.bytes(8) + struct.pack('<I', PARTS),
               OBJECT: put_at(r.bytes(0x2c), 0x28, r.choice([0, r.getrandbits(32)])),
               THINGS: b''.join(struct.pack('<I', r.choice([0, OBJECT, OBJECT])) + r.bytes(16) for _ in range(4)),
               CODE: struct.pack('<%dI' % len(CODE_WORDS), *CODE_WORDS),
               VARS: r.bytes(0x40)}
    table = b''.join(struct.pack('<II', r.getrandbits(32), r.choice([PARTS + 0x60 + 0x40 * i] * 3 + [0])) + r.bytes(12) for i in range(4))
    regions[PARTS] = table.ljust(0x60, b'\0') + b''.join(r.bytes(0x30) + struct.pack('<3f', value(r), value(r), value(r)) + r.bytes(4) for _ in range(4))
    scripts, nodes = bytearray(), bytearray()
    for i in range(count):
        ctx, more = script(r, i, count, ids, len(nodes) // 0x20)
        scripts += ctx
        nodes += more
    regions[SCRIPTS] = scripts
    if nodes:
        regions[NODES] = nodes
    # Ride_PartAxes' direction: written through its pointer to the stack (so not logged); its last argument is null
    strings(log, 9, 0, out=4)
    fill(log, 9, 2, struct.pack('<3f', value(r), value(r), value(r)))
    return Call(0x5516b0, [], regions, ret='a')


def put_at(data, at, word):
    return data[:at] + struct.pack('<I', word) + data[at + 4:]


if __name__ == '__main__':
    B.main(case)
