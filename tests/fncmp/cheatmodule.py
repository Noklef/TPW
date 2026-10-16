# The cheat system's module read and write (src/game/core/cheatmodule: 0x405570, 0x4055d0) against
# the originals: streams with plenty of room, one byte left, or none.
# usage: cheatmodule.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put, record_stream  # noqa: E402

B = Bench([(0x405570, 0x4055d0), (0x4055d0, 0x405630), (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80)])
CHUNK, STREAM, SINK = RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    write = r.random() < 0.5
    left = r.choice([0, 1, 2, 2, 5, 0x100])
    cheats = bytearray(r.bytes(0x20))
    regions = {RAM: cheats, CHUNK: put(bytearray(r.bytes(0x10)), 0, 'I', STREAM), STREAM: record_stream(STREAM, SINK, left),
               SINK: r.bytes(0x20)}
    return Call(0x4055d0 if write else 0x405570, [RAM, CHUNK], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
