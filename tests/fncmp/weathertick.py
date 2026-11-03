# The weather's next intensity, constructor and tick (src/game/world/weather: 0x512e10, 0x511f70, 0x512880) against the originals, on weather.py's world: the clock's
# conversion always succeeding (a failure leaves the quarter stack garbage, sic), months of 1-12 so that the quarter stays in the table 0x7854c4.
# - Next: the table's percents about the random numbers (0, 100, any), spreads above 0 or not.
# - Ctor: thing_ctors.py's thing constructor regions, centre (made or not); WeatherFx_ReloadTextures (0x57d850) the recorder 20.
# - Tick: a due message or not (dates of 0 or any past the funny time now); +0x34 negative, 0 or any, +0x30 about it, the step small; the bolts on or off, +0x3c about
#   [0x78551c]; +0x44 1, 0 or below, so that the thunder sounds; the post (0x40fb10, k 21: the message's bytes), agent B's 0x580320 (k 19 of 8) and Sound_SetAmbientLevel (0x51e830,
#   cdecl k 18: its handle would be a wild pointer here) recorders. Divisors 0 now and then (crash in both).
# usage: weathertick.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Next,Ctor,Tick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, RAM, _recorder, blob_bytes, fb, fill, put, set_return  # noqa: E402
import funnytime as ft  # noqa: E402
import thing_ctors  # noqa: E402
import weather as wx  # noqa: E402

CALLS = {'Next': (0x512e10, 'a'), 'Ctor': (0x511f70, 'a'), 'Tick': (0x512880, 'v')}
RANGES = [(0x512e10, 0x512efc), (0x511f70, 0x511ffe), (0x512880, 0x512c2c), (0x4f8770, 0x4f87fc), (0x516330, 0x516390), (0x5127a0, 0x512890), (0x512f00, 0x5132a0), (0x512c50, 0x512e01),
          (0x51bfc0, 0x51c060), (0x51c300, 0x51c360), (0x57d910, 0x57dc80), (0x5856e0, 0x585720), (0x5fc530, 0x5fc560), (0x5fc570, 0x5fc5b0), (0x5fc660, 0x5fc750), (0x5fcc50, 0x5fccf0),
          (0x67c320, 0x67c3e0), (0x67c7f0, 0x67c840), (0x4f8690, 0x4f8740), (0x423b50, 0x423b70), (0x5da3c0, 0x5da3c1),
          (0x40f240, 0x40f2c0), (0x6b5930, 0x6b5960), (0x6b5b80, 0x6b5c00), (0x6b65a0, 0x6b65f0), (0x6b6830, 0x6b68e0), (0x6b6b40, 0x6b6bb0), (0x6b8930, 0x6b89a0), (0x6b8b40, 0x6b8b70),
          (0x4d91f0, 0x4d9290), (0x502d20, 0x502d70), (0x50afe0, 0x50b090), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200), (0x50ffe0, 0x510100), (0x516270, 0x516320), (0x518e00, 0x518e70),
          (0x5d1750, 0x5d1751), (0x67b050, 0x67b0d0)]
B = Bench(RANGES + thing_ctors.CENTRE_RANGES, hooks={**thing_ctors.CENTRE_HOOKS, 0x40fb10: _recorder(21, 1), 0x57d850: _recorder(20, 0), 0x580320: _recorder(19, 8), 0x51e830: _recorder(18, 1, cdecl=True)})
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
K = ft.K['FileTimeToSystemTime']
W, WEATHER = wx.W, wx.WEATHER


def world(r):
    """weather.py's world and renderer; the conversion succeeding with a month of 1-12; the table."""
    regions = dict(wx.world_case(r, 'SetMode').regions)
    log = regions[LOG]
    st = bytearray(r.bytes(16))
    struct.pack_into('<H', st, 2, r.randint(1, 12))
    fill(log, K, 1, bytes(st), 0)
    set_return(log, K, 1)
    table = bytearray(r.bytes(0x3c))
    for row in range(5):
        put(table, 12 * row + 4, 'I', r.choice([0, 0, 1, 7, 30, r.randrange(1 << 20), 0x80000000, 0xffffffff, 0x40000000]))
        put(table, 12 * row + 8, 'I', r.choice([0, 100, 101, r.randrange(101), r.getrandbits(32)]))
    regions[0x7854c4] = table
    return regions, log


def tick(r, regions, log):
    w = bytearray(regions[WEATHER])
    w[0x2c] = r.choice([0, 0, 1, r.getrandbits(8)])
    put(w, 0x24, 'I', r.choice([0, 0x7fffffff, 0xffffffff, r.getrandbits(32)]))   # (+0x20's high dword: due or not)
    put(w, 0x14, 'I', r.choice([0, 0x7fffffff, 0xffffffff, r.getrandbits(32)]))   # (+0x10's)
    put(w, 0x18, 'I', r.getrandbits(32))
    target = r.choice([-1, 0, 0, 5, 50, r.randrange(200), r.getrandbits(32)])
    put(w, 0x34, 'I', target & 0xffffffff)
    put(w, 0x30, 'I', r.choice([0, 1, 2, target, target + 1, target - 1, target + 5, target - 5, r.randrange(200), r.getrandbits(32)]) & 0xffffffff)
    w[0x38] = r.choice([0, 1, 1, r.getrandbits(8)])
    put(w, 0x3c, 'I', r.choice([0, 1, 5, r.randrange(100), r.getrandbits(32)]))
    put(w, 0x40, 'I', r.choice([0, 1, r.randrange(10), r.getrandbits(32)]))
    put(w, 0x44, 'I', r.choice([1, 1, 2, 0, 0xffffffff, r.randrange(100), r.getrandbits(32)]))
    for at in (0x48, 0x4c, 0x50):
        put(w, at, 'I', r.choice([fb(r.uniform(-500, 500)), fb(0.0), r.getrandbits(32)]))
    regions[WEATHER] = w
    block = bytearray(regions[0x785500])
    put(block, 0, 'i', r.choice([0, 1, 5, 5, r.randrange(50), -1]))   # (the step)
    put(block, 0x1c, 'I', r.choice([0, 1, 5, r.randrange(100), r.getrandbits(32)]))   # (0x78551c)
    put(block, 0x20, 'I', r.choice([1, 2, 100, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, 0]))
    put(block, 0x28, 'I', r.choice([0, 3, r.randrange(100), r.getrandbits(32)]))
    put(block, 0x2c, 'I', r.choice([1, 2, 100, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, 0]))
    put(block, 0x30, 'I', r.choice([1, 100, 500, 1000, 0x7fffffff, r.getrandbits(31) | 1, r.getrandbits(31) | 1, 0]))
    regions[0x785500] = block
    regions[0x7a0314] = struct.pack('<I', wx.size(r))
    regions[0x7a0374] = struct.pack('<I', wx.size(r))
    while regions.get(0x788d3c, b'') in (b'', bytes(4)):   # (made already: the original's inlined making copies stack garbage, sic)
        thing_ctors.centre(r, regions, empty=True)
    blob_bytes(log, 21, 0, 0x0f0f if w[0x2c] else 0xff0f)   # (message 1 ends at +0xc; message 2 has a +0xc)


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    regions, log = world(r)
    if name == 'Tick':
        tick(r, regions, log)
    elif name == 'Ctor':
        keep = {k: regions[k] for k in (0x80239c, 0x7cf83c, 0x8bcbac, W + 0x2a0, W + 0x1da708, 0x7854c4, ft.LOG, ft.IAT)}
        regions.update(thing_ctors.thing_regions(r))
        regions.update(keep)
        regions[WEATHER] = r.bytes(0x70)
        thing_ctors.centre(r, regions)
    return Call(address, [WEATHER], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
