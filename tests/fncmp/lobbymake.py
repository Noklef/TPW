# The lobby's models made (src/game/frontend/lobbymake: 0x5d7fd0, 0x5d8870, 0x5dfa60) against the originals: real strings on the heap, a resource list of 0-3 whose names differ or match (case folded)
# in one or both or all (the first match is kept), the model's allocation failing now and then, the new ride's instance by flag 0x4000. Recorders (driver.cpp's kLobbyMakeEnv): load 20, Ride_Create 21, Holder_Sleep 22,
# Obj_RemoveFromRect 23, LobbyModel_TurnY 24, LobbyModel_SetPosition 25, Sound_RegisterLobbyCategories 26; the scene's slots 6 and 7, the owner's slot 12.
# Marker (0x5dc3e0, RideUser_Acquire 28, LobbyModel_AlignTo 29, LobbyModel_SetScale 30): that model made, the record's point any floats,
# the record's name its own North Pole in any case or not, its number about the scale's two limits or any.
# usage: lobbymake.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Res,Make,Ctor,Marker)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, RAND_SEED, Bench, Call, _recorder, blob, fb, heap, log_region, put, set_return, strings, vtable  # noqa: E402

HOOKS = {0x4629d0: _recorder(20, 11, cdecl=True), 0x463060: _recorder(21, 10, cdecl=True), 0x45bdd0: _recorder(22, 1, cdecl=True), 0x45be80: _recorder(23, 3, cdecl=True),
         0x5d8380: _recorder(24, 1), 0x5d8190: _recorder(25, 1), 0x51e8f0: _recorder(26, 1, cdecl=True), 0x5d8130: _recorder(28, 0), 0x5d81c0: _recorder(29, 1), 0x5d83b0: _recorder(30, 3)}
B = Bench([(0x5d7fd0, 0x5d809b), (0x5d8870, 0x5d8a0f), (0x5dfa60, 0x5dfc04), (0x5dc3e0, 0x5dc550), (0x46b3d0, 0x46b3e0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)], hooks=HOOKS)
OPS = os.environ.get('OPS', 'Res,Make,Make,Ctor,Marker').split(',')
SCENE, LIST, NODES, RES, NAMES, SELF, VT, OWNER, VTO, INST, POINT = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x100, RAM + 0x300, RAM + 0x500, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xc00, RAM + 0xe00
WIDE = RAM + 0xe40
SCENE_AT, TABLE = 0xf83774, 0x7a4610
WORDS = [b'hero', b'HERO', b'Hero', b'guide', b'GUIDE', b'pack', b'PACK', b'pack2', b'', b'a longer name of a model']


def names(r):
    """The name table: each word at its offset; returns the region and the addresses."""
    region = bytearray()
    at = {}
    for w in WORDS:
        at[w] = NAMES + len(region)
        region += w + b'\0'
    return region, at


def folded(r, word):
    return r.choice([w for w in WORDS if w.lower() == word.lower()])


def world(r, at, name, pack):
    log = log_region()
    strings(log, 20, 3)
    blob(log, 25, 0, 7)
    set_return(log, 20, r.getrandbits(32))
    set_return(log, 26, r.getrandbits(32))
    ride = r.randrange(0, 8)
    set_return(log, 21, ride)
    inst = bytearray(r.bytes(0x100))
    put(inst, 4, 'I', r.getrandbits(32) | 0x4000 if r.random() < 0.5 else r.getrandbits(32) & ~0x4000)
    n = r.choice([0, 1, 2, 3])
    match = n > 0 and r.random() < 0.5
    nodes, res = bytearray(0x20), bytearray(r.bytes(0x100))
    for k in range(n):
        put(nodes, 8 * k, 'II', RES + 0x40 * k, NODES + 8 * (k + 1) if k + 1 < n else 0)
        put(res, 0x40 * k + 0xc, 'I', at[folded(r, name) if match else r.choice(WORDS)])
        put(res, 0x40 * k + 0x20, 'I', at[folded(r, pack) if match else r.choice(WORDS)])
    fail = 1 if (match and r.random() < 0.2) or r.random() < 0.01 else 0   # with no match the first is the resource's block: the original faults on it, sic
    return {SCENE: put(put(bytearray(r.bytes(0x40)), 0, 'I', VT), 8, 'I', LIST), LIST: struct.pack('<4I', 0, NODES if n else 0, 0, r.randrange(0, 0x100)),
            NODES: nodes, RES: res, VT: vtable({6: 1, 7: 1}), VTO: vtable({1: 1, 12: 1}), OWNER: struct.pack('<I', VTO) + r.bytes(0xc), INST: inst, TABLE: struct.pack('<8I', *[INST] * 8),
            SCENE_AT: struct.pack('<I', SCENE), LOG: log, HEAP: heap(fail=fail), RAND_SEED: r.bytes(4)}


def case(r):
    region, at = names(r)
    op = r.deal(OPS)
    name, pack, pack2 = r.choice(WORDS), r.choice(WORDS), r.choice(WORDS)
    regions = world(r, at, name, pack)
    if op != 'Make':
        regions[HEAP] = heap()   # (strings fault on a failed allocation in both)
    regions[NAMES] = region
    a, b, c = r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)
    if op == 'Res':
        regions[SELF] = r.bytes(0x2c)
        return Call(0x5d7fd0, [SELF, at[name], at[pack], r.randrange(-8, 0x100) & 0xffffffff, r.choice([0, 0, 1, r.getrandbits(32)]), a, b, c], regions, conv='t', ret='a')
    if op == 'Make':
        return Call(0x5d8870, [SCENE, at[name], at[pack], r.choice([0, 0, 1, r.getrandbits(32)]), a, b, c], regions, conv='t', ret='a')
    if op == 'Marker':
        blob(regions[LOG], 29, 0, 7)
        blob(regions[LOG], 25, 0, 7)
        regions[SELF] = r.bytes(0x20)
        title = r.choice(['North Pole', 'NORTH pole', 'North Pol', 'North Poles', 'Paris', ''])
        regions[WIDE] = b''.join(struct.pack('<H', ord(ch)) for ch in title) + b'\0\0'
        number = r.choice([r.randrange(0, 300), r.randrange(-100, 100), 80, 79, 240, 241, 0, -1, r.getrandbits(32) - (1 << 31)])
        record = put(bytearray(r.bytes(0x50)), 0x14, 'I', WIDE)
        for k in range(3):
            put(record, 0x34 + 4 * k, 'I', r.choice([fb(r.uniform(-500, 500)), fb(0.0), r.f()]))
        regions[RAM + 0x1000] = bytes(put(record, 0x40, 'i', number))
        return Call(0x5dc3e0, [SELF, OWNER, RAM + 0x1000], regions, conv='t', ret='a')
    regions[SELF] = r.bytes(0x74)
    regions[POINT] = r.bytes(0xc)
    regions[WIDE] = b''.join(struct.pack('<H', ord(ch)) for ch in 'text') + b'\0\0'
    degrees = r.choice([fb(r.uniform(-360, 360)), fb(90.0), fb(0.0), r.f()])
    return Call(0x5dfa60, [SELF, OWNER, r.getrandbits(32), at[r.choice(WORDS)], at[name], at[pack], at[pack2], degrees, POINT, WIDE, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
