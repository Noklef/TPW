# The object's day message of src/game/world/objectmsg (0x4dd320) against the original: six history rings (lengths from none to 30, indexes at or past them, temp values),
# and messages of number 0xb, 0x1b (the staff member word equal to the object's or not) and others (the number comes from the message's slot 0, a recorder).
# usage: objectmsg.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4dd320, 0x4dd4e0), (0x5da3c0, 0x5da3c1)])
OBJ, MSG, VT = RAM, RAM + 0x1000, RAM + 0x1100
BASES = (0x1a8, 0x230, 0x2b8, 0x70, 0xf8, 0x340)


def case(r):
    obj = bytearray(r.bytes(0x400))
    for base in BASES:
        n = r.choice([0, 1, 2, 3, 5, 0x1e, -1 & 0xffffffff])
        put(obj, base + 0x80, 'I', n)
        put(obj, base + 0x7c, 'I', r.choice([0, 1, 2, 4, 0x1d, 0x1e, 0x7fffffff, 0xffffffff, r.randrange(0x1e)]))
        put(obj, base + 0x84, 'B', r.choice([0, 1, 5]))
    kind = r.choice([0xb, 0xb, 0x1b, 0x1b, 0, 1, 0x1c, 0xffffffff])
    msg = put(bytearray(r.bytes(0x20)), 0, 'I', VT)
    if kind == 0x1b and r.random() < 0.7:
        put(msg, 8, 'H', obj[0x5e] | obj[0x5f] << 8)
    log = log_region()
    set_return(log, 0, kind)
    return Call(0x4dd320, [OBJ, MSG], {OBJ: obj, MSG: msg, VT: vtable({0: 0}), LOG: log}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
