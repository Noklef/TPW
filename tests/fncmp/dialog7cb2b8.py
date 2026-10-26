# The dialog [0x7cb2b8]'s code (src/game/ui/dialog7cb2b8: 0x4a1440) against the originals. ShowRightOf: windowtree.py's rect_env() tree
# (1-7 windows with GUI rectangles, margins, parents, objects and handlers; shown or hidden), the window its root mostly, one of the others or
# now and then 0 (a fault in both: its children are read all the same); the limit about the windows' right edges (their inner ones) or any,
# the upper half random.
# usage: dialog7cb2b8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowRightOf)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, log_region  # noqa: E402
from window import RANGES  # noqa: E402
from windowtree import rect_env  # noqa: E402

CALLS = {'ShowRightOf': 0x4a1440}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(sorted(set(RANGES + [
    (0x4a1440, 0x4a14e1), (0x6589f9, 0x658abb), (0x658ccc, 0x658daf), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
    (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65d42e, 0x65d4b0), (0x65d547, 0x65d56a), (0x65d56b, 0x65d58c), (0x65d773, 0x65d7cc),
    (0x65d90b, 0x65da9d), (0x65db25, 0x65dc00), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b), (0x661700, 0x661800)])))


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = rect_env(r, regions, log)
    window = r.choice(wins[:1] * 4 + wins + [0] if r.random() < 0.03 else wins[:1] * 4 + wins)
    rights = []
    for w in wins:
        right, = struct.unpack_from('<h', regions[w], 0x10)
        margin, = struct.unpack_from('<h', regions[w], 0xfc)
        rights.append(right - margin)
    limit = r.choice([r.choice(rights) + r.choice([-1, 0, 0, 1]), r.randrange(-40, 80), r.randrange(-40, 80), 0x7fff, 0x8000, r.getrandbits(16)])
    return Call(CALLS['ShowRightOf'], [window, (limit & 0xffff) | r.choice([0, 0, r.getrandbits(16)]) << 16], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
