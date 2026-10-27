# A staff kind's advice test (src/game/world/staff: StaffAdvice_KindLow 0x509360) against the original: the dword +0x64 set now and then;
# the kind 0-4 mostly (others now and then); the world's staff chains (heads +0x1da73c, +0x1da73e, +0x1da740, +0x1da744, +0x1da742 of
# [0x80239c]; each member's next +0x210) of 0-8 members, their floats +0x1f8 and +0x1fc around 15 mostly (fractions, negatives, past 255,
# huge, infinite or NaN now and then); for kind 0 agent C's unported sums 0x4c8180 / 0x4c8150 thiscall recorders (k 16 / 17, driver.cpp's
# kStaffAdviceEnv) answering small counts, 0 or any, on the record of the world's handle +0x1da720.
# Its user (Update: StaffAdvice_Update 0x508f70) with the same surroundings, the world being funnytime.py's (its ticks) and the game
# clock ([[0x7cf6ec]] + 0x2a0) at a rate of 4 (whole seconds) mostly, so its elapsed time is about 24 30-day months (or any); the kind's
# state +0x24 + 12 * kind 0-5 or any and its flag; the message centre [0x788d3c] made, the posts (agent B's unported 0x40fb10) a recorder
# (k 21) hashing the message's 16 bytes.
# Its caller (Tick: StaffAdvice_Tick 0x508e70) with Update's surroundings, see tick(). Its caller (Message: StaffHQ_HandleMessage 0x508a30)
# with Tick's or KindLow's surroundings, see message().
# usage: staffadvice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=KindLow,Update,Tick,Message)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return, vtable  # noqa: E402
sys.path.insert(0, HERE)
import funnytime  # noqa: E402

# (Tick's: Staff_TrainAll and the analyser's count, over no things)
TICK_RANGES = [(0x40f6f0, 0x40f720), (0x40fb10, 0x40fc90), (0x40ff80, 0x4104c0), (0x410610, 0x410730), (0x41a960, 0x41a980), (0x41a990, 0x41a9c0),
               (0x4c7fa0, 0x4c8080), (0x4c8150, 0x4c81c0), (0x4c8ae0, 0x4c8c50), (0x4d01f0, 0x4d0270), (0x4f8800, 0x4f88c0), (0x4fa990, 0x4faa10),
               (0x505a10, 0x505b80), (0x508e70, 0x508f50), (0x50b350, 0x50b370), (0x50b550, 0x50b6b0), (0x50c800, 0x50c9c0), (0x516130, 0x5161c0),
               (0x51a280, 0x51a2a0), (0x5195d0, 0x519620), (0x536310, 0x536370), (0x536390, 0x5363b0), (0x5fc530, 0x5fc580), (0x5fc660, 0x5fc750),
               (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0), (0x67c320, 0x67c3e0), (0x67c7f0, 0x67c840)]
B = Bench([(0x509360, 0x50996d), (0x519510, 0x51954d), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860), (0x508f70, 0x509336), (0x40f6f0, 0x40f710),
           (0x508a30, 0x508ba8), (0x5099c0, 0x509ae0), (0x4d6330, 0x4d6360), (0x4d8410, 0x4d8434)]
          + funnytime.RANGES + TICK_RANGES, hooks={0x4c8180: _recorder(16, 1), 0x4c8150: _recorder(17, 1), 0x40fb10: _recorder(21, 1),
                                              0x4d6260: _recorder(18, 1)})
SELF, WORLD, THINGS, RECORD = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x1800
HEADS = (0x1c, 0x1e, 0x20, 0x24, 0x22)   # (the world's +0x1da73c... from WORLD = its +0x1da720)
SLOTS = 40


def value(r):
    if r.random() < 0.85:
        return r.choice([r.uniform(0, 30), float(r.randrange(31)), r.uniform(10, 20), r.uniform(-5, 300)])
    return r.choice([256.0 + r.randrange(20), -1.5, 1e12, -1e12, float('inf'), float('nan'), 255.99, 14.999])


MONTHS24 = 24 * 30 * 86400   # (seconds)
CLOCK, CLOCK_AT = RAM + 0x200, RAM + 0x300   # (Update's; its world block at funnytime's TICKS + 0x14, past the ticks)


def chains(r, world_at, log):
    """The world block (its +0x1da720) at world_at, its record and chains, and the sums' answers: the regions."""
    world = bytearray(r.bytes(0x30))
    table = bytearray(r.bytes(20 * SLOTS))
    things = bytearray(r.bytes(0x400 * (SLOTS - 1)))
    rec = r.randrange(1, SLOTS)
    put(world, 0, 'H', rec)
    put(table, 20 * rec, 'I', RECORD)
    free = [h for h in range(1, SLOTS) if h != rec]
    r.shuffle(free)
    for head in HEADS:
        n = r.choice([0, 1, 3, 4, 4, 5, 6, 8]) if r.random() < 0.9 else 0
        chain, free = free[:n], free[n:]
        for i, h in enumerate(chain):
            t = 0x400 * (h - 1)
            put(table, 20 * h, 'I', THINGS + t)
            put(things, t + 0x1f8, 'ff', value(r), value(r))
            put(things, t + 0x210, 'H', chain[i + 1] if i + 1 < len(chain) else 0)
        put(world, head, 'H', chain[0] if chain else 0)
    for k in (16, 17):
        set_return(log, k, r.choice([0, 1, 2, 5, 10, 20, 100, 1000, -3, r.getrandbits(32)]))
    return {world_at: world, 0x7cfb90: table, THINGS: things, RECORD: r.bytes(0x10)}


def update(r, self, kind):
    regions = {}
    log = funnytime.clock_regions(r, regions)   # ([0x80239c] = funnytime.WORLD, its ticks)
    regions.update(chains(r, funnytime.TICKS + 0x14, log))
    clock = bytearray(r.bytes(0x40))
    ticks = MONTHS24 + r.choice([-1, 0, 1, 0, -86400, 86400, r.randrange(-MONTHS24, MONTHS24)])
    if r.random() < 0.8:
        put(clock, 0x1c, 'I', 4)
        put(regions[funnytime.TICKS], 0, 'I', ticks & 0xffffffff)
    else:
        put(clock, 0x1c, 'I', funnytime.u32(r))
    for k in (0, 1, 2, 3, 4, 5):   # (the states and flags)
        put(self, 0x24 + 12 * k, 'II', r.choice([0, 1, 2, 3, 4, 5, 1, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]))
    if r.random() < 0.5:
        put(self, 0x64, 'I', 0)
    blob_bytes(log, 21, 0, 0xffff)
    regions.update({SELF: self, CLOCK: clock, CLOCK_AT: struct.pack('<I', CLOCK - 0x2a0), 0x7cf6ec: struct.pack('<I', CLOCK_AT),
                    0x788d3c: struct.pack('<I', RAM + 0x380), LOG: log})
    return Call(0x508f70, [SELF, kind if kind < 6 else r.randrange(5)], regions, conv='t')



def tick(r, self):
    """StaffAdvice_Tick: Update's surroundings; the world's +0x1da710 0 mostly; the kinds' stamps +0x2c + 12 * kind the world's +0x1da70c
    or not (their flags set or not); the analyser's kept count (+0x1f0b8 of the world's record: stamp, count, flag) kept or taken again over
    no things (0x7cf56c 0, as for Staff_TrainAll's walks)."""
    c = update(r, self, 0)
    regions = c.regions
    ticks = regions[funnytime.TICKS]
    put(ticks, 4, 'I', r.choice([0] * 6 + [1, r.getrandbits(32)]))
    month = struct.unpack_from('<I', ticks, 0)[0]
    for k in range(5):
        put(self, 0x2c + 12 * k, 'I', month if r.random() < 0.3 else r.choice([0, month ^ 1, r.getrandbits(32)]))
    cache = bytearray(r.bytes(0xc))
    put(cache, 0, 'I', (month - 4 * r.choice([0, 1, 5, 6, 100])) & 0xffffffff)
    put(cache, 4, 'I', r.choice([0, 0, 0, 1, 7]))
    cache[8] = r.choice([1, 1, 1, 0])
    regions.update({RECORD + 0x1f0b8: cache, 0x7cf56c: struct.pack('<I', 0), 0x7cf83c: struct.pack('<I', RAM + 0x3f00)})
    return Call(0x508e70, [SELF], regions, conv='t')

MSG, MSG_VT, RE = RAM + 0x100, RAM + 0x140, RAM + 0x40000   # (Message's; RE: the RE cells' base, Map_RECell's map + 0x1b1104)


def message(r, self):
    """StaffHQ_HandleMessage: the message's type (its vtable's slot 0, recorder k 0) 0xc, 0xe or another. 0xc: Tick's surroundings. 0xe:
    KindLow's chains (the guards' at +0x1da744) with each thing's state +0x19c 0, 1 or another, the message's handle +8 one of the things
    (or the record), the RE cell of that thing's cell its word +6 above 0 or not; agent C's unported 0x4d6260 a thiscall recorder (k 18
    of 1, driver.cpp's kStaffAdviceEnv)."""
    kind = r.choice([0xc, 0xe, 0xe, 0xe, 0xe, 0xd, 0, r.getrandbits(32)])
    if kind == 0xc:
        regions = tick(r, self).regions
        log = regions[LOG]
        h = r.randrange(0x10000)
    else:
        log = log_region()
        regions = chains(r, WORLD, log)
        regions.update({SELF: self, LOG: log, 0x80239c: struct.pack('<I', WORLD - 0x1da720), 0x8023a0: struct.pack('<I', (RE - 0x1b1104) & 0xffffffff)})
        table, things = regions[0x7cfb90], regions[THINGS]
        for t in range(SLOTS - 1):
            put(things, 0x400 * t + 0x19c, 'I', r.choice([0, 1, 0, 1, 2, 0x12, r.getrandbits(32)]))
        rec = struct.unpack_from('<H', regions[WORLD], 0)[0]
        h = r.randrange(1, SLOTS)
        if h != rec:
            put(table, 20 * h, 'I', THINGS + 0x400 * (h - 1))
            thing = things[0x400 * (h - 1):0x400 * h]
        else:
            thing = regions[RECORD]
        pos = (thing[7] << 7) + thing[5]
        regions[RE + 10 * pos] = put(bytearray(r.bytes(10)), 6, 'h', r.choice([1, 1, 5, 0x7fff, 0, -1, -0x8000]))
    msg = put(bytearray(r.bytes(0x10)), 0, 'I', MSG_VT)
    put(msg, 8, 'H', h)
    set_return(log, 0, kind)
    regions.update({MSG: msg, MSG_VT: vtable({}, 1)})
    return Call(0x508a30, [SELF, MSG], regions, conv='t')


def case(r):
    self = put(bytearray(r.bytes(0x80)), 0x64, 'I', r.choice([0] * 6 + [1, r.getrandbits(32)]))
    kind = r.choice([0, 0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, r.getrandbits(32)])
    op = r.deal(OPS)
    if op == 'Message':
        return message(r, self)
    if op == 'Tick':
        return tick(r, self)
    if op == 'Update':
        return update(r, self, kind)
    log = log_region()
    regions = chains(r, WORLD, log)
    regions.update({SELF: self, LOG: log, 0x80239c: struct.pack('<I', WORLD - 0x1da720)})
    return Call(0x509360, [SELF, kind], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'KindLow,Update,Tick,Message').split(',') if k]


if __name__ == '__main__':
    B.main(case)
