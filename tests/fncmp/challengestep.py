# The challenges' step (src/game/world/challengestep: 0x4d1e90) against the original, on challenges.py's world (the calendar clock, its imports). Challenge_Met, Won, Lost, OfferNext and the post recorders as driver.cpp's kChallengeStepEnv; the rest real.
# Ticks and the next event's time near each other in days, so that the days compared and posted are small; the mode setting read or not yet.
# usage: challengestep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, Bench, Call, _recorder, blob, heap, put, set_return  # noqa: E402
from challenges import C, WORLD, offer  # noqa: E402
from funnytime import RANGES as CLOCK_RANGES  # noqa: E402
import thing_ctors  # noqa: E402

AT_EXIT, MET, WON, LOST, OFFER, POST = 10, 11, 12, 13, 14, 21
DAY = 864000000000
B = Bench([(0x4d1e90, 0x4d2314), (0x4d2840, 0x4d285d), (0x5fc570, 0x5fc593), (0x4d3220, 0x4d3500), (0x519510, 0x51954d), (0x4c9050, 0x4c9061), (0x51a280, 0x51a287), (0x550ca0, 0x550d90), (0x40f7a0, 0x40f803), (0x409cd0, 0x409cdd), (0x5da3c0, 0x5da3c1), (0x5dc650, 0x5dc654)] + CLOCK_RANGES + thing_ctors.CENTRE_RANGES,
          hooks={**thing_ctors.CENTRE_HOOKS, 0x67a7d0: _recorder(AT_EXIT, 1, cdecl=True), 0x4d1660: _recorder(MET, 2), 0x4d2fb0: _recorder(WON, 0), 0x4d3140: _recorder(LOST, 0), 0x4d2a70: _recorder(OFFER, 1), 0x40fb10: _recorder(POST, 1)})


def case(r):
    mem = bytearray(r.bytes(0x600))
    call = offer(r, mem, 'Accept')
    regions = call.regions
    for i in range(0x14):
        put(mem, 0x100 + 0xc + 0x30 * i, 'I', r.choice([r.randrange(0x30), r.randrange(0x170)]))
        put(mem, 0x100 + 0xc + 0x30 * i + 4, 'i', r.deal([0, 1, 7, 30, 30, 365, -30, r.randrange(1, 100)]))
    put(mem, 0x100 + 0x3cc, 'i', r.deal([0, 1, 19, r.randrange(20), r.randrange(20), r.randrange(20), -1, 20]))
    mem[0x100 + 0x3d8] = r.deal([0, 1, 1, 1, 0xff])
    mem[0x100 + 0x3d9] = r.deal([0, 1, 1, 1, 0xff])
    mem[0x100 + 0x3da] = r.deal([0, 0, 1, 0xff])
    clock = regions[WORLD + 0x2a0]
    world = regions[WORLD + 0x1da700]
    rate = r.deal([15000, 15000, 15000, 3600])
    put(clock, 0x1c, 'I', rate)
    ticks = r.deal([0, 1, 2, 3, 4, 8, 22, 23, 24, 92, 93, 100, 101, 400, 0x4000, r.randrange(1 << 12)])
    put(world, 0xc, 'I', ticks)
    start = struct.unpack_from('<Q', clock, 0)[0] & 0x00ffffffffffffff
    put(clock, 0, 'Q', start)
    elapsed = ((rate * ticks) // 4) * 10000000
    put(mem, 0x100 + 0x3d0, 'Q', (start + elapsed + r.deal([0, 1, -1, 1, 2, 7, 30, -7, 365]) * DAY + r.deal([0, 0, 1, -1, 12345678])) & 0xffffffffffffffff)
    regions[0x7857c8] = struct.pack('<i', r.deal([0, 1, 2, 7, 30, 1000, -1]))
    regions[0x7857d0] = struct.pack('<i', r.deal([0, 10, 25, 50, 100, 1000, -1000]))
    regions[0xfb3b7c] = struct.pack('<I', r.deal([0, 0, 0, 0, 1, 2]))
    regions[0xfb34c3] = bytes([r.deal([1, 1, 1, 0xff, 0, 0xfe])])
    regions[0x7a1a8c] = struct.pack('<I', r.deal([0, 0, 0, 0x800000, 0x1000000, 0x2000000, r.getrandbits(32)]))
    put(world, 0x10, 'I', r.deal([0, 0, 0, 1, 0x10000]))   # the flag, +0x1da710
    regions[HEAP] = heap()
    thing_ctors.centre(r, regions)
    log = regions[LOG]
    blob(log, POST, 0, 0xfef)
    set_return(log, MET, r.deal([0, 1, 0x100, 0xff, 0]))
    return Call(0x4d1e90, [C], regions, conv='t', ret='v', masks=thing_ctors.centre_masks(regions, 0x10))


if __name__ == '__main__':
    B.main(case)
