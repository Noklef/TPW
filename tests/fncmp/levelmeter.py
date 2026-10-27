# The level meter's functions (src/game/ui/levelmeter: 0x4a95b0, 0x4a9fc0, 0x4a9c70) against the originals. The window has random bytes with its
# dwords +0xa0: the position (16.16, often near the phases' limits 0x1f / 0x2f / 0x3f), the clock, the speed (small, negative or any),
# and the phase (0-3 mostly). Its children (+0x5c, then +0x54) are the light 0xd2d06 (now and then missing: a fault in both once it is told)
# among 0-2 decoys. The light's objects +0x98 / +0x9c are recorders of slot 3 (k 3 and 22, their event's word +4 hashed) or 0. Its
# default handler +0x118 is a cdecl recorder. The GUI clock [0x77c490]'s slot 0 is a recorder with a random result.
# SetLevel: the level 0 a third of the time, else small or any. OnMessage: the message 0x1e mostly.
# Research (0x4a9c70): the group -1 mostly, 0-4, now and then 5-6; the research panel [0x7cc1d8] whose children are the five meters (in any
# order; one missing now and then: a fault in both), each a SetLevel window with its own light (slot 3 of +0x98 / +0x9c the recorder 23,
# its event hashed); the game [0x7cf83c] (also [0x80239c]) whose word +0x1da722 picks the lab at 0x7cfb90: its groups' current projects
# (+0x140c + 16 * group, +8; groups 0-6) -1 or 0-3, their ids (+0xc + 20 * current) 0 or not, their dwords +4 (Table_ScaledBySum's) small or any;
# the chain of 0-3 things from the game's +0x1da742 (their +0x1e4 0-5, linked by +0x210), the globals 0x7852f8 / 0x78548c 0 or not; the
# sound globals on / started or not, the manager's engine's slot 3 the recorder 3 of three.
# Progress (0x4a9bd0): the group 0-4 mostly, now and then 5, 6 or -1; the lab as research.py's (its projects' costs and done any floats,
# the groups' current projects -1 or any, the dearest +0x1464 any or the current), picked as Research's; the research panel's five meters (one
# missing now and then: a fault in both), each with its gauge 0xd2d05 (missing now and then: a fault in both) among 0-2 decoys, the gauge's
# slot 7 the recorder 7.
# usage: levelmeter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLevel,OnMessage,Research,Progress)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'SetLevel': 0x4a95b0, 'OnMessage': 0x4a9fc0, 'Research': 0x4a9c70, 'Progress': 0x4a9bd0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x4a95b0, 0x4a96af), (0x4a9fc0, 0x4aa0b1), (0x65e6d5, 0x65e73d), (0x65d3a3, 0x65d40e), (0x65d7bc, 0x65d832),
           (0x65f66e, 0x65f697), (0x65968e, 0x6596a2), (0x4a9c70, 0x4a9f84), (0x519550, 0x51958d), (0x5039f0, 0x503a20), (0x503980, 0x5039f0),
           (0x4c9e60, 0x4c9ed0), (0x4c9ed0, 0x4c9ef0), (0x519510, 0x519550), (0x51bc40, 0x51bc70), (0x6b5b80, 0x6b5b9a), (0x5da3c0, 0x5da3c1),
           (0x4a9bd0, 0x4a9c68), (0x503ac0, 0x503b26)])
WINDOW, KIDS, OBJ98, OBJ9C, VT98, VT9C, CLOCK, CLOCKVT = RAM, RAM + 0x400, RAM + 0x1000, RAM + 0x1040, RAM + 0x1080, RAM + 0x1100, RAM + 0x1200, RAM + 0x1240
KID = 0x120
LIGHT = 0xd2d06
METERS = [0xd2d01, 0xd2d07, 0xd2d08, 0xd2d09, 0xd2d0a]
PANEL, MWINS, LIGHTS, LAB, GAME, THINGS = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x6000, RAM + 0x100000, RAM + 0x8000
SND, SVT, ENGINE, EVT = RAM + 0x1300, RAM + 0x1340, RAM + 0x1380, RAM + 0x1400


def research_case(r):
    """Research (above)"""
    log = log_region()
    vt = put(vtable({}, size=8), 0xc, 'I', recorder(23, 2))
    regions = {LOG: log, VT98: vt, OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98), OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT98),
               CLOCK: put(bytearray(r.bytes(8)), 0, 'I', CLOCKVT), CLOCKVT: vtable({0: 0}, size=4), 0x77c490: struct.pack('<I', CLOCK)}
    set_return(log, 0, r.getrandbits(32))
    blob_bytes(log, 23, 1, 0x30)
    ids = list(METERS) if r.random() < 0.95 else r.sample(METERS, 4)
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        w = bytearray(r.bytes(0x140))
        limit = r.choice([0x1f, 0x2f, 0x3f, 0])
        pos = r.choice([(limit << 16) + r.randrange(-0x30000, 0x30000), r.getrandbits(32), 0, 0x200000, 0x300000])
        put(w, 0xa0, '4I', pos & 0xffffffff, r.getrandbits(32), r.choice([0, 0x8000, r.getrandbits(32)]), r.choice([0, 1, 2, 3] * 3 + [4]))
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', MWINS + 0x200 * (i + 1) if i + 1 < len(ids) else 0)
        put(w, 0x5c, 'I', LIGHTS + 0x200 * i)
        regions[MWINS + 0x200 * i] = w
        k = bytearray(r.bytes(KID))
        put(k, 0x8c, 'I', LIGHT)
        put(k, 0x54, 'I', 0)
        put(k, 0x5c, 'I', 0)
        put(k, 0x98, 'I', OBJ98 if r.random() < 0.8 else 0)
        put(k, 0x9c, 'I', OBJ9C if r.random() < 0.8 else 0)
        regions[LIGHTS + 0x200 * i] = k
    regions[PANEL] = put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', MWINS), 0x8c, 'I', 0)
    regions[0x7cc1d8] = struct.pack('<I', PANEL)
    lab = bytearray(r.bytes(0x1480))
    for g in range(7):   # (5 and 6 too: the groups asked for past the meters)
        cur = r.choice([-1, -1, 0, 1, 2, 3])
        put(lab, 0x140c + 16 * g + 8, 'i', cur)
        put(lab, 0x140c + 16 * g + 4, 'I', r.choice([0, 1, 50, 100, 1000, r.randrange(200), r.getrandbits(32)]))
    for p in range(4):
        put(lab, 0xc + 20 * p, 'H', r.choice([0, 1, 7, r.getrandbits(16)]))
    regions[LAB] = lab
    slot = r.randrange(1, 4)
    slots = bytearray(r.bytes(20 * 8))
    put(slots, 20 * slot, 'I', LAB)
    n = r.choice([0, 1, 2, 3])
    things = bytearray(r.bytes(0x220 * 3))
    for t in range(n):
        put(slots, 20 * (4 + t), 'I', THINGS + 0x220 * t)
        put(things, 0x220 * t + 0x1e4, 'I', r.randrange(6))
        put(things, 0x220 * t + 0x210, 'H', 5 + t if t + 1 < n else 0)
    regions[THINGS] = things
    regions[0x7cfb90] = slots
    regions[GAME + 0x1da700] = put(put(bytearray(r.bytes(0x60)), 0x22, 'H', slot), 0x42, 'H', 4 if n else 0)
    regions[0x7cf83c] = struct.pack('<I', GAME)
    regions[0x80239c] = struct.pack('<I', GAME)
    regions[0x7852f8] = struct.pack('<I', r.choice([0, 1, 2, 7, 100, r.randrange(1, 1000)]))
    regions[0x78548c] = struct.pack('<I', r.choice([0, 1, 3, 100, r.randrange(1, 1000)]))
    g = bytearray(r.bytes(0x20))
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', SND)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    regions[0x802bc0] = g
    regions[SND] = put(put(bytearray(r.bytes(0x10)), 0, 'I', SVT), 4, 'I', ENGINE)
    regions[SVT] = vtable({}, size=8)
    regions[ENGINE] = put(bytearray(r.bytes(0x20)), 0, 'I', EVT)
    regions[EVT] = vtable({3: 3})
    regions[0x7cc1d0] = struct.pack('<I', r.getrandbits(32))
    group = r.choice([-1] * 5 + [0, 1, 2, 3, 4, 5, 6])
    return Call(CALLS['Research'], [group & 0xffffffff], regions, ret='v')


def progress_case(r):
    """Progress (above)"""
    from research import fbits
    log = log_region()
    regions = {LOG: log, VT98: vtable({7: 1})}
    ids = list(METERS) if r.random() < 0.95 else r.sample(METERS, 4)
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        w = bytearray(r.bytes(0x100))
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', MWINS + 0x200 * (i + 1) if i + 1 < len(ids) else 0)
        kids = [r.choice([0, LIGHT, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]
        kids.insert(r.randrange(len(kids) + 1), 0xd2d05 if r.random() < 0.97 else LIGHT)
        put(w, 0x5c, 'I', (RAM + 0x10000) + 0x1000 * i)
        regions[MWINS + 0x200 * i] = w
        for j, kid in enumerate(kids):   # (the meter's children, 0x100 apart)
            kw = bytearray(r.bytes(0x90))
            put(kw, 0, 'I', VT98)
            put(kw, 0x8c, 'I', kid)
            put(kw, 0x54, 'I', (RAM + 0x10000) + 0x1000 * i + 0x100 * (j + 1) if j + 1 < len(kids) else 0)
            regions[(RAM + 0x10000) + 0x1000 * i + 0x100 * j] = kw
    regions[PANEL] = put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', MWINS), 0x8c, 'I', 0)
    regions[0x7cc1d8] = struct.pack('<I', PANEL)
    lab = bytearray(r.bytes(0x1480))
    for i in range(0x100):
        cost = fbits(r)
        put(lab, 0xc + i * 20 + 0xc, 'II', cost, r.choice([cost, cost, fbits(r)]) if r.random() < 0.5 else fbits(r))
    for g in range(7):
        put(lab, 0x140c + 16 * g + 8, 'i', r.choice([-1, -2, 0, r.randrange(0x100), r.randrange(0x100)]))
    group = r.choice([0, 1, 2, 3, 4] * 4 + [5, 6, -1])
    current = struct.unpack_from('<i', lab, 0x140c + group * 16 + 8)[0]
    put(lab, 0x1464, 'i', r.choice([0, r.randrange(0x100), r.randrange(0x100), max(current, 0)]))
    regions[LAB] = lab
    slot = r.randrange(1, 4)
    regions[0x7cfb90] = put(bytearray(r.bytes(20 * 4)), 20 * slot, 'I', LAB)
    regions[GAME + 0x1da700] = put(bytearray(r.bytes(0x60)), 0x22, 'H', slot)
    regions[0x7cf83c] = struct.pack('<I', GAME)
    return Call(CALLS['Progress'], [group & 0xffffffff], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Progress':
        return progress_case(r)
    if op == 'Research':
        return research_case(r)
    log = log_region()
    regions = {LOG: log, VT98: put(vtable({}, size=8), 0xc, 'I', recorder(3, 2)), VT9C: put(vtable({}, size=8), 0xc, 'I', recorder(22, 2)),
               OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98), OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT9C),
               CLOCK: put(bytearray(r.bytes(8)), 0, 'I', CLOCKVT), CLOCKVT: vtable({0: 0}, size=4), 0x77c490: struct.pack('<I', CLOCK)}
    set_return(log, 0, r.getrandbits(32))
    blob_bytes(log, 3, 1, 0x30)
    blob_bytes(log, 22, 1, 0x30)
    w = bytearray(r.bytes(0x140))
    limit = r.choice([0x1f, 0x2f, 0x3f, 0x10, 0])
    pos = r.choice([(limit << 16) + r.randrange(-0x30000, 0x30000), r.getrandbits(22), r.getrandbits(32), 0, 0x200000, 0x300000])
    speed = r.choice([r.randrange(0, 0x40000), r.randrange(-0x40000, 0), r.getrandbits(32), 0, 0x8000])
    phase = r.choice([0, 1, 2, 3] * 4 + [4, 0xffffffff, r.getrandbits(32)])
    put(w, 0xa0, '4I', pos & 0xffffffff, r.getrandbits(32), speed & 0xffffffff, phase)
    put(w, 0x118, 'I', recorder(5, 4, cdecl=True))
    ids = ([LIGHT] if r.random() < 0.95 else []) + [r.choice([0, LIGHT + 1, LIGHT - 1, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        k = bytearray(r.bytes(KID))
        put(k, 0x8c, 'I', ident)
        put(k, 0x54, 'I', KIDS + KID * (i + 1) if i + 1 < len(ids) else 0)
        put(k, 0x98, 'I', OBJ98 if r.random() < 0.8 else 0)
        put(k, 0x9c, 'I', OBJ9C if r.random() < 0.8 else 0)
        regions[KIDS + KID * i] = k
    put(w, 0x5c, 'I', KIDS if ids else 0)
    regions[WINDOW] = w
    if op == 'SetLevel':
        level = r.choice([0] * 5 + [r.randrange(1, 101)] * 6 + [r.randrange(-40, 0), r.getrandbits(32), 0x7fff, 0x8000])
        return Call(CALLS[op], [WINDOW, level & 0xffffffff], regions, ret='v')
    message = r.choice([0x1e] * 6 + [0x1d, 0x1f, 0, r.getrandbits(32)])
    return Call(CALLS[op], [WINDOW, message, r.getrandbits(32), r.getrandbits(32)], regions)


if __name__ == '__main__':
    B.main(case)
