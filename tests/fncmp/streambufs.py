# The wide streambuf block read and write of src/game/core/streambufs (0x4f0700, 0x4f07c0) against the originals: get and put areas (pointer null or not, the room word with
# its two top bits set or clear, smaller or larger than the request), and an underflow / overflow slot that is a recorder returning anything, sometimes 0xffff.
# The pointer setup and setg / setp (0x4f6020, 0x4f6820, 0x4f6850): any object bytes, the pointers to their own words, to one or to the object.
# usage: streambufs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Read,Write,Grow,NRead,NWrite,InitBuf,Setg,Setp)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Read': 0x4f0700, 'Write': 0x4f07c0, 'Grow': 0x4f6740, 'NRead': 0x5ceaf0, 'NWrite': 0x5ceba0, 'InitBuf': 0x4f6020, 'Setg': 0x4f6820,
         'Setp': 0x4f6850}
B = Bench([(CALLS[k], CALLS[k] + 0xc0) for k in ('Read', 'Write', 'Grow', 'NRead', 'NWrite')] + [(0x67acb0, 0x67acb1), (0x4f6020, 0x4f6080),
                                                                                              (0x4f6820, 0x4f6850), (0x4f6850, 0x4f6880)])
OBJ, VT, GP, CNT, PP, PC, GET, PUT, DST, SRC = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x440, RAM + 0x480, RAM + 0x4c0, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000


def room(r):
    return r.choice([0, 1, 2, 3, 5, 0x20, 0x40, r.randrange(0x40), 0x40000002, 0x80000005, 0xc0000003, 0xffffffff, 0x40])


def case(r):
    name = r.choice(OPS)
    if name == 'InitBuf':  # any object bytes
        return Call(CALLS[name], [OBJ + r.choice([0, 0x20])], {OBJ: r.bytes(0x80)}, conv='t', ret='a')
    if name in ('Setg', 'Setp'):
        obj = bytearray(r.bytes(0x400))
        slots = (0xc, 0x1c, 0x2c) if name == 'Setg' else (0x10, 0x20, 0x30)
        for i, off in enumerate(slots):  # the pointers go to their own words, or all to one, or the object itself
            put(obj, off, 'I', r.choice([OBJ + 0x200 + 4 * i, OBJ + 0x200 + 4 * i, OBJ + 0x200, OBJ + off]))
        a = r.choice([0, 1, 100, 0x7fffffff, r.getrandbits(32)])
        b = r.choice([0, 2, 100, 0x80000000, r.getrandbits(32)])
        c = r.choice([0, 8, 0x1000, r.getrandbits(32)])
        return Call(CALLS[name], [OBJ, a, b, c] if name == 'Setg' else [OBJ, a, b], {OBJ: obj}, conv='t', ret='a')
    if name == 'Grow':
        a = r.randrange(4)
        b = a + r.choice([0, 1, 2, 3])
        obj = bytearray(r.bytes(0x40))
        put(obj, 0x10, 'I', GET + 4 * a)
        put(obj, 0x20, 'I', GET + 4 * b - 4)
        put(obj, 0x24, 'I', r.choice([0, HEAP + 0x40, HEAP + 0x100]))
        return Call(CALLS[name], [OBJ, r.choice([4, 8, 0x10, 0x13, 0x40, 0x41])], {OBJ: obj, GET: r.bytes(0x40), HEAP: heap()}, conv='t', ret='a')
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
    slot = 5 if name in ('Read', 'NRead') else 1
    if r.random() < 0.5:
        set_return(log, slot, r.choice([0xffff, 0x41, 0x12345678, 0xfffe, 0x1ffff, 0xffffffff, 0xff]))
    count = r.choice([0, 1, 2, 3, 5, 8, 0x10, -1, 0x30])
    return Call(CALLS[name], [OBJ, DST if name in ('Read', 'NRead') else SRC, count & 0xffffffff], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
