# The lobby models' methods (src/game/frontend/lobbymodel: 0x5d8190, 0x5d8380-0x5d8440) against
# the originals, on the animator bench's world (tests/fncmp/animator.py) with the model's matrix
# hung off it: positions from anywhere (the matrix itself too, NaNs), turns, scales, plays and
# time left, with and without a model.
# usage: lobbymodel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Position,TurnY,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, put  # noqa: E402
import animator  # noqa: E402

B = Bench([(0x5d8190, 0x5d81be), (0x5d8380, 0x5d8463), (0x45bf20, 0x45bf46), (0x46f150, 0x475010),
           (0x67a400, 0x690000)], sines=256)
RIDES = 0x7a4610
OBJ, MATRIX, VEC = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200
CALLS = ['Position', 'TurnY', 'Scale', 'Play', 'TimeLeft']


def case(r):
    op = r.choice(OPS)
    regs = animator.case(r).regions
    put(regs[animator.MODEL], 0x78, 'I', MATRIX)
    rides = bytearray(r.bytes(0x10))
    put(rides, 4, 'I', animator.ANIM)
    obj = bytearray(r.bytes(0x10))
    put(obj, 4, 'I', 1 if op in ('Position', 'TurnY', 'Scale') or r.random() < 0.8 else 0)
    matrix = bytearray(r.bytes(0x60))
    for k in range(16):
        put(matrix, 0x10 + 4 * k, 'I', animator.fl(r) if r.random() < 0.9 else animator.snan(r))
    vec = bytearray(r.bytes(0x10))
    for k in range(3):
        put(vec, 4 * k, 'I', animator.fl(r) if r.random() < 0.85 else r.choice([animator.snan(r), animator.nan(r)]))
    regs.update({RIDES: rides, OBJ: obj, MATRIX: matrix, VEC: vec})
    if op == 'Position':
        return Call(0x5d8190, [OBJ, r.choice([VEC, VEC, MATRIX + 0x10 + 0x2c, MATRIX + 0x10 + 0x34])], regs, conv='t')
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
