# The mesh queues (src/game/render/meshqueue: 0x579c70, 0x57c260, 0x57c270, 0x57d1d0) against the originals:
# queues from empty to full (and beyond), any entries.
# usage: meshqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Init2,Add2,Init3d)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x579c70, 0x579c7b), (0x57c260, 0x57c293), (0x57d1d0, 0x57d2f7)])
CALLS = ['Clear', 'Init2', 'Add2', 'Init3d']


def case(r):
    op = r.choice(OPS)
    if op == 'Clear':
        return Call(0x579c70, [RAM], {RAM: r.bytes(0x408)}, conv='t')
    if op == 'Init2':
        return Call(0x57c260, [], {0x8bb630: r.bytes(0x10)})
    if op == 'Init3d':
        return Call(0x57d1d0, [RAM], {RAM: r.bytes(0xa1c0)}, conv='t', ret='a')
    queue = bytearray(r.bytes(0x408))
    put(queue, 0, 'I', r.choice([0, 1, 7, 0xff, 0x100, 0x101, r.randrange(0x100)]))
    return Call(0x57c270, [RAM, r.getrandbits(32)], {RAM: queue}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
