# The cheat system's module read and write (src/game/core/cheatmodule: 0x405570, 0x4055d0) against
# the originals: streams with plenty of room, one byte left, or none.
# usage: cheatmodule.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put, record_stream  # noqa: E402

B = Bench([(0x599c30, 0x599cf1), (0x59c860, 0x59c890), (0x59c890, 0x59c8b6), (0x405570, 0x4055d0), (0x4055d0, 0x405630), (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80)])
CHUNK, STREAM, SINK = RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    if r.random() < 0.2:  # the advisor module's write: a 0x168 block and a counted buffer, with room or not
        left = r.choice([0, 4, 8, 0x100, 0x170, 0x17c, 0x400, 0x1000])
        count = r.choice([0, 4, 0x40, 0x100])
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x800), 0xf79660: r.bytes(0x168),
                   0xf79790: put(put(bytearray(r.bytes(0x20)), 4, 'I', count), 0x10, 'I', r.choice([0, RAM + 0x1000, RAM + 0x1000])),
                   RAM + 0x1000: r.bytes(0x100)}
        return Call(0x599c30, [STREAM], regions)
    write = r.random() < 0.5
    advisor = r.random() < 0.4
    left = r.choice([0, 1, 2, 2, 5, 0x100])
    cheats = bytearray(r.bytes(0x20))
    regions = {RAM: cheats, CHUNK: put(bytearray(r.bytes(0x10)), 0, 'I', STREAM), STREAM: record_stream(STREAM, SINK, left),
               SINK: r.bytes(0x20)}
    if advisor:
        return Call(0x59c890 if write else 0x59c860, [RAM, CHUNK], regions, conv='t', ret='a')
    return Call(0x4055d0 if write else 0x405570, [RAM, CHUNK], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
