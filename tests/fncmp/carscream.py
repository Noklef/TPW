# A coaster car's scream update (src/game/coaster/carsound: 0x4393e0 CoasterCar_UpdateScream) against the original, over soundset.py's
# senders' world for 0x51c300 (Sound_Stop: the sound globals, on and started either 0 now and then, the soundobject bench's object, ids of
# its voices or not) with rssescream.py's and soundplay.py's recorders added: the manager's vtable slot 2 (the play command, hashed), the
# object's slots 3 (the parameter) and 5 (the position, hashed), their results now and then set (0 too). The link's +0x9c entries (0x38
# bytes, five: the bits 2 and 4 of the first byte any) at the car's +0x74 float (0-4, halves rounding either way); the car's flags (0x100,
# 0x10 either), its +0x6c sound and +0x70 scream (0, a voice's id or any), its +0x10 level; the link's +8 bit 0x40 now and then; 0x785a2c
# 0 mostly; the position ints small, large or negative.
# usage: carscream.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, blob, put, set_return, vtable  # noqa: E402

_ops = os.environ.pop('OPS', None)   # (soundset.py reads OPS as its senders' addresses)
import soundset  # noqa: E402
import soundobject as so  # noqa: E402
import rssescream  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(sorted(set([(0x4393e0, 0x439570), (0x551130, 0x551284), (0x551290, 0x5512c8), (0x5512d0, 0x5512f4), (0x551300, 0x551312),
                      (0x551320, 0x551560), (0x551560, 0x551600), (0x51bfc0, 0x51c048), (0x51bc40, 0x51bc70), (0x51c270, 0x51c2bc),
                      (0x6b5b60, 0x6b5b75), (0x6b5b80, 0x6b5b9a), (0x5da3c0, 0x5da3c1)] + soundset.RANGES)))
LINK, ENTRIES, CAR, MVT = RAM + 0x30000, RAM + 0x30100, RAM + 0x30400, RAM + 0x30500


def case(r):
    c = soundset.sender_case(r, 0x51c300)
    pick = c.args[0]
    regions = dict(c.regions)
    log = regions[LOG]
    blob(log, 2, 1, 0x7f)
    blob(log, 5, 1, 0x7)
    for k in (2, 3, 5):
        if r.random() < 0.6:
            set_return(log, k, r.choice([0, pick, r.getrandbits(32)]))
    regions[soundset.SMGR] = put(bytearray(regions[soundset.SMGR]), 0, 'I', MVT)
    regions.update({MVT: vtable({2: 2}, 4), so.OVT: vtable({3: 3, 5: 2, 6: 2}), 0x803a24: r.bytes(4),
                    0x785a2c: struct.pack('<I', r.choice([0] * 6 + [1, r.getrandbits(32)]))})
    entries = bytearray(r.bytes(0x38 * 5))
    for i in range(5):
        put(entries, 0x38 * i, 'B', r.getrandbits(8) & ~6 | r.choice([0, 2, 4, 6]))
    link = put(bytearray(r.bytes(0xa0)), 0x9c, 'I', ENTRIES)
    put(link, 8, 'B', link[8] & ~0x40 | (0x40 if r.random() < 0.2 else 0))
    car = bytearray(r.bytes(0x80))
    put(car, 0, 'I', r.getrandbits(32) & ~0x110 | r.choice([0, 0x100, 0x100, 0x10, 0x110]))
    put(car, 0x10, 'I', r.choice(list(range(18)) + [r.getrandbits(32)]))
    put(car, 0x6c, 'I', r.choice([0, 0, pick, r.getrandbits(32)]))
    put(car, 0x70, 'I', r.choice([0, 0, pick, pick, r.getrandbits(32)]))
    put(car, 0x74, 'f', r.choice([0.0, 1.0, 2.0, 3.0, 4.0, 0.5, 1.5, 2.5, 3.5, 0.49, -0.4, r.uniform(-0.49, 4.49)]))
    regions.update({LINK: link, ENTRIES: entries, CAR: car})
    return Call(0x4393e0, [LINK, CAR, rssescream.coord(r), rssescream.coord(r), rssescream.coord(r)], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
