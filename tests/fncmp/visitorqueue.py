# A visitor's steps to and in an object's queue (src/game/world/visitorsteps: Visitor_StepWalkToQueue 0x4ffbc0, Visitor_StepInQueue 0x5006b0) against
# the originals, on visitorqueueedited.py's park (objectmsg.py's queue, the ride records, visitorleave.py's Visitor_SendAway, the thoughts) with
# thingenv.py's definitions for the object's base rating and fill rate. Recorders (driver.cpp's VisitorStepsEnv): Sprite_WalkNav 1 of 0,
# Sprite_SetDestinationCell 3 of 1 (a word), Object_BackOfQueueTo 6 of 1 (returning a word's address, filling its out word), Visitor_SetState 26 of 1,
# Visitor_FindQueueDestination 11 of 0, Visitor_ChooseMinorDest 12 of 0, Visitor_RatingGap 13 of 1, Visitor_WontPay 14 of 1; the thoughts' as thoughtshow.py.
#   Walk: the answer 0 mostly (or 2, others), the world's +0x1da710, the counter +0x2c about 11, +0x1dc 0 now and then, the back of the queue
#   the visitor's own cell mostly; the limit +0x40 small or large (the room), a rating gap about 45 either way; the object out of the queue (a
#   visitor walking to it), the flag +0x32 & 8 for the fill rate.
#   Step: the answer 0 or 2 mostly, won't pay or will; the object's state +0x19c, +0x68 and the element 0 of its ride record for the claim.
#   Wait (Visitor_StepQueueWait 0x4ffff0): the place +0x1f1 its queue position mostly, the call +0x1f8 and the object's word +0x6c its handle; the clocks +0x208, +0x1fc against the world's +0x1da70c
#   about their limits, the moods; Sprite_SetDestination 2 of 2 (low words).
# usage: visitorqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Walk,Step,Wait)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import thingenv  # noqa: E402
import visitorqueueedited as edited  # noqa: E402
from fncmp import LOG, LOG_BLOBS, LOG_BYTES, RAM, Bench, _recorder, fb, fill, put, set_return, strings  # noqa: E402
from objectmsg import OBJ  # noqa: E402
from thingenv import POOL  # noqa: E402

B = Bench([(0x4ffbc0, 0x4fff13), (0x5006b0, 0x50086a), (0x4e1670, 0x4e168b), (0x5012f0, 0x50138d), (0x4dd4e0, 0x4dd510), (0x4dda20, 0x4ddb90),
           (0x4ddb90, 0x4ddd20), (0x4ddd20, 0x4ddf50), (0x4ddf50, 0x4de040), (0x4e0860, 0x4e0a70), (0x4e0a70, 0x4e0b90), (0x4e2670, 0x4e2720), (0x4faa00, 0x4faa70),
           (0x4754e0, 0x475550), (0x4fea70, 0x4feb20), (0x502430, 0x502460), (0x50b350, 0x50b370), (0x50be80, 0x50c075), (0x50c0d0, 0x50c1a0),
           (0x519610, 0x519660), (0x51bfc0, 0x51c060), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1), (0x412e90, 0x412f3b),
           (0x51a280, 0x51a2f0), (0x4fdc60, 0x4fdc89), (0x67a830, 0x67a860),
           (0x4ffff0, 0x5006b0), (0x4dedf0, 0x4df150), (0x4e0370, 0x4e03c8), (0x4e2b50, 0x4e2b80), (0x441970, 0x4419b0), (0x436860, 0x436880), (0x4217f0, 0x421810), (0x4fc800, 0x4fc8a0), (0x516330, 0x516380)],
          hooks={0x4fa2a0: _recorder(1, 0), 0x4fa530: _recorder(3, 1), 0x4fa5f0: _recorder(2, 2), 0x4de110: _recorder(6, 1), 0x501db0: _recorder(26, 1), 0x501160: _recorder(11, 0),
                 0x4fd570: _recorder(12, 0), 0x4fd4e0: _recorder(13, 1), 0x4fde50: _recorder(14, 1), 0x550ca0: _recorder(0, 0),
                 0x67a7d0: _recorder(1, 1, cdecl=True), 0x475550: _recorder(2, 1, cdecl=True), 0x475a10: _recorder(3, 7, cdecl=True)})
CALLS = {'Walk': 0x4ffbc0, 'Step': 0x5006b0, 'Wait': 0x4ffff0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
BACK, WORLD = RAM + 0x43000, RAM + 0x44000


def next_rand(seed):
    """World_Rand's next answer from this seed (0x516330)."""
    n = (seed * 0x19660d + 0x3c6ef35f) & 0xffffffff
    n = (n >> 13 | n << 19) & 0xffffffff
    return 0x100000000 - n if n >> 31 else n


def outside(c):
    """Whether the case's visitor is outside the queue."""
    t = c.args[0] - POOL
    pool = c.regions[POOL]
    h, slots = struct.unpack_from('<H', c.regions[OBJ], 0x3c)[0], c.regions[0x7cfb90]
    while h:
        at = struct.unpack_from('<I', slots, 20 * h)[0] - POOL
        if at == t:
            return False
        h = struct.unpack_from('<H', pool, at + 0x228)[0]
    return True


def case(r):
    op = r.deal(OPS)
    c = edited.case(r)
    if op == 'Walk':
        while not outside(c):   # (a visitor added a second time would make a loop of the queue)
            c = edited.case(r)
    log = c.regions[LOG]
    regions = c.regions
    t = c.args[0] - POOL
    pool = regions[POOL]
    obj = regions[OBJ] = regions[OBJ] + bytearray(r.bytes(0x200))
    defs, _, _ = thingenv.park(r, count=0)
    regions[thingenv.DEFS], items = defs[thingenv.DEFS], bytearray(defs[thingenv.ITEMS])
    regions[thingenv.ITEMS] = items
    item = r.randrange(struct.unpack_from('<i', defs[thingenv.DEFS], 0x528)[0])
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, item * thingenv.ITEM)[0])
    put(items, item * thingenv.ITEM + 0x13c, 'I', r.choice([0, 1, 50, 0x100, r.getrandbits(32)]))
    obj[0x32] = r.choice([0, 0, 1, 8, r.getrandbits(8)])
    put(obj, 0x50, 'B', r.randrange(4))
    put(obj, 0x58, 'I', r.choice([0, 1, 5, r.randrange(1000)]))
    put(obj, 0x68, 'I', r.choice([0, 1, 1, 1]))
    put(obj, 0x19c, 'I', r.choice([0, 0, 2, 3, 1, 4]))
    # (the recorders' answers and fills)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 3, 0x10000)
    set_return(log, 1, r.choice([0, 0, 0, 0, 0, 0, 2, 1, 3]) if op == 'Walk' else r.choice([0, 0, 0, 2, 1]))
    set_return(log, 3, r.choice([0, 1, 1]))
    set_return(log, 11, r.choice([0, 1]))
    set_return(log, 13, r.choice([0, 44, 45, 46, -44, -45, -46, 50, -50, 100, r.randrange(-50, 51)]))
    set_return(log, 14, r.choice([0, 0, 1, 2]))
    pool[t + 5], pool[t + 7] = r.randrange(128), r.randrange(128)
    own = (pool[t + 7] << 7) + pool[t + 5] + 1
    set_return(log, 6, BACK)
    strings(log, 6, 0, out=1)
    fill(log, 6, 0, struct.pack('<H', r.choice([0, own, r.getrandbits(16)])), which=0)
    regions[BACK] = struct.pack('<H', own if r.random() < 0.85 else own ^ r.randrange(1, 0x10000))
    pool[t + 0x2c] = r.choice([0, 5, 10, 11, 12, 0xff, r.getrandbits(8)])
    # (the world's +0x1da710, read by Get1da710)
    regions[0x80239c] = struct.pack('<I', WORLD - 0x1da000)
    regions[WORLD] = bytearray(r.bytes(0x1000))
    put(regions[WORLD], 0x710, 'I', r.choice([0, 0, 0, 1]))
    if op == 'Walk' and r.random() < 0.05:
        put(pool, t + 0x1dc, 'H', 0)
    if op == 'Wait':
        hs = struct.unpack_from('<H', pool, t)[0]
        # (k 2 is Sprite_SetDestination here: visitorleave.py's blob of the sound manager's k 2 read its y as a pointer, so the told-to-come path always
        # faulted in both. The sound stays off instead: Away covers it.)
        struct.pack_into('<II', log, LOG_BLOBS + 8 * 2, 0, 0)
        regions[0x802bc0] = put(bytearray(regions[0x802bc0]), 8, 'I', 0)
        struct.pack_into('<I', log, LOG_BYTES + 4 * 2, 0x30000)   # (x, y: low words)
        set_return(log, 2, r.choice([0, 1, 1]))
        h, place, slots = struct.unpack_from('<H', obj, 0x3c)[0], 0, regions[0x7cfb90]
        while h:   # (its place in the queue)
            at = struct.unpack_from('<I', slots, 20 * h)[0] - POOL
            if at == t:
                break
            place += 1
            h = struct.unpack_from('<H', pool, at + 0x228)[0]
        pool[t + 0x1f1] = r.choice([place, place, place, place, 0, place + 1, place + 2, place + 3, r.getrandbits(8)]) & 0xff
        put(pool, t + 0x1f4, 'I', r.choice([0, 0, 1, 2]))
        put(pool, t + 0x1f8, 'I', r.choice([0, 0, 1]))
        put(obj, 0x6c, 'H', r.choice([hs, hs, 0, hs ^ 1]))
        put(obj, 0x2c, 'I', r.choice([0, 0, 1]))
        put(obj, 0x44, 'I', fb(r.choice([0.0, 10.0, 49.0, 50.0, 51.0, 80.0, 100.0, r.uniform(0, 256)])))
        now = r.getrandbits(32)
        put(regions[WORLD], 0x70c, 'I', now)
        put(pool, t + 0x208, 'I', (now - r.choice([0, 1, 0x1e, 0x1e, 0x1f, 0x1f, 0x100])) & 0xffffffff)
        put(pool, t + 0x1fc, 'I', (now - 0x64 + r.choice([-1, -1, 0, 0, 1, 1000])) & 0xffffffff)
        put(pool, t + 0x19c, 'I', fb(r.choice([0.0, 5.0, 9.9, 10.0, 19.9, 20.0, 50.0, 80.0, 80.9, 81.0, 256.0, 300.0, r.uniform(0, 100)])))
        put(pool, t + 0x1ac, 'I', fb(r.choice([0.0, 50.0, 80.0, 81.0, 300.0, r.uniform(0, 100)])))
        # (the heading near either end, so that a turn wraps both ways; the world's generator on a tenth half the time, so that it turns)
        put(pool, t + 0x1c, 'i', r.choice([r.randrange(0x800), r.randrange(400), 0x7ff - r.randrange(400)]))
        seed = r.getrandbits(32)
        while r.random() < 0.9 and next_rand(seed) % 10:
            seed = r.getrandbits(32)
        put(regions[WORLD], 0x708, 'I', seed)
        # (the definition's word +0x9c: 1 with the object's +0x2c set or not; 3 the linked kind)
        put(items, item * thingenv.ITEM + 0x9c, 'I', r.choice([0, 1, 1, 3]))
    c.address = CALLS[op]
    return c


if __name__ == '__main__':
    B.main(case)
