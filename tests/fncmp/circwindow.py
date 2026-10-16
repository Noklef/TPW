# The circular buffer window scans (src/game/core/circbuf: 0x4801d0, 0x480380) against the originals:
# buffers of 144 with the cursor and entry count anywhere (the cursor below the count or not, wrapped or
# not), samples of both signs, small and large, and the window kinds 0, 1, 2 and others; the output
# starting below, equal to or above the result.
# usage: circwindow.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Max,Min)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4801d0, 0x4802a0), (0x480380, 0x480456)])
CALLS = {'Max': 0x4801d0, 'Min': 0x480380}
OUT = RAM + 0x400


def case(r):
    op = r.choice(OPS)
    buf = bytearray(r.bytes(0x250))
    sample = lambda: r.choice([0, 0, 1, -1, 5, -5, 100, -100, 0x7fffffff, -0x80000000, r.randrange(-50, 50)])  # noqa: E731
    for i in range(0x90):
        put(buf, 4 + 4 * i, 'i', sample())
    count = r.choice([0, 1, 2, 5, 12, 13, 36, 37, 144, r.randrange(0, 145)])
    head = r.choice([0, 1, count - 1, count, r.randrange(0, 144), 143]) if count else r.choice([0, 1, 5])
    put(buf, 0x244, 'i', max(0, min(143, head)))
    put(buf, 0x248, 'i', count)
    buf[0x24c] = r.choice([0, 1, 1, 0xff])
    out = r.choice([0, 1, -1, 5, -5, 100, -100, 0x7fffffff, -0x80000000, sample()])
    return Call(CALLS[op], [RAM, OUT, r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])],
                {RAM: buf, OUT: put(bytearray(8), 0, 'i', out)})


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
