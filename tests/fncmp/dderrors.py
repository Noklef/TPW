# The video library's DirectDraw error names (src/game/gfx/dderrors: 0x672e30) against the original: the code one of the table's (read
# from the exe) half the time, else 0x887600xx-0x887602xx or any.
# usage: dderrors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import x87emu  # noqa: E402
from fncmp import Bench, Call  # noqa: E402

CALLS = {'Name': 0x672e30}
B = Bench([(0x672e30, 0x672e54)])


def exe_bytes(va, n):
    for base, data in x87emu.exe_sections():
        if base <= va < base + len(data):
            return bytes(data[va - base:va - base + n])
    raise KeyError(va)


CODES = struct.unpack('<184I',exe_bytes(0x705ee8, 0x2e0))[::2]


def case(r):
    op = r.deal(OPS)
    code = r.choice(CODES) if r.random() < 0.5 else 0x88760000 + r.randrange(0x300) if r.random() < 0.8 else r.getrandbits(32)
    return Call(CALLS[op], [code], {}, conv='f', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
