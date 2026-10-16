# The renderer's precision switches (src/game/core/fpu: 0x591220-0x591250) against the originals:
# the control word they return and the one they leave. The words loaded keep the exceptions
# masked (the game's never unmask) and the reserved bits as the hardware reads them back.
# usage: fpucw.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Single,Load,LoadFull)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x591220, 0x591261)])
CALLS = {'Single': 0x591220, 'Load': 0x591240, 'LoadFull': 0x591250}


def case(r):
    op = r.choice(OPS)
    def word():
        return (r.getrandbits(32) & 0xffff1f00) | 0x7f
    args = [] if op == 'Single' else [word()]
    return Call(CALLS[op], args, {RAM: bytes(4)}, ret='wc' if op == 'Single' else 'c', cw=word() & 0xffff)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
