# The game system module's save and load (src/game/world/gamesystem: 0x550520, 0x5506e0) against the originals, through a record stream with room for
# the bytes or not: any values in the globals.
# usage: gamesystem.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Save,Load)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

CALLS = {'Save': 0x550520, 'Load': 0x5506e0}
B = Bench([(0x550520, 0x5506e0), (0x5506e0, 0x5508b0), (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80)])
STREAM, SINK = RAM, RAM + 0x1000
GLOBALS = [0x878c74, 0x87879c, 0x878a1c, 0x877d34, 0x8786c0, 0x8786bc]


def case(r):
    name = r.choice(OPS)
    # A load from a stream that runs short leaves the original's uninitialised locals in the globals: only the save is given room that is too small.
    left = r.choice([0, 4, 0x1c, 0x20, 0x28, 0x100, r.randrange(0x30)]) if name == 'Save' else r.choice([0x28, 0x28, 0x100])
    regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100)}
    for g in GLOBALS:
        regions[g] = r.bytes(4)
    return Call(CALLS[name], [STREAM], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
