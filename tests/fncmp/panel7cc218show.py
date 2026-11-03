# The game panel's ride settings shown (src/game/ui/panel7cc218: Panel7cc218_ShowRide, 0x4af030) against the original (panel7cc218.py and panel7cc218settings.py bench the module's others). The
# panel's window +0x10 (none now and then) has the children 0x3e30, 0x3e2f, 0x3e2d (each missing now and then) and 0x3e31 among decoys; 0x3e30 and 0x3e2d have a child 0x3e2e (missing rarely: a fault
# in both); each window's slot 7 the recorder 20. The thing [0x7c2658] and its definition through ObjectDefs_Find's cache 0x7897d0 (panel7cc218settings.py's): its ranges +0x134 / +0x138, +0x12c / +0x130,
# +0x124 / +0x128 mostly apart (equal: +0x124 hides, the others divide by 0 in both), the level +0x50 0-5 and its tables. Window705b20_SetRange, Window_SetShown and Panel7cc218_ShowSettings thiscall
# recorders 21-23 (their own benches have them; driver.cpp's kPanel7cc218ShowEnv).
# usage: panel7cc218show.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import window  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, vtable  # noqa: E402

B = Bench(window.RANGES + [(0x4af030, 0x4af1f4), (0x4dd4e0, 0x4dd500), (0x412e90, 0x412f00), (0x5da3c0, 0x5da3c1)],
          hooks={0x66b47d: _recorder(21, 2), 0x65d9dd: _recorder(22, 1), 0x4aec30: _recorder(23, 1)})
PANEL, ROOT, KIDS, THING, DEF, VT = RAM + 0x40000, RAM + 0x40100, RAM + 0x41000, RAM + 0x40800, RAM + 0x48000, RAM + 0x40900
KID = 0x120


def chain(r, regions, base, wanted):
    """windows of the ids (those not dropped) among decoys, linked by +0x54: returns the first or 0"""
    ids = [ident for ident in wanted] + [r.choice([0x3e2c, 0x3e34, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1]))]
    r.shuffle(ids)
    at = {}
    for i, ident in enumerate(ids):
        kid = bytearray(r.bytes(KID))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', base + KID * (i + 1) if i + 1 < len(ids) else 0)
        put(kid, 0x5c, 'I', 0)
        regions[base + KID * i] = kid
        at[ident] = base + KID * i
    return (base if ids else 0), at


def case(r):
    log = log_region()
    regions = {LOG: log, VT: put(vtable({}, size=8), 0x1c, 'I', recorder(20, 1))}
    wanted = [ident for ident in (0x3e30, 0x3e2f, 0x3e2d, 0x3e31) if r.random() < 0.9]
    first, at = chain(r, regions, KIDS, wanted)
    for k, ident in enumerate((0x3e30, 0x3e2d)):
        if ident in at:
            inner, _ = chain(r, regions, KIDS + 0x1000 * (k + 1), [0x3e2e] if r.random() < 0.97 else [])
            put(regions[at[ident]], 0x5c, 'I', inner)
    regions[ROOT] = put(bytearray(r.bytes(KID)), 0x5c, 'I', first)
    ident = r.getrandbits(16)
    thing = bytearray(r.bytes(0x60))
    put(thing, 0xe, 'H', ident)
    thing[0x50] = r.choice([0, 1, 2, 3, 4, 5])
    regions[THING] = thing
    regions[0x7c2658] = struct.pack('<I', THING)
    regions[0x7897d0] = struct.pack('<HHI', ident, r.getrandbits(16), DEF)
    d = bytearray(r.bytes(0x400))

    def values():
        low = r.choice([0, 1, 5, 10, r.randrange(-100, 100), r.getrandbits(31)])
        return low, r.choice([low + r.randrange(1, 200), low + r.randrange(1, 200), low - 1, r.getrandbits(32) - (1 << 31)]) & 0xffffffff

    put(d, 0x134, 'II', *[v & 0xffffffff for v in values()])
    put(d, 0x12c, 'II', *[v & 0xffffffff for v in values()])
    low, high = values()
    put(d, 0x124, 'II', low & 0xffffffff, r.choice([low & 0xffffffff, high, high, high]))
    put(d, 0x70, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    for k in range(6):
        put(d, 0x19c + (k << 6), 'i', r.choice([0, 1, 100, -100, r.randrange(1 << 15)]))
        put(d, 0x1ac + (k << 6), 'i', r.choice([0, 1, 100, -100, r.randrange(1 << 15)]))
    regions[DEF] = d
    regions[PANEL] = put(bytearray(r.bytes(0x40)), 0x10, 'I', ROOT if r.random() < 0.95 else 0)
    return Call(0x4af030, [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
