# The record serializers of things (src/game/world/things: 0x50b090, and 0x509bc0 on it), a person's thoughts (world/person: 0x50bad0), the
# object controls (world/objectdefs: 0x4d3aa0), a sprite script reference (sprite/scriptref: 0x4d3ee0), staff training (world/staff: 0x50c640),
# the forecast (world/forecast: 0x5132a0), an RE cell (world/mapcell: 0x502490), the mechanic and staff HQs (world/headquarters: 0x4da960,
# 0x508bb0), the calendar clock (core/funnytime: 0x4f7f30), the map / track cells (world/mapcell: 0x4d0b30, 0x4d8dd0, 0x50c200), the
# challenges (world/challenges: 0x4d0f90, 0x4d2320), the nav (world/nav: 0x50d150), the weather (world/weather: 0x512010), the research
# lab, its projects and groups (world/research: 0x502dc0, 0x504210, 0x504760), the staff pool and its entries (world/staff: 0x507850,
# 0x5070a0), a circular buffer of 30 u32 (core/circbuf: 0x4e2c10, the count kept small) and the sprite's nav call (world/nav: 0x4f9440)
# against the originals: saves and loads through a record stream with room for all the fields, none, or up to and about each field's end;
# versions in and out of 0..1; the object and the sink compared after.
# usage: serializers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

B = Bench([(0x50b090, 0x50b34c), (0x4d3aa0, 0x4d3d09), (0x4d3ee0, 0x4d4100), (0x50c640, 0x50c7f5), (0x5132a0, 0x5134c4), (0x50bad0, 0x50be40),
           (0x509bc0, 0x509be2), (0x4fae10, 0x4fae7e), (0x416e10, 0x416e15), (0x5f5e20, 0x5f5e80), (0x5da3c0, 0x5da3c1),
           (0x502490, 0x5025b3), (0x4da960, 0x4daa80), (0x508bb0, 0x508e63), (0x4f7f30, 0x4f8257),
           (0x424c20, 0x424ca0), (0x4d0b30, 0x4d0f80), (0x4d0f90, 0x4d1670), (0x4d2320, 0x4d27b0), (0x4d37e0, 0x4d3860), (0x4d8dd0, 0x4d9200),
           (0x50c200, 0x50c330), (0x5134d0, 0x513550), (0x510240, 0x5102c0), (0x50d150, 0x50dd90), (0x512010, 0x5127b0),
           (0x41a860, 0x41a970), (0x504210, 0x504590), (0x5070a0, 0x5074b0), (0x4e2c10, 0x4e2fb0), (0x4f9440, 0x4f9470), (0x502dc0, 0x5031b0),
           (0x504760, 0x504a30), (0x507850, 0x507bc0)])
OBJ, STREAM, SINK = RAM, RAM + 0x2000, RAM + 0x3000
CELL = [1, 2, 4, 1, 2, 2, 12, 4, 1]
CHALLENGE = [4, 4, 4, 2, 2, 4, 4, 4, 1, 4, 4, 4, 1, 1, 2]
FIELDS = {0x50b090: [2, 2, 2, 2], 0x4d3aa0: [0x20] * 0x96 + [4, 2], 0x4d3ee0: [4, 4, 4], 0x50c640: [4] * 5 + [1], 0x5132a0: [8, 4, 1],
          0x50bad0: [4] * 0x23, 0x509bc0: [2, 2, 2, 2], 0x502490: [2] * 5,
          0x4da960: [2] * 5, 0x508bb0: [2] * 4 + [4, 2] + [0xc] * 5 + [4] * 5 + [1], 0x4f7f30: [8, 8, 4, 4, 4],
          0x4d0b30: CELL, 0x4d8dd0: CELL + [4, 2, 4, 4, 2, 1, 4, 2], 0x50c200: CELL + [2], 0x4d0f90: CHALLENGE,
          0x4d2320: [2] * 4 + CHALLENGE * 0x14 + [4, 8, 1, 1, 1, 4, 2],
          0x50d150: [8] * 4 + [4] * 7 + [1] + [4] * 4 + [8, 4, 4, 4, 8, 4] + [8, 4] * 5 + [8],
          0x512010: [2] * 4 + [4, 8, 8] + [8, 4, 1] + [4, 4, 1, 4, 4, 4, 12, 4, 4, 4, 4, 1],
          0x504210: [2, 4, 1, 1, 4, 4], 0x5070a0: [4, 4, 1, 1, 1, 1, 4, 4], 0x504760: [4, 4, 4, 1],
          0x502dc0: [2] * 4 + [2, 4, 1, 1, 4, 4] * 0x100 + [4, 4, 4, 1] * 5 + [4, 4, 4, 1],
          0x507850: [4, 4, 1, 1, 1, 1, 4, 4] * 0x20 + [4, 1] * 5 + [4, 1], 0x4e2c10: [4, 4, 1, 4]}
FIELDS[0x4f9440] = FIELDS[0x50d150]
COUNTS = [0, 1, 2, 30, 31, 40, 0xffffffff, 0x80000000]  # the circular buffer's mNumEntries (+0x80 saved, the stream's second dword loaded)


def case(r):
    at = r.deal(list(FIELDS))
    sizes = FIELDS[at]
    if at == 0x4e2c10:
        count = r.choice(COUNTS + [r.randrange(41)])
        sizes = sizes + [4] * (count if count < 0x80000000 else 0)
    ends = [sum(sizes[:i + 1]) for i in range(len(sizes))]
    total = ends[-1]
    end = r.choice(ends[:3] + ends[-3:])
    left = r.choice([total, total, total, 0x1400, 0, end, end - 1, end + 1, r.randrange(total)])
    regions = {OBJ: bytearray(r.bytes(0x1300 if at != 0x502dc0 else 0x1480)), STREAM: record_stream(STREAM, SINK, left), SINK: bytearray(r.bytes(0x1400))}
    if at == 0x4e2c10:
        regions[OBJ][0x80:0x84] = regions[SINK][4:8] = count.to_bytes(4, 'little')
    save = r.choice([0, 1, 1, r.getrandbits(32)])
    version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
    return Call(at, [OBJ, STREAM, save, version & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
