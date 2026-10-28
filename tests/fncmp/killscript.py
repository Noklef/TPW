# A ride script killed (src/game/rsse/scriptkill: RSSE_KillScript 0x559060) and its record freed (RSSE_FreeContext 0x558500) against the
# originals, with their callees run. The script list (0x8791b0) holds 1-4 records (ids distinct, linked both ways), each with its +0x30
# dwords (0-4, -1 now and then; set or not), +0x2c walker slots (0-4: words +0x16 / +0x18 4 / 2 mostly, keys 1-8), other blocks (0 or any
# address: GlobalFree is a stub), 0-3 object nodes (+0xb0: types 0-11), the flying ride +0x9c (none: flyingclear.py has its record's
# free), the byte +0xb9, the ride +0xc8 and the handles +0xcc / +0xd0 / +0xd4; the ids +0xc / +0x14 another record's, 0 or none in the
# list, the parent +0x10 too (none now and then: that faults in both, sic). One of three worlds:
# - parts: partstate.py's activation with flyingclear.py's rides 1-3 (a model of 0-8 parts keyed 1-8), the flags' bit 4 either way;
# - effect: rideeffect.py's (ridebounds.py's ride and box, particles.py's systems, things and definitions with kill effects at +0x64);
# - sound: soundset.py's sender world for one of Sound_Stop (+0xd0), Sound_Forward6 (object nodes 3-10) or Sound_RemoveOccluder (+0xd4);
# the sound off in the others. Kill: the id one of the list's mostly, the system off now and then.
# usage: killscript.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Free,Kill)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import flyingclear as fc  # noqa: E402
import particles  # noqa: E402
import partstate  # noqa: E402
import ridebounds  # noqa: E402
import soundset  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(fc.merged(fc.RANGES + [(0x558500, 0x5587ea), (0x559060, 0x559344), (0x51e710, 0x51e730), (0x466b70, 0x466c7f), (0x412e90, 0x412eec),
                                 (0x67a830, 0x67a860), (0x5da3c0, 0x5da3c1)]), hooks=fc.HOOKS)
CTX, ARR, OBJ, FLY = RAM + 0x30000, RAM + 0x31000, RAM + 0x32000, RAM + 0x34000
DEFS, THINGS = RAM + 0x38000, RAM + 0x39000
FREE, KILL = 0x558500, 0x559060


def handle(r, regions):
    """a particle system's handle: a live one of particles.py's mostly, else its slot or any dword."""
    live = r.randrange(6)
    serial = struct.unpack_from('<H', regions[0x80cefc], 0x140 * live + 0xa)[0]
    return r.choice([serial << 16 | live] * 3 + [live, r.getrandbits(32)])


def things(r, regions, effects):
    """rideeffect.py's things (slots 1-7, types 0x10-0x16, now and then one no definition has) and definitions (kill effect +0x64)."""
    slots, th = bytearray(r.bytes(0x14 * 8)), bytearray(r.bytes(0x20 * 8))
    for k in range(8):
        put(slots, 0x14 * k, 'I', THINGS + 0x20 * k)
        put(th, 0x20 * k + 0xe, 'H', 0x10 + k if r.random() < 0.97 else 0x99)
    defs, table = bytearray(r.bytes(0x80 * 8)), bytearray(r.bytes(0x738))
    for i, k in enumerate(r.sample(range(8), 8)):
        put(table, 4 * i, 'I', DEFS + 0x80 * k)
        put(defs, 0x80 * k, 'H', 0x10 + k)
        put(defs, 0x80 * k + 0x64, 'I', r.choice(effects))
    put(table, 0x528, 'i', 8)
    k = r.randrange(8)
    put(table, 0x730, 'HHI', r.choice([0x10 + k, 0x30]), 0, DEFS + 0x80 * k)
    regions.update({0x7cfb90: slots, THINGS: th, 0x7890a0: table, DEFS: defs})


def world(r):
    regions = dict(particles.spawn(r).regions)
    regions[HEAP] = heap()
    kind = r.choice(['parts', 'parts', 'effect', 'sound'])
    fn = sarg = None
    if kind == 'parts':
        partstate.activation(r, regions)
        regions[LOG] = log_region()   # (Ride_Destroy's recorder)
        put(regions[partstate.INSTANCE], 4, 'I', fc.MODEL)
        model = put(bytearray(r.bytes(0x100)), 0x48, 'H', r.choice([0, 1, 4, 8]))
        put(model, 0x7c, 'I', fc.PARTS)
        pt = bytearray(r.bytes(0xa0))
        for i in range(8):
            put(pt, i * 0x14, '2I', r.choice([0x80, 0x80, 0x81, 1, 0, r.getrandbits(32)]), r.choice([1, 2, 3, 4, 5, 8]))
        regions.update({0x7a4610: struct.pack('<4I', 0, fc.HOLDER, fc.HOLDER, fc.HOLDER),
                        fc.HOLDER: put(bytearray(r.bytes(0x40)), 8, 'I', partstate.INSTANCE), fc.MODEL: model, fc.PARTS: pt})
        owners = [0, 1, 2, 3, 3]
    else:
        regions.update(global_heap())
        if kind == 'effect':
            c = ridebounds.box(r)
            regions.update(c.regions)
            owners = [0] + [c.args[0]] * 4
            things(r, regions, [0, 0, 1, 0x68, r.randrange(0x69)])
        else:
            fn = r.choice([0x51c300, 0x51c2c0, 0x51c6b0])
            sound = soundset.sender_case(r, fn)
            regions.update(sound.regions)
            sarg = sound.args[0]
            table = bytearray(regions[0x6fd000])   # (GlobalFree served too)
            for a, v in global_heap().items():
                table[a - 0x6fd000:a - 0x6fd000 + 4] = v
            regions[0x6fd000] = table
            owners = [0]
            things(r, regions, [0])
    regions.setdefault(0x802bc0, bytes(0x20))   # (the sound off outside the sound world)
    # the flying rides: a record with no slot in use (flyingclear.py frees busy ones) at some of the indices the records name, and others
    fly = bytearray(0x194)
    for i in r.sample([1, 2, 50, 51, 99, r.randrange(1, 100)], 3):
        put(fly, 4 * i, 'I', FLY)
    rec = bytearray(r.bytes(0x5c + 20 * 0xe4))
    for i in range(20):
        put(rec, 0x5c + i * 0xe4 + 0x80, 'I', 0)
    regions.update({0x803ac8: r.bytes(0x10), 0x8791f8: fly, FLY: rec})
    return regions, kind, fn, sarg, owners


def record(r, regions, k, ids, kind, fn, sarg, owners):
    c = bytearray(r.bytes(0x100))
    put(c, 8, 'i', ids[k])
    other = lambda: r.choice([0, 0, 0, r.choice(ids), r.choice(ids), 999])  # noqa: E731
    put(c, 0xc, 'i', other())
    put(c, 0x14, 'i', other())
    taken = struct.unpack_from('<i', c, 0xc)[0], struct.unpack_from('<i', c, 0x14)[0]
    parents = [i for i in ids if i not in taken] or [0]   # (one freed first is not in the list any more: that faults, as 999 does)
    put(c, 0x10, 'i', r.choice([0] * 12 + [r.choice(parents)] * 12 + [999]))
    owner = r.choice(owners)   # (no ride: no keyed parts set and no walker out (the loader's), else the null ride faults in both)
    arr = bytearray(r.bytes(0x100))
    for off in range(0x18, 0x3c, 4):
        put(c, off, 'I', r.choice([0, r.getrandbits(32)]))
    n = r.choice([0, 1, 2, 3, 4, -1])
    put(c, 0x4c, 'i', n)
    put(c, 0x30, 'I', r.choice([ARR + 0x100 * k] * 6 + [0]) if n > 0 else r.choice([0, r.getrandbits(32)]))
    for i in range(4):
        put(arr, 4 * i, 'I', r.choice([0, 1, r.getrandbits(32)]) if owner else 0)
    w = r.choice([0, 1, 2, 3, 4, -1])
    put(c, 0x7c, 'i', w)
    put(c, 0x2c, 'I', r.choice([ARR + 0x100 * k + 0x40] * 6 + [0]) if w > 0 else r.choice([0, r.getrandbits(32)]))
    for i in range(4):
        s = 0x40 + 0x20 * i
        put(arr, s + 2, 'h', r.choice([1, 2, 3, 4, 8, -1]))
        put(arr, s + 0x16, 'HH', r.choice([4, 4, 4, 3]), r.choice([2, 2, 2, 1, 0]) if owner else r.choice([1, 0]))
    regions[ARR + 0x100 * k] = arr
    # the object list: 0-3 nodes, linked both ways
    nodes = r.choice([0, 0, 1, 2, 3])
    ob = bytearray(r.bytes(0x10 * 3))
    for j in range(nodes):
        put(ob, 0x10 * j, 'II', OBJ + 0x100 * k + 0x10 * (j + 1) if j + 1 < nodes else 0, OBJ + 0x100 * k + 0x10 * (j - 1) if j else 0)
        if kind == 'sound':
            kinds = [3, 4, 10] if fn == 0x51c2c0 else [0, 11, 0x40]
        else:
            kinds = [1, 2, 3, 7, 10, 0, 11, r.getrandbits(32)]
        t = r.choice(kinds)
        put(ob, 0x10 * j + 8, 'II', t, sarg if kind == 'sound' and fn == 0x51c2c0 else handle(r, regions))
    put(c, 0xb0, 'I', OBJ + 0x100 * k if nodes else 0)
    regions[OBJ + 0x100 * k] = ob
    put(c, 0x98, 'II', r.choice([0, 1, 2, 50, 51, 99, 100]), r.choice([0, 1, 2, 50, 51, 99, 100]))
    c[0xb9] = r.choice([0, 0, 1, 0x80])
    put(c, 0xac, 'H', 0 if kind == 'parts' else r.choice([0, 1, 2, 5, 7]))
    put(c, 0xc8, 'I', owner)
    put(c, 0xcc, 'I', r.choice([0, 0, handle(r, regions)]))
    put(c, 0xd0, 'I', r.choice([0, sarg]) if kind == 'sound' and fn == 0x51c300 else 0 if kind == 'sound' else r.choice([0, r.getrandbits(32)]))
    put(c, 0xd4, 'I', r.choice([0, sarg, sarg]) if kind == 'sound' and fn == 0x51c6b0 else 0 if kind == 'sound' else r.choice([0, r.getrandbits(32)]))
    return c


def case(r):
    op = r.choice(OPS)
    regions, kind, fn, sarg, owners = world(r)
    n = r.choice([1, 2, 3, 4])
    ids = r.sample(range(1, 40), n)
    for k in range(n):
        c = record(r, regions, k, ids, kind, fn, sarg, owners)
        put(c, 0, 'II', CTX + 0x100 * (k + 1) if k + 1 < n else 0, CTX + 0x100 * (k - 1) if k else 0)
        regions[CTX + 0x100 * k] = c
    head = CTX if r.random() < 0.95 else 0
    rsse = bytearray(r.bytes(0x18))
    put(rsse, 0, 'I', r.choice([1] * 9 + [0]) if op == KILL else r.getrandbits(32))
    put(rsse, 0xc, 'i', r.choice([n, r.randrange(-5, 50)]))
    put(rsse, 0x10, 'II', head, r.randrange(-2, 20) & 0xffffffff)
    regions[0x8791a0] = rsse
    flags = r.getrandbits(8) | r.choice([0, r.getrandbits(32)])
    if kind != 'parts':
        flags &= ~4
    if op == FREE:
        target = r.choice([CTX + 0x100 * r.randrange(n)] * 9 + [0])
        return Call(FREE, [target, flags], regions, ret='a')
    return Call(KILL, [r.choice([r.choice(ids)] * 9 + [0, 999]), flags], regions, ret='v')


OPS_ALL = {'Free': FREE, 'Kill': KILL}
OPS = [OPS_ALL[k] for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
