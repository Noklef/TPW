# The balance files of src/game/core/balancefile against the originals: BalanceFile_SetDefaults (0x4013e0) and BalanceFile_Release (0x401250)
# on a file whose vtable is one of the game's balance vtables (the driver points their slot 0 at the ported schema getters), with a made-up schema
# laid over that getter's table (types 0-11 and others, groups of type 2 closed by a type-3 record's count, a type-3 record after the 0xc end so
# a group's search always stops) and values entries of a few strides; and the six schema getters themselves. A theme's record's destructor
# (0x5b0cd0) with its theme balance file (vtable 0x702800) or none and a name of any block.
# usage: balancefile.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Defaults,Release,Schema,Dtor,Init,ObjCtor,Advisor,Find,Theme)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

GETTERS = {0x402a50: 0x73fb40, 0x402b10: 0x73fc70, 0x402b80: 0x743f00, 0x402bf0: 0x7442c0, 0x402c60: 0x744a40, 0x402cd0: 0x746a20, 0x415e90: 0x72e120}
VTABLES = {0x6fd618: 0x73fb40, 0x6fd620: 0x73fc70, 0x6fd628: 0x743f00, 0x6fd630: 0x7442c0, 0x6fd638: 0x744a40, 0x6fd640: 0x746a20,
           0x6fd8a8: 0x744b30, 0x6fd8b0: 0x739ea8, 0x700398: 0x739b98, 0x6fd958: 0x72e120, 0x702800: 0x73f8a8}
DTORS = {0x402a70: (0x784e30, 0x6fd618), 0x402ac0: (0x784c20, 0x6fd618), 0x402b30: (0x785040, 0x6fd620), 0x402ba0: (0x785828, 0x6fd628),
         0x402c10: (0x785868, 0x6fd630), 0x402c80: (0x7858e0, 0x6fd638), 0x402cf0: (0x7858f0, 0x6fd640),
         0x412d40: (0, 0x6fd8a8), 0x414410: (0, 0x6fd8b0), 0x423dc0: (0, 0x6fd630), 0x59bf10: (0, 0x6fd958)}   # the static (global, vtable) and thiscall (0, vtable) destructors
B = Bench([(0x401250, 0x40127a), (0x4013e0, 0x40161d), (0x5f5680, 0x5f5690), (0x412c90, 0x412ca2), (0x4143f0, 0x414402), (0x4d1e60, 0x4d1e72), (0x5da3c0, 0x5da3c1)]
          + [(a, a + 0x12) for a in GETTERS] + [(a, a + 0x10) for a in DTORS] + [(0x401030, 0x401212), (0x5f5660, 0x5f5680), (0x5b0d60, 0x5b0d64), (0x412c20, 0x412c89), (0x415e70, 0x415e8e), (0x401280, 0x4013dc),
           (0x5b0cd0, 0x5b0d34), (0x5b0d40, 0x5b0d54), (0x5f61e0, 0x5f61f3)])
VALS, FLAGS, DATA = RAM + 0x800, RAM + 0x900, RAM + 0x1000
INIT = RAM + 0x4000
GROUPED = [0, 1, 2, 2, 2, 3, 3, 4, 6, 9, 10, 0xd]   # (Init: groups nested, cut short and empty more often)
TYPES = [0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 10, 11, 0xd, 0xffffffff]


def schema(r, counts=(0, 1, 2, 3, 5, -1), kinds=None):
    n = r.choice([0, 1, 2, 4, 6, 8, 10, 12])
    types = [r.choice(kinds or TYPES) for _ in range(n)]
    if 2 in types and (3 not in types or max(k for k, t in enumerate(types) if t == 2) > max(k for k, t in enumerate(types) if t == 3)):
        if types[-1] == 2:
            types.append(3)
        else:
            types[-1] = 3
    types += [0xc, 3]
    table = bytearray(r.bytes(0x3c * len(types)))
    for k, t in enumerate(types):
        put(table, 0x3c * k, 'I', t)
        put(table, 0x3c * k + 0x34, 'i', r.choice(counts))
    return len(types) - 2, table


def defaults(r, address, at=RAM, vt=None):
    vt = vt or r.choice(list(VTABLES))
    n, table = schema(r)
    vals, regions = bytearray(0xc * max(n, 1)), {}
    for k in range(n):
        put(vals, 0xc * k, 'IiI', DATA + 0x80 * k + 0x40, r.choice([0, 1, 1, 2, 3, -1]), r.choice([0, FLAGS + 4 * k]))
        regions[DATA + 0x80 * k] = r.bytes(0x80)
    obj = put(put(bytearray(r.bytes(0x10)), 0, 'I', r.choice([vt, vt, r.getrandbits(32)]) if address in DTORS else vt), 4, 'I',
              r.choice([VALS, VALS, VALS, 0]) if address != 0x4013e0 else VALS)
    regions.update({at: obj, VALS: vals, FLAGS: r.bytes(0x40), VTABLES[vt]: table, HEAP: heap(), **global_heap()})
    if address in DTORS and DTORS[address][0]:
        return Call(address, [], regions, conv='c', ret='v')
    if address == 0x401250 or address in DTORS:
        return Call(address, [RAM], regions, conv='t', ret='v')
    return Call(address, [RAM, r.choice([0, 1, 1, 2, 0xffffffff])], regions, conv='t', ret='v')


def init(r, ctor=False):   # BalanceFile_Init: a file at INIT (its values from +8, room on both sides for the index running off), the made-up schema;
    vt = 0x6fd8a8 if ctor else r.choice(list(VTABLES))   # ctor: ObjectDef_Ctor of the definition whose balance file (+4) is at INIT
    n, table = schema(r, [0, 1, 1, 2, 3, 3, -1], r.choice([None, GROUPED]))
    obj = bytearray(r.bytes(0x2800))
    put(obj, 0x800, 'II', vt, r.getrandbits(32))
    if ctor:
        put(obj, 0x800, 'I', r.getrandbits(32))
    return Call(0x412c20 if ctor else 0x401030, [INIT - 4 if ctor else INIT], {INIT - 0x800: obj, VTABLES[vt]: table, HEAP: heap(), **global_heap()}, conv='t', ret='a' if ctor else 'v')


def advisor(r):   # AdvisorScoring_Dtor (0x415e80, thiscall) or its static one (0x415e70, gAdvisorScoring 0xfb3540): the balance file at +0xc
    address = r.choice([0x415e80, 0x415e70])
    base = 0xfb3540 if address == 0x415e70 else RAM + 0x200
    c = defaults(r, 0x401250, base + 0xc, 0x6fd958)
    regions = c.regions
    regions[base + 0xc] = put(bytearray(regions[base + 0xc]), 0, 'I', r.getrandbits(32))
    regions[base] = r.bytes(0xc)
    if address == 0x415e70:
        return Call(address, [], regions, conv='c', ret='v')
    return Call(address, [base], regions, conv='t', ret='v')


NAMES = [b'Speed', b'Cost', b'Speed2', b'Spee', b'', b'Ride Cost', b'Ride Time', b'cost']
FIND_TYPES = [0, 1, 1, 2, 3, 3, 4, 6, 9, 10]


def theme(r):   # ThemeRecord_Dtor (0x5b0cd0, thiscall): the record at RAM + 0x200, its theme balance file (vtable 0x702800, got back) at +0x1c
    c = defaults(r, 0x401250, RAM + 0x400, 0x702800)   # or none, its name (a String) at +0x20
    regions = c.regions
    regions[RAM + 0x400] = put(bytearray(regions[RAM + 0x400]), 0, 'I', r.getrandbits(32))
    record = bytearray(r.bytes(0x30))
    put(record, 0x1c, 'I', r.choice([RAM + 0x400, RAM + 0x400, 0]))
    put(record, 0x20, 'I', r.getrandbits(32))
    put(record, 0x28, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
    regions[RAM + 0x200] = bytes(record)
    return Call(0x5b0cd0, [RAM + 0x200], regions, conv='t', ret='v')


def find(r):   # BalanceFile_FindKey: names from a few (near misses among them) on a made-up schema, the section and key asked for from them
    vt = r.choice(list(VTABLES))
    pool = r.sample(NAMES, 3)   # (a few names a case, so sections and keys are found often)
    n, table = schema(r, counts=(0, 1, 2, 3, 5, -1), kinds=FIND_TYPES)
    for k in range(n + 2):
        table[0x3c * k + 4:0x3c * k + 0x24] = (r.choice(pool) + bytes(0x20))[:0x20]
    group, name = (r.choice(pool + [b'Other']) + bytes(0x10) for _ in range(2))
    regions = {RAM: put(bytearray(r.bytes(0x20)), 0, 'I', vt), VTABLES[vt]: table, RAM + 0x100: group, RAM + 0x140: name, RAM + 0x180: r.bytes(8)}
    args = [RAM, RAM + 0x100, RAM + 0x140, RAM + 0x180, r.choice([0, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, 2, 3, 5, 0xffffffff, r.getrandbits(32)])]
    return Call(0x401280, args, regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS + [k for k in ('Dtor', 'Schema') if k in OPS])   # (a deck, those two twice: they share out many functions)
    if op == 'Defaults':
        return defaults(r, 0x4013e0)
    if op == 'Release':
        return defaults(r, 0x401250)
    if op == 'Dtor':
        address = r.deal(list(DTORS))
        at, vt = DTORS[address]
        return defaults(r, address, at or RAM, vt)
    if op == 'Init':
        return init(r)
    if op == 'ObjCtor':
        return init(r, True)
    if op == 'Advisor':
        return advisor(r)
    if op == 'Theme':
        return theme(r)
    if op == 'Find':
        return find(r)
    return Call(r.deal(list(GETTERS)), [r.getrandbits(32), r.choice([0, 1, r.randrange(64), r.getrandbits(32)])], {}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Defaults,Release,Schema,Dtor,Init,ObjCtor,Advisor,Find,Theme').split(',') if k]

if __name__ == '__main__':
    B.main(case)
