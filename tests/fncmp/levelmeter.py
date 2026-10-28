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
# the groups' current projects -1 or any, the dearest +0x1464 any or the current; now and then plain costs, the current one's up to three
# times the dearest's, so the gauge's value goes negative), picked as Research's; the research panel's five meters (one
# missing now and then: a fault in both), each with its gauge 0xd2d05 (missing now and then: a fault in both) among 0-2 decoys, the gauge's
# slot 7 the recorder 7.
# Name (0x4aa120): the group 0-4 mostly, now and then 5, 6 or -1 (no meter: a fault in both); research.py's defs() (up to six definitions in
# 0x7890a0, their full names +0x4f4 texts of NAMES, never none: the object texts' path not taken; the control table in the game [0x7cf83c] =
# [0x80239c] at +0x1d943c, its words +0x1da722 / +0x1da742 the lab's slot and the things' head) and a lab of research.py's whose first eight
# projects' ids are mostly ones with a definition and a control record (else a miss: the language's text 0xb9, panelparts.py's texts(), then
# Update's run; an id with a definition but no record faults in both), their bytes +9 0-0x10 (group 4's value: the definition's dword
# +0x1b8 + 0x40 * it), the groups' current projects -1 or 0-7; the panel's five meters (one missing now and then) with their lights (as
# Research's) and gauges 0xd2d05 (missing now and then: a fault in both; slot 7 the recorder 7, +0x9c an object whose slot 3 is the recorder
# 24, the text it is sent hashed: blob_inner_text()) among 0-1 decoys; Research's clock, sound and things. The formatter 0x6acd60 is the log
# stub with its two text arguments hashed (text_args(); the buffer it writes is what the gauge is sent).
# Refresh (0x4aaec0) and Refresh481520 (0x481520, thiscall on 0x7b51f0): as Name, but the lab, definitions, control records and save keys research.py's
# choose_case() makes for Choose (0x5034f0, ResearchLab_NextProject; its group too), the lab moved to LAB2.
# usage: levelmeter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLevel,OnMessage,Research,Progress,Name,Refresh,Refresh481520)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import panelparts  # noqa: E402
import research  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log4, blob_bytes, blob_inner_text, log_region, put, recorder, set_return, strings, text_args, vtable  # noqa: E402

CALLS = {'SetLevel': 0x4a95b0, 'OnMessage': 0x4a9fc0, 'Research': 0x4a9c70, 'Progress': 0x4a9bd0, 'Name': 0x4aa120, 'Refresh': 0x4aaec0,
         'Refresh481520': 0x481520}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x409410, 0x409490), (0x410ac0, 0x410b20), (0x412e90, 0x412f00), (0x412fb0, 0x413030), (0x414670, 0x414690), (0x4146b0, 0x414800),
           (0x47d460, 0x47d4b0), (0x481520, 0x481540), (0x485b00, 0x485b50), (0x4a95b0, 0x4a96c0), (0x4a9bd0, 0x4a9f84), (0x4a9fc0, 0x4aa0b1),
           (0x4aa120, 0x4aa310), (0x4aaec0, 0x4aaf20), (0x4c9e60, 0x4c9ef0), (0x4d3d10, 0x4d3df0), (0x4d4b70, 0x4d4ba0), (0x5034f0, 0x5036b0),
           (0x503980, 0x503b40), (0x504040, 0x5040a0), (0x519510, 0x5195a0), (0x51bc40, 0x51bc80), (0x5aef50, 0x5af0f0), (0x5af780, 0x5af800),
           (0x5b30b0, 0x5b3120), (0x5da3c0, 0x5da3c1), (0x65968e, 0x6596a2), (0x65d3a3, 0x65d40e), (0x65d7bc, 0x65d832), (0x65e6d5, 0x65e73d),
           (0x65f5b7, 0x65f60e), (0x65f66e, 0x65f697), (0x67acd0, 0x67ad00), (0x67af00, 0x67afb0), (0x67b0d0, 0x67b120), (0x6acb70, 0x6accb0),
           (0x6ace60, 0x6aceb0), (0x6adc80, 0x6add40), (0x6b00e0, 0x6b0140), (0x6b5b80, 0x6b5b9a)],
          hooks={0x6acd60: _cdecl_log4, 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})   # (research.py's)
WINDOW, KIDS, OBJ98, OBJ9C, VT98, VT9C, CLOCK, CLOCKVT = RAM, RAM + 0x400, RAM + 0x1000, RAM + 0x1040, RAM + 0x1080, RAM + 0x1100, RAM + 0x1200, RAM + 0x1240
KID = 0x120
LIGHT = 0xd2d06
METERS = [0xd2d01, 0xd2d07, 0xd2d08, 0xd2d09, 0xd2d0a]
PANEL, MWINS, LIGHTS, LAB, GAME, THINGS = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x6000, RAM + 0x100000, RAM + 0x8000
SND, SVT, ENGINE, EVT = RAM + 0x1300, RAM + 0x1340, RAM + 0x1380, RAM + 0x1400
GOBJ, GVT, GAUGEVT = RAM + 0x1500, RAM + 0x1540, RAM + 0x1600   # (Name / Refresh: research.py's regions take RAM + 0x2000 to + 0xa200)
NPANEL, NMETERS, NKIDS, LAB2, NAMES = RAM + 0x30000, RAM + 0x31000, RAM + 0x32000, RAM + 0x40000, RAM + 0x48000


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
    dearest = struct.unpack_from('<i', lab, 0x1464)[0]
    if 0 <= current < 0x100 and current != dearest and r.random() < 0.4:   # (plain costs: left 0-300%, past 100 the gauge negative)
        top = r.uniform(2, 1000)
        put(lab, 0xc + dearest * 20 + 0xc, 'f', top)
        cost = top * r.uniform(0.01, 3)
        put(lab, 0xc + current * 20 + 0xc, 'ff', cost, cost * r.choice([0, r.random()]))
    regions[LAB] = lab
    slot = r.randrange(1, 4)
    regions[0x7cfb90] = put(bytearray(r.bytes(20 * 4)), 20 * slot, 'I', LAB)
    regions[GAME + 0x1da700] = put(bytearray(r.bytes(0x60)), 0x22, 'H', slot)
    regions[0x7cf83c] = struct.pack('<I', GAME)
    return Call(CALLS['Progress'], [group & 0xffffffff], regions, ret='v')


def lab_bytes(r):
    """a lab as research.py's case() makes it (its projects' groups and costs any), the projects' bytes +9 0-0x10, the dearest +0x1464 any"""
    lab = bytearray(r.bytes(0x1800))
    for i in range(0x100):
        p = 0xc + i * 20
        put(lab, p + 4, 'I', r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
        cost = research.fbits(r)
        put(lab, p + 0xc, 'II', cost, r.choice([cost, cost, research.fbits(r)]) if r.random() < 0.5 else research.fbits(r))
        lab[p + 9] = r.randrange(0x11)
    for g in range(7):   # (the groups' current projects: Update reads all five)
        put(lab, 0x140c + 16 * g + 8, 'i', r.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7]))
    put(lab, 0x1464, 'i', r.choice([0, 1, r.randrange(8), r.randrange(0x100)]))
    return lab


def meters_env(r, regions, log):
    """Name / Refresh: the panel, its meters with their lights and gauges, the clock, the sound and the things (the module comment)"""
    regions.update({VT98: put(vtable({}, size=8), 0xc, 'I', recorder(23, 2)), OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98),
                    OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT98), CLOCK: put(bytearray(r.bytes(8)), 0, 'I', CLOCKVT),
                    CLOCKVT: vtable({0: 0}, size=4), 0x77c490: struct.pack('<I', CLOCK), GAUGEVT: vtable({7: 1}),
                    GVT: put(vtable({}, size=8), 0xc, 'I', recorder(24, 2)), GOBJ: put(bytearray(r.bytes(0x20)), 0, 'I', GVT)})
    set_return(log, 0, r.getrandbits(32))
    blob_bytes(log, 23, 1, 0x30)
    blob_bytes(log, 24, 1, 0, (0x10, 0))
    blob_inner_text(log, 24, 0)
    ids = list(METERS) if r.random() < 0.97 else r.sample(METERS, 4)
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        w = bytearray(r.bytes(0x140))
        limit = r.choice([0x1f, 0x2f, 0x3f, 0])
        pos = r.choice([(limit << 16) + r.randrange(-0x30000, 0x30000), r.getrandbits(32), 0, 0x200000, 0x300000])
        put(w, 0xa0, '4I', pos & 0xffffffff, r.getrandbits(32), r.choice([0, 0x8000, r.getrandbits(32)]), r.choice([0, 1, 2, 3] * 3 + [4]))
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', NMETERS + 0x200 * (i + 1) if i + 1 < len(ids) else 0)
        put(w, 0x5c, 'I', NKIDS + 0x400 * i)
        regions[NMETERS + 0x200 * i] = w
        kids = [LIGHT, 0xd2d05 if r.random() < 0.97 else LIGHT + 1] + [r.getrandbits(32) for _ in range(r.choice([0, 0, 1]))]
        r.shuffle(kids)
        for j, kid in enumerate(kids):
            k = bytearray(r.bytes(KID))
            put(k, 0, 'I', GAUGEVT)
            put(k, 0x8c, 'I', kid)
            put(k, 0x54, 'I', NKIDS + 0x400 * i + KID * (j + 1) if j + 1 < len(kids) else 0)
            put(k, 0x5c, 'I', 0)
            put(k, 0x98, 'I', OBJ98 if r.random() < 0.8 else 0)
            put(k, 0x9c, 'I', (GOBJ if kid == 0xd2d05 else OBJ9C) if r.random() < 0.9 else 0)
            regions[NKIDS + 0x400 * i + KID * j] = k
    regions[NPANEL] = put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', NMETERS), 0x8c, 'I', 0)
    regions[0x7cc1d8] = struct.pack('<I', NPANEL)
    slot = r.randrange(1, 4)
    slots = bytearray(r.bytes(20 * 8))
    put(slots, 20 * slot, 'I', LAB2)
    n = r.choice([0, 1, 2, 3])
    things = bytearray(r.bytes(0x220 * 3))
    for t in range(n):
        put(slots, 20 * (4 + t), 'I', THINGS + 0x220 * t)
        put(things, 0x220 * t + 0x1e4, 'I', r.randrange(6))
        put(things, 0x220 * t + 0x210, 'H', 5 + t if t + 1 < n else 0)
    regions[THINGS] = things
    regions[0x7cfb90] = slots
    table = regions[research.TABLE]   # (the game: research.py's defs() puts its control table at +0x1d943c)
    put(table, 0x1da722 - 0x1d943c, 'H', slot)
    put(table, 0x1da742 - 0x1d943c, 'H', 4 if n else 0)
    regions[0x7cf83c] = struct.pack('<I', research.TABLE - 0x1d943c)
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


def name_case(r, op):
    """Name / Refresh / Refresh481520 (the module comment)"""
    from language import text
    log = log_region()
    text_args(log, 2)
    strings(log, 0x3d, 0, 2)
    lab = lab_bytes(r)
    if op == 'Name':
        regions = {}
        research.defs(r, regions)
        lst, items, table = regions[research.DEFS], regions[research.ITEMS], regions[research.TABLE]
        have = [struct.unpack_from('<H', items, 0x600 * i)[0] for i in range(struct.unpack_from('<i', lst, 0x528)[0])]
        known = [struct.unpack_from('<H', table, 0x20 * k)[0] for k in range(struct.unpack_from('<i', table, 0x12c0)[0])]
        good = [i for i in have if i in known] or research.IDS
        for p in range(8):
            put(lab, 0xc + 20 * p, 'H', r.choice(good) if r.random() < 0.85 else r.choice(research.IDS + [0, r.getrandbits(16)]))
        group = r.choice([0, 1, 2, 3, 4] * 8 + [5, 6, 0xffffffff])
    else:
        call = research.choose_case(r, 'Choose', lab)
        regions = dict(call.regions)
        regions.pop(research.LAB)
        group = call.args[1]
    regions[LAB2] = lab
    regions[LOG] = log
    items = regions[research.ITEMS]
    names = bytearray()
    for i in range(6):
        put(items, 0x600 * i + 0x4f4, 'I', NAMES + 0x100 * i)
        name = text(r) if r.random() < 0.9 else bytes(r.randrange(0x20, 0x7f) for _ in range(0x41))   # (up to 0x41: GetFullName's 0x42 then ends it)
        names += (name.decode('latin-1').encode('utf-16-le') + bytes(0x100))[:0x100]
    regions[NAMES] = names
    regions[0x78b530] = r.bytes(0x400)
    panelparts.texts(r, regions, [0xb9])
    meters_env(r, regions, log)
    if op == 'Refresh481520':
        return Call(CALLS[op], [0x7b51f0, group], regions, conv='t')
    return Call(CALLS[op], [group], regions)


def case(r):
    op = r.deal(OPS)
    if op in ('Name', 'Refresh', 'Refresh481520'):
        return name_case(r, op)
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
