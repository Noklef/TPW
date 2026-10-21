# The movie player's second DCT video (src/game/gfx/tqivideo: 0x6710c0 the dequantiser) against the original (the emulator running its
# x87 and __ftol). Quality: 100 in a sixth of cases, else 0-110 mostly, now and then any (the factors' low dwords); out random before.
# usage: tqivideo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Quality)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Quality': 0x6710c0}
B = Bench([(0x6710c0, 0x671192), (0x67a830, 0x67a860)])
OUT = RAM


def case(r):
    op = r.deal(OPS)
    quality = 100 if r.random() < 1 / 6 else r.randrange(111) if r.random() < 0.85 else r.choice([-1, 107, 108, r.getrandbits(32)])
    return Call(CALLS[op], [OUT, quality], {OUT: r.bytes(0x100)}, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
