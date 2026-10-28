# The visitor constructor (src/game/world/visitorsteps: Visitor_Ctor 0x4faec0, thiscall (cell)) against the original: the moving person's
# constructor's environment (thing_ctors.py's Sprite, as staffctor.py: the thing table and cells, the made message centre, the world's random
# state), personscripts.py's script list for ScriptRef_Start (its sprite folders, or folder 0's count put in when the list is not ready),
# gBalanceLevel's exit level and variance, peep type count and table, cash percentage and prankery likelihood (small mostly, now and then
# any, the counts never 0), the park's +0x1da70c and visitor count +0x1da714, the crossings 0x7855dc small, the visitor's cell's type plain
# or not (AnalyserCell_TypePlain, through [0x8023a0]). The calls through VisitorStepsEnv are recorders (driver.cpp): Visitor_SetState 26 of
# 1, Sprite_SetDestination 2 of 2 (x and y logged as their low words), Person_PlaceSprite 9 of 1, Sprite_SpawnParticle 4 of 1; the post
# 0x40fb10 the objects' recorder 21 (the message's bytes 0-9 logged).
# usage: visitorctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import personscripts  # noqa: E402
import thing_ctors  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return  # noqa: E402
from staffctor import merged  # noqa: E402

HOOKS = dict(thing_ctors.CENTRE_HOOKS)
HOOKS.update({0x501db0: _recorder(26, 1), 0x4fa5f0: _recorder(2, 2), 0x4fa030: _recorder(9, 1), 0x4fa470: _recorder(4, 1),
              0x40fb10: _recorder(21, 1)})
B = Bench(merged([(0x4faec0, 0x4fb317), (0x40f8b0, 0x40f8d0), (0x415e00, 0x415e80), (0x4d4140, 0x4d41a0), (0x4fa990, 0x4fa9fa),
                  (0x536300, 0x53644c), (0x51aaf0, 0x51ab1a), (0x4d8610, 0x4d8750), (0x516330, 0x516380), (0x5da3c0, 0x5da3c1),
                  # (thing_ctors.py's for the moving person)
                  (0x4d91f0, 0x4d9290), (0x502d20, 0x502d70), (0x50afe0, 0x50b090), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200),
                  (0x50ffe0, 0x510110), (0x510160, 0x510190), (0x516270, 0x516320), (0x5d1750, 0x5d1751), (0x4f8940, 0x4f8ae0),
                  (0x4d3ed0, 0x4d3ef0), (0x50b350, 0x50b370), (0x4f8690, 0x4f8740),
                  # (personscripts.py's script list under ScriptRef_Start)
                  (0x475010, 0x476a30), (0x540b90, 0x540da0), (0x541f60, 0x541fb0), (0x4d4100, 0x4d4134), (0x5f5e20, 0x5f5e90),
                  (0x5f5fa0, 0x5f5fc0), (0x5f5660, 0x5f56a0), (0x4217f0, 0x421810), (0x4d27a0, 0x4d2850), (0x412e90, 0x412f00),
                  (0x4033a0, 0x4033f0), (0x402d70, 0x403220)] + thing_ctors.CENTRE_RANGES),
          hooks=HOOKS)
SCRIPTS = RAM + 0x50000


def case(r):
    thing_ctors.OPS = ['Sprite']
    sprite = thing_ctors.case(r)
    regions = dict(sprite.regions)
    visitor, cell = sprite.args[0], sprite.args[1]
    v = bytearray(r.bytes(0x240))
    personscripts.script_list(r, regions, v, base=SCRIPTS)
    regions[visitor] = v
    if 0x763f88 not in regions:
        regions[0x763f88] = put(bytearray(r.bytes(12)), 8, 'I', r.choice([1, 2, 3, 4]))
    regions[0x7cf83c] = struct.pack('<I', RAM)
    regions[RAM + 0x1da70c] = r.bytes(0xc)
    # (gBalanceLevel: 0x78504c exit level, 0x785050 its variance, 0x785054 cash percentage; 0x7850dc prankery; 0x7850e8 the types' cash, 12
    # bytes apart; 0x7851d4 the type count)
    small = lambda lo, hi: r.randrange(lo, hi) if r.random() < 0.9 else r.choice([r.getrandbits(31), 0x7fffffff, -r.randrange(1, 100)])  # noqa: E731
    variance = r.choice([1, 2, 5, 10, 50, small(1, 1000)]) or 1
    regions[0x78504c] = struct.pack('<iii', small(-100, 1000), variance, small(0, 60))
    regions[0x7850dc] = struct.pack('<I', r.choice([0, 1, 10, 50, 99, 100, 101, r.getrandbits(32)]))
    count = r.choice([1, 2, 3, 4, 5, 6, 8, 10])
    types = bytearray(r.bytes(12 * 20))
    for i in range(20):
        put(types, 12 * i, 'i', r.choice([0, 10, 50, 100, 1000, 30000, small(0, 500)]))
    regions[0x7850e8] = types
    regions[0x7851d4] = struct.pack('<i', count)
    regions[0x7855dc] = struct.pack('<4i', *[r.choice([r.randrange(128), r.randrange(-2, 130)]) for _ in range(4)])
    # (the visitor's cell, 1-0x100 (thing_ctors.py's cells): its type)
    cells = thing_ctors.SPRITE_CELLS
    index = (cell & 0xffff) - 1
    if 0 <= index < 0x100:
        start = cells if index < 0x41 else cells + 0x44 * 0x41
        blk = bytearray(regions[start])
        put(blk, 0x44 * (index - (0 if index < 0x41 else 0x41)) + 8, 'I', r.choice([0, 1, 3, 9, 0xa, 2, 4, 0xb, 0x17]))
        regions[start] = blk
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 2, 0x30000)
    blob_bytes(log, 21, 0, 0x3ff)
    set_return(log, 2, r.choice([0, 1, 1]))
    regions[LOG] = log
    return Call(0x4faec0, [visitor, cell], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
