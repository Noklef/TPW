# The lobby models' methods (src/game/frontend/lobbymodel: 0x5d8190, 0x5d81c0, 0x5d8380-0x5d8440) against
# the originals, on the animator bench's world (tests/fncmp/animator.py) with the model's matrix
# hung off it: positions from anywhere (the matrix itself too, NaNs), turns, scales, plays and
# time left, with and without a model; the alignment to v also with x and z 0 (a zero length), huge or tiny ones.
# Fly (0x5d9b50): a lobby flyer's step, see fly().
# usage: lobbymodel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Position,TurnY,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
import animator  # noqa: E402

B = Bench([(0x5d8190, 0x5d81be), (0x5d81c0, 0x5d8380), (0x5d8380, 0x5d8463), (0x45bf20, 0x45bf46), (0x46f150, 0x475010), (0x5d9b50, 0x5d9da1),
           (0x67a400, 0x690000)], sines=256)
RIDES = 0x7a4610
OBJ, MATRIX, VEC, FLYER, SCENE = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200, RAM + 0xa300, RAM + 0xa400
CALLS = ['Position', 'TurnY', 'Scale', 'Play', 'TimeLeft', 'Align', 'Fly']


def fly(r, regs):
    """0x5d9b50 (thiscall): a flyer (its lobby model +8 the bench's), its box (centre +0x10, extent +0x1c), speed (+0x28), reach (+0x2c:
    the squared distance; now and then past the target's), position (+0x30), target (+0x3c: now and then the position itself, a zero
    length), heading (+0x48) unit-ish or any; the scene ([0xf82884]) its frame time (+8) small or any, its generator (+0x34) any."""
    flyer = bytearray(r.bytes(0x60))
    put(flyer, 8, 'I', OBJ)
    pos = [r.uniform(-50, 50) for _ in range(3)]
    target = [p + r.choice([r.uniform(-20, 20), r.uniform(-1, 1), 0.0]) for p in pos]
    put(flyer, 0x10, '6f', *(r.uniform(-50, 50) for _ in range(3)), *(r.uniform(0, 60) for _ in range(3)))
    put(flyer, 0x28, '2f', r.uniform(0, 20), r.choice([r.uniform(0, 4), r.uniform(0, 400), 0.0]))
    put(flyer, 0x30, '6f', *pos, *target)
    h = [r.uniform(-1, 1) for _ in range(3)]
    put(flyer, 0x48, '3f', *h)
    for off in range(0x10, 0x54, 4):
        if r.random() < 0.03:
            put(flyer, off, 'I', r.choice([animator.fl(r), animator.snan(r), animator.nan(r), 0]))
    scene = bytearray(r.bytes(0x40))
    put(scene, 8, 'I', r.choice([fb(r.uniform(0, 0.1)), fb(r.uniform(0, 2)), animator.fl(r)]))
    regs.update({FLYER: flyer, SCENE: scene, 0xf82884: struct.pack('<I', SCENE)})
    return Call(0x5d9b50, [FLYER], regs, conv='t')


def case(r):
    op = r.deal(OPS)
    regs = animator.case(r).regions
    put(regs[animator.MODEL], 0x78, 'I', MATRIX)
    rides = bytearray(r.bytes(0x10))
    put(rides, 4, 'I', animator.ANIM)
    obj = bytearray(r.bytes(0x10))
    put(obj, 4, 'I', 1 if op in ('Position', 'TurnY', 'Scale', 'Align') or r.random() < 0.8 else 0)
    matrix = bytearray(r.bytes(0x60))
    for k in range(16):
        put(matrix, 0x10 + 4 * k, 'I', animator.fl(r) if r.random() < 0.9 else animator.snan(r))
    vec = bytearray(r.bytes(0x10))
    for k in range(3):
        put(vec, 4 * k, 'I', animator.fl(r) if r.random() < 0.85 else r.choice([animator.snan(r), animator.nan(r)]))
    if op == 'Align' and r.random() < 0.3:
        k = r.random()
        if k < 0.4:
            put(vec, 0, 'I', r.choice([0, 0x80000000]))
            put(vec, 8, 'I', r.choice([0, 0x80000000]))
        else:
            mag = 3.4e38 if k < 0.6 else 2e-38 if k < 0.8 else 2e-40   # (lengths and crosses out of the float range, or denormal)
            put(vec, 0, '3f', *(r.uniform(-mag, mag) for _ in range(3)))
    regs.update({RIDES: rides, OBJ: obj, MATRIX: matrix, VEC: vec})
    if op == 'Fly':
        put(obj, 4, 'I', 1)
        return fly(r, regs)
    if op == 'Position':
        return Call(0x5d8190, [OBJ, r.choice([VEC, VEC, MATRIX + 0x10 + 0x2c, MATRIX + 0x10 + 0x34])], regs, conv='t')
    if op == 'Align':
        return Call(0x5d81c0, [OBJ, r.choice([VEC, VEC, VEC, MATRIX + 0x10 + 0x20, MATRIX + 0x10 + 0x8])], regs, conv='t')
    if op == 'TurnY':
        return Call(0x5d8380, [OBJ, animator.fl(r)], regs, conv='t')
    if op == 'Scale':
        return Call(0x5d83b0, [OBJ, animator.fl(r), animator.fl(r), animator.fl(r)], regs, conv='t')
    if op == 'Play':
        return Call(0x5d83f0, [OBJ, r.randrange(4), r.choice([0, 1, 2, 0x100])], regs, conv='t', ret='a')
    return Call(0x5d8440, [OBJ], regs, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
