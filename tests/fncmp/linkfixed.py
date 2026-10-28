# A new coaster link's fixed pieces (src/game/coaster/registry: CoasterLink_AddFixedPieces 0x436ac0) against the original. Its calls are
# cdecl recorders (the driver's CoasterLinksEnv): Model_FindPartByKey4 0x44b220 (9 of 3, answering from a table: a part 0-3 or -1),
# CoasterLink_AddEntry 0x432c10 (10 of 6, an entry; the position logged by its three dwords), CoasterLink_AddCar 0x434060 (11 of 9),
# Car_SetChannelRates 0x430ca0 (12 of 2), CoasterEdit_SetBounds 0x434f10 (13 of 0) and Coaster_FlashCar 0x441840 (14 of 2); __ftol and
# the empty Debug_Printf run. The link: +0x40 set now and then, +0x10 a ride of four (the table 0x7a4610: holders whose +8 object's
# +0x28 -> +4 list of four 0x14-byte entries points at part objects (+0x30 a position: floats, a signalling NaN now and then) or none,
# and whose +4 -> +0x78 frame has a float +0x44, a quiet NaN now and then), +4 a type whose flags byte has 0x20 or not; the entry's +0x1c / +0x20 / +0x24 random;
# the flag 0, 1 or any; the game 0x790fec's +4 any.
# usage: linkfixed.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, log_region, put, set_return  # noqa: E402

HOOKS = {0x44b220: _recorder(9, 3, cdecl=True), 0x432c10: _recorder(10, 6, cdecl=True), 0x434060: _recorder(11, 9, cdecl=True),
         0x430ca0: _recorder(12, 2, cdecl=True), 0x434f10: _recorder(13, 0, cdecl=True), 0x441840: _recorder(14, 2, cdecl=True)}
RANGES = [(0x436ac0, 0x436d0a), (0x5da3c0, 0x5da3d0)]
B = Bench(RANGES, hooks=HOOKS)
LINK, TYPE, HOLD, LISTS, PARTS, ENTRY, CAR, GAME, TABLE = (RAM + k * 0x1000 for k in range(9))


def case(r):
    log = log_region()
    answers(log, 9, TABLE)
    blob(log, 10, 5, 0b111)
    set_return(log, 10, ENTRY)
    set_return(log, 11, CAR)
    table = struct.pack('<40I', *[r.choice([0, 1, 2, 3, 0xffffffff]) for _ in range(40)])
    link = bytearray(r.bytes(0x50))
    put(link, 4, 'I', TYPE)
    put(link, 0x10, 'I', r.randrange(4))
    put(link, 0x40, 'I', 0 if r.random() < 0.85 else r.getrandbits(32))
    ttype = bytearray(r.bytes(0x100))
    ttype[0] = r.getrandbits(8) & ~0x20 | r.choice([0, 0x20])
    hold = bytearray(r.bytes(0x400))
    lists = bytearray(r.bytes(0x400))
    parts = bytearray(r.bytes(0x1000))
    for k in range(4):
        o = 0x100 * k
        put(hold, o + 8, 'I', HOLD + o + 0x20)            # (the object)
        put(hold, o + 0x20 + 0x28, 'I', LISTS + o)        # (its parts' list)
        put(hold, o + 0x20 + 4, 'I', HOLD + o + 0x60)     # (its model)
        put(hold, o + 0x60 + 0x78, 'I', HOLD + o + 0xa0)  # (the frame)
        r.floats(hold, [o + 0xa0 + 0x44])
        if r.random() < 0.15:
            put(hold, o + 0xa0 + 0x44, 'I', 0x7fc00000 | r.getrandbits(22) | r.getrandbits(1) << 31)   # (a quiet NaN)
        put(lists, o + 4, 'I', LISTS + o + 0x10)
        for j in range(4):
            e = o + 0x10 + 0x14 * j
            obj = PARTS + 0x400 * k + 0x100 * j
            put(lists, e + 4, 'I', obj if r.random() < 0.85 else 0)
            p = 0x400 * k + 0x100 * j + 0x30
            r.floats(parts, [p, p + 4, p + 8])
            if r.random() < 0.15:
                put(parts, p + 4 * r.choice([0, 1, 1, 2]), 'I', 0x7f800001 | r.getrandbits(22))
    regions = {LOG: log, LINK: link, TYPE: ttype, HOLD: hold, LISTS: lists, PARTS: parts, ENTRY: r.bytes(0x48), CAR: r.bytes(0x10),
               GAME: r.bytes(0x10), TABLE: table, 0x790fec: struct.pack('<I', GAME),
               0x7a4610: struct.pack('<4I', *[HOLD + 0x100 * k for k in range(4)])}
    return Call(0x436ac0, [LINK, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])], regions)


if __name__ == '__main__':
    B.main(case)
