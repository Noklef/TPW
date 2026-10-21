# The record serializers of things (src/game/world/things: 0x50b090, and 0x509bc0 on it), a person's thoughts (world/person: 0x50bad0), the
# object controls (world/objectdefs: 0x4d3aa0), a sprite script reference (sprite/scriptref: 0x4d3ee0), staff training (world/staff: 0x50c640),
# the forecast (world/forecast: 0x5132a0), an RE cell (world/mapcell: 0x502490), the mechanic and staff HQs (world/headquarters: 0x4da960,
# 0x508bb0), the calendar clock (core/funnytime: 0x4f7f30), the map / track cells (world/mapcell: 0x4d0b30, 0x4d8dd0, 0x50c200), the
# challenges (world/challenges: 0x4d0f90, 0x4d2320), the nav (world/nav: 0x50d150), the weather (world/weather: 0x512010), the research
# lab, its projects and groups (world/research: 0x502dc0, 0x504210, 0x504760), the staff pool and its entries (world/staff: 0x507850,
# 0x5070a0), circular buffers of 30, 144, 50 and 5 u32, of u16 and of 50 u8 (core/circbuf: 0x4e2c10, 0x4cdf00, 0x4ce2d0, 0x4ce660, 0x4cdb80,
# 0x4cea00; the counts small, about the size or outside it), the sprite's nav call (world/nav: 0x4f9440)
# a theme's save record (core/globalsave: 0x5b0d70), the bank account (world/bankaccount: 0x4cf920), the park's gates (world/parkgates:
# 0x4cf050), an analyser value (world/analyser: 0x4cd9c0), the save's park information (core/parkinfo: 0x418da0)
# an advisor message (world/advisorscore: 0x59ba30) and the advisor thing (world/advisor: 0x599f60; its object and sink larger, elsewhere)
# against the originals: saves and loads through a record stream with room for all the fields, none, or up to and about each field's end;
# versions in and out of 0..1; the object and the sink compared after.
# usage: serializers.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=5b0d70,... the functions' addresses)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

RANGES = [(0x50b090, 0x50b34c), (0x4d3aa0, 0x4d3d09), (0x4d3ee0, 0x4d4100), (0x50c640, 0x50c7f5), (0x5132a0, 0x5134c4), (0x50bad0, 0x50be40),
          (0x509bc0, 0x509be2), (0x4fae10, 0x4fae7e), (0x416e10, 0x416e15), (0x5f5e20, 0x5f5e80), (0x5da3c0, 0x5da3c1),
          (0x502490, 0x5025b3), (0x4da960, 0x4daa80), (0x508bb0, 0x508e63), (0x4f7f30, 0x4f8257),
          (0x424c20, 0x424ca0), (0x4d0b30, 0x4d0f80), (0x4d0f90, 0x4d1670), (0x4d2320, 0x4d27b0), (0x4d37e0, 0x4d3860), (0x4d8dd0, 0x4d9200),
          (0x50c200, 0x50c330), (0x5134d0, 0x513550), (0x510240, 0x5102c0), (0x50d150, 0x50dd90), (0x512010, 0x5127b0),
          (0x41a860, 0x41a970), (0x504210, 0x504590), (0x5070a0, 0x5074b0), (0x4e2c10, 0x4e2fb0), (0x4f9440, 0x4f9470), (0x502dc0, 0x5031b0),
          (0x504760, 0x504a30), (0x507850, 0x507bc0), (0x5b0d70, 0x5b118f), (0x4cf920, 0x4d0182), (0x4cdf00, 0x4ce28c),
          (0x4ce2d0, 0x4ce65a), (0x4ce660, 0x4ce9da), (0x4cdb80, 0x4cdefa), (0x4cea00, 0x4ced6f), (0x4cf050, 0x4cf3c8),
          (0x4cd9c0, 0x4cdb51), (0x418da0, 0x4193c3), (0x59ba30, 0x59be35),
          (0x599f60, 0x59a54d)]
B = Bench(RANGES)
OBJ, STREAM, SINK = RAM, RAM + 0x2000, RAM + 0x3000
BIG_STREAM, BIG_SINK = RAM + 0x5000, RAM + 0x6000
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
          0x507850: [4, 4, 1, 1, 1, 1, 4, 4] * 0x20 + [4, 1] * 5 + [4, 1], 0x4e2c10: [4, 4, 1, 4],
          0x5b0d70: [1] * 6 + [1, 4] * 4 + [2, 2] * 0x21 + [1, 1]}
FIELDS[0x4f9440] = FIELDS[0x50d150]
FIELDS[0x4cf920] = [2, 2, 2, 2] + [4] * 7 + [4] * 0x40
FIELDS[0x4cdf00] = FIELDS[0x4ce2d0] = FIELDS[0x4ce660] = FIELDS[0x4e2c10]
FIELDS[0x4cdb80] = [4, 4, 1, 2]
FIELDS[0x4cf050] = [4, 4, 4, 4, 1, 1]
FIELDS[0x4cea00] = [4, 4, 1, 1]
FIELDS[0x4cd9c0] = [4, 1]
FIELDS[0x59ba30] = [2, 4, 4, 4, 1, 4, 1, 1]
FIELDS[0x599f60] = [2] * 4 + [4, 4] + FIELDS[0x59ba30] * 8 + [4] * 3 + [0x10] * 0x15f + FIELDS[0x59ba30] + [1, 4]
FIELDS[0x418da0] = [2] * 0x10 + [2] * 0x80 + [4] * 3 + [2] * 0x10 + [2] * 0x10 + [2] * 0x32 + [2] * 0x32 + [2] * 3
COUNTS = [0, 1, 2, 30, 31, 40, 0xffffffff, 0x80000000]  # the circular buffers' mNumEntries (saved, the stream's second dword loaded)
RINGS = {0x4e2c10: 0x80, 0x4cdf00: 0x248, 0x4ce2d0: 0xd0, 0x4ce660: 0x1c, 0x4cdb80: 0xc, 0x4cea00: 0x38}  # (where each keeps it)
SIZES = {0x4cdf00: 144, 0x4ce2d0: 50, 0x4ce660: 5, 0x4cdb80: 3, 0x4cea00: 50}   # (their sizes, but 30's: the counts about them)
ELEMENTS = {0x4cdb80: 2, 0x4cea00: 1}   # (their entries' size, else 4)


def case(r):
    at = r.deal(OPS)
    sizes = FIELDS[at]
    if at in RINGS:
        n = SIZES.get(at)
        count = r.choice(COUNTS + [r.randrange(41)] + ([n, n + 1, n + 6, r.randrange(n + 6)] if n else []))
        sizes = sizes + [ELEMENTS.get(at, 4)] * (count if count < 0x80000000 else 0)
    ends = [sum(sizes[:i + 1]) for i in range(len(sizes))]
    total = ends[-1]
    end = r.choice(ends[:3] + ends[-3:])
    left = r.choice([total, total, total, 0x1400, 0, end, end - 1, end + 1, r.randrange(total)])
    if at == 0x599f60:   # (the advisor: 0x16f0 bytes, and a sink for them all)
        regions = {OBJ: bytearray(r.bytes(0x16f0)), BIG_STREAM: record_stream(BIG_STREAM, BIG_SINK, left), BIG_SINK: bytearray(r.bytes(0x1700))}
        save = r.choice([0, 1, 1, r.getrandbits(32)])
        version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
        return Call(at, [OBJ, BIG_STREAM, save, version & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')
    regions = {OBJ: bytearray(r.bytes(0x1300 if at != 0x502dc0 else 0x1480)), STREAM: record_stream(STREAM, SINK, left), SINK: bytearray(r.bytes(0x1400))}
    if at in RINGS:
        regions[OBJ][RINGS[at]:RINGS[at] + 4] = regions[SINK][4:8] = count.to_bytes(4, 'little')
    save = r.choice([0, 1, 1, r.getrandbits(32)])
    version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
    return Call(at, [OBJ, STREAM, save, version & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')


OPS = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k] or list(FIELDS)

if __name__ == '__main__':
    B.main(case)
