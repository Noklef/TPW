# The thing browser's panel (src/game/ui/thingbrowser: 0x4b3490, 0x4b3580) against the originals: personsprite.py's sprite as the thing
# shown [0x7cc308] (or none), its kind +2 staff (4-8) or another, the count +0x1d8 0 or not, +0x30 0, 1 or any, the mode +0x19c 0-6 or any;
# the panel [0x7cc304] a window whose children (+0x5c, then +0x54) hold the ids 0x147 / 0x148 / 0x14a among others (their vtable's
# slots 9 and 7 recorders of one argument; 0x148's objects +0x98 / +0x9c recorders of its notification, the event's word +4 hashed), the
# last code [0x756b98] 0-3, 9 or any; personsprite.py's clock and camera (ShowName ends on Camera_CentreOnSprite). The text formatter
# 0x6acd60 is a log stub on both sides (driver: TextFormatEnv), so the count's argument object, its fifth argument, is not compared.
# GroupHandler (0x4b3b90): radiogroup.py's group (its children's notifications masked by the looks they end with: Select's states and,
# for b other than 1, the child a set to 1), the message 0x100 mostly (else the default handler +0x118 fncmp's logger), a one of the
# children's ids, the selected one, -1 or another (no child: SetState of 0 faults in both), b 1, 0 or any; the group's parent +0x60 a
# window (its byte +4 refusing posts now and then) or 0, window.py's queues.
# usage: thingbrowser.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowName,ShowDetail,GroupHandler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import radiogroup  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _cdecl_log4, blob_bytes, log_region, put, vtable  # noqa: E402
from window import RANGES  # noqa: E402
from personsprite import THING, centre_env, sprite_env  # noqa: E402

CALLS = {'ShowName': 0x4b3490, 'ShowDetail': 0x4b3580, 'GroupHandler': 0x4b3b90}
B = Bench(sorted(set(RANGES + radiogroup.RANGES + [(0x4b3b90, 0x4b3c09), (0x65d55a, 0x65d56b), (0x475480, 0x4754f0), (0x4d8c50, 0x4d8c70), (0x4f9f00, 0x4fa2b0), (0x4fe900, 0x4fe960), (0x50b350, 0x50b370), (0x510160, 0x510190),
           (0x53c8d0, 0x53c9d0), (0x5da3c0, 0x5da3c1), (0x486860, 0x4868c1), (0x42aab0, 0x42ab1d),
           (0x4b3490, 0x4b3576), (0x4b3580, 0x4b3612), (0x65d7bc, 0x65d832), (0x65d3a3, 0x65d40c), (0x506860, 0x506867), (0x5064f0, 0x506523),
           (0x485ec0, 0x485eea)])), hooks={0x6acd60: _cdecl_log4})
PANEL, KIDS, VT, OBJ, OVT = RAM + 0x30000, RAM + 0x30200, RAM + 0x30800, RAM + 0x30900, RAM + 0x30a00
PARENT = RAM + 0x3000   # (GroupHandler's: past radiogroup.py's group and children, before window.py's queues' end)


def group_case(r):
    log = log_region()
    regions = {LOG: log}
    g, kids = radiogroup.group_env(r, regions, log)
    put(g, 0x60, 'I', r.choice([PARENT] * 5 + [0]))
    put(g, 0x118, 'I', CDECL_LOG4)
    g[4] = 0
    parent = bytearray(r.bytes(0x120))
    parent[4] = r.choice([0, 0, 0, 1])
    regions[PARENT] = parent
    selected = int.from_bytes(g[0x138:0x13c], 'little')
    message = r.choice([0x100] * 6 + [0x101, 0, r.getrandbits(32)])
    a = r.choice([k[0] for k in kids] * 3 + [selected, 0xffffffff, 7])
    b = r.choice([1, 1, 0, 0, 2, r.getrandbits(32)])
    i = radiogroup.first(kids, a)
    if i is None and r.random() < 0.85:   # (mostly not the fault)
        b = 1
    states = radiogroup.select_states(kids, selected, a) if message == 0x100 else {}
    if message == 0x100 and b != 1 and i is not None:
        states[i] = 1
    radiogroup.masks(log, kids, states)
    return Call(CALLS['GroupHandler'], [radiogroup.GROUP, message, a, b], regions, ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'GroupHandler':
        return group_case(r)
    log = log_region()
    regions = {LOG: log}
    thing = sprite_env(r, regions)
    centre_env(r, regions)
    thing[2] = r.choice([4, 5, 6, 7, 8, 1, 2, 3, 9, 0, r.getrandbits(8)])
    put(thing, 0x1d8, 'I', r.choice([0, 0, 1, 5, r.getrandbits(32)]))
    put(thing, 0x30, 'I', r.choice([0, 0, 1, 2, 0x29b, r.getrandbits(32)]))
    put(thing, 0x19c, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, r.getrandbits(32)]))
    regions[0x7cc308] = struct.pack('<I', THING if r.random() < 0.9 else 0)
    ids = [0x147, 0x148, 0x14a] + r.choice([[], [1], [0x149, 0x147]])
    r.shuffle(ids)
    for i, n in enumerate(ids):
        kid = bytearray(r.bytes(0xa0))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', n)
        put(kid, 0x54, 'I', KIDS + 0xa0 * (i + 1) if i + 1 < len(ids) else 0)
        put(kid, 0x98, 'I', r.choice([OBJ, 0]))
        put(kid, 0x9c, 'I', r.choice([OBJ, 0]))
        regions[KIDS + 0xa0 * i] = kid
    regions[PANEL] = put(bytearray(r.bytes(0x90)), 0x5c, 'I', KIDS)
    regions[0x7cc304] = struct.pack('<I', PANEL)
    regions[VT] = vtable({9: 1, 7: 1})
    regions[OBJ] = struct.pack('<I', OVT)
    regions[OVT] = vtable({3: 2})
    blob_bytes(log, 3, 1, 0x30)
    regions[0x756b98] = struct.pack('<I', r.choice([0, 1, 2, 3, 9, r.getrandbits(32)]))
    regions[0x78ba94] = r.bytes(4)
    return Call(CALLS[op], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
