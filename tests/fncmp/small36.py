# The wide streambuf block read and write of src/game/core/small36 (0x4f0700, 0x4f07c0) against the originals: get and put areas (pointer null or not, the room word with
# its two top bits set or clear, smaller or larger than the request), and an underflow / overflow slot that is a recorder returning anything, sometimes 0xffff.
# usage: small36.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Read,Write)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Read': 0x4f0700, 'Write': 0x4f07c0}
B = Bench([(a, a + 0xc0) for a in CALLS.values()])
OBJ, VT, GP, CNT, PP, PC, GET, PUT, DST, SRC = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x440, RAM + 0x480, RAM + 0x4c0, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000


def room(r):
    return r.choice([0, 1, 2, 3, 5, 0x20, 0x40, r.randrange(0x40), 0x40000002, 0x80000005, 0xc0000003, 0xffffffff, 0x40])


def case(r):
    name = r.choice(OPS)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0, 'I', VT)
    put(obj, 0x1c, 'I', GP)
    put(obj, 0x2c, 'I', CNT)
    put(obj, 0x20, 'I', PP)
    put(obj, 0x30, 'I', PC)
    log = log_region()
    regions = {OBJ: obj, VT: vtable({1: 1, 5: 0}), LOG: log, GP: put(bytearray(4), 0, 'I', GET + 2 * r.randrange(4) if r.random() < 0.85 else 0), CNT: put(bytearray(4), 0, 'I', room(r)),
               PP: put(bytearray(4), 0, 'I', PUT + 2 * r.randrange(4) if r.random() < 0.85 else 0), PC: put(bytearray(4), 0, 'I', room(r)),
               GET: r.bytes(0x100), PUT: r.bytes(0x100), DST: r.bytes(0x100), SRC: r.bytes(0x100)}
    slot = 5 if name == 'Read' else 1
    if r.random() < 0.5:
        set_return(log, slot, r.choice([0xffff, 0x41, 0x12345678, 0xfffe, 0x1ffff]))
    count = r.choice([0, 1, 2, 3, 5, 8, 0x10, -1, 0x30])
    return Call(CALLS[name], [OBJ, DST if name == 'Read' else SRC, count & 0xffffffff], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
