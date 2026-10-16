# The sprite frames' and images' constructors (src/game/gfx/spriteframe: 0x588d70, 0x588320)
# against the originals: any bytes before.
# usage: spriteframe.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Frame,Image)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x588320, 0x588342), (0x588d70, 0x588d8f)])
CALLS = {'Frame': 0x588d70, 'Image': 0x588320}


def case(r):
    op = r.choice(OPS)
    return Call(CALLS[op], [RAM], {RAM: r.bytes(0x50)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
