# A coaster car's looped sounds kept up with it (src/game/coaster/carsound: CoasterCar_UpdateSounds 0x439570) against the original: Camera_Followed (k 1), Sound_SetParam (k 2), Sound_SetPosition (k 3) and
# CoasterCar_UpdateScream (k 4) recorders. Followed or not, silenced or not (the place's arguments then logged as 0: the original passes stack garbage), a car in +0x10 or none.
# usage: carsoundupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x439570, 0x439667), (0x67a400, 0x690000)],
          hooks={0x42a4e0: _recorder(1, 0, cdecl=True), 0x51bc40: _recorder(2, 3, cdecl=True), 0x51c270: _recorder(3, 4, cdecl=True), 0x4393e0: _recorder(4, 5, cdecl=True)})
LINK, TYPE, CAR, THING, TABLE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def case(r):
    log = log_region()
    thing = r.getrandbits(32) | 1
    set_return(log, 1, r.choice([thing, thing, r.getrandbits(32)]))
    answers(log, 2, TABLE)
    answers(log, 3, TABLE + 0x100)
    silent = r.random() < 0.3
    if silent:
        strings(log, 4, 0, out=0b11100)
    car = bytearray(r.bytes(0x80))
    put(car, 0x10, 'I', r.choice([0, r.getrandbits(32)]))
    put(car, 0x34, 'I', THING)
    for off in (0x3c, 0x40, 0x44, 0x48, 0x4c, 0x50):
        put(car, off, 'I', r.f())
    put(car, 0x78, 'I', r.f())
    regions = {LOG: log, LINK: put(bytearray(r.bytes(8)), 4, 'I', TYPE), TYPE: put(bytearray(r.bytes(0x60)), 0x50, 'I', r.f()), CAR: car, THING: put(bytearray(r.bytes(0x10)), 0xc, 'I', thing),
               TABLE: r.bytes(0x200), 0x785a2c: put(bytearray(4), 0, 'I', r.getrandbits(32) | 1 if silent else 0)}
    return Call(0x439570, [LINK, CAR], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
