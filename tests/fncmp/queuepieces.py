# The queue pieces (src/game/world/queuepieces: 0x5229e0, 0x522a90) against the originals. Place: the table at 0x763388 (8 pieces and some
# bytes past them; the piece 0-9) its types one of coaster.py's claim environment's or any, the flag bytes any; x and y 1 or 2 mostly, the
# angle 0, 90, 180, 270, 360 or any; with claim, Ride_Claim over coaster.claim()'s rides, some of them the piece's type with the key 0x33a
# and the position and turn the description has (the two words at 0x816d68 1 or 2); without, Ride_Create (0x463060, agent B's, unported) a
# cdecl recorder (k 30, driver.cpp's kQueuePiecesEnv) logging the description as a hash of its four dwords, its result now and then set.
# Remove: Ride_Destroy (0x463c80) a cdecl recorder (k 31) of the ride.
# usage: queuepieces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Place,Remove)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import coaster  # noqa: E402
from fncmp import LOG, Bench, Call, _recorder, blob, log_region, put, set_return  # noqa: E402

B = Bench([(0x5229e0, 0x522a8a), (0x522a90, 0x522a9e), (0x463ab0, 0x463c7a), (0x412e90, 0x412eea), (0x436820, 0x4368f0), (0x5da3c0, 0x5da3d0)],
          hooks={0x463060: _recorder(30, 10, cdecl=True), 0x463c80: _recorder(31, 1, cdecl=True)})
CALLS = {'Place': 0x5229e0, 'Remove': 0x522a90}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    if op == 'Remove':
        return Call(0x522a90, [r.choice([0, 1, 5, r.getrandbits(32)])], {LOG: log})
    claim = r.random() < 0.5
    regions = dict(coaster.claim(r, 'Claim').regions) if claim else {}
    m = struct.unpack('<I', regions[0x7a439c])[0] if claim else 3
    x, y = r.choice([1, 2, 2, r.getrandbits(32)]), r.choice([1, 2, 2, r.getrandbits(32)])
    angle = r.choice([0, 0, 90, 180, 270, 360, r.getrandbits(32)])
    turn = (360 - angle) & 0xffffffff
    turn = 0 if turn == 360 else turn
    table = bytearray(r.bytes(0x80))
    for k in range(8):
        put(table, 12 * k, 'I', r.randrange(m + 1) if r.random() < 0.8 else r.getrandbits(32))
        table[12 * k + 8] = r.choice([0, 1, r.getrandbits(8)])
    piece = r.choice(list(range(8)) + [8, 9])
    regions[0x763388] = table
    regions[0x816d68] = struct.pack('<2I', r.choice([1, 2]), r.choice([1, 2]))
    if claim:   # (some rides of the piece's type with its key, position and turn)
        type_ = struct.unpack_from('<I', table, 12 * piece)[0]
        types = struct.unpack('<%dI' % (m + 1), regions[0x7ae6a8])
        cars, deep = bytearray(regions[coaster.CARS]), bytearray(regions[coaster.SPL])
        for k in range(len(cars) // 0x100):
            if r.random() < 0.4:
                put(cars, 0x100 * k, 'I', types[type_] if type_ <= m and r.random() < 0.8 else r.getrandbits(32))
                put(cars, 0x100 * k + 0xe4, '3I', r.choice([0x33a, 0x33a, 0x33b]), x if r.random() < 0.9 else 1, y if r.random() < 0.9 else 1)
                put(cars, 0x100 * k + 0xf0, '2I', *struct.unpack('<2I', regions[0x816d68]))
                if k < len(deep) // 0x20:
                    put(deep, 0x20 * k + 0x10, 'I', turn if r.random() < 0.9 else 1)
        regions[coaster.CARS], regions[coaster.SPL] = cars, deep
    else:
        blob(log, 30, 2, 0xf)
        if r.random() < 0.5:
            set_return(log, 30, r.choice([0, 1, r.getrandbits(32)]))
    regions[LOG] = log
    return Call(0x5229e0, [x, y, r.getrandbits(32), piece, angle, r.choice([1, 0x100, r.getrandbits(32) | 1]) if claim else 0],
                regions, ret='a')


if __name__ == '__main__':
    B.main(case)
