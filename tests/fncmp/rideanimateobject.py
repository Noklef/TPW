# An object's ride animated (src/game/coaster/rideanimate: 0x463030 Object_AnimateRide) against the original: Ride_AnimateOne a cdecl recorder (k 5).
# usage: rideanimateobject.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x463030, 0x463057)], hooks={0x44e380: _recorder(5, 2, cdecl=True)})
OBJECT, RIDE = RAM, RAM + 0x200


def case(r):
    obj = bytearray(r.bytes(0xb8))
    put(obj, 0xb4, 'I', r.choice([0, RIDE, RIDE, RIDE]))
    return Call(0x463030, [OBJECT], {LOG: log_region(), OBJECT: obj, RIDE: r.bytes(0x10)}, ret='v')


if __name__ == '__main__':
    B.main(case)
