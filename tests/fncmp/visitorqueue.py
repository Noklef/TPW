# A visitor's steps to and in an object's queue (src/game/world/visitorsteps: Visitor_StepWalkToQueue 0x4ffbc0, Visitor_StepInQueue 0x5006b0) against
# the originals, on visitorqueueedited.py's park (objectmsg.py's queue, the ride records, visitorleave.py's Visitor_SendAway, the thoughts) with
# thingenv.py's definitions for the object's base rating and fill rate. Recorders (driver.cpp's VisitorStepsEnv): Sprite_WalkNav 1 of 0,
# Sprite_SetDestinationCell 3 of 1 (a word), Object_BackOfQueueTo 6 of 1 (returning a word's address, filling its out word), Visitor_SetState 26 of 1,
# Visitor_FindQueueDestination 11 of 0, Visitor_ChooseMinorDest 12 of 0, Visitor_RatingGap 13 of 1, Visitor_WontPay 14 of 1; the thoughts' as thoughtshow.py.
#   Walk: the answer 0 mostly (or 2, others), the world's +0x1da710, the counter +0x2c about 11, +0x1dc 0 now and then, the back of the queue
#   the visitor's own cell mostly; the limit +0x40 small or large (the room), a rating gap about 45 either way; the object out of the queue (a
#   visitor walking to it), the flag +0x32 & 8 for the fill rate.
#   Step: the answer 0 or 2 mostly, won't pay or will; the object's state +0x19c, +0x68 and the element 0 of its ride record for the claim.
# usage: visitorqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Walk,Step)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import thingenv  # noqa: E402
import visitorqueueedited as edited  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, _recorder, fill, put, set_return, strings  # noqa: E402
from objectmsg import OBJ  # noqa: E402
from thingenv import POOL  # noqa: E402

B = Bench([(0x4ffbc0, 0x4fff13), (0x5006b0, 0x50086a), (0x4e1670, 0x4e168b), (0x5012f0, 0x50138d), (0x4dd4e0, 0x4dd510), (0x4dda20, 0x4ddb90),
           (0x4ddb90, 0x4ddd20), (0x4ddd20, 0x4ddf50), (0x4ddf50, 0x4de040), (0x4e0860, 0x4e0a70), (0x4e0a70, 0x4e0b90), (0x4e2670, 0x4e2720), (0x4faa00, 0x4faa70),
           (0x4754e0, 0x475550), (0x4fea70, 0x4feb20), (0x502430, 0x502460), (0x50b350, 0x50b370), (0x50be80, 0x50c075), (0x50c0d0, 0x50c1a0),
           (0x519610, 0x519660), (0x51bfc0, 0x51c060), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1), (0x412e90, 0x412f3b),
           (0x51a280, 0x51a2f0), (0x4fdc60, 0x4fdc89), (0x67a830, 0x67a860)],
          hooks={0x4fa2a0: _recorder(1, 0), 0x4fa530: _recorder(3, 1), 0x4de110: _recorder(6, 1), 0x501db0: _recorder(26, 1), 0x501160: _recorder(11, 0),
                 0x4fd570: _recorder(12, 0), 0x4fd4e0: _recorder(13, 1), 0x4fde50: _recorder(14, 1), 0x550ca0: _recorder(0, 0),
                 0x67a7d0: _recorder(1, 1, cdecl=True), 0x475550: _recorder(2, 1, cdecl=True), 0x475a10: _recorder(3, 7, cdecl=True)})
CALLS = {'Walk': 0x4ffbc0, 'Step': 0x5006b0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
BACK, WORLD = RAM + 0x43000, RAM + 0x44000


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
    c.address = CALLS[op]
    return c


if __name__ == '__main__':
    B.main(case)
