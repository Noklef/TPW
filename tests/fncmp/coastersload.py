# A park's coasters read (src/game/coaster/coastersload: 0x437b60) against the original: 0-2 coasters of a stream (a sequenced read through the real Stream_Read) made to their shape: their pieces, trains, entries, records and seats; all
# the flags. Its calls recorders (driver.cpp's kCoastersLoadCalls, from CALLS below: coastersload.py --table prints them). The stream is never short: past its end the original goes on over its stack, which differs between the sides. A case
# that lets the entry's position go unset (a part of -1) logs it as 0 (stack garbage); the others log its bytes.
# usage: coastersload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_fold, log_region, put, seq_read, set_return, strings, vtable  # noqa: E402

BEGIN_END, READ, SET_LENGTH, SET_E0, SET_ANGLES, PLACE_CARS, POSITION, FIND_PART, ENTRY_POSITION, ADD_ENTRY, ADD_CAR, SET_TOOLS, SET_STATE, PIECE_AT, CLAIM, REMOVE, SLEEP, PLAYING, SETUP, END_ALL, FLASH, SET_UP_CARS, FIXED, ID_ADD, PLACE = range(1, 26)
CALLS = {0x442cd0: (BEGIN_END, 0), 0x435020: (PLACE, 7), 0x444150: (SET_LENGTH, 4), 0x4441d0: (SET_E0, 4), 0x444270: (SET_ANGLES, 5), 0x432df0: (PLACE_CARS, 4), 0x435710: (POSITION, 2), 0x44b220: (FIND_PART, 3), 0x4386d0: (ENTRY_POSITION, 3),
         0x432c10: (ADD_ENTRY, 6), 0x434060: (ADD_CAR, 9), 0x442880: (SET_TOOLS, 1), 0x435730: (SET_STATE, 2), 0x43e520: (PIECE_AT, 2), 0x463ab0: (CLAIM, 9), 0x45be80: (REMOVE, 3), 0x45bdd0: (SLEEP, 1), 0x472f40: (PLAYING, 1), 0x43ba90: (SETUP, 4),
         0x4426a0: (END_ALL, 0), 0x441840: (FLASH, 2), 0x436d10: (SET_UP_CARS, 3), 0x436ac0: (FIXED, 5), 0x468760: (ID_ADD, 7)}
HOOKS = {addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CALLS.items()}
B = Bench([(0x437b60, 0x4386d0), (0x5f5e20, 0x5f5e50), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
STREAM, VT, DATA, ANSWERS = RAM, RAM + 0x80, RAM + 0x200, RAM + 0x4000
SEAT_KIND, RECORD_KIND, FLOAT_AT, FLOAT_HOLDER, PARTS_AT, PARTS, BUFFERS = (RAM + 0x5000 + 0x100 * k for k in range(7))
TYPE, COASTER, RECORDS, SEATS, CARS, TRAIN, MODEL, MODEL_DATA, HOLDER, LINK = (RAM + 0x10000 + 0x1000 * k for k in range(10))
ENTRIES = RAM + 0x20000
IDS = (2, 5)
SEAT_IDS = 8
RECORD_COUNT, SEATS_PER = 2, 2


def dwords(*values):
    return struct.pack('<%dI' % len(values), *[v & 0xffffffff for v in values])


def seats_region(r, regions):
    """The two records' seats; for each its two buffers or not, and returns which have both (those read them)."""
    block = bytearray(r.bytes(0x60 * RECORD_COUNT * SEATS_PER))
    both = []
    for j in range(RECORD_COUNT * SEATS_PER):
        first, second = r.choice([0, 1, 1]), r.choice([0, 1, 1])
        put(block, 0x60 * j + 4, 'I', SEAT_KIND)
        put(block, 0x60 * j + 0x28, 'I', BUFFERS + 0x100 * (2 * j) if first else 0)
        put(block, 0x60 * j + 0x2c, 'I', BUFFERS + 0x100 * (2 * j + 1) if second else 0)
        regions[BUFFERS + 0x100 * (2 * j)] = r.bytes(0x20)
        regions[BUFFERS + 0x100 * (2 * j + 1)] = r.bytes(0x20)
        both.append(first and second)
    regions[SEATS] = block
    return both


def world(r, regions):
    """What the coasters point to. Returns the type's trains, a record's seats, the seats' buffers."""
    trains, seats = r.choice([0, 1, 2]), r.choice([0, 1, 2])
    kind = bytearray(r.bytes(0x300))
    put(kind, 0, 'I', r.choice([0x20, 0x120, 0, 0xffffffdf]))
    put(kind, 0x2e8, 'I', trains)
    regions[TYPE] = kind
    seat_kind = bytearray(r.bytes(0x10))
    put(seat_kind, 0xc, 'I', r.choice([0, RAM + 0x9000, RAM + 0x9000]))
    regions[SEAT_KIND] = seat_kind
    regions[RECORD_KIND] = dwords(0, seats)
    records = bytearray(r.bytes(0x80 * RECORD_COUNT))
    for i in range(RECORD_COUNT):
        put(records, 0x80 * i + 4, 'I', RECORD_KIND)
        put(records, 0x80 * i + 0x28, 'I', SEATS + 0x60 * SEATS_PER * i)
    regions[RECORDS] = records
    both = seats_region(r, regions)
    # An entry's holder: +4 what has the float, +0x28 the parts.
    holder = bytearray(r.bytes(0x40))
    put(holder, 4, 'I', FLOAT_HOLDER)
    put(holder, 0x28, 'I', PARTS_AT)
    regions[HOLDER] = holder
    regions[FLOAT_HOLDER] = bytes(0x78) + dwords(FLOAT_AT)
    regions[FLOAT_AT] = bytes(0x44) + struct.pack('<f', r.f())
    regions[PARTS_AT] = dwords(0, PARTS)
    parts = bytearray(r.bytes(0x20))
    put(parts, 4, 'I', r.choice([0, RAM + 0xa000, RAM + 0xa000]))
    regions[PARTS] = parts
    table = [0] * SEAT_IDS
    for i in range(1, SEAT_IDS):
        entry = bytearray(r.bytes(0xd0))
        put(entry, 4, 'I', r.choice([0x4000, 0, 0x4001, 0xffffbfff]))
        put(entry, 8, 'I', HOLDER)
        regions[ENTRIES + 0x100 * i] = entry
        table[i] = ENTRIES + 0x100 * i
    regions[0x7a4610] = dwords(*table)
    model = bytearray(r.bytes(0x20))
    put(model, 0x18, 'I', MODEL_DATA)
    regions[MODEL] = model
    data = bytearray(r.bytes(0x50))
    data[0] = r.choice([8, 0, 9, 0xf7])
    part = r.choice([-1, 0, 0, 0])
    put(data, 0x2c, 'i', part)
    regions[MODEL_DATA] = data
    cars = bytearray(r.bytes(0x200 * 3))
    for i in range(3):
        put(cars, 0x200 * i, 'I', r.choice([2, 2, 0, 1]))
        put(cars, 0x200 * i + 0x64, 'I', CARS + 0x200 * (i + 1) if i < 2 else 0)
        put(cars, 0x200 * i + 0x70, 'I', r.choice([1, 2]))
        put(cars, 0x200 * i + 0xb0, 'I', r.randrange(3))
        put(cars, 0x200 * i + 0x100, 'I', MODEL)
    regions[CARS] = cars
    trains_block = bytearray(r.bytes(0x200))
    for i in range(2):
        put(trains_block, 0x100 * i + 4, 'I', r.choice([0, 0, 2, 1]))
        put(trains_block, 0x100 * i + 0x28, 'I', TRAIN + 0x100 if i == 0 and r.random() < 0.3 else 0)
        put(trains_block, 0x100 * i + 0x30, 'I', r.randrange(4))
        for j in range(3):
            put(trains_block, 0x100 * i + 0x34 + 4 * j, 'I', CARS + 0x200 * j)
    regions[TRAIN] = trains_block
    return trains, seats, both, kind[0] & 0x20, part


def coaster(r, regions, at, cid, following):
    c = bytearray(r.bytes(0x150))
    put(c, 4, 'I', TYPE)
    put(c, 0x10, 'I', cid)
    put(c, 0x18, 'I', following)
    put(c, 0x3c, 'I', r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    put(c, 0x4c, 'I', r.choice([0, TRAIN, TRAIN]))
    put(c, 0x54, 'I', CARS)
    put(c, 0x58, 'I', CARS + 0x200 * r.randrange(3))
    put(c, 0x128, 'I', RECORDS)
    regions[at] = c


def stream_of(r, count, trains, seats, both, placed):
    """Past the first coaster's pieces the log can be full, so that the placed answers are frozen at one of the two: the later pieces have no words, and the stream's shape does not depend on them."""
    data = bytearray(r.bytes(4))
    put(data, 0, 'I', count)
    data += r.bytes(12)
    for c in range(count):
        header = bytearray(r.bytes(0x20))
        header[0] = r.choice([r.getrandbits(8), r.getrandbits(8), r.getrandbits(8) & 0xfe, r.getrandbits(8) | 3])
        put(header, 0xa, 'H', r.choice(IDS) if r.random() < 0.98 else 9)
        pieces = r.choice([0, 1, 2, 2])
        put(header, 0x16, 'H', pieces)
        data += header
        words = r.randrange(3)
        data += dwords(words) + r.bytes(2 * words)
        for i in range(pieces):
            p = bytearray(r.bytes(0x22))
            n = r.randrange(3) if c == 0 else 0
            p[0x21] = n
            data += p
            if c == 0 and placed[i]:
                data += r.bytes(2 * n)
        for t in range(trains):
            data += r.bytes(0x1b)
            for s in range(seats):
                seat = bytearray(r.bytes(0xb))
                seat[4], seat[5] = r.randrange(4), r.randrange(4)
                put(seat, 7, 'I', r.randrange(1, SEAT_IDS) if r.random() < 0.9 else 0)
                if seat[7] == 0:
                    seat[0] |= 1   # (a seat of id 0 not flagged faults on its null entry)
                data += seat
                if both[t * SEATS_PER + s]:
                    data += r.bytes(4 * seat[4]) + r.bytes(4 * seat[5])
    return bytes(data)


def case(r):
    log = log_region()
    log_fold(log)
    regions = {LOG: log, STREAM: dwords(VT, 0, 0, 0), VT: vtable({2: 2})}
    trains, seats, both, flagged, part = world(r, regions)
    second = COASTER + 0x400
    coaster(r, regions, COASTER, IDS[0], second)
    coaster(r, regions, second, IDS[1], 0)
    regions[0x790fe0] = dwords(COASTER if r.random() < 0.98 else 0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32))
    regions[0x790afc] = r.bytes(12)
    regions[0x791ce0] = r.bytes(4)
    regions[0x785a2c] = dwords(r.choice([0, 1]))
    placed = [r.choice([0, 1, 1, 1]) for _ in range(2)]
    data = stream_of(r, r.deal([0, 1, 1, 1, 1, 2, 2]), trains, seats, both, placed) + r.bytes(0x20)
    regions[DATA] = data
    seq_read(log, 2, DATA, len(data), buffer=0, count=1)
    strings(log, 2, 0, out=1)
    fill(log, PLACE, 3, dwords(*placed), which=0, records=2)
    fill(log, PLACE, 4, dwords(*[CARS + 0x200 * r.randrange(3) for _ in range(2)]), which=1, records=2)
    strings(log, PLACE, 0, out=1 << 3 | 1 << 4)
    fill(log, POSITION, 1, r.bytes(12), which=2)
    strings(log, POSITION, 0, out=1 << 1)
    fails = r.random() < 0.3
    regions[ANSWERS] = dwords(*[0xffffffff if fails and r.random() < 0.5 else r.randrange(0x100) for _ in range(40)])
    answers(log, FIND_PART, ANSWERS)
    blob(log, ENTRY_POSITION, 1, 1)
    fill(log, ENTRY_POSITION, 2, r.bytes(12), which=3)
    strings(log, ENTRY_POSITION, 0, out=1 << 2)
    regions[LINK] = r.bytes(0x30)
    set_return(log, ADD_ENTRY, LINK)
    if (part == -1) if flagged else fails:
        strings(log, ADD_ENTRY, 0, out=1 << 5)
    else:
        blob(log, ADD_ENTRY, 5, 7)
    return Call(0x437b60, [STREAM], regions, ret='a')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (k, n) in sorted(CALLS.items()):
            print('    {0x%06x, jpeg_recorder(%d, %d)},' % (addr, k, n))
        sys.exit(0)
    B.main(case)
