# An object definition destroyed (src/game/world/objectdefs: 0x412cb0 ObjectDef_Dtor) and the definitions' effects let go (0x4145f0
# ObjectDefs_ReleaseEffects) against the originals, over soundset.py's senders' world for 0x51b620 (SoundCategory_Release: the sound
# globals, on and started either 0 now and then, the soundobject bench's object, its slot 8 logged): each definition's four effects
# (+0x4e0) 0 now and then, else among 0x68, 0x69 or any (ParticleEffect_Clear118: the setup word 0x816d40 0 now and then, the effect
# definitions 0x804370), its category (+0x4f0) one of the object's ids, 0 or any. Dtor: the balance file at +4 balancefile.py's
# ObjectDefBalance_Dtor file (vtable 0x6fd8a8 mostly, values or none; its schema, values and data moved to RAM + 0x30000 on).
# Effects: 0-4 definitions in the table, a null entry past them, the count +0x528 theirs, more, fewer, 0 or negative; +0x72c any.
# Free (0x412d50 ObjectDefs_Free) and FreeAll (0x412a90 ObjectDefs_FreeAll, the table at 0x7890a0): Effects' table, each definition's
# balance file Dtor's (one file, copied into each), its four texts (+0x4f4) 0, the shared 0x6fd880 or any (freed through the
# GlobalFree import: hashed), the definitions freed (Mem_Free: hashed); the count theirs, fewer, 0 or negative (no null entries below it).
# usage: objectdefdtor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dtor,Effects,Free,FreeAll)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put  # noqa: E402

_ops = os.environ.pop('OPS', None)   # (soundset.py and balancefile.py read OPS as their own)
import soundset  # noqa: E402
import balancefile as bf  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

bf.VALS, bf.FLAGS, bf.DATA = RAM + 0x30800, RAM + 0x30900, RAM + 0x31000   # (past the sound benches' regions)
DEF, DEFS, TABLE = RAM + 0x38000, RAM + 0x40000, RAM + 0x48000
SETUP, EFFECTS = 0x816d40, 0x804370
B = Bench(sorted(set([(0x412cb0, 0x412d32), (0x4145f0, 0x414663), (0x51fa80, 0x51faa0), (0x412d40, 0x412d4b), (0x401250, 0x40127a),
                      (0x4013e0, 0x40161d), (0x412c90, 0x412ca2), (0x5f5680, 0x5f5690), (0x5f5660, 0x5f5680), (0x67a400, 0x6902c0), (0x412d50, 0x412e59), (0x412a90, 0x412b8b)]
                     + soundset.RANGES)))


def effects(r, def_, value):
    for k in range(4):
        put(def_, 0x4e0 + 4 * k, 'I', r.choice([0, 0, 0x68, 0x69, r.randrange(0x69), r.randrange(0x69), r.getrandbits(32)]))
    put(def_, 0x4f0, 'I', r.choice([value, value, 0, r.getrandbits(32)]))


def case(r):
    op = r.deal(OPS)
    c = soundset.sender_case(r, 0x51b620)
    value = c.args[0]
    regions = dict(c.regions)
    setup = bytearray(r.bytes(0x10))
    put(setup, 0, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    regions.update({SETUP: setup, EFFECTS: r.bytes(0x69 * 0x140)})
    if op == 'Dtor':
        b = bf.defaults(r, 0x412d40, DEF + 4, 0x6fd8a8)
        file = b.regions.pop(DEF + 4)
        def_ = bytearray(r.bytes(0x500))
        def_[4:4 + len(file)] = file
        effects(r, def_, value)
        regions.update(b.regions)
        regions[DEF] = def_
        return Call(0x412cb0, [DEF], regions, conv='t', ret='v')
    if op in ('Free', 'FreeAll'):
        b = bf.defaults(r, 0x412d40, DEF + 4, 0x6fd8a8)
        file = b.regions.pop(DEF + 4)
        regions.update(b.regions)
        n = r.choice([0, 1, 1, 2, 3, 4])
        table = bytearray(r.bytes(0x740))
        for k in range(n):
            def_ = bytearray(r.bytes(0x508))
            def_[4:4 + len(file)] = file
            effects(r, def_, value)
            for t in range(4):
                put(def_, 0x4f4 + 4 * t, 'I', r.choice([0, 0x6fd880, 0x6fd880, r.getrandbits(32)]))
            regions[DEFS + 0x800 * k] = def_
            put(table, 4 * k, 'I', DEFS + 0x800 * k)
        put(table, 4 * n, 'I', 0)
        put(table, 0x528, 'i', r.choice([n, n, n, max(n - 1, 0), 0, -1, -0x80000000]))
        if op == 'Free':
            regions[TABLE] = table
            return Call(0x412d50, [TABLE], regions, conv='t', ret='v')
        regions[0x7890a0] = table
        return Call(0x412a90, [], regions, ret='v')
    n = r.choice([0, 1, 1, 2, 3, 4])
    table = bytearray(r.bytes(0x740))
    for k in range(n):
        def_ = bytearray(r.bytes(0x500))
        effects(r, def_, value)
        regions[DEFS + 0x800 * k] = def_
        put(table, 4 * k, 'I', DEFS + 0x800 * k)
    put(table, 4 * n, 'I', 0)
    put(table, 0x528, 'i', r.choice([n, n, n, n + 1, n + 3, max(n - 1, 0), 0, -1, -0x80000000]))
    regions[TABLE] = table
    return Call(0x4145f0, [TABLE], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Dtor,Effects,Free,FreeAll').split(',') if k]


if __name__ == '__main__':
    B.main(case)
