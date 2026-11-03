# The panel 0x7ca178's figures (src/game/ui/panel7ca178: Panel7ca178_ShowStats, 0x4973a0) against the original. The window +0x14 (none now and then) has the children 0x15bbd and 0x15bbb (each missing now and
# then) and 0x15bc0 (not checked); the window +0x10 has 0x15bc4 and 0x15bb4; every window's slots 5 and 7 recorders 21 and 20. The shown thing [0x7c2658]: its ring +0x1ac (head +0x224, count +0x228, wrapped
# +0x22c), its float +0x44 (0-100 mostly; NaN, huge and negative now and then) and +0x68. The panel's vtable slot 9 recorder 22, its +8 the status or not. Object_Status cdecl recorder 23, Object_CanOpen,
# Object_ScrapRefund and Window705a50_SetState thiscall recorders 24-26 (their own benches have them; driver.cpp's kPanel7ca178StatsEnv).
# usage: panel7ca178stats.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import window  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench(window.RANGES + [(0x4973a0, 0x4975c0), (0x4dd4e0, 0x4dd500), (0x412e90, 0x412f00), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860), (0x4e0440, 0x4e0450)],
          hooks={0x485f60: _recorder(23, 2, cdecl=True), 0x4df290: _recorder(24, 0), 0x4e2400: _recorder(25, 0), 0x668bec: _recorder(26, 2)})
PANEL, W14, W10, KIDS, THING, DEF, VT, PANEL_VT = RAM + 0x40000, RAM + 0x40100, RAM + 0x40300, RAM + 0x41000, RAM + 0x42000, RAM + 0x48000, RAM + 0x40800, RAM + 0x40900
KID = 0x120
FLOATS = [0.0, 1.0, 24.5, 25.0, 50.0, 99.9, 100.0, 255.5, 256.0, 1e6, -1.0, -300.5, 1e30, float('nan'), float('inf')]


def children(r, regions, parent, base, wanted):
    ids = list(wanted) + [r.choice([0x15bb0, 0x15bbe, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1]))]
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        kid = bytearray(r.bytes(KID))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', base + KID * (i + 1) if i + 1 < len(ids) else 0)
        regions[base + KID * i] = kid
    regions[parent] = put(bytearray(r.bytes(KID)), 0x5c, 'I', base if ids else 0)


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 26, 0b11)   # (the state and the notify are BYTEs)
    regions = {LOG: log, VT: put(put(vtable({}, size=8), 0x1c, 'I', recorder(20, 1)), 0x14, 'I', recorder(21, 1)),
               PANEL_VT: put(vtable({}, size=10), 0x24, 'I', recorder(22, 0))}
    children(r, regions, W14, KIDS, [k for k in (0x15bbd, 0x15bbb) if r.random() < 0.9] + [0x15bc0])
    children(r, regions, W10, KIDS + 0x800, [0x15bc4, 0x15bb4])
    ident = r.getrandbits(16)
    thing = bytearray(r.bytes(0x300))
    put(thing, 0xe, 'H', ident)
    put(thing, 0x44, 'f', r.choice([r.uniform(0, 100)] * 8 + FLOATS))
    put(thing, 0x68, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    for k in range(30):
        put(thing, 0x1ac + 4 * k, 'I', r.choice([0, 1, r.randrange(1000), r.getrandbits(32)]))
    put(thing, 0x224, 'i', r.randrange(30))
    put(thing, 0x228, 'i', r.choice([0, 1, r.randrange(30), r.randrange(30), 29, 30, 30, 31, r.randrange(30, 40)]))
    thing[0x22c] = r.choice([0, 0, 1, 1, r.getrandbits(8)])
    regions[THING] = thing
    regions[0x7c2658] = struct.pack('<I', THING)
    regions[0x7897d0] = struct.pack('<HHI', ident, r.getrandbits(16), DEF)
    regions[DEF] = r.bytes(0x100)
    status = r.choice([0xb, 0xc, 0xd, 0xe, 0xf, 0xa, 0x10, 0, r.randrange(32), r.getrandbits(32)])
    set_return(log, 23, status)
    set_return(log, 24, r.choice([0, 1, 1, 0x100, r.getrandbits(32)]))
    set_return(log, 25, r.getrandbits(32))
    panel = bytearray(r.bytes(0x40))
    put(panel, 0, 'I', PANEL_VT)
    put(panel, 0x14, 'I', W14 if r.random() < 0.9 else 0)
    put(panel, 0x10, 'I', W10)
    put(panel, 8, 'I', status if r.random() < 0.3 else r.getrandbits(32))
    regions[PANEL] = panel
    return Call(0x4973a0, [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
