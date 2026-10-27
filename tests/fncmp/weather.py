# The weather (src/game/world/weather) against the originals. The lightning bolt ends (0x512c50): the map's size in cells ([0x7a0314] /
# [0x7a0374]: 0, 1, the game's 128-256, odd ones, past 2^24 / 10 and any), any world seed (World_Rand on [0x7cf83c]), the end apart
# from the start or overlapping it.
# The mode, intensity and message handler (0x5131f0, 0x512f00, 0x5127a0): the world at W (its clock +0x2a0 funnytime.py's, its random
# state and ticks at +0x1da708), the weather renderer at FX (+0 any bits; +0x3310-+0x3360 and the landscape's points as weatherfx.py's
# wind), the three ranges at 0x785504 (the game's 1-100 split, overlapping, empty, reversed, extreme) and the most drops 0x785530 any;
# the angle +0x5c whole or odd degrees from -1500 to 1500 (a NaN, an infinity or a huge one loops for ever in both); the sound +0x54
# none (sound on: the manager's slot 2, the play command hashed) or one (stopped: the manager's engine, no mixer, its own slot 6);
# the message a recorder vtable (slot 0 answering 1, 2 or any), its +8 any value, +0xc any days; the byte +0x64 any way.
# usage: weather.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Bolt,SetMode,SetIntensity,Message)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, blob, fb, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x512c50, 0x512e01), (0x516330, 0x516390), (0x423b50, 0x423b70), (0x4f8690, 0x4f8740), (0x5127a0, 0x512890), (0x512f00, 0x5132a0),
           (0x51bfc0, 0x51c060), (0x51c300, 0x51c360), (0x57d910, 0x57dc80), (0x5856e0, 0x585720), (0x5fc530, 0x5fc560), (0x5fc660, 0x5fc750),
           (0x5fcc50, 0x5fccf0), (0x67c320, 0x67c3e0), (0x67c7f0, 0x67c840), (0x6b5930, 0x6b5960), (0x6b5bd0, 0x6b5c00), (0x6b65a0, 0x6b65f0),
           (0x6b6830, 0x6b68e0), (0x6b6b40, 0x6b6bb0), (0x6b8930, 0x6b89a0), (0x6b8b40, 0x6b8b70), (0x5da3c0, 0x5da3c1)])
WEATHER, START, END, WORLD = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000
W = RAM + 0x20000   # (the world of the mode, intensity and message cases)
FX, MSG, MVT, MGR, ENG, EVT, VT = (RAM + 0x2000 + 0x100 * i for i in range(7))
CALLS = {'Bolt': 0x512c50, 'SetMode': 0x5131f0, 'SetIntensity': 0x512f00, 'Message': 0x5127a0}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def size(r):
    return r.choice([0, 1, 2, 128, 128, 192, 256, 256, r.randrange(1, 300), 0x1999999, 0x2000000, 0xffffffff, r.getrandbits(32)])


def ranges(r):
    """The three [low, high] pairs at 0x785504 and the most drops at 0x785530."""
    game = [1, 40, 41, 100, 60, 100]
    if r.random() < 0.4:
        lows = game
    else:
        lows = []
        for _ in range(3):
            lo = r.choice([0, 1, 20, 50, 99, 100, r.randrange(-5, 106), -0x80000000, 0x7fffffff])
            hi = r.choice([lo, lo + 30, 100, r.randrange(-5, 106), lo - 3, 0x7fffffff]) & 0xffffffff
            lows += [lo & 0xffffffff, hi]
    block = bytearray(r.bytes(0x40))
    struct.pack_into('<6I', block, 4, *[x & 0xffffffff for x in lows])
    put(block, 0x30, 'I', r.choice([0, 1, 100, 500, 1000, 0x7fffffff, r.getrandbits(32)]))
    return block


def world_case(r, name):
    import funnytime
    regions = {}
    log = funnytime.clock_regions(r, regions)
    regions[0x80239c] = struct.pack('<I', W)
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x1c, 'I', funnytime.u32(r))
    regions.update({W + 0x2a0: clock, W + 0x1da708: struct.pack('<II', r.getrandbits(32), funnytime.u32(r)) + r.bytes(8),
                    0x7cf83c: struct.pack('<I', W), 0x8bcbac: struct.pack('<I', FX), 0x785500: ranges(r)})
    weather = bytearray(r.bytes(0x70))
    put(weather, 0x5c, 'I', r.choice([fb(r.randrange(-1500, 1500)), fb(r.uniform(-1500, 1500)), 0, 0x80000000, fb(360.0), fb(45.0)]))
    put(weather, 0x54, 'I', r.choice([0, 0, r.getrandbits(32)]))
    weather[0x64] = r.choice([0, 0, 0, 1, r.getrandbits(8)])
    regions[WEATHER] = weather
    fx = bytearray(r.bytes(0x10))
    regions.update({FX: fx, FX + 0x3310: r.bytes(0x50), 0x8c1828 + 0x1f70: r.bytes(0x30)})
    # the sound: on and started (or not now and then), the manager's slot 2 the play (the command hashed), its engine's own slot 6 the stop's
    g = bytearray(r.bytes(0x20))
    struct.pack_into('<II', g, 8, r.choice([1] * 6 + [0, r.getrandbits(32)]), MGR)
    put(g, 0x14, 'I', r.choice([1] * 6 + [0]))
    blob(log, 2, 1, 0x7f)
    set_return(log, 2, r.choice([0, r.getrandbits(32)]))
    strings(log, 6, 0, out=1)
    eng = bytearray(r.bytes(0x30))
    struct.pack_into('<I', eng, 0, EVT)
    put(eng, 0x20, 'I', 0)
    regions.update({0x802bc0: g, MGR: struct.pack('<II', MVT, ENG) + r.bytes(8), MVT: vtable({2: 2}), ENG: eng, EVT: vtable({6: 2})})
    regions[0x803a20] = r.bytes(4)
    value = r.choice([0, 1, 2, 40, 41, 59, 60, 61, 99, 100, 101, r.randrange(-5, 106), -1, 0x80000000, 0x7fffffff, r.getrandbits(32)]) & 0xffffffff
    if name == 'SetMode':
        return Call(0x5131f0, [WEATHER, r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])], regions, conv='t')
    if name == 'SetIntensity':
        return Call(0x512f00, [WEATHER, value], regions, conv='t')
    set_return(log, 0, r.choice([1, 1, 2, 2, 0, 3, r.getrandbits(32)]))
    regions.update({MSG: struct.pack('<III', VT, r.getrandbits(32), value) + struct.pack('<i', r.choice([0, 1, 7, -1, r.randrange(-1000, 1000),
                                                                                                      r.getrandbits(31)])), VT: vtable({0: 0})})
    return Call(0x5127a0, [WEATHER, MSG], regions, conv='t')


def case(r):
    name = r.deal(OPS)
    if name != 'Bolt':
        return world_case(r, name)
    regions = {WEATHER: r.bytes(0x70), RAM + 0x100: r.bytes(0x200), WORLD: r.bytes(0x10),
               0x7cf83c: struct.pack('<I', WORLD - 0x1da708), 0x7a0314: struct.pack('<I', size(r)), 0x7a0374: struct.pack('<I', size(r))}
    end = r.choice([END, END, END, END, START, START + 4, START + 8, START - 4])
    return Call(0x512c50, [WEATHER, START, end, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
