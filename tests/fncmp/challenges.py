# The challenges' orphan drop, current challenge and schema slot (src/game/world/challenges:
# 0x4d1c80, 0x4d1e60, 0x4d2840) against the originals: ids and prerequisites drawn from a few
# values (so that they match, chains drop in order, and some name themselves); current indexes
# about 0 and 20; and the guard's state checks (world/staff: 0x4d6330, 0x4d6350). The offer accepted or declined (0x4d2df0, 0x4d2ec0):
# waiting or not, target times and prizes about 0 and the wraps, independent or not; the world's calendar clock and its imports
# (funnytime.py's), the money record and park through the world's handles (as stafftrain.py). The load from data/challenges.sam
# (0x4d1ce0): the file served as balanceload.py serves a stack balance file (its values block the heap's first, at VALUES), its text
# comment lines or a "Challenges[row].Type n" line, gBalanceLevel's 20 indices rows of the file (1-59; 0 or less skipped); and the
# file's destructor (0x4d1e80) with or without a values block. The constructor (0x4d1bd0): the thing made (thing_ctors.py's thing table), added to the
# message centre (empty maps), then that load. A challenge finished (0x4d1a00): its kind (+0) one Kind_ToIdC maps or not, named by another
# (+0x18) or not, its +0x14 any; the message centre made or not (thing_ctors.py's), the post (agent B's 0x40fb10 through the objects' env)
# recorder 21 logging the message's bytes 0-0x1f but 0x11-0x13 (the stack's).
# usage: challenges.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Drop,Current,Schema,Guard,Accept,Decline,LoadSam,Dtor,Ctor,Finish)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE as IAT, LOG, Bench, Call, RAM, _recorder, blob_bytes, fill, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from funnytime import IMPORTS, K, RANGES as CLOCK_RANGES  # noqa: E402
import balanceload  # noqa: E402
import thing_ctors  # noqa: E402

FILE_RANGES = [(0x401030, 0x401212), (0x401250, 0x40127a), (0x401280, 0x4013dc), (0x4013e0, 0x40161d), (0x401650, 0x4017a0),
               (0x4017a0, 0x402720), (0x402720, 0x402a1e), (0x405710, 0x405742), (0x5f5660, 0x5f5690), (0x5b0d60, 0x5b0d64),
               (0x412c90, 0x412ca2), (0x4143f0, 0x414402), (0x5f56f0, 0x5f5a30), (0x5f5ad0, 0x5f5bc0), (0x5f5c70, 0x5f5d0e),
               (0x5f5e20, 0x5f5e90), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x6902c0), (0x583280, 0x5832f0), (0x585710, 0x58571d)]
B = Bench([(0x4d1c80, 0x4d1cdc), (0x4d1e60, 0x4d1e74), (0x4d2840, 0x4d285d), (0x4d6330, 0x4d6348),
           (0x4d6350, 0x4d635f), (0x4d2df0, 0x4d2eb7), (0x4d2ec0, 0x4d2fac), (0x4d0190, 0x4d01f0), (0x5195d0, 0x519610),
           (0x519510, 0x519550), (0x5da3c0, 0x5da3c1), (0x4d1ce0, 0x4d1e5e), (0x4d1e80, 0x4d1e8c), (0x4d1bd0, 0x4d1c73), (0x4d91f0, 0x4d9290),
           (0x50afe0, 0x50b090), (0x50b350, 0x50b370), (0x516270, 0x516320), (0x67b050, 0x67b0d0), (0x4d1a00, 0x4d1add),
           (0x409cd0, 0x409cdd), (0x6bb1a0, 0x6bb1a4), (0x4d3500, 0x4d3670), (0x40f7a0, 0x40f803)] + CLOCK_RANGES + FILE_RANGES + thing_ctors.CENTRE_RANGES,
          hooks={**thing_ctors.CENTRE_HOOKS, 0x40fb10: _recorder(21, 1)})
VALUES = balanceload.VALUES
LEVEL = 0x7857d4   # gBalanceLevel's challenge indices (20 dwords)
SCHEMA = 12        # the BalanceChallenges schema's records: a group of 10 fields (60 rows) and its end
FIELDS = ['Type', 'FollowupType', 'TargetTime', 'TargetVal', 'TargetObj', 'TargetObj2', 'TargetStaffType', 'Prize', 'CheckAtEndOnly', 'Independent']


def load_sam(r, mem):
    """Challenges_LoadSam: the file as balanceload serves a stack file; its first heap block (the 12 values entries, 0x90 bytes) at VALUES
    and the next from VALUES + 0x90; the text comment lines, or lines setting a row's Type and a field; the level's indices rows in range,
    the typed row's often or only, or none positive."""
    regions = balanceload.common(r, 'Check', True)
    log = regions[LOG]   # the file always read (found, a few bytes, opened, every allocation served): an unread one leaves the rows
    size = r.choice([16, 40, 96, 96, 200])   # uninitialised stack in the original, which only the emulator zeroes
    set_return(log, balanceload.FIND_FIRST, 0x1234)
    fill(log, balanceload.FIND_FIRST, 1, bytes(0x20) + struct.pack('<I', size) + bytes(0x10), 0)
    fill(log, balanceload.OPEN, 0, struct.pack('<i', 0), 1)
    set_return(log, balanceload.ATTRIBUTES, 0x20)
    put(regions[HEAP], 0, 'I', VALUES)
    block = regions.pop(VALUES + 0x180)
    typed = None
    if r.random() < 0.7:   # (the text two 48-byte lines in turn, the block starting at a line: "Challenges[row].Field value" sets a field of a
        typed = r.choice([1, 2, 3, r.randrange(60)])   # row; the first line a Type, so that the row's records fill successive slots)
        lines = []
        for field in ('Type', r.choice(FIELDS + ['CheckAtEndOnly', 'Independent', 'TargetObj', 'TargetObj2', 'TargetObj2'])):
            value = r.choice([1, 2, 3, 9, -1, 0x12345, r.randrange(100), r.getrandbits(31)])
            line = b'Challenges[%d].%s %d' % (r.choice([typed, typed, r.randrange(60)]), field.encode(), value)
            lines.append(line + b' ' * (47 - len(line)) + b'\n')
        text = b''.join(lines)
        block[:] = text * (len(block) // 96) + block[len(block) // 96 * 96:]
    regions[VALUES + 0x90] = block[:0x1000]
    rows = [r.randrange(1, 60) for _ in range(20)]
    mode = r.random()
    if typed is not None and mode < 0.3:
        rows = [typed] * 20   # (every slot filled when the row's mType is set)
    elif typed is not None and mode < 0.6:
        rows = [r.choice([typed, typed, typed, r.randrange(1, 60), 0, -1]) for _ in range(20)]
    elif mode < 0.7:
        rows = [r.choice([0, 0, -1, -0x80000000]) for _ in range(20)]   # (none positive: nothing copied, a 0 not row 0)
    elif mode < 0.85:
        rows[r.randrange(20)] = r.choice([0, -1, -0x80000000])
    regions[LEVEL] = struct.pack('<20i', *rows)
    regions[RAM] = mem
    return Call(0x4d1ce0, [C], regions, conv='t')


def ctor(r, mem):
    """Challenges_Ctor (0x4d1bd0): the thing constructed (thing_ctors.py's thing table, its map cells moved clear of the file's texts), added to the
    message centre's three slots (their maps empty, so each add takes one 0x20-byte node from the heap: the heap starts 0x60 below the file's values
    block so that the load finds it where load_sam put it), then the load as load_sam serves it."""
    c = load_sam(r, mem)
    things = thing_ctors.thing_regions(r, cells=RAM + 0x24000)
    for k in (HEAP, RAM, RAM + 0x300):   # (the file's heap; the object's block; Thing_Add's out, which the constructor does not use)
        things.pop(k, None)
    c.regions.update(things)
    thing_ctors.centre(r, c.regions, empty=True)
    put(c.regions[HEAP], 0, 'I', VALUES - 0x60)
    c.address, c.ret = 0x4d1bd0, 'a'
    return c


def dtor(r, mem):
    """BalanceChallenges_Dtor: the file at C with a values block (12 entries: values at RAM + 0x3000 + 0x200 * j, a stride of 1 or 2) or none."""
    log = log_region()
    iat = import_table(log, {})
    for at, v in global_heap().items():
        iat[at - IAT:at - IAT + 4] = v
    regions = {IAT: iat, LOG: log, HEAP: heap(0x100), RAM: mem}
    put(mem, 0x100, 'I', r.getrandbits(32))
    if r.random() < 0.8:
        entries = bytearray(0xc * SCHEMA)
        for j in range(SCHEMA):
            put(entries, 0xc * j, 'IiI', RAM + 0x3000 + 0x200 * j, r.choice([1, 1, 2]), RAM + 0x5000 + 4 * j)
            regions[RAM + 0x3000 + 0x200 * j] = r.bytes(0x200)
        regions[RAM + 0x5000] = r.bytes(0x40)
        regions[RAM + 0x1000] = entries
        put(mem, 0x104, 'I', RAM + 0x1000)
    else:
        put(mem, 0x104, 'I', 0)
    return Call(0x4d1e80, [C], regions, conv='t')
C = RAM + 0x100
MONEY, PARK, WORLD = RAM + 0x1000, RAM + 0x40000, RAM + 0x80000
DAYS = [0, 1, 7, 30, 365, -1, -30, 0x7fffffff, -0x80000000, 0x10000]
AMOUNTS = [0, 1, 2, 3, 1000, 5001, -1, -2, -3, 0x7fffffff, -0x80000000]


def offer(r, mem, op):
    """Accept / Decline: the challenges in mem at C, the world (clock, ticks, handles), the money record and park, the imports."""
    mem[0x100 + 0x3da] = r.choice([0, 1, 1, 1, 0xff])
    for i in range(0x14):
        c = 0x100 + 0xc + i * 0x30
        put(mem, c + 4, 'i', r.choice(DAYS + [r.randrange(-1000, 1000)]))
        put(mem, c + 0x14, 'i', r.choice(AMOUNTS + [r.randrange(-100000, 100000)]))
        mem[c + 0x1d] = r.choice([0, 0, 1, 0xff])
    index = r.choice([0, 1, 19, r.randrange(20)] + ([] if op == 'Decline' and mem[0x100 + 0x3da] else [-1, 20, r.getrandbits(32) - (1 << 31)]))
    put(mem, 0x100 + 0x3cc, 'i', index)   # (out of range only where the decline would not read it: the original faults)
    log = log_region()
    fill(log, K['FileTimeToSystemTime'], 1, r.bytes(16), 0)
    set_return(log, K['FileTimeToSystemTime'], r.choice([0, 1]))
    clock = bytearray(r.bytes(0x20))
    put(clock, 0x1c, 'I', r.choice([15000, 1, 0, r.getrandbits(32)]))
    world = bytearray(r.bytes(0x40))
    put(world, 0xc, 'I', r.choice([0, 4, r.randrange(1 << 20), r.getrandbits(32)]))   # the ticks, +0x1da70c
    put(world, 0x20, 'H', 2)   # the park, +0x1da720 ([0x7cf83c]'s too)
    put(world, 0x26, 'H', 1)   # the money record, +0x1da726
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * 1, 'I', MONEY)
    put(slots, 20 * 2, 'I', PARK)
    regions = {RAM: mem, LOG: log, IAT: import_table(log, IMPORTS), WORLD + 0x2a0: clock, WORLD + 0x1da700: world, 0x7cfb90: slots,
               MONEY: r.bytes(0x200), PARK + 0x1fc90: r.bytes(4), 0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD),
               0x7857c4: struct.pack('<i', r.choice(DAYS))}
    return Call(0x4d2df0 if op == 'Accept' else 0x4d2ec0, [C], regions, conv='t')


def finish(r, mem):
    """Challenge_Finish (0x4d1a00) on one of the 20 challenges."""
    at = 0x100 + 0xc + 0x30 * r.randrange(0x14)
    put(mem, at, 'I', r.choice([r.randrange(0x30), r.randrange(0x170), r.getrandbits(32)]))
    put(mem, at + 0x18, 'I', r.choice([0, 0, 0, 1, 0x10000, r.getrandbits(32)]))
    put(mem, at + 0x2c, 'B', r.choice([0, 1, 0xff]))
    regions = {RAM: mem, HEAP: heap(), LOG: log_region()}
    thing_ctors.centre(r, regions)
    blob_bytes(regions[LOG], 21, 0, 0xfff1ffff)
    return Call(0x4d1a00, [RAM + at], regions, conv='t')


def case(r):
    mem = r.bytes(0x600)
    op = r.deal(OPS)
    if op == 'Finish':
        return finish(r, bytearray(mem))
    if op in ('Accept', 'Decline'):
        return offer(r, bytearray(mem), op)
    if op == 'LoadSam':
        return load_sam(r, mem)
    if op == 'Ctor':
        return ctor(r, mem)
    if op == 'Dtor':
        return dtor(r, bytearray(mem))
    op = ['Drop', 'Current', 'Schema'].index(op) if op != 'Guard' else 3
    if op == 0:
        k = r.choice([3, 6, 25, 1 << 32])
        for i in range(0x14):
            put(mem, 0x100 + 0xc + i * 0x30, 'I', r.choice([0, r.randrange(k)]) if k < 1 << 32 else r.getrandbits(32))
            put(mem, 0x100 + 0xc + i * 0x30 + 0x18, 'I', r.choice([0, r.randrange(min(k, 30))]))
        return Call(0x4d1c80, [C], {RAM: mem}, conv='t')
    if op == 1:
        put(mem, 0x100 + 0x3cc, 'i', r.choice([-1, 0, 1, 19, 20, 21, r.randrange(-3, 23), r.getrandbits(32) - (1 << 31)]))
        return Call(0x4d2840, [C], {RAM: mem}, conv='t', ret='a')
    if op == 2:
        return Call(0x4d1e60, [C, r.choice([0, 1, r.randrange(30), r.getrandbits(32)])], {RAM: mem}, conv='t', ret='a')
    put(mem, 0x100 + 0x19c, 'I', r.choice([0, 1, 2, 0x12, 0x13, r.randrange(30), r.getrandbits(32), 0x100, 0x112]))
    return Call(r.choice([0x4d6330, 0x4d6350]), [C], {RAM: mem}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Drop,Current,Schema,Guard,Guard,Accept,Decline,LoadSam,LoadSam,Dtor,Ctor,Finish,Finish').split(',') if k]

if __name__ == '__main__':
    B.main(case)
