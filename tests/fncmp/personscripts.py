# A person's sprite script and ride timers (src/game/world/personscripts: 0x4fdc90, 0x4fe950,
# 0x50c0d0, 0x4f9ed0, 0x4fe900, 0x50be40, 0x4f9e60) against the originals, with the script list not ready (0x7b49f0 zero): script slots set
# or not; clock values on and off the 20-tick beat. Place (0x4f9e60): see place() below.
# usage: personscripts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Tick,Jump,Kill,Keep,Release,Move,After,Place)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, fb, global_heap, heap, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import terrainheight  # noqa: E402

# (Place's: the function, Terrain_HeightAt, the reference's IsAt / Start, Set4_4217f0, SpriteScript_Start's allocation, clock and sprite folders)
PLACE_RANGES = [(0x4f9e60, 0x4f9ed0), (0x4527f0, 0x452a70), (0x4d4100, 0x4d4134), (0x4d4140, 0x4d416d), (0x4217f0, 0x421800), (0x5f5660, 0x5f56a0),
                (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x402d70, 0x403220), (0x4033a0, 0x4033f0)]
B = Bench(PLACE_RANGES + [(0x50be40, 0x50be7c), (0x4f9ed0, 0x4f9efc), (0x4fe900, 0x4fe944), (0x4d4170, 0x4d4186), (0x475550, 0x47558b), (0x4fdc90, 0x4fdcb6), (0x4fdc60, 0x4fdc90), (0x4fe950, 0x4fe977), (0x50c0d0, 0x50c0f2), (0x50bab0, 0x50bac4), (0x475010, 0x476a30),
           (0x540b90, 0x540da0), (0x541f60, 0x541fb0), (0x5da3c0, 0x5da3c1)])
CALLS = ['Tick', 'Jump', 'Kill', 'Keep', 'Release', 'Move', 'After', 'Place']
CLOCK = RAM + 0x1000
PLACE, PSCRIPTS, PTABLE, PSPRITES = RAM + 0x8000, RAM + 0x8100, RAM + 0x8400, RAM + 0x9000   # (clear of terrainheight.py's RAM)
ENTRIES = [0, 0x74db60, 0x74dbc0, 0x74dc20, 0x74dc48, 0x74dce8, 0x74dd28, 0x74dda8, 0x74dd68, 0x74dac0, 0x74e100, 0x74e160, 0x74e280,
           0, 1, 2]   # (the table of entry points at 0x75a418)


def place(r):
    """Place (0x4f9e60): terrainheight.py's ground and point (x, z), the person at PLACE with script_list's scripts, y any float."""
    c = terrainheight.case(r)
    regions = dict(c.regions)
    x, z = c.args
    person = bytearray(r.bytes(0x40))
    anim = script_list(r, regions, person)
    regions[PLACE] = person
    y = r.choice([0.0, 1.5, -3.25, 100.0, r.uniform(-60, 60), r.uniform(-60, 60), float('nan'), float('inf'), 1e30])
    args = [PLACE, x, fb(y), z, r.getrandbits(32), anim]
    return Call(0x4f9e60, args, regions, conv='t', ret='a')


def script_list(r, regions, person, anim=None):
    """Place's script list (into regions): the person's reference (+0xc) slot one of three scripts (or past them, or an empty one, or 0)
    with the list ready, or the list not ready; the entry point (anim, or one drawn: mostly 1-12) the scripts' +0xc or the reference's +4
    half the time each. Returns the entry point."""
    ready = r.random() < 0.85
    slot = r.choice([0, 0, 1, 2, 3, 3, 4, 5]) if ready else r.choice([0, 0, 1, 2])
    put(person, 0xc, 'I', slot)
    put(person, 0x20, 'II', r.randrange(5), r.randrange(14))   # (the sprite and folder ScriptRef_Start starts with)
    if ready:   # (SpriteScript_Start's: its script from the heap through GlobalAlloc, the clock frozen, the sprite folders as partstate.py's)
        regions.update(global_heap())
        regions[HEAP] = heap()
        regions[0x785970] = put(bytearray(r.bytes(0x80)), 0x38, 'I', 1)
        folders = bytearray(r.bytes(14 * 12))
        for f in range(14):
            put(folders, 12 * f + 4, 'II', r.randrange(8), r.choice([1, 2, 3, 4]))
        regions[0x763f88] = folders
        regions[PSPRITES] = r.bytes(0x240 * 4)
        regions[0x876b48] = b''.join(struct.pack('<I', PSPRITES + 0x240 * r.randrange(4)) for _ in range(16))
    # (with the list ready only the code entry points 1-12: 0 and 13-15 are action codes, which go through the sprite folders)
    if anim is None:
        anim = r.randrange(1, 13) if ready else r.randrange(16)
    put(person, 0x10, 'I', anim if r.random() < 0.4 else r.choice([0, 1, r.randrange(16)]))
    scripts = bytearray()
    for k in range(3):
        s = bytearray(r.bytes(0x120))
        code = ENTRIES[anim] if r.random() < 0.5 and 1 <= anim <= 12 else r.choice(ENTRIES[1:13])
        put(s, 0xc, 'I', (code - 0x74dab8) // 4)
        scripts += s
    regions.update({PSCRIPTS: scripts,
                    PTABLE: struct.pack('<4I', 0, PSCRIPTS, r.choice([0, PSCRIPTS + 0x120]), PSCRIPTS + 0x240),
                    0x7b49f0: put(put(bytearray(0x2c), 0, 'Ii', PTABLE, 4), 0x24, 'I', 1 if ready else 0)})
    return anim


def case(r):
    op = r.choice(OPS)
    if op == 'Place':
        return place(r)
    person = bytearray(r.bytes(0x400))
    regions = {RAM: person, 0x7b49f0: bytearray(16)}
    if op == 'Tick':
        regions[0x802390] = put(bytearray(r.bytes(0x20)), 0xc, 'I', CLOCK - 0x1da70c)
        regions[CLOCK] = put(bytearray(r.bytes(0x20)), 0, 'I', r.choice([0, 20, 40, 1, 19, 21, 0x7fffffff, r.getrandbits(32), 20 * r.randrange(1000)]))
        return Call(0x4fdc90, [RAM], regions, conv='t')
    if op == 'After':
        stamp = r.choice([0, 12, 13, 100, 0xfffffff0, r.getrandbits(32)])
        put(person, 0x8c, 'I', (stamp + r.choice([-13, -12, -11, 0, 5, r.getrandbits(32)])) & 0xffffffff)
        put(person, 0x84, 'I', r.choice([0, 0, 1, 2, 7]))
        regions[0x7cf6e0] = put(bytearray(r.bytes(0x20)), 0xc, 'I', RAM + 0x800)
        regions[RAM + 0x800] = put(bytearray(8), 0, 'I', RAM + 0x1000 - 0x1da70c)
        regions[RAM + 0x1000] = put(bytearray(r.bytes(0x20)), 0, 'I', stamp)
        return Call(0x50be40, [RAM], regions, conv='t')
    if op == 'Release':
        for off in (0xc, 0x30, 0x210):
            put(person, off, 'I', r.choice([0, 0, 1, 2, 7]))
        person[2] = r.choice([0, 1, 1, 2])
        return Call(0x4f9ed0, [RAM], regions, conv='t')
    if op == 'Move':
        put(person, 0x210, 'I', r.choice([0, 0, 1, 2, 7]))
        return Call(0x4fe900, [RAM] + [r.getrandbits(32) for _ in range(4)], regions, conv='t')
    if op == 'Jump':
        put(person, 0x210, 'I', r.choice([0, 0, 1, 2, 7, 0xffffffff]))
        return Call(0x4fe950, [RAM], regions, conv='t')
    put(person, 0x84, 'I', r.choice([0, 0, 1, 2, 7, 0xffffffff]))
    return Call(0x50bab0 if op == 'Keep' else 0x50c0d0, [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
