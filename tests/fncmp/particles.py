# The particle systems' handle accessors (src/game/render/particles: 0x5222e0, 0x522320,
# 0x51fd80, 0x51fa80, 0x51feb0, 0x51ff70, 0x51ff10, 0x51ffd0, 0x520470, 0x51fe30, 0x520030, 0x51fd90, 0x5200b0, 0x5204c0, 0x521810, 0x521850, 0x520e00, 0x520560) against the originals:
# handles live, stale (serial off), past the slots or 0, before and after set-up; effect numbers
# about 0x69; ages and lifetimes about the quarters, negative and zero.
# usage: particles.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alive,Effector,...)
# AtWindow (0x486b90): Spawn's set-up and a window (rectangle and insets random) through the GUI rectangle code.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG2, LOG, RAM, Bench, Call, log_region, put, set_return  # noqa: E402

RANGES = [(0x5222e0, 0x52231b), (0x522320, 0x522356), (0x51fd80, 0x51fd8a), (0x51fa80, 0x51faa0),
          (0x51feb0, 0x51ff06), (0x51ff10, 0x51ff68), (0x51ff70, 0x51ffc6), (0x51ffd0, 0x520028),
          (0x520470, 0x5204ba), (0x51fe30, 0x51fea4), (0x520030, 0x5200a4), (0x51fd90, 0x51fe2d), (0x5200b0, 0x520123), (0x5204c0, 0x52055f), (0x521810, 0x521845), (0x521850, 0x52188a), (0x520e00, 0x520f0c),
          (0x520560, 0x520d60), (0x520d60, 0x520e00), (0x521e60, 0x5222e0), (0x521d60, 0x521e60), (0x522360, 0x5224f0),
          (0x5214a0, 0x521807), (0x521890, 0x521930), (0x5da3c0, 0x5da3c1), (0x521930, 0x521d54), (0x520f10, 0x5214a0), (0x520130, 0x520469), (0x5224f0, 0x5226fa),
          (0x486b90, 0x486c70), (0x659d00, 0x659d43), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aa90), (0x66bff7, 0x66c00b),
          (0x65d42e, 0x65d49e), (0x65d773, 0x65d7bc), (0x65a43e, 0x65a493)]
B = Bench(RANGES)
SYSTEMS, EFFECTS, EFFECTORS, SETUP = 0x80cefc, 0x804370, 0x816504, 0x816d40
CALLS = {'Alive': 0x5222e0, 'Effector': 0x522320, 'Reduced': 0x51fd80, 'Clear': 0x51fa80, 'End': 0x51feb0,
         'Age': 0x51ff70, 'Set2a': 0x51ff10, 'SetA2': 0x51ffd0, 'Quarter': 0x520470, 'VecA': 0x51fe30, 'VecB': 0x520030, 'Scaled': 0x51fd90, 'EndAll': 0x5200b0, 'Release': 0x5204c0, 'RandS': 0x521810, 'Rand': 0x521850, 'Move': 0x520e00, 'Emit': 0x520560, 'Spawn': 0x521e60, 'Update': 0x5214a0, 'SpawnVel': 0x521930, 'Effectors': 0x520f10, 'Tick': 0x520130, 'AtWindow': 0x486b90}
NODES, SINES = RAM + 0x1000, RAM + 0x2000


def case(r):
    op = r.deal(OPS)
    setup = bytearray(r.bytes(0x10))
    put(setup, 0, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    if op == 'Reduced':
        return Call(0x51fd80, [r.choice([0, 1, r.getrandbits(32)])], {0x80ced0: r.bytes(0x10)})
    if op == 'Clear':
        effect = r.choice([0, 0x68, 0x69, r.randrange(0x69), r.getrandbits(32)])
        return Call(0x51fa80, [effect], {SETUP: setup, EFFECTS: r.bytes(0x69 * 0x140)})
    if op == 'Effector':
        slot = r.choice([0, 0x13, 0x14, r.randrange(0x14), r.getrandbits(16)])
        effectors = bytearray(r.bytes(0x15 * 0x68))  # one past the slots
        serial = r.getrandbits(16)
        if slot <= 0x14:
            put(effectors, slot * 0x68, 'H', r.choice([serial, serial, serial ^ 1]))
        return Call(0x522320, [serial << 16 | slot], {SETUP: setup, EFFECTORS - 0x10: bytes(0x10) + effectors}, ret='a')
    if op == 'Move':
        return move(r)
    if op == 'Emit':
        return emit(r)
    if op == 'Spawn':
        return spawn(r)
    if op == 'Update':
        return update(r)
    if op == 'Effectors':
        return pull(r)
    if op == 'Tick':
        return tick(r)
    if op == 'AtWindow':  # 0x486b90: Spawn's cases at a window's centre: its rectangle and insets small or edges, shown or not (no parents)
        c = spawn(r)
        win = bytearray(r.bytes(0x120))
        for k in list(range(0xc, 0x14, 2)) + list(range(0xf8, 0x100, 2)):
            put(win, k, 'h', r.choice([0, 1, 2, 5, 0x7fff, -0x8000, -1, r.randrange(-50, 50), r.randrange(-700, 700), r.randrange(-700, 700)]))
        win[0x48] = r.choice([0, 0, 0, 1, 2, 3])
        put(win, 0x60, 'I', 0)
        c.regions[RAM + 0x3000] = win
        c.regions[0x786b84] = struct.pack('<I', r.choice([0, 0, 0, 0, 1, 0x100, r.getrandbits(32)]))
        return Call(0x486b90, [c.args[0], RAM + 0x3000], c.regions, ret='v')
    if op == 'SpawnVel':  # 0x521930: Spawn's cases with a velocity, the systems' scales about 1024
        c = spawn(r)
        defs = c.regions[0x804370]
        for k in range(0x69):
            put(defs, 0x140 * k + 0xb0, 'i', r.choice([0, 1024, 512, 2048, -1024, r.randrange(-0x10000, 0x10000), r.getrandbits(32) - (1 << 31)]))
        velocity = [r.choice([0, 1, -1, 1023, -1025, r.randrange(-0x100000, 0x100000), r.getrandbits(32)]) & 0xffffffff for _ in range(3)]
        return Call(0x521930, c.args + velocity, c.regions, ret='a')
    systems = bytearray(r.bytes(0x78 * 0x140))
    if op in ('RandS', 'Rand'):
        n = r.choice([0, 1, 2, 3, 7, 100, -1, -7, 0x7fffffff, -0x80000000, r.randrange(-1000, 1000)])
        return Call(CALLS[op], [n & 0xffffffff], {0x80cee0: r.bytes(8)}, ret='a')
    if op == 'Release':
        heads = bytearray(r.bytes(0x50))
        slot = r.choice([0, 1, 0x77, r.randrange(0x78), r.randrange(0x78), -1, -5])
        pick = lambda: r.choice([-1, -1, 0, 1, r.randrange(0x78), slot if slot >= 0 else 2])
        if slot >= 0:
            put(systems, slot * 0x140 + 0xd0, 'hh', pick(), pick())
        put(heads, 0x1c, 'hh', r.choice([slot, slot, pick()]), pick())
        return Call(0x5204c0, [slot & 0xffffffff], {0x816d00: heads, SYSTEMS: systems})
    if op == 'EndAll':
        heads = bytearray(r.bytes(0x50))
        put(heads, 0x40, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
        effectors = bytearray(r.bytes(0x14 * 0x68))

        def chain(slots, count, buf, base, link, flag):
            order = r.sample(range(slots), r.choice([0, 1, 2, 4]) if slots > 4 else 0)
            for k, s in enumerate(order):
                put(buf, s * base + link, 'h', order[k + 1] if k + 1 < len(order) else -1)
                buf[s * base + flag] = r.choice([0, 1, 0xff])
            return order[0] if order else -1

        put(heads, 0x1c, 'hh', chain(0x78, 0, systems, 0x140, 0xd0, 5), chain(0x14, 0, effectors, 0x68, 0x60, 0x45))
        regions = {0x816d00: heads, SYSTEMS: systems, 0x8164fc: effectors}
        return Call(0x5200b0, [], regions)
    if op == 'Quarter':
        slot = r.randrange(0x78)
        life = r.choice([0, 1, 4, r.randrange(1, 200), -r.randrange(1, 200), r.getrandbits(32)])
        age = r.choice([0, -2, r.randrange(-10, 300), r.getrandbits(32)])
        if life & 0xffffffff == 0xffffffff and (age << 2) & 0xffffffff == 0x80000000:
            age = 0  # the original faults
        put(systems, slot * 0x140 + 0x20, 'I', age & 0xffffffff)
        put(systems, slot * 0x140 + 0xd4, 'I', life & 0xffffffff)
        return Call(0x520470, [slot], {SYSTEMS: systems}, ret='a')
    slot = r.choice([0, 0x77, 0x78, r.randrange(0x78), r.getrandbits(16)])
    serial = r.getrandbits(16)
    if slot < 0x78:
        put(systems, slot * 0x140 + 0xa, 'H', r.choice([serial, serial, serial ^ 0x100]))
    handle = r.choice([serial << 16 | slot] * 6 + [0, slot])
    if handle == 0:  # slot 0 with serial 0 would pass but for the check of 0
        put(systems, 0xa, 'H', 0)
    args = [handle]
    if op in ('VecA', 'VecB', 'Scaled'):
        args += [r.choice([0, 16, -16, -1, -1023, -1024, 1023, 1025, 0x7fffffff, -0x80000000, r.getrandbits(32)]) & 0xffffffff for _ in range(3)]
        if op == 'Scaled' and slot < 0x78:
            put(systems, slot * 0x140 + 0xb0, 'I', r.choice([1, 1, 0x400, 3, r.getrandbits(32)]))
        return Call(CALLS[op], args, {SETUP: setup, SYSTEMS: systems}, ret='v')
    if op != 'Alive' and op != 'End':
        args.append(r.choice([0, 0xffff, 0x10000, r.getrandbits(32)]))
    return Call(CALLS[op], args, {SETUP: setup, SYSTEMS: systems}, ret='a' if op == 'Alive' else 'v')


def move(r):
    small = lambda: r.choice([0, 1, -1, 2, -2, 3, -3, r.randrange(-5000, 5000), r.randrange(-5000, 5000), r.getrandbits(32)])
    p = bytearray(r.bytes(0x80))
    y = r.randrange(-0x8000, 0x8000)
    put(p, 0x14, 'iii', r.randrange(-0x8000, 0x8000), y, r.randrange(-0x8000, 0x8000))
    put(p, 0x24, 'h', r.choice([0, 1, 512, 1024, -1024, 100, r.randrange(-0x8000, 0x8000)]))
    put(p, 0x28, 'h', r.choice([0, 1, 10, -10, r.randrange(-0x8000, 0x8000)]))
    put(p, 0x2c, 'III', *[small() & 0xffffffff for _ in range(3)])
    put(p, 0x76, 'H', r.choice([0, 1, 1, 1, 0x100, r.getrandbits(16)]))
    if r.random() < 0.1:
        put(p, 0x14, 'III', *[r.getrandbits(32) for _ in range(3)])
    log = log_region()
    if r.random() < 0.9:  # the ground height about the particle (x16), else what the logger makes of its arguments
        set_return(log, 0x3e, (y + r.randrange(-6000, 6000)) * 16 + r.randrange(16))
    if r.random() < 0.3:  # the ground at the moved particle, or as far below it as its new y velocity (the boundaries)
        s32 = lambda v: (v & 0xffffffff) - ((v & 0x80000000) << 1)
        drag, gravity, vy, y0 = struct.unpack_from('<h2xh2x', p, 0x24) + struct.unpack_from('<i', p, 0x30) + struct.unpack_from('<i', p, 0x18)
        product = s32(vy * drag)
        vy = s32(vy - (abs(product) // 1024) * (1 if product >= 0 else -1) - gravity)  # (the division truncates toward 0)
        y1 = s32(y0 + vy)
        set_return(log, 0x3e, (s32(y1 - r.choice([0, 0, vy, vy, 1, -1, vy + 1, vy - 1])) << 4) + r.randrange(16))
    return Call(0x520e00, [RAM], {RAM: p, LOG: log, 0x80cef0: put(bytearray(r.bytes(0x10)), 4, 'I', CDECL_LOG2)}, ret='v')


def emit(r):
    """0x520560: a system of the first four (each field about its branches), the free list of 0x34-byte records (from [0x80cef8]) Take hands out (at times
    empty), the sines at [0x80ceec] (about +-0x10000, at times any), the seed and the emit count."""
    slot = r.randrange(4)
    systems = bytearray(r.bytes(0x140 * 4))
    e = 0x140 * slot
    small = lambda: r.choice([0, 1, -1, 2, 3, 0x10, 0x100, -0x100, 0x1000, r.randrange(-0x10000, 0x10000), r.getrandbits(32)])
    put(systems, e + 0x78, 'i', r.choice([0, 1, 3, 4, 7, -4, -1, 0x40, 0x1000, r.randrange(-0x100, 0x100)]))
    for off in (0x6c, 0x6d, 0xc2):
        systems[e + off] = r.choice([0, 0, 1, 0x80])
    put(systems, e + 0x58, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    extents = [r.choice([0, small(), r.randrange(1, 0x8000), -r.randrange(1, 0x8000)]) for _ in range(3)]
    if r.random() < 0.2:
        extents = [0, 0, 0]
    elif r.random() < 0.2:
        extents[r.randrange(3)] = 0
    put(systems, e + 0x44, 'III', *(v & 0xffffffff for v in extents))
    put(systems, e + 0xa0, 'H', r.choice([0, 0, 1, r.getrandbits(16)]))
    put(systems, e + 0xa8, 'H', r.choice([0, 0, 1, r.getrandbits(16)]))
    put(systems, e + 0xac, 'I', r.choice([0, 0, 8, 7, -7, 0x400, -0x400, r.randrange(-0x4000, 0x4000), r.getrandbits(32)]) & 0xffffffff)
    put(systems, e + 0xc4, 'h', r.choice([0, 0, 1, -1, r.randrange(-0x8000, 0x8000)]))
    put(systems, e + 0x54, 'I', r.choice([0, 7, -7, 0x800, r.randrange(-0x4000, 0x4000), r.getrandbits(32)]) & 0xffffffff)
    put(systems, e + 0x5c, 'I', r.choice([0, 0, small()]) & 0xffffffff)
    put(systems, e + 0x50, 'h', r.choice([0, 0, 1, -1, 0x100, r.randrange(-0x8000, 0x8000)]))
    nodes = bytearray(r.bytes(0x34 * 12))
    free = r.sample(range(12), r.choice([0, 1, 3, 12]) if r.random() < 0.2 else r.choice([1, 3, 11]))
    put(systems, e + 6, 'h', r.choice([-1] + [k for k in range(12) if k not in free]))  # the system's own list (Take links the record in front)
    for a, b in zip(free, free[1:] + [-1]):
        put(nodes, 0x34 * a + 0x30, 'h', b)
    if r.wild or r.random() < 0.1:
        sines = r.bytes(0x800)
    else:
        sines = b''.join(struct.pack('<i', r.randrange(-0x10000, 0x10001)) for _ in range(0x200))
    regions = {0x80cefc: systems, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: struct.pack('<h', free[0] if free else -1),
               0x80ceec: struct.pack('<I', SINES), SINES: sines, 0x80cee0: r.bytes(4), 0x80ced0: r.bytes(4)}
    return Call(0x520560, [slot], regions, ret='a')


def spawn(r):
    """0x521e60: the effect definitions (each one's burst small, its link none, itself, a later effect or a voice, so that the chain ends), six systems with a
    free list and a live list, the voices' templates and instances (the counter mostly 0: the voice mark indexes by the whole handle), and Emit's set-up."""
    defs = bytearray(r.bytes(0x69 * 0x140))
    for k in range(0x69):
        d = 0x140 * k
        put(defs, d, 'h', r.choice([1, 1, 0, -1, 2]))
        put(defs, d + 0xe, 'h', r.choice([0, 0, 1, 2, -1]))
        put(defs, d + 0x12, 'h', r.choice([0, 1, -1, 0x10, r.randrange(-0x8000, 0x8000)]))
        put(defs, d + 0x60, 'I', r.choice([0, 0, 1, 2, 3]))
        for off in (5, 0xc0, 0xc1, 0xba):
            defs[d + off] = r.choice([0, 0, 1, 0x80])
        put(defs, d + 0xb8, 'H', r.choice([0, 0, 1, r.getrandbits(16)]))
        for off in (0x84, 0x88, 0x8c):
            put(defs, d + off, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
        if defs[d + 0xba]:
            link = r.choice([-1, 0, 5, 19, 20, r.randrange(20)])
        else:
            link = r.choice([-1, -1, k, 0x69, r.randrange(k + 1, 0x6a)])
        put(defs, d + 0xb4, 'i', link)
    systems = bytearray(r.bytes(0x140 * 6))
    order = r.sample(range(6), 6)
    nfree = r.choice([0, 1, 2, 3, 6]) if r.random() < 0.15 else r.choice([3, 4, 6])
    free, live = order[:nfree], order[nfree:]
    for chain in (free, live):
        for i, a in enumerate(chain):
            put(systems, 0x140 * a + 0xd0, 'hh', chain[i + 1] if i + 1 < len(chain) else -1, chain[i - 1] if i else -1)
    nodes = bytearray(r.bytes(0x34 * 12))
    records = r.sample(range(12), r.choice([0, 1, 3, 12]))
    for a, b in zip(records, records[1:] + [-1]):
        put(nodes, 0x34 * a + 0x30, 'h', b)
    instances = bytearray(r.bytes(0x68 * 12))
    vfree = r.sample(range(12), r.choice([0, 2, 12]))
    for i, a in enumerate(vfree):
        put(instances, 0x68 * a + 0x60, 'hh', vfree[i + 1] if i + 1 < len(vfree) else -1, -1)
    heads = bytearray(r.bytes(0x28))  # 0x816d1c: the live systems, the voices' active and free lists, the systems' and the records' free lists; 0x816d40 the set-up
    put(heads, 0, 'hhhhh', live[0] if live else -1, -1, free[0] if free else -1, vfree[0] if vfree else -1, records[0] if records else -1)
    put(heads, 0x24, 'I', 0 if r.random() < 0.06 else 1)
    state = bytearray(r.bytes(0x2c))  # 0x80ced0: the emit count, reduced, the seed, the serials, the sines, the scale factor, the records
    put(state, 8, 'I', r.choice([0, 0, 0, 1]))
    put(state, 0x18, 'HH', r.getrandbits(16), r.choice([0, 0, 0, 0, 0, 0, 0, 0, 0, 1]))
    put(state, 0x1c, 'I', SINES)
    put(state, 0x20, 'I', r.choice([0, 1024, 512, 2000, r.getrandbits(12)]))
    put(state, 0x28, 'I', NODES)
    sines = b''.join(struct.pack('<i', r.randrange(-0x10000, 0x10001)) for _ in range(0x200))
    effect = r.choice([0x69, -1]) if r.random() < 0.08 else r.choice([0, 1, 0x68, r.randrange(0x69), r.randrange(0x69)])
    pos = [r.choice([0, 15, 16, -16, 0x12345, r.randrange(-0x100000, 0x100000), r.getrandbits(32)]) & 0xffffffff for _ in range(3)]
    regions = {0x804370: defs, 0x80cefc: systems, 0x80ced0: state, 0x816d1c: heads, NODES: nodes, SINES: sines, 0x8164fc: instances,
               0x80c6b0: r.bytes(0x68 * 20)}
    return Call(0x521e60, [effect & 0xffffffff] + pos, regions, ret='a')


def update(r):
    """0x5214a0: Spawn's set-up (for the death effects), six systems (a live list of up to four, the rest free) each with a list of up to three of 16
    records (at times another system's), the rest of the records free; ages about 0, lives from 1, each field about its branches."""
    regions = spawn(r).regions
    defs = regions[0x804370]
    for k in range(0x69):  # no voice marks: a second voice start's would index past the instances by its counter (Spawn's cases cover the mark)
        if defs[0x140 * k + 0xba]:
            put(defs, 0x140 * k + 0xb8, 'H', 0)
    systems = bytearray(r.bytes(0x140 * 6))
    order = r.sample(range(6), 6)
    nlive = r.choice([0, 1, 2, 3, 4, 4])
    live, free = order[:nlive], order[nlive:]
    for chain in (live, free):
        for i, a in enumerate(chain):
            put(systems, 0x140 * a + 0xd0, 'hh', chain[i + 1] if i + 1 < len(chain) else -1, chain[i - 1] if i else -1)
    nodes = bytearray(r.bytes(0x34 * 16))
    records = r.sample(range(16), 16)

    def link(chain):
        for i, a in enumerate(chain):
            put(nodes, 0x34 * a + 0x30, 'hh', chain[i + 1] if i + 1 < len(chain) else -1, chain[i - 1] if i else -1)
    for a in range(6):
        e = 0x140 * a
        put(systems, e + 2, 'H', r.choice([0, 0, 0, 0, 0, 0, 1, 2, 0x8000]))
        chain = [records.pop() for _ in range(min(len(records), r.choice([0, 1, 2, 3])))]
        link(chain)
        put(systems, e + 6, 'h', chain[0] if chain else -1)
        for k in chain:
            n = 0x34 * k
            put(nodes, n + 0x1c, 'ii', r.choice([1, 2, 15, 16, 100, r.randrange(1, 0x10000)]), r.choice([0, 0, 1, 2, 5, 100, -1, r.randrange(-0x100, 0x10000)]))
            put(nodes, n + 0x24, 'Hh', r.choice([1, 2, 0]), a if r.random() < 0.9 else r.randrange(6))
            put(nodes, n, 'iiiiii', *[r.randrange(-0x40000, 0x40000) for _ in range(6)])
        put(systems, e + 0x98, 'i', r.choice([-1, -1, -1, 0, 0x69, r.randrange(0x69)]))
        put(systems, e + 0x20, 'i', r.choice([0, 1, -1, -2, 100]))
        put(systems, e + 0x58, 'I', r.choice([0, 0, 1, 2, 3]))
        put(systems, e + 0x78, 'i', r.choice([0, 1, -1, 100]))
        put(systems, e + 0x96, 'H', r.choice([0, 0, 1, 4, 16, 0x8000, r.getrandbits(16)]))
        put(systems, e + 0x74, 'h', r.choice([0, 0x400, 0x1000, -0x400, r.randrange(-0x8000, 0x8000)]))
        put(systems, e + 0xa6, 'h', r.choice([0, 0x400, 0x2000, r.randrange(-0x8000, 0x8000)]))
        low = r.choice([0, 1, -1, r.randrange(-0x100, 0x100), r.randrange(-0x8000, 0x8000)])
        put(systems, e + 0xbe, 'h', low)
        put(systems, e + 0xc6, 'H', r.choice([low, low, low + 1, low + 0x10, r.randrange(-0x8000, 0x8000)]) & 0xffff)
        put(systems, e + 0x7c, 'i', r.choice([0, 0, 100, 1024, -100, r.getrandbits(32) - (1 << 31)]))
        put(systems, e + 0x80, 'i', r.choice([0, 1, 10, r.randrange(-0x1000, 0x1000)]))
        put(systems, e + 0x84, 'iii', *[r.choice([0, 1, 10, -10, r.randrange(-0x1000, 0x1000)]) for _ in range(3)])
        for off in (5, 0x6f, 0xaa):
            systems[e + off] = r.choice([0, 0, 1])
        put(systems, e + 0x128, 'iiiiii', *[r.choice([0x7fffffff, -0x80000000, 0, r.randrange(-0x40000, 0x40000)]) for _ in range(6)])
    link(records)  # the rest: the free list
    heads = bytearray(regions[0x816d1c])
    put(heads, 0, 'h', live[0] if live else -1)
    put(heads, 4, 'h', free[0] if free else -1)
    put(heads, 8, 'h', records[0] if records else -1)
    put(heads, 0x24, 'I', 1)
    regions.update({0x80cefc: systems, NODES: nodes, 0x816d1c: heads})
    return Call(0x5214a0, [], regions, ret='v')


def pull(r):
    """0x520f10: Update's systems and records (positions near the effectors, so the squared distances stay in range), up to four active effectors of 12
    (the rest at times free, for the end voices), each field about its branches; the ground callback the logger, its height about the effectors'."""
    regions = update(r).regions
    systems, nodes, heads, state = regions[0x80cefc], regions[NODES], regions[0x816d1c], regions[0x80ced0]
    for a in range(6):
        systems[0x140 * a + 4] = r.choice([0, 1, 2, 0xff, r.getrandbits(8)])
        put(systems, 0x140 * a + 0x9c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    for k in range(16):
        put(nodes, 0x34 * k, 'iii', *[r.randrange(-0x2000, 0x2000) for _ in range(3)])
        put(nodes, 0x34 * k + 0x2a, 'h', r.choice([0, 1, 2, -1, -2, r.randrange(-0x8000, 0x8000)]))
    instances = bytearray(r.bytes(0x68 * 12))
    order = r.sample(range(12), 12)
    nactive = r.choice([0, 1, 2, 3, 4, 4])
    nfree = r.choice([0, 1, 2, 8])
    active, free = order[:nactive], order[nactive:nactive + nfree]
    for chain in (active, free):
        for i, a in enumerate(chain):
            put(instances, 0x68 * a + 0x60, 'hh', chain[i + 1] if i + 1 < len(chain) else -1, chain[i - 1] if i else -1)
    small = lambda: r.choice([0, 0, 1, -1, 7, 0x100, -0x100, r.randrange(-0x1000, 0x1000)])
    for a in active:
        e = 0x68 * a
        put(instances, e + 4, 'i', r.choice([-1, -1, 0, 1, 2, -2, r.getrandbits(32) - (1 << 31)]))
        instances[e + 0xa] = r.choice([0, 0, 0, 1])
        instances[e + 0xb] = r.choice([0, 1, 1])
        put(instances, e + 0xc, 'iii', *[r.randrange(-0x1000, 0x1000) for _ in range(3)])
        put(instances, e + 0x18, 'iii', *[small() for _ in range(3)])
        if r.random() < 0.2:
            put(instances, e + 0x18, 'iii', 0, 0, 0)
        put(instances, e + 0x24, 'i', r.choice([0, 0, 100, 1024, -100, r.randrange(-0x8000, 0x8000)]))
        put(instances, e + 0x28, 'i', r.choice([0, 0, 1, 10, -10, r.randrange(-0x100, 0x100)]))
        put(instances, e + 0x2c, 'i', r.choice([0, 0, 1, 0x100, 0x1000, r.randrange(0, 0x10000)]))
        put(instances, e + 0x30, 'i', r.choice([0, 1, 2, 2, 3, -1]))
        put(instances, e + 0x34, 'iiii', *[r.choice([0, 0, 1, 7, 0x100, -0x100]) for _ in range(3)], r.choice([0, 1, -1, 100, 0x1000, r.randrange(-0x10000, 0x10000)]))
        voice = r.choice([0, 0, 1, 0xff])
        instances[e + 0x49] = voice
        put(instances, e + 0x46, 'h', r.choice([-1, 0, 19, 20, r.randrange(20)]) if voice else r.choice([-1, 0x69, r.randrange(0x69), r.randrange(0x69)]))
        put(instances, e + 0x4a, 'H', r.choice([0, 0, 0, 1, 0x100]))
        put(instances, e + 0x4c, 'i', r.choice([0, 0, 1, 2, 100, -1, -5]))
    put(heads, 2, 'h', active[0] if active else -1)
    put(heads, 6, 'h', free[0] if free else -1)
    log = log_region()
    if r.random() < 0.9:  # the ground height about the effectors' (x16), else what the logger makes of its arguments
        set_return(log, 0x3e, r.randrange(-0x2000, 0x2000) * 16 + r.randrange(16))
    s32 = lambda v: (v & 0xffffffff) - ((v & 0x80000000) << 1)
    trunc = lambda a, b: abs(a) // b * (1 if a >= 0 else -1)
    boundary = r.random() if active else 1
    if boundary < 0.3:  # the first effector moving, the ground at its new y or as far below it as its new y velocity (the bounce's boundaries)
        e = 0x68 * active[0]
        instances[e + 0xa], instances[e + 0xb] = 0, 1
        put(instances, e + 0x4c, 'i', 100)
        y0, vy, drag, gravity = struct.unpack_from('<i', instances, e + 0x10) + struct.unpack_from('<i', instances, e + 0x1c) + struct.unpack_from('<ii', instances, e + 0x24)
        vy = s32(vy - trunc(s32(drag * vy), 1024) - gravity)
        y1 = s32(y0 + vy)
        set_return(log, 0x3e, (s32(y1 - r.choice([0, 0, vy, vy, 1, -1, vy + 1, vy - 1])) << 4) + r.randrange(16))
    elif boundary < 0.55:  # the first effector still, unjittered, in mode 2, its radius about the distance of a particle it pulls
        e = 0x68 * active[0]
        instances[e + 0xa] = 1
        put(instances, e + 0x4c, 'i', 100)
        put(instances, e + 4, 'i', -1)
        put(instances, e + 0x30, 'i', 2)
        put(instances, e + 0x34, 'iii', 0, 0, 0)
        found, a = [], struct.unpack_from('<h', heads, 0)[0]
        while a >= 0:  # (the first record of the first live system with the word +2 clear: pulled once its owner's +0x9c is 0; its age not 0)
            if struct.unpack_from('<Hh', systems, 0x140 * a + 2)[0] == 0 and struct.unpack_from('<h', systems, 0x140 * a + 6)[0] >= 0:
                found.append(struct.unpack_from('<h', systems, 0x140 * a + 6)[0])
            a = struct.unpack_from('<h', systems, 0x140 * a + 0xd0)[0]
        if found:
            p = 0x34 * found[0]
            put(systems, 0x140 * struct.unpack_from('<h', nodes, p + 0x26)[0] + 0x9c, 'I', 0)
            put(nodes, p + 0x20, 'i', r.choice([1, 5, 100]))
            d = sum(trunc(c - q, 4) ** 2 for c, q in zip(struct.unpack_from('<iii', instances, e + 0xc), struct.unpack_from('<iii', nodes, p))) // 256 + 10
            target = d + r.choice([0, 0, 1, -1])
            radius = next((v for v in range(max(0, int((256 * d) ** 0.5) - 2), int((256 * d) ** 0.5) + 40) if v * v // 256 == target), 0)
            put(instances, e + 0x2c, 'i', radius)
    put(state, 0x24, 'I', CDECL_LOG2)
    regions.update({0x8164fc: instances, LOG: log})
    return Call(0x520f10, [], regions, ret='v')


def tick(r):
    """0x520130: the effectors' cases (so Update's and the effectors' set-up), the frame counter about 1, each live system's fields about the
    branches: ages about the end, links to live or stale systems and effectors of the regions, emitters with small spreads, emit counts small."""
    regions = pull(r).regions
    systems, heads, state, instances = regions[0x80cefc], regions[0x816d1c], regions[0x80ced0], regions[0x8164fc]
    put(heads, 0x24, 'I', 0 if r.random() < 0.05 else 1)
    put(state, 4, 'i', r.choice([0, 1, 1, 2, 5, -1, r.randrange(-0x100, 0x100)]))
    for a in range(6):
        e = 0x140 * a
        put(systems, e + 0x26, 'H', r.choice([0, 0, 0, 1]))
        put(systems, e + 0x20, 'i', r.choice([0, 1, 2, -1, -1, -2, 5, 100]))
        put(systems, e + 0xd4, 'i', r.choice([0, 1, 4, 8, 100, -1]))
        put(systems, e + 0x10, 'h', r.choice([-1, -1, 0, r.randrange(20), r.randrange(0x69), 0x69]))
        for off in (0xbb, 0xba, 0xab):
            systems[e + off] = r.choice([0, 0, 1])
        put(systems, e + 0xbc, 'H', r.choice([0, 0, 1]))
        put(systems, e + 0xb8, 'H', r.choice([0, 0, 1]))
        if systems[e + 0xba]:
            k = r.randrange(12)
            serial = struct.unpack_from('<H', instances, 0x68 * k + 8)[0]
        else:
            k = r.randrange(6)
            serial = struct.unpack_from('<H', systems, 0x140 * k + 0xa)[0]
        put(systems, e + 0xb4, 'I', r.choice([-1, -1, serial << 16 | k, serial << 16 | k, (serial ^ 1) << 16 | k, 0]) & 0xffffffff)
        if systems[e + 0xba] and struct.unpack_from('<I', systems, e + 0xb4)[0] == 0 and r.random() < 0.7:
            put(instances, 8, 'H', 0)  # (handle 0 names effector 0 when its serial is 0)
        put(systems, e + 0xc8, 'i', r.choice([0, 0, 1, 0x800, -1, r.randrange(-0x2000, 0x2000)]))
        put(systems, e + 0x54, 'I', r.getrandbits(32))
        put(systems, e + 0x2c, 'iii', *[r.choice([0, 0, 0, 1, -1, r.randrange(-0x100, 0x100)]) for _ in range(3)])
        put(systems, e + 0x28, 'H', r.choice([0, 0, 1]))
        put(systems, e + 0xe, 'H', r.choice([0, 0, 0, 1, 2]))
        put(systems, e, 'h', r.choice([0, 1, 2, -1]))
        put(systems, e + 0xcc, 'i', r.choice([0, 1, -1, 0x80, r.randrange(-0x400, 0x400)]))
        put(systems, e + 0x50, 'h', r.choice([0, 1, -1, 0x80, r.randrange(-0x400, 0x400)]))
        for off in range(0x68, 0x6c):
            systems[e + off] = r.choice([0, 0, 1, 2, 3, 0xff, 0xfd, 0xf8])
        put(systems, e + 8, 'H', r.choice([0, 0, 1, 2, 5, 0x100]))
        put(systems, e + 0x64, 'h', r.choice([0, 1, 3, 10, 0x100, -1]))
        put(systems, e + 0x66, 'h', r.choice([0, 1, 2, -1, 5]))
    regions[0x80436c] = r.bytes(4)
    return Call(0x520130, [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
