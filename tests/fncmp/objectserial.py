# An object's serializer (src/game/world/object: 0x4db7d0) against the original: saves and loads through a record stream with room for all
# the fields, none, or up to and about each field's end; versions in and out of 0..1. The six histories' counts (each +0x80 saved, the
# stream's second dword of each loaded) small, 30, 31 or negative; the date's SYSTEMTIME through FileTimeToSystemTime / SystemTimeToFileTime
# recorders filling their outs (the first always succeeding: on a failure the original saves stack dwords), the SYSTEMTIME handed over logged
# as the hash of its dwords but the day's (the day of the week beside it is the original's stack). The stats +0x44..+0x4c whole bytes, about
# them or any; the price per use loaded about 0 and 500. The object and the sink compared after.
# usage: objectserial.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
import serializers  # noqa: E402
from fncmp import IMPORT_TABLE as IAT, LOG, Bench, Call, RAM, blob, fb, fill, import_table, log_region, put, record_stream, set_return  # noqa: E402

B = Bench([(0x4db7d0, 0x4dcf90), (0x4ced70, 0x4cedf0), (0x4cee50, 0x4ceed0), (0x5fc5c0, 0x5fc65e), (0x5fc660, 0x5fc733)] + serializers.RANGES)
OBJ, STREAM, SINK = RAM, RAM + 0x2000, RAM + 0x3000
IMPORTS = dict(funnytime.IMPORTS)
IMPORTS['SystemTimeToFileTime'] = (41, 0x6fd150, 2, 0, 0)  # (its SYSTEMTIME a blob)
TO, FROM = IMPORTS['FileTimeToSystemTime'][0], IMPORTS['SystemTimeToFileTime'][0]
HISTS = (0xf8, 0x70, 0x1a8, 0x230, 0x2b8, 0x340)
COUNTS = [0, 1, 2, 30, 31, 0xffffffff, 0x80000000]


def layout(counts):
    """The stream's field sizes for the histories' counts; the offsets of the counts, the date's and the price's."""
    sizes = [2, 2, 2, 2, 4, 2] + [4] * 8 + [4, 2] + [2] * 0x42 + [4, 4, 4, 2, 2, 2, 2, 2, 4, 2, 2, 4, 2]
    marks = []
    counts = list(counts)
    for item in 'hhnhnhhh':
        if item == 'n':
            sizes.append(4)
            continue
        n = counts.pop(0)
        marks.append(sum(sizes) + 4)
        sizes += [4, 4, 1, 4] + [4] * (n if n < 0x80000000 else 0)
    price = sum(sizes) + 1 + 1 + 4 + 2 + 4 * 3
    sizes += [1, 1, 4, 2, 4, 4, 4, 4] + [4] * 10 + [1]
    return sizes, marks, price


def case(r):
    counts = [r.choice(COUNTS + [r.randrange(32)]) for _ in HISTS]
    sizes, marks, price = layout(counts)
    ends = [sum(sizes[:i + 1]) for i in range(len(sizes))]
    total = ends[-1]
    end = r.choice(ends) if r.random() < 0.7 else r.choice(ends[-4:])
    left = r.choice([total, total, total, 0x1400, 0, end, end - 1, end + 1, r.randrange(total)])
    obj = bytearray(r.bytes(0x450))
    sink = bytearray(r.bytes(0x1400))
    for at, mark, n in zip(HISTS, marks, counts):
        put(obj, at + 0x80, 'I', n)
        put(sink, mark, 'I', n)
    for off in (0x44, 0x48, 0x4c):
        put(obj, off, 'I', fb(r.choice([0.0, 1.0, 50.5, 100.0, 255.0, 255.9, 256.0, -1.0])) if r.random() < 0.7 else r.f(1000.0))
    put(sink, price, 'I', r.choice([0, 1, 499, 500, 501, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)]))
    log = log_region()
    fill(log, TO, 1, r.bytes(16), 0)
    set_return(log, TO, r.choice([1, r.getrandbits(32) | 1]))
    fill(log, FROM, 1, r.bytes(8), 1)
    if r.random() < 0.7:
        set_return(log, FROM, r.choice([0, 1, r.getrandbits(32)]))
    blob(log, FROM, 0, 0b1101)
    regions = {OBJ: obj, STREAM: record_stream(STREAM, SINK, left), SINK: sink, LOG: log, IAT: import_table(log, IMPORTS)}
    save = r.choice([0, 0, 1, 1, r.getrandbits(32)])
    version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
    return Call(0x4db7d0, [OBJ, STREAM, save, version & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
