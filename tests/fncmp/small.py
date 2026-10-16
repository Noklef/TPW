# The small functions of src/game/core/small against the originals: any object bytes and
# arguments (no division by zero), dword ranges empty or not.
# usage: small.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Word,Div,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5b3210, 0x5b3223), (0x617920, 0x617927), (0x617ad0, 0x617ad7), (0x6185f0, 0x6185f7), (0x631270, 0x631271),
           (0x631970, 0x631974), (0x6322e0, 0x632306), (0x6338b0, 0x6338c0), (0x633a00, 0x633a2e), (0x6339e0, 0x633a00),
           (0x633b60, 0x633b68), (0x6339a0, 0x6339d5), (0x6344c0, 0x6344e4), (0x6404d0, 0x6404ef), (0x631280, 0x6312cc), (0x6428b0, 0x6428c0), (0x4f76b0, 0x4f7718), (0x48f650, 0x48f68c), (0x50f240, 0x50f273), (0x642100, 0x642106), (0x642b20, 0x642b26)])
# name: (address, convention, argument shape, returns); shape letters: o object, p pointer,
# d any dword, D nonzero dword, W destination word (or 0) then source word, R dword range then dest
CALLS = {'Word': (0x5b3210, 'c', 'W', 'v'), 'V748': (0x617920, 't', 'o', 'v'), 'V770': (0x617ad0, 't', 'o', 'v'),
         'G64': (0x6185f0, 't', 'o', 'a'), 'Ret2': (0x631270, 'c', '', 'v'), 'G3c': (0x631970, 't', 'o', 'a'),
         'Copy': (0x6322e0, 'c', 'R', 'a'), 'C770': (0x6338b0, 't', 'o', 'a'), 'C80': (0x633a00, 't', 'o', 'a'),
         'Take': (0x6339e0, 't', 'op', 'a'), 'Set10': (0x633b60, 't', 'od', 'a'), 'Div': (0x6428b0, 'c', 'dD', 'a'), 'Words': (0x6339a0, 't', 'op', 'a'), 'Copy4': (0x6344c0, 't', 'oq', 'a'),
         'Rev': (0x6404d0, 'c', 'dL', 'a'), 'WCmp': (0x4f76b0, 't', 'S', 'a'), 'Pair2': (0x48f650, 't', 'T', 'a'), 'Stack': (0x50f240, 'c', 'K', 'v'), 'Init': (0x631280, 'c', 'o', 'a'),
         'K1': (0x642100, 'c', '', 'a'), 'K2': (0x642b20, 'c', '', 'a')}


def case(r):
    address, conv, shape, ret = CALLS[r.choice(OPS)]
    regions = {RAM: r.bytes(0x200), RAM + 0x1000: r.bytes(0x200)}
    args = []
    if shape == 'R':
        a = r.randrange(0, 0x40) * 4
        b = a + r.choice([0, 4, 8, 0x20, 0x40])
        args = [RAM + a, RAM + b, RAM + 0x1000 + r.choice([0, 4, 0x40]) if r.random() < 0.8 else RAM + a + 4]
    elif shape == 'T':
        obj = bytearray(r.bytes(0x100))
        obj[0x44:0x46] = bytes([r.choice([0, 1, 0xff]), 0])
        regions = {RAM: obj, RAM + 0x1000: r.bytes(0x20)}
        args = [RAM, r.choice([RAM + 0x1000, RAM + 0x1008, 0]), r.choice([RAM + 0x1004, RAM + 0x1000, 0])]
    elif shape == 'K':
        import struct
        base = RAM + 0x10
        top = base + r.choice([-8, 0, 4, 8, 16, 24, 0x38, 0x40])
        regions = {RAM: r.bytes(0x100), 0x7cf070: bytearray(r.bytes(8)) + struct.pack('<II', base, top) + bytes(8)}
    elif shape == 'S':
        base = [r.choice([0x41, 0x42, 0x100, 0x8000, 0xffff, 0]) for _ in range(6)]
        a, b = list(base), list(base)
        for _ in range(r.choice([0, 0, 1, 2])):
            b[r.randrange(6)] = r.choice([0x41, 0x42, 0x7fff, 0x8000, 0xffff])
        la, lb = r.randrange(7), r.randrange(7)
        import struct
        sa, sb = struct.pack('<6H', *a), struct.pack('<6H', *b)
        obj = bytearray(0x20)
        struct.pack_into('<II', obj, 4, RAM + 0x100 if r.random() < 0.95 else 0, la)
        struct.pack_into('<II', obj, 0x14, RAM + 0x120 if r.random() < 0.9 else 0, lb)
        regions = {RAM: bytes(obj), RAM + 0x100: sa + bytes(0x14), RAM + 0x120: sb + bytes(0x14), 0x6fd6c0: bytes(0x20)}
        args = [RAM, RAM + 0x10]
    elif shape == 'W':
        args = [r.choice([0, RAM + 0x10, RAM + 0x11]), RAM + 0x20 + r.choice([0, 1])]
    else:
        for ch in shape:
            if ch == 'o':
                args.append(RAM + r.choice([0, 0x10]))
            elif ch == 'q':
                args.append(r.choice([0, RAM + 0x100, RAM + 0x104]))
            elif ch == 'L':
                args.append(r.choice([0, 1, 2, 3, 8, 15, 16, 31, 0xffffffff]) & 0xffffffff)
            elif ch == 'p':
                args.append(RAM + 0x100 + r.choice([0, 4]))
            elif ch == 'd':
                args.append(r.choice([0, 1, 7, 0x7fffffff, 0x80000000, 0xfffffff0, r.getrandbits(32)]))
            else:
                args.append(r.choice([1, 2, 3, 7, 0x10, 0xfffffffe, 0x7fffffff, r.randrange(1, 100)]))
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
