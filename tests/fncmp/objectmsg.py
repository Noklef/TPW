# The object's day message of src/game/world/objectmsg (0x4dd320) against the original: six history rings (lengths from none to 30, indexes at or past them, temp values),
# and messages of number 0xb, 0x1b (the staff member word equal to the object's or not) and others (the number comes from the message's slot 0, a recorder).
# usage: objectmsg.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from thingenv import POOL, THING, park  # noqa: E402
from fncmp import Bench, Call, LOG, RAM, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4dd320, 0x4dd4e0), (0x4ddb90, 0x4ddd20), (0x4ddf50, 0x4de040), (0x502430, 0x502460), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1)])
OBJ, MSG, VT = RAM, RAM + 0x1000, RAM + 0x1100
BASES = (0x1a8, 0x230, 0x2b8, 0x70, 0xf8, 0x340)


def queue(r, op):
    count = r.choice([1, 2, 3, 4, 6])
    regions, handles, things = park(r, count=count, kinds=(1,), defs=False)
    pool = bytearray(regions[POOL])
    order = r.sample(range(count), r.randrange(0, count + 1))
    for k, i in enumerate(order):
        put(pool, THING * i + 0x228, 'HH', handles[order[k + 1]] if k + 1 < len(order) else 0, handles[order[k - 1]] if k else 0)
        put(pool, THING * i + 0x220, 'i', r.choice([11, 12, 14, 8, 3, 15]) if r.random() < 0.9 else 3)
        put(pool, THING * i + 0x224, 'i', r.choice([11, 13, 5, 14]))
    obj = bytearray(r.bytes(0x100))
    put(obj, 0x3c, 'H', handles[order[0]] if order else 0)
    regions[POOL] = pool
    regions[OBJ] = obj
    regions[RAM + 0x1da000] = r.bytes(0x1000)
    if op == 'Add':
        rest = [i for i in range(count) if i not in order] or [r.randrange(count)]
        person = handles[r.choice(rest)]
        return Call(0x4ddb90, [OBJ, person | r.choice([0, 0, 0x10000])], regions, conv='t', ret='v')
    if order:
        person = handles[r.choice(order)]
        # every link must stay valid for the walk: the state predicate may end it early (then the person is not found: avoided)
        for i in order:
            put(regions[POOL], THING * i + 0x220, 'i', 11)
    else:
        person = r.choice([0, handles[0]])
    return Call(0x4ddf50, [OBJ, person | r.choice([0, 0, 0x10000])], regions, conv='t', ret='a')


def case(r):
    if r.random() < 0.7:
        return queue(r, r.choice(['Add', 'Pos']))
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
