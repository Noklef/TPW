# The matrix library 0x46f150-0x470990 and 0x470d10 (src/game/math/matrix) against the originals.
# Known: atan2 and fsin/fcos may differ in the last bit of the double (the 32-bit CRT and Python both
# stand in for the x87's extended fpatan/fsin), and fsin/fcos past ~1e6 (the x87 reduces by a 66-bit pi).
# usage: matrix.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x46f150, 0x470990), (0x470d10, 0x470e90), (0x67a400, 0x690000)], sines=256)
M, V, OUT = RAM, RAM + 0x1000, RAM + 0x2000  # matrices, vectors, outputs


def matrix(r, buf, off, n=16):
    """n floats at off: mostly one scale, sometimes NaNs differing only in sign, or a rotation-ish
    matrix near the unit scale (the usual input)."""
    scale = r.choice([1, 1, 10, None, 1e38, 1e-19, 1e-38, 1e-42])  # extremes: squares and products leave float range
    for k in range(n):
        if r.random() < 0.04:
            put(buf, off + 4 * k, 'I', r.choice([0x7fc00000, 0xffc00000]))
        else:
            put(buf, off + 4 * k, 'I', r.f(scale))
    if r.random() < 0.1:  # a column or row of zeros: singular, degenerate axes
        for k in r.sample(range(n), r.randrange(1, 5)):
            put(buf, off + 4 * k, 'I', 0)
    return buf


def case(r):
    op = r.choice(OPS)
    m = matrix(r, bytearray(r.bytes(0x1000)), 0)
    matrix(r, m, 0x40)
    v = matrix(r, bytearray(r.bytes(0x100)), 0, 12)
    regs = {M: m, V: v, OUT: bytearray(r.bytes(0x100))}
    if op in ('Identity', 'ResetRotation'):
        return Call({'Identity': 0x46f150, 'ResetRotation': 0x46f180}[op], [M], regs)
    if op in ('Heading', 'Roll'):
        return Call({'Heading': 0x46f1c0, 'Roll': 0x46f200}[op], [M], regs, ret='d')
    if op in ('AngleX', 'AngleY', 'AngleZ'):
        return Call({'AngleX': 0x46f210, 'AngleY': 0x46f220, 'AngleZ': 0x46f230}[op], [V], regs, ret='d')
    if op.startswith('Rot'):
        address = {'RotX4': 0x46f240, 'RotX': 0x46f350, 'RotY4': 0x46f420, 'RotZ4': 0x46f4f0, 'RotZ': 0x46f5c0,
                   'RotY': 0x46f650}[op]
        return Call(address, [M, r.f(1), r.f(1)], regs)
    if op in ('EulerFast', 'Euler'):
        return Call({'EulerFast': 0x46f6f0, 'Euler': 0x46f870}[op], [M, r.randrange(16), V], regs)
    if op == 'SetAxisLengths':
        return Call(0x46f910, [M, V, V + 4, V + 8], regs)
    if op == 'ScaleAxesV':
        return Call(0x46fa70, [M, V], regs)
    if op == 'ScaleAxes':
        return Call(0x46fad0, [M, r.f(), r.f(), r.f()], regs)
    if op == 'FromPitchYaw':
        return Call(0x46fb50, [M, r.f(7), r.f(7)], regs)
    if op == 'SetAxes':
        return Call(0x46fbb0, [M] + [r.choice([V, V + 12, V + 24, 0]) for _ in range(3)], regs)
    if op in ('TransformPoint', 'TransformVector'):
        return Call({'TransformPoint': 0x46fc00, 'TransformVector': 0x46fca0}[op], [M, V], regs)
    if op in ('Mul', 'PreMul', 'PreMul3', 'PreMul33'):
        b = r.choice([M + 0x40, M + 0x40, M])
        a, b = (M, b) if r.random() < 0.5 else (b, M)
        return Call({'Mul': 0x4700f0, 'PreMul': 0x4702d0, 'PreMul3': 0x4704c0, 'PreMul33': 0x470d10}[op], [a, b], regs)
    if op == 'LUDecompose':
        return Call(0x46fe20, [M, OUT, OUT + 0x10], regs)
    if op == 'LUBack':
        put(regs[OUT], 0, '4i', *[r.randrange(4) for _ in range(4)])
        return Call(0x46fd40, [M, OUT, V], regs)
    if op == 'Invert':
        return Call(0x470040, [r.choice([M + 0x40, M]), M], regs, ret='a')
    if op == 'LookAlong':
        if r.random() < 0.3:  # mixed scales: an axis component near the denormals, as are the products it meets
            for buf, off in [(m, 0x10), (m, 0x14), (m, 0x18), (v, 0), (v, 4), (v, 8)]:
                put(buf, off, 'I', r.f(r.choice([1, 1e-38])))
        return Call(0x470780, [M, V], regs)
    if op == 'Distance':
        return Call(0x470940, [V, V + 12], regs, ret='d')
    n = r.randrange(1, 17)
    keys = regs[V]
    for k in range(n):
        put(regs[OUT], 4 * k, 'i', 4 * r.randrange(12))
    return Call(0x470630, [n, OUT, OUT + 0x80, 0, V], {**regs, V: keys})


OPS_ALL = ['Identity', 'ResetRotation', 'Heading', 'Roll', 'AngleX', 'AngleY', 'AngleZ', 'RotX4', 'RotX', 'RotY4',
           'RotZ4', 'RotZ', 'RotY', 'EulerFast', 'Euler', 'SetAxisLengths', 'ScaleAxesV', 'ScaleAxes',
           'FromPitchYaw', 'SetAxes', 'TransformPoint', 'TransformVector', 'Mul', 'PreMul', 'PreMul3', 'PreMul33',
           'LUDecompose', 'LUBack', 'Invert', 'LookAlong', 'Distance', 'Sort']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
