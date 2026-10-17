# The record serializers of things (src/game/world/things: 0x50b090), the object controls (world/objectdefs: 0x4d3aa0), a sprite script reference
# (sprite/scriptref: 0x4d3ee0), staff training (world/staff: 0x50c640) and the forecast (world/forecast: 0x5132a0) against the originals: saves and
# loads through a record stream with room for all the fields, none, or up to and about each field's end; versions in and out of 0..1; the object
# and the sink compared after the call.
# usage: serializers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

B = Bench([(0x50b090, 0x50b34c), (0x4d3aa0, 0x4d3d09), (0x4d3ee0, 0x4d4100), (0x50c640, 0x50c7f5), (0x5132a0, 0x5134c4), (0x416e10, 0x416e15),
           (0x5f5e20, 0x5f5e80), (0x5da3c0, 0x5da3c1)])
OBJ, STREAM, SINK = RAM, RAM + 0x2000, RAM + 0x3000
FIELDS = {0x50b090: [2, 2, 2, 2], 0x4d3aa0: [0x20] * 0x96 + [4, 2], 0x4d3ee0: [4, 4, 4], 0x50c640: [4] * 5 + [1], 0x5132a0: [8, 4, 1]}


def case(r):
    at = r.deal(list(FIELDS))
    ends = [sum(FIELDS[at][:i + 1]) for i in range(len(FIELDS[at]))]
    total = ends[-1]
    end = r.choice(ends[:3] + ends[-3:])
    left = r.choice([total, total, total, 0x1400, 0, end, end - 1, end + 1, r.randrange(total)])
    regions = {OBJ: r.bytes(0x1300), STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x1400)}
    save = r.choice([0, 1, 1, r.getrandbits(32)])
    version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
    return Call(at, [OBJ, STREAM, save, version & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
