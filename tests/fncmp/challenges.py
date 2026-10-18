# The challenges' orphan drop, current challenge and schema slot (src/game/world/challenges:
# 0x4d1c80, 0x4d1e60, 0x4d2840) against the originals: ids and prerequisites drawn from a few
# values (so that they match, chains drop in order, and some name themselves); current indexes
# about 0 and 20; and the guard's state checks (world/staff: 0x4d6330, 0x4d6350). The offer accepted or declined (0x4d2df0, 0x4d2ec0):
# waiting or not, target times and prizes about 0 and the wraps, independent or not; the world's calendar clock and its imports
# (funnytime.py's), the money record and park through the world's handles (as stafftrain.py). The load from data/challenges.sam
# (0x4d1ce0): the file served as balanceload.py serves a stack balance file (its values block the heap's first, at VALUES), its text
# comment lines or a "Challenges[row].Type n" line, gBalanceLevel's 20 indices rows of the file (1-59; 0 or less skipped); and the
# file's destructor (0x4d1e80) with or without a values block.
# usage: challenges.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Drop,Current,Schema,Guard,Accept,Decline,LoadSam,Dtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE as IAT, LOG, Bench, Call, RAM, fill, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from funnytime import IMPORTS, K, RANGES as CLOCK_RANGES  # noqa: E402
import balanceload  # noqa: E402

FILE_RANGES = [(0x401030, 0x401212), (0x401250, 0x40127a), (0x401280, 0x4013dc), (0x4013e0, 0x40161d), (0x401650, 0x4017a0),
               (0x4017a0, 0x402720), (0x402720, 0x402a1e), (0x405710, 0x405742), (0x5f5660, 0x5f5690), (0x5b0d60, 0x5b0d64),
               (0x412c90, 0x412ca2), (0x4143f0, 0x414402), (0x5f56f0, 0x5f5a30), (0x5f5ad0, 0x5f5bc0), (0x5f5c70, 0x5f5d0e),
               (0x5f5e20, 0x5f5e90), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x6902c0), (0x583280, 0x5832f0), (0x585710, 0x58571d)]
B = Bench([(0x4d1c80, 0x4d1cdc), (0x4d1e60, 0x4d1e74), (0x4d2840, 0x4d285d), (0x4d6330, 0x4d6348),
           (0x4d6350, 0x4d635f), (0x4d2df0, 0x4d2eb7), (0x4d2ec0, 0x4d2fac), (0x4d0190, 0x4d01f0), (0x5195d0, 0x519610),
           (0x519510, 0x519550), (0x5da3c0, 0x5da3c1), (0x4d1ce0, 0x4d1e5e), (0x4d1e80, 0x4d1e8c)] + CLOCK_RANGES + FILE_RANGES)
VALUES = balanceload.VALUES
LEVEL = 0x7857d4   # gBalanceLevel's challenge indices (20 dwords)
SCHEMA = 12        # the BalanceChallenges schema's records: a group of 10 fields (60 rows) and its end


def load_sam(r, mem):
    """Challenges_LoadSam: the file as balanceload serves a stack file; its first heap block (the 12 values entries, 0x90 bytes) at VALUES
    and the next from VALUES + 0x90; the text comment lines, or a Type line for one row; the level's indices rows mostly in range."""
    regions = balanceload.common(r, 'Check', True)
    log = regions[LOG]   # the file always read (found, a few bytes, opened, every allocation served): an unread one leaves the rows
    size = r.choice([3, 16, 40, 40])   # uninitialised stack in the original, which only the emulator zeroes
    set_return(log, balanceload.FIND_FIRST, 0x1234)
    fill(log, balanceload.FIND_FIRST, 1, bytes(0x20) + struct.pack('<I', size) + bytes(0x10), 0)
    fill(log, balanceload.OPEN, 0, struct.pack('<i', 0), 1)
    set_return(log, balanceload.ATTRIBUTES, 0x20)
    put(regions[HEAP], 0, 'I', VALUES)
    block = regions.pop(VALUES + 0x180)
    if r.random() < 0.5:   # (the text a 32-byte line, a block starting at either half: "Challenges[row].Type n" sets that row's mType)
        line = b'Challenges[%d].Type %d' % (r.choice([1, 2, 3, r.randrange(60)]), r.choice([1, 2, 3, 9, -1, r.randrange(100)]))
        line = line + b' ' * (31 - len(line)) + b'\n'
        block[:] = line * (len(block) // 32) + block[len(block) // 32 * 32:]
    regions[VALUES + 0x90] = block[:0x1000]
    rows = [r.randrange(1, 60) for _ in range(20)]
    if r.random() < 0.7:
        rows[r.randrange(20)] = r.choice([0, -1, -0x80000000])
    if r.random() < 0.5:
        rows = [r.choice(rows[:3]) for _ in rows]   # (repeated rows: the same record into successive slots when its mType is set)
    regions[LEVEL] = struct.pack('<20i', *rows)
    regions[RAM] = mem
    return Call(0x4d1ce0, [C], regions, conv='t')


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


def case(r):
    mem = r.bytes(0x600)
    op = r.deal(OPS)
    if op in ('Accept', 'Decline'):
        return offer(r, bytearray(mem), op)
    if op == 'LoadSam':
        return load_sam(r, mem)
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


OPS = [k for k in os.environ.get('OPS', 'Drop,Current,Schema,Guard,Guard,Accept,Decline,LoadSam,LoadSam,Dtor').split(',') if k]

if __name__ == '__main__':
    B.main(case)
