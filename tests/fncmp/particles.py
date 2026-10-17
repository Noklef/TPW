# The particle systems' handle accessors (src/game/render/particles: 0x5222e0, 0x522320,
# 0x51fd80, 0x51fa80, 0x51feb0, 0x51ff70, 0x51ff10, 0x51ffd0, 0x520470, 0x51fe30, 0x520030, 0x51fd90, 0x5200b0, 0x5204c0, 0x521810, 0x521850, 0x520e00, 0x520560) against the originals:
# handles live, stale (serial off), past the slots or 0, before and after set-up; effect numbers
# about 0x69; ages and lifetimes about the quarters, negative and zero.
# usage: particles.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alive,Effector,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG2, LOG, RAM, Bench, Call, log_region, put, set_return  # noqa: E402

B = Bench([(0x5222e0, 0x52231b), (0x522320, 0x522356), (0x51fd80, 0x51fd8a), (0x51fa80, 0x51faa0),
           (0x51feb0, 0x51ff06), (0x51ff10, 0x51ff68), (0x51ff70, 0x51ffc6), (0x51ffd0, 0x520028),
           (0x520470, 0x5204ba), (0x51fe30, 0x51fea4), (0x520030, 0x5200a4), (0x51fd90, 0x51fe2d), (0x5200b0, 0x520123), (0x5204c0, 0x52055f), (0x521810, 0x521845), (0x521850, 0x52188a), (0x520e00, 0x520f0c),
           (0x520560, 0x520d60), (0x520d60, 0x520e00)])
SYSTEMS, EFFECTS, EFFECTORS, SETUP = 0x80cefc, 0x804370, 0x816504, 0x816d40
CALLS = {'Alive': 0x5222e0, 'Effector': 0x522320, 'Reduced': 0x51fd80, 'Clear': 0x51fa80, 'End': 0x51feb0,
         'Age': 0x51ff70, 'Set2a': 0x51ff10, 'SetA2': 0x51ffd0, 'Quarter': 0x520470, 'VecA': 0x51fe30, 'VecB': 0x520030, 'Scaled': 0x51fd90, 'EndAll': 0x5200b0, 'Release': 0x5204c0, 'RandS': 0x521810, 'Rand': 0x521850, 'Move': 0x520e00, 'Emit': 0x520560}
NODES, SINES = RAM + 0x1000, RAM + 0x2000


def case(r):
    op = r.choice(OPS)
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


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
