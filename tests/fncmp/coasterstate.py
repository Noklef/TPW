# A coaster's state and mode set (src/game/coaster/cars: 0x435730 Coaster_SetState, Set; 0x442c70 Coaster_ResumeRunning, Resume) against
# the originals. The transitions dealt: 1 -> 2 (Wake) and 2 -> 1 (Sleep) over carwake.py's world (its car the first of 0-2, a second with
# 0-2 segments without strips), 4 / 8 -> 2 (Unload) and 1 / 2 / 4 / 8 -> 8 (Advance) over coaster.py's Unload world (its train the first of
# 0-2, a second without cars), any other pair (Other: the cars none when it is a Wake or a Sleep) or the mode only (Mode). The old mode
# 0, 0x10, 0x20, 0x40 or a mix, the new one 0, 0x10, 0x20, 0x40 or any; the value's other bits now and then set. Coaster_FlashCar's
# old car (+0x144) none or one whose ride is not flashing, its cars (+0x54 / +0x64 by +0x3c bit 1) none or one whose ride is (no
# highlight calls: Highlight_Flash's own cases are endtool.py's). Resume: its pieces (+0x60 / +0x64) with flags 0x400 / 0x50000 / 0xa0000
# mixed, then a state 1 (Wake's world), 4 / 8 (Unload's) or another. ModeAt (0x43b270 Coaster_SetModeAt) / StepAt (0x43b2f0
# Coaster_StepStateAt): Unload's world, the coaster at its index (0-3) of its table 0x790bd0, its state 1, 2, 4, 8 or any, its mode any;
# which 0-3 or any, on 0, 1 or any.
# usage: coasterstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Resume,ModeAt,StepAt)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (carwake.py and coaster.py read OPS as their own)
import carwake  # noqa: E402
import coaster  # noqa: E402
import holder  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(sorted(set(carwake.RANGES + [(0x435730, 0x435a40), (0x442c70, 0x442ce0), (0x43b390, 0x43b610), (0x44b4c0, 0x44b520),
                                       (0x441840, 0x4418cb), (0x4346c0, 0x434700), (0x43b270, 0x43b2ae), (0x43b2f0, 0x43b328)])),
          hooks=holder.HOOKS)
SEGS2, WCOAST, FCAR, OLDCAR, FRIDE, ORIDE, PIECE = (RAM + 0x31000, RAM + 0x32000, RAM + 0x38000, RAM + 0x38100, RAM + 0x38200,
                                                     RAM + 0x38300, RAM + 0x38400)
OLD_MODES = [0, 0x10, 0x20, 0x40, 0x40, 0x30, 0x70, 0x50]
NEW_MODES = [0, 0x10, 0x20, 0x40]


def wake_world(r, sleep):
    """carwake.py's world: its car the first of the coaster's cars, a second after it (0x80 on) with segments SEGS2 (no strips)."""
    regions = carwake.world(r, 'Sleep' if sleep else 'Wake')
    second = bytearray(r.bytes(0x80))
    put(second, 8, 'II', 0, r.choice([0, 1, 2]))
    put(second, 0x28, 'I', SEGS2)
    segs = bytearray(r.bytes(0x60 * 2))
    for k in range(2):
        put(segs, 0x60 * k + 4, 'I', carwake.TYPES + r.choice([0, 0x10]))
        put(segs, 0x60 * k + 0xc, 'I', 0)
    regions[carwake.CAR] = bytes(regions[carwake.CAR]) + bytes(second)
    regions[SEGS2] = segs
    regions[WCOAST] = r.bytes(0x150)
    return regions, WCOAST, carwake.CAR


def unload_world(r):
    """coaster.py's Unload world: its train the first of the coaster's cars, a second (0x80 on) without cars."""
    regions = dict(coaster.queue(r, 'Unload').regions)
    cars = bytearray(regions[coaster.CARS])
    put(cars, 0x80 + 0xc, 'I', 0)
    regions[coaster.CARS] = cars
    return regions, coaster.COAST, coaster.CARS


def flash(r, regions, c):
    """Coaster_FlashCar's cars: the old one (+0x144) none or OLDCAR (its ride not flashing), +0x54..+0x64 none or FCAR (flashing)."""
    put(c, 0x3c, 'I', r.getrandbits(32) & ~6 | (4 if r.random() < 0.85 else 0) | r.choice([0, 2]))
    for off in (0x54, 0x58, 0x60, 0x64):
        put(c, off, 'I', r.choice([FCAR, FCAR, 0]))
    put(c, 0x144, 'I', r.choice([0, OLDCAR]))
    table = bytes(regions[0x7a4610])[:16]
    table += bytes(16 - len(table)) + struct.pack('<2I', FRIDE, ORIDE)
    regions.update({0x7a4610: table, FCAR: put(bytearray(r.bytes(0x80)), 0x70, 'I', 4), OLDCAR: put(bytearray(r.bytes(0x80)), 0x70, 'I', 5),
                    FRIDE: put(bytearray(r.bytes(8)), 4, 'I', r.getrandbits(32) | 0x4000000),
                    ORIDE: put(bytearray(r.bytes(8)), 4, 'I', r.getrandbits(32) & ~0x4000000)})


def at_case(r, op):
    """ModeAt / StepAt: Unload's world, the coaster at its index of the table 0x790bd0."""
    regions, co, cars = unload_world(r)
    c = bytearray(regions[co])
    flash(r, regions, c)
    put(c, 0xf4, 'I', r.choice([1, 1, 2, 0]))
    put(c, 0x128, 'I', cars)
    put(c, 8, 'I', r.getrandbits(32) & ~0x7f | r.choice(OLD_MODES) | r.choice([1, 2, 4, 8, r.randrange(16)]))
    put(c, 0x2c, 'I', r.getrandbits(32))
    regions[co] = c
    table = bytes(regions[0x790bd0])   # (the Unload world's table: the coaster at an index of it)
    index = next(k for k in range(4) if struct.unpack_from('<I', table, 4 * k)[0] == co)
    if op == 'ModeAt':
        return Call(0x43b270, [index, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, ret='v')
    return Call(0x43b2f0, [index, r.choice([0, 1, 1, 0, r.getrandbits(32)])], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op in ('ModeAt', 'StepAt'):
        return at_case(r, op)
    kind = r.deal(['Wake', 'Sleep', 'Unload', 'Advance', 'Other', 'Mode']) if op == 'Set' else r.deal(['Wake', 'Unload', 'Other'])
    if kind == 'Wake':
        old, new = 1, 2
    elif kind == 'Sleep':
        old, new = 2, 1
    elif kind == 'Unload':
        old, new = r.choice([4, 8]), 2
    elif kind == 'Advance':
        old, new = r.choice([1, 2, 4, 8]), 8
    elif kind == 'Mode':
        old, new = r.randrange(16), 0
    else:
        old, new = r.randrange(16), r.randrange(16)
    if op == 'Resume':
        new = 2
        if kind == 'Other':
            old = r.choice([0, 2, 3, 5, 9, 15])
    if kind in ('Wake', 'Sleep'):
        regions, co, cars = wake_world(r, kind == 'Sleep')
    else:
        regions, co, cars = unload_world(r)
    c = bytearray(regions[co])
    flash(r, regions, c)
    count = r.choice([1, 1, 2, 0])
    if kind == 'Other' and (old, new) in ((1, 2), (2, 1)):
        count = 0
    put(c, 0xf4, 'I', count)
    put(c, 0x128, 'I', cars)
    put(c, 8, 'I', r.getrandbits(32) & ~0x7f | r.choice(OLD_MODES) | old)
    put(c, 0x2c, 'I', r.getrandbits(32))
    if op == 'Resume':
        put(c, 0x60, 'I', PIECE)
        put(c, 0x64, 'I', PIECE + 0x10)
        pieces = bytearray(r.bytes(0x20))
        for k in range(2):
            put(pieces, 0x10 * k + 8, 'I', r.getrandbits(32) & ~0xf0400 | r.choice([0, 0x400]) | r.choice([0, 0x10000, 0x40000, 0x50000])
                | r.choice([0, 0x20000, 0x80000, 0xa0000]))
        regions[PIECE] = pieces
        regions[co] = c
        return Call(0x442c70, [co], regions, ret='v')
    regions[co] = c
    value = new | (r.choice(NEW_MODES + [r.getrandbits(32) & 0x70]) if kind != 'Wake' or r.random() < 0.5 else 0)
    if r.random() < 0.3:
        value |= r.getrandbits(32) & ~0x7f
    return Call(0x435730, [co, value], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Set,Set,Resume,ModeAt,StepAt').split(',') if k]

if __name__ == '__main__':
    B.main(case)
