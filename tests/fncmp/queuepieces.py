# The queue pieces (src/game/world/queuepieces: 0x5229e0, 0x522a90) against the originals. Place: the table at 0x763388 (8 pieces and some
# bytes past them; the piece 0-9) its types one of coaster.py's claim environment's or any, the flag bytes any; x and y 1 or 2 mostly, the
# angle 0, 90, 180, 270, 360 or any; with claim, Ride_Claim over coaster.claim()'s rides, some of them the piece's type with the key 0x33a
# and the position and turn the description has (the two words at 0x816d68 1 or 2); without, Ride_Create (0x463060, agent B's, unported) a
# cdecl recorder (k 30, driver.cpp's kQueuePiecesEnv) logging the description as a hash of its four dwords, its result now and then set.
# Remove: Ride_Destroy (0x463c80) a cdecl recorder (k 31) of the ride. PlaceTrack / PlaceAnimated / PlaceEnd (0x522b90 / 0x522d50 / 0x522f20):
# the same over the tables at 0x7633e8 / 0x7634d8 / 0x763608 with the two words at 0x8179f8, the matching rides at the position the turn
# moves (90: y + 1, 180: both, 270: x + 1). PlaceAnimated's ride (when not 0) plays: without claim the recorder's result 0 or 1 mostly,
# entry 1 of 0x7a4610 animator.py's animator; with claim no ride matches (a claimed ride is a car, not an animator).
# Destroy / DestroyTrack / DestroyAnimated / DestroyEnd (0x5229b0 / 0x522b60 / 0x522d20 / 0x522ef0): the four tables (and a row past each) with
# their name pointers null or into a block of '-', 'a' and 'q' characters, the rest any; Ride_Destroy's recorder logs the rows' +0 in order.
# DestroyAll (0x52f190): the four in turn over the four tables at once.
# usage: queuepieces.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Place,Remove,PlaceTrack,PlaceAnimated,PlaceEnd,Destroy,DestroyTrack,DestroyAnimated,DestroyEnd,DestroyAll)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animator  # noqa: E402
import coaster  # noqa: E402
from fncmp import LOG, Bench, Call, _recorder, blob, log_region, put, set_return  # noqa: E402

B = Bench([(0x5229e0, 0x522a8a), (0x522a90, 0x522a9e), (0x463ab0, 0x463c7a), (0x412e90, 0x412eea), (0x436820, 0x4368f0), (0x5da3c0, 0x5da3d0),
           (0x522b90, 0x522c59), (0x522d50, 0x522e3e), (0x522f20, 0x522fe9),
           (0x5229b0, 0x5229d5), (0x522b60, 0x522b8a), (0x522d20, 0x522d45), (0x522ef0, 0x522f15), (0x52f190, 0x52f1a4),
           # (animator.py's, for Animator_Play)
           (0x46f150, 0x475010), (0x67a400, 0x690000), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172), (0x4031e0, 0x403202),
           (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)], sines=256,
          hooks={0x463060: _recorder(30, 10, cdecl=True), 0x463c80: _recorder(31, 1, cdecl=True)})
CALLS = {'Place': 0x5229e0, 'Remove': 0x522a90, 'PlaceTrack': 0x522b90, 'PlaceAnimated': 0x522d50, 'PlaceEnd': 0x522f20,
         'Destroy': 0x5229b0, 'DestroyTrack': 0x522b60, 'DestroyAnimated': 0x522d20, 'DestroyEnd': 0x522ef0,
         'DestroyAll': 0x52f190}
# (the table, its rows, whether a name starting '-' is passed over)
DESTROYS = {'Destroy': (0x763388, 8, False), 'DestroyTrack': (0x7633e8, 20, True), 'DestroyAnimated': (0x7634d8, 25, False),
            'DestroyEnd': (0x763608, 28, False)}
NAMES = 0x30000000
# (the table, the description's last dword, whether the turn moves the position)
PLACES = {'Place': (0x763388, 0x816d68, False), 'PlaceTrack': (0x7633e8, 0x8179f8, True), 'PlaceAnimated': (0x7634d8, 0x8179f8, True),
          'PlaceEnd': (0x763608, 0x8179f8, True)}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    if op == 'Remove':
        return Call(0x522a90, [r.choice([0, 1, 5, r.getrandbits(32)])], {LOG: log})
    if op in DESTROYS:
        table_at, rows, _ = DESTROYS[op]
        names = bytes(r.choice(b'-a-q') for _ in range(0x40))
        table = bytearray(r.bytes(12 * rows + 12))
        for k in range(rows):
            put(table, 12 * k + 4, 'I', r.choice([0, 0, NAMES + r.randrange(0x40), NAMES + r.randrange(0x40)]))
        return Call(CALLS[op], [], {LOG: log, table_at: table, NAMES: names}, conv='c')
    if op == 'DestroyAll':
        regions = {LOG: log, NAMES: bytes(r.choice(b'-a-q') for _ in range(0x40))}
        for table_at, rows, _ in DESTROYS.values():
            table = bytearray(r.bytes(12 * rows))
            for k in range(rows):
                put(table, 12 * k + 4, 'I', r.choice([0, 0, NAMES + r.randrange(0x40), NAMES + r.randrange(0x40)]))
            regions[table_at] = table
        return Call(CALLS[op], [], regions, conv='c')
    table_at, words_at, moves = PLACES[op]
    animated = op == 'PlaceAnimated'
    claim = r.random() < 0.5
    regions = dict(coaster.claim(r, 'Claim').regions) if claim else {}
    m = struct.unpack('<I', regions[0x7a439c])[0] if claim else 3
    x, y = r.choice([1, 2, 2, r.getrandbits(32)]), r.choice([1, 2, 2, r.getrandbits(32)])
    angle = r.choice([0, 0, 90, 180, 270, 360, r.getrandbits(32)])
    turn = (360 - angle) & 0xffffffff
    turn = 0 if turn == 360 else turn
    px, py = x, y
    if moves:
        px, py = (x + (turn in (180, 270))) & 0xffffffff, (y + (turn in (90, 180))) & 0xffffffff
    table = bytearray(r.bytes(0x80))
    for k in range(8):
        put(table, 12 * k, 'I', r.randrange(m + 1) if r.random() < 0.8 else r.getrandbits(32))
        table[12 * k + 8] = r.choice([0, 1, r.getrandbits(8)])
    piece = r.choice(list(range(8)) + [8, 9])
    regions[table_at] = table
    regions[words_at] = struct.pack('<2I', r.choice([1, 2]), r.choice([1, 2]))
    if claim and not animated:   # (some rides of the piece's type with its key, position and turn)
        type_ = struct.unpack_from('<I', table, 12 * piece)[0]
        types = struct.unpack('<%dI' % (m + 1), regions[0x7ae6a8])
        cars, deep = bytearray(regions[coaster.CARS]), bytearray(regions[coaster.SPL])
        for k in range(len(cars) // 0x100):
            if r.random() < 0.4:
                put(cars, 0x100 * k, 'I', types[type_] if type_ <= m and r.random() < 0.8 else r.getrandbits(32))
                put(cars, 0x100 * k + 0xe4, '3I', r.choice([0x33a, 0x33a, 0x33b]), px if r.random() < 0.9 else 1, py if r.random() < 0.9 else 1)
                put(cars, 0x100 * k + 0xf0, '2I', *struct.unpack('<2I', regions[words_at]))
                if k < len(deep) // 0x20:
                    put(deep, 0x20 * k + 0x10, 'I', turn if r.random() < 0.9 else 1)
        regions[coaster.CARS], regions[coaster.SPL] = cars, deep
    elif not claim:
        blob(log, 30, 2, 0xf)
        if animated:
            regions.update(animator.world(r)[0])
            regions[0x7a4610] = struct.pack('<II', r.getrandbits(32), animator.ANIM)
            set_return(log, 30, r.choice([0, 1, 1, 1]))
        elif r.random() < 0.5:
            set_return(log, 30, r.choice([0, 1, r.getrandbits(32)]))
    regions[LOG] = log
    return Call(CALLS[op], [x, y, r.getrandbits(32), piece, angle, r.choice([1, 0x100, r.getrandbits(32) | 1]) if claim else 0],
                regions, ret='a')


if __name__ == '__main__':
    B.main(case)
