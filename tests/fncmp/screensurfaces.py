# The screen surfaces source (src/game/gfx/screensurfaces: 0x47e2f0, 0x47e390, 0x47e570) against
# the original: the constructor (through SurfaceInfo_Ctor) and the base destructor on random
# bytes, and the bounds from any screen size; the surfaces wrapped on first use (0x47e4b0 / 0x47e510): done or not, the
# display's surface a COM recorder whose QueryInterface leaves an interface (a recorder too), none, or what was there, and
# whose GetSurfaceDesc (its buffer logged as 0) fills in a size and 8 or 16 bits.
# usage: screensurfaces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, fill, log_region, strings, vtable  # noqa: E402

B = Bench([(0x47e2f0, 0x47e385), (0x47e390, 0x47e397), (0x47e570, 0x47e59e), (0x5fa680, 0x5fa697), (0x47e4b0, 0x47e580),
           (0x5f8f90, 0x5f9050)])
CALLS = ('Ctor', 'Dtor', 'Bounds', 'Surface10', 'Surface14')
DISPLAY, SURF, VT = RAM + 0x100, RAM + 0x140, RAM + 0x180


def wrap(r, regs, op):
    done, iface = (0xa8, 0xac) if op == 'Surface10' else (0xa4, 0xb0)
    o = regs[RAM] = bytearray(regs[RAM])
    struct.pack_into('<I', o, done, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    struct.pack_into('<I', o, iface, r.choice([0, SURF]))
    log = log_region()
    strings(log, 22, 0, 2)
    if r.random() < 0.8:
        fill(log, 0, 2, struct.pack('<I', r.choice([SURF, SURF, 0])))
    desc = bytearray(r.bytes(0x6c))
    struct.pack_into('<3I', desc, 4, 0x100f, r.choice([480, 600, 0, r.getrandbits(32)]), r.choice([640, 800, 1, r.getrandbits(32)]))
    struct.pack_into('<I', desc, 0x54, r.choice([8, 8, 16, 16, 32, r.getrandbits(32)]))
    fill(log, 22, 1, bytes(desc), 1)
    regs.update({0x879a24: struct.pack('<I', DISPLAY), DISPLAY: r.bytes(0x10) + struct.pack('<2I', SURF, SURF) + r.bytes(0x8),
                 SURF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 24, com={0: 3, 22: 2}), LOG: log})


def case(r):
    op = r.choice(OPS)
    regs = {RAM: r.bytes(0xc0)}
    if op.startswith('Surface'):
        wrap(r, regs, op)
        return Call(0x47e4b0 if op == 'Surface10' else 0x47e510, [RAM], regs, conv='t', ret='a')
    if op == 'Ctor':
        return Call(0x47e2f0, [RAM], regs, conv='t', ret='a')
    if op == 'Dtor':
        return Call(0x47e390, [RAM], regs, conv='t')
    size = lambda: r.choice([640, 800, 1024, 0, 1, r.getrandbits(32)]).to_bytes(4, 'little')  # noqa: E731
    regs.update({0x7a1968: size(), 0x7a1930: size()})
    return Call(0x47e570, [RAM, RAM + r.choice([0, 0x10, 0x80])], regs, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
