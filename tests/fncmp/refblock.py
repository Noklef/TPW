# The video library's block copies (src/game/gfx/refblock: 0x676a70 into the 16x16 buffer, 0x676b50 into an 8x8 one) against the
# originals: a random source block, the pitch 5-0x40 dwords (the source sized for it), swapped any (mostly 0 / 1), the bias any byte (the
# stack slot's other bytes random), the out buffer random; Copy8's source split into base and offset at random.
# usage: refblock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Copy16,Copy8)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Copy16': 0x676a70, 'Copy8': 0x676b50}
B = Bench([(0x676a70, 0x676b47), (0x676b50, 0x676bbe)])
SOURCE, OUT = RAM, RAM + 0x8000


def case(r):
    op = r.deal(OPS)
    pitch = r.choice([5, 8, 16, 0x20, 0x40, r.randrange(5, 0x41)])
    bias = r.getrandbits(32)
    regions = {SOURCE: r.bytes(pitch * 8 * 8 + 0x20), OUT: r.bytes(0x200)}
    if op == 'Copy16':
        swapped = r.choice([0, 1, 1, r.getrandbits(32)])
        return Call(CALLS[op], [SOURCE, swapped, pitch, OUT, bias], regions, conv='f', ret='v')
    offset = r.randrange(0x100)
    return Call(CALLS[op], [SOURCE - offset, offset, pitch, OUT, bias], regions, conv='f', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
