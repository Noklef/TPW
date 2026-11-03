# A visitor comes to the object of +0x1dc (src/game/world/visitorsteps Visitor_ArriveAtObject, 0x5014e0) against the original: object_checks.py's Stand object and
# definitions in a slot of the table 0x7cfb90, its cells +0x36 and +0x38 the same or not, in a block of the map: each cell's word its id, the direction +0xd of the
# eight (rotated when the two are the same) or rarely 0 or two bits (no step: a fault in both), now and then at the map's edge; the cell stepped to of type 3, 9 or
# others and a word that names another cell now and then. Sprite_SetDestinationCell (3 of 1), Visitor_SetState (26 of 1), Visitor_VisitObject (15 of 1),
# Thing_PlaceAt (16 of 2) and Object_Reopen (17 of 0) recorders: driver.cpp's VisitorStepsEnv.
# Waiting (0x4e1410 Object_ArriveWaiting): the same visitor in a slot of its own, its handle the low word of the ride record's element 1 (or 0); the record found by the object's
# +0x24 or none, its count +0x8c small, the list ready or not.
# usage: visitorarrive.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Arrive,Waiting)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import object_checks as oc  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x5014e0, 0x501650), (0x4dedf0, 0x4df150), (0x412e90, 0x412f00), (0x4dd4e0, 0x4dd510), (0x50b350, 0x50b370), (0x522850, 0x522860), (0x4d8c00, 0x4d8c40),
           (0x4d96f0, 0x4d9a70), (0x4d8300, 0x4d8328), (0x536320, 0x536340), (0x5da3c0, 0x5da3c1), (0x495cf0, 0x495d50),
           (0x4e1410, 0x4e14e0), (0x55a070, 0x55a100), (0x55a390, 0x55a3e0)],
          hooks={0x4fa530: _recorder(3, 1), 0x501db0: _recorder(26, 1), 0x4fd970: _recorder(15, 1), 0x4fa930: _recorder(16, 2), 0x4df150: _recorder(17, 0)})
VISITOR, MAP, HOST, RIDE = RAM + 0x60000, RAM + 0x100000, RAM + 0x70000, RAM + 0x71000
CALLS = {'Arrive': 0x5014e0, 'Waiting': 0x4e1410}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
ROWS, COLUMNS, CELL = 5, 0x80, 0x44
PAIRS = [(0, -1), (0, 1), (-1, 1), (1, 1), (-1, -1), (1, 0), (-1, 0), (1, -1)]   # (lineends.py's: 0x7cdba0 and on)
DIRECTIONS = [0x01, 0x10, 0x20, 0x08, 0x80, 0x04, 0x40, 0x02]


def waiting(r, c):
    """Waiting: see the header."""
    regions = c.regions
    table = regions[0x7cfb90]
    h = next(k for k in range(1, 16) if struct.unpack_from('<I', table, 20 * k)[0] != oc.OBJ)
    put(table, 20 * h, 'I', VISITOR)
    rid = r.choice([1, 2, 7])
    regions[HOST] = put(bytearray(r.bytes(0x40)), 0x24, 'I', r.choice([rid] * 5 + [3]))
    node = bytearray(r.bytes(0x100))
    put(node, 0, 'I', 0)
    put(node, 8, 'I', rid)
    put(node, 0x1c, 'I', RIDE + 0x100)
    put(node, 0x8c, 'i', r.choice([2, 2, 2, 3, 1, 0]))
    regions[RIDE] = node
    regions[RIDE + 0x100] = put(bytearray(r.bytes(0x10)), 4, 'I', r.choice([h, h, h, 0]) | r.choice([0, r.getrandbits(16)]) << 16)
    regions[0x8791a0] = struct.pack('<I', r.choice([1, 1, 1, 0]))
    regions[0x8791b0] = struct.pack('<I', RIDE)
    return Call(CALLS['Waiting'], [HOST], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    c = oc.stand(r)
    regions = c.regions
    cells = bytearray(CELL * COLUMNS * ROWS)
    for i in range(COLUMNS * ROWS - 1):
        put(cells, i * CELL, 'H', i + 1)
        put(cells, i * CELL + 8, 'I', r.choice([0, 1, 3, 9, 3, 9, 2, 0x15]) if i % 7 == 0 else r.choice([0, 1, 2, 2, 0x15]))
    column = r.randrange(1, 0x7f) if r.random() < 0.9 else r.choice([0, 0x7f])
    here = (r.randrange(1, ROWS - 1) << 7) + column + 1
    cells[(here - 1) * CELL + 0xd] = r.choice(DIRECTIONS * 4 + [0, 3])
    for pair in PAIRS:
        there = here + (pair[1] << 7) + pair[0]
        if 1 <= there <= COLUMNS * ROWS - 1:
            put(cells, (there - 1) * CELL + 8, 'I', r.choice([0, 1, 3, 9, 0x15]))
            if r.random() < 0.1:
                put(cells, (there - 1) * CELL, 'H', r.randrange(1, COLUMNS * ROWS))
    obj = bytearray(regions[oc.OBJ])
    put(obj, 0x38, 'H', here)
    put(obj, 0x36, 'H', here if r.random() < 0.4 else r.randrange(1, COLUMNS * ROWS))
    slot = r.randrange(1, 16)
    table = bytearray(r.bytes(20 * 16))
    put(table, 20 * slot, 'I', oc.OBJ)
    visitor = bytearray(r.bytes(0x240))
    put(visitor, 0x1dc, 'H', slot)
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 3, 1 << 16)            # (the cell, a word with a stale stack word above)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 16, 1 << 16 | 1 << 17)  # (x and y, the same)
    set_return(log, 3, r.choice([0, 1, 1, 1, 0x100]))
    pairs = b''.join(struct.pack('<ii', *pair) for pair in PAIRS)
    regions.update({oc.OBJ: obj, VISITOR: visitor, 0x7cfb90: table, MAP: cells, LOG: log, 0x7cdba0: pairs,
                    0x8023a0: struct.pack('<I', MAP), 0x80239c: struct.pack('<I', MAP - 0x294 - CELL)})
    c = Call(CALLS['Arrive'], [VISITOR], regions, conv='t', ret='a')
    return waiting(r, c) if op == 'Waiting' else c


if __name__ == '__main__':
    B.main(case)
