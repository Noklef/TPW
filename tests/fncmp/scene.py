# 0x448000-0x470000 (render mode, model part tables) against the originals.
# usage: scene.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x44a590, 0x44a640), (0x44b140, 0x44b2e0), (0x44b3c0, 0x44b410)])
HOLDER, MODEL, PARTS, FACES, FACES2 = (RAM + k * 0x1000 for k in range(5))


def mode(r, op):
    bits = r.choice([r.getrandbits(32), 1 << r.randrange(3, 15), 0, r.getrandbits(32) & 0x7838])
    if op in ('Size', 'Depth'):
        return Call({'Size': 0x44a590, 'Depth': 0x44a5d0}[op], [], {0x79d86c: put(bytearray(4), 0, 'I', bits)}, ret='a')
    return Call({'ResetScale': 0x44a600, 'Epsilon': 0x44a620}[op], [], {0x79d9e4: r.bytes(4), 0x79d9e8: r.bytes(4)})


def parts(r, op):
    """A holder (HOLDER: +0 -> HOLDER + 0x20, +8 -> HOLDER + 0x20, whose +4 is MODEL) and a model with
    +0x48 parts at +0x7c (PARTS) and faces at +0x70 (FACES, 0xa0 each) and +0x74 (FACES2, 0x58 each)."""
    n = r.choice([0, 1, r.randrange(1, 12)])
    holder, model, ps = r.bytes(0x40), r.bytes(0x80), r.bytes(12 * 0x14)
    put(holder, 0, 'I', HOLDER + 0x20)
    put(holder, 8, 'I', HOLDER + 0x20)
    put(holder, 0x24, 'I', MODEL)
    first, base = r.randrange(0, 13), r.randrange(0, 4)
    put(model, 0x44, '3H', first, base, n)
    put(model, 0x70, 'I', FACES)
    put(model, 0x74, 'I', FACES2)
    put(model, 0x7c, 'I', PARTS)
    keys = [r.getrandbits(32) for _ in range(3)]
    for i in range(12):
        put(ps, i * 0x14, 'I', r.choice([r.getrandbits(32), 1 << r.randrange(32), 0, r.getrandbits(32) & ~0x3da1f83]))
        put(ps, i * 0x14 + 4, 'I', r.choice(keys + [r.getrandbits(32)]))
        put(ps, i * 0x14 + 8, 'I', r.choice(keys + [r.getrandbits(32)]))
    mask = r.choice([r.getrandbits(32), 1 << r.randrange(32), 0, r.getrandbits(32) & ~0x3da1f83, 0x3da1f83 & r.getrandbits(32),
                     r.getrandbits(32) & ~0x3da1f82 | 1])
    key = r.choice(keys + [r.getrandbits(32)])
    regs = {HOLDER: holder, MODEL: model, PARTS: ps, FACES: r.bytes(16 * 0xa0), FACES2: r.bytes(16 * 0x58)}
    if op in ('MarkMask', 'MarkValue'):
        return Call({'MarkMask': 0x44b140, 'MarkValue': 0x44b1b0}[op], [HOLDER, mask if op == 'MarkMask' else key], regs)
    if op == 'Count':
        return Call(0x44b2b0, [HOLDER, mask], regs, ret='a')
    if op == 'Nth':
        return Call(0x44b270, [HOLDER, mask, r.choice([0, 1, 2, r.randrange(n + 2), r.getrandbits(32)])], regs, ret='a')
    return Call({'Key4': 0x44b220, 'Key8': 0x44b3c0}[op], [HOLDER, mask, key], regs, ret='a')


KINDS = {'Size': mode, 'Depth': mode, 'ResetScale': mode, 'Epsilon': mode, 'MarkMask': parts, 'MarkValue': parts,
         'Count': parts, 'Nth': parts, 'Key4': parts, 'Key8': parts}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    return KINDS[op](r, op)


if __name__ == '__main__':
    B.main(case)
