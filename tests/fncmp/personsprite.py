# A person's sprite placed for a moment (0x4fa030 Person_PlaceSprite, src/game/world/person) against the original: t as person.py's Lerp
# has it (both ranges' ends, NaN, infinities, denormals; the first position is at t - 0.35), the nav's and last positions near or far
# apart; personscripts.py's script list behind the reference +0xc, the slots +0xb4 and +0x210 scripts of it or 0; +0x28 set or not, the
# timer +0x198 about the counter 0x7cdb98 (the square face: dynfaces.py's queue), the byte +2 1 or not; the bob row 0x7cedd4 (0-20 or
# any) and its time 0x7cedd8 about 1.0 (NaN too). CentreOnSprite (0x486860): the same sprite (or none) and the clock [0x8786bc] about
# the last tick's [0x878a1c] (0-300 ms on, behind, or any); the camera's point and its three cleared dwords compared.
# PlaceAll (0x518f90): the thing table's chain of 1-5 things (handles 1-9, [0x7cf56c] the first's entry or none), the sprite one of them (a person or
# not, its byte +3 clear mostly), the others 0x40 bytes of kinds about the people's with byte +3 set (or no person's kind): one sprite placed or none.
# UpdateSpeed (0x4fa870): the words +0xc0-+0xc4 small, about 100 or any, the speed +0xc8 (NaN, infinity, denormal, any bits), the word at
# 0x75c7fc (0 too, the high half set or not), the reference's period +0x14 as its entry point +0x10 (script_list).
# Particle (0x4fa470): the script list ready and the reference +0xc one of its scripts, their x / z (+0x88 / +0x90) about
# terrainheight.py's ground and point less the dwords 0x785584 / 0x785588 (small or any) times 10; particles.py's Spawn set-up and effect.
# SpeedParticle (0x4f7c60): UpdateSpeed's person with Particle's, the byte +0x3a0 set or not.
# TickSpeed (0x505490): UpdateSpeed's surroundings; the world [0x80239c]'s ticks +0x1da70c with their low two bits the handle +0's mostly,
# Person_KillScript84After's world ([[0x7cf6ec]]) the same mostly, the stamp +0xbc about the ticks less 12 (the script +0xb4 sprite_env's).
# usage: personsprite.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=PlaceSprite,CentreOnSprite,PlaceAll,UpdateSpeed,Particle,SpeedParticle,TickSpeed)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
import particles  # noqa: E402
import terrainheight  # noqa: E402
from personscripts import PSCRIPTS, PTABLE, script_list  # noqa: E402

RANGES = []
for a, b in sorted([(0x475480, 0x4754f0), (0x4d8c50, 0x4d8c70), (0x4f9f00, 0x4fa2b0), (0x4fe900, 0x4fe960), (0x50b350, 0x50b370), (0x510160, 0x510190),
           (0x53c8d0, 0x53c9d0), (0x5da3c0, 0x5da3c1), (0x486860, 0x4868c1), (0x42aab0, 0x42ab1d), (0x518f90, 0x51905c),
           (0x4fa870, 0x4fa928), (0x510190, 0x5101e9), (0x4d4190, 0x4d41d0), (0x475010, 0x475480), (0x4754f0, 0x476a30),
           (0x540b90, 0x540d78), (0x541f60, 0x541f70), (0x505490, 0x5054c9), (0x50be40, 0x50be7c), (0x5f5660, 0x5f56a0),
           (0x4fa470, 0x4fa52b), (0x4f7c60, 0x4f7c88), (0x519410, 0x519426), (0x4527f0, 0x452a70), (0x67a830, 0x67a860)] + particles.RANGES):
    if RANGES and a <= RANGES[-1][1]:
        RANGES[-1] = (RANGES[-1][0], max(RANGES[-1][1], b))
    else:
        RANGES.append((a, b))
B = Bench(RANGES)
CALLS = {'PlaceSprite': 0x4fa030, 'CentreOnSprite': 0x486860, 'PlaceAll': 0x518f90, 'UpdateSpeed': 0x4fa870, 'Particle': 0x4fa470,
         'SpeedParticle': 0x4f7c60, 'TickSpeed': 0x505490}
GROUND = RAM + 0x30000   # (terrainheight.py's regions, clear of particles.py's at RAM + 0x1000 and the script list's)
TICKS, OTHER_TICKS, HOLDER = RAM + 0x28000, RAM + 0x28100, RAM + 0x28200   # (the worlds' +0x1da70c; [0x7cf6ec]'s holder)
OTHERS = RAM + 0x24000   # (PlaceAll's other things, 0x40 bytes each)
PEOPLE = [1, 4, 5, 6, 7, 8, 0x12]
THING = RAM + 0x20000


def word(r):
    return r.choice([0, 1, 4, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])


def sprite_env(r, regions, size=0x240):
    """the sprite THING (size bytes) and what placing it reads (see the header); returns its bytes"""
    thing = bytearray(r.bytes(size))
    script_list(r, regions, thing)
    for off in (0xb4, 0x210):
        put(thing, off, 'I', r.choice([0, 0, 1, 2, 3, 4]))
    put(thing, 0x28, 'I', r.choice([0, 0, 0, 1, word(r)]))
    thing[2] = r.choice([1, 1, 1, 0, 2, r.getrandbits(8)])
    for at in (0xdc, 0xe0):   # (person.py's Lerp: the nav (+0xd4)'s position, then the last one about it)
        pos = word(r) if r.random() < 0.3 else r.randrange(0x800000)
        put(thing, at, 'I', pos)
        put(thing, 0x190 + at - 0xdc, 'I', r.choice([pos, (pos + r.randrange(-0x2000, 0x2000)) & 0xffffffff, (pos ^ 0x80000000), word(r)]))
    counter = word(r)
    regions[0x7cdb98] = struct.pack('<I', counter)
    put(thing, 0x198, 'I', r.choice([0, counter, counter + 1, counter + 2, counter - 1, word(r)]) & 0xffffffff)
    regions[THING] = thing
    n = r.choice([0, 1, 2, 5, 0x806])   # (dynfaces.py's queue)
    regions.update({0x820b00: r.bytes(0xa0 * 9), 0x872168: r.bytes(0x40), 0x871ee0: struct.pack('<I', n), 0x763c98: bytes([r.choice([0, 1, 1])]) + bytes(3),
                    0x874fc4: struct.pack('<I', r.choice([0, 0, 1, 5]))})
    bob = r.choice([0.0, 0.5, 0.9, 0.9000001, 0.95, 1.0, 1.05, -1.0, 1e30, float('nan'), r.uniform(0, 1.2)])
    regions[0x7cedd4] = struct.pack('<If', r.choice([0, 1, 18, 19, 20, 21, 0xffffffff, r.getrandbits(32)]), bob)
    return thing


def centre_env(r, regions):
    """CentreOnSprite's clock [0x8786bc] about the last tick's [0x878a1c] and the camera's point and cleared dwords"""
    last = r.getrandbits(32)
    now = r.choice([last + r.randrange(0, 300), last + r.randrange(0, 300), last - r.randrange(1, 300), r.getrandbits(32)])
    regions[0x878a1c] = struct.pack('<I', last)
    regions[0x8786bc] = struct.pack('<I', now & 0xffffffff)
    regions[0x7908f0] = r.bytes(12)
    regions[0x790a88] = r.bytes(12)


def speed_env(r, regions):
    """UpdateSpeed's words, speed, period and the word at 0x75c7fc (see the header)"""
    thing = regions[THING]
    for off in (0xc0, 0xc2, 0xc4):
        put(thing, off, 'H', r.choice([0, 1, 50, 99, 100, 101, 0x295, 0x7fff, 0xffff, r.getrandbits(16)]))
    put(thing, 0xc8, 'f', r.choice([0.0, -0.0, 0.5, 1.0, 2.0, 3.0, -1.0, 1e-40, 1e30, float('nan'), float('inf'), r.uniform(-5, 5)]))
    if r.random() < 0.1:
        thing[0xc8:0xcc] = r.bytes(4)
    put(thing, 0x14, 'I', r.choice([0, 0, 1, 100, r.getrandbits(32)]))
    regions[0x75c7fc] = struct.pack('<I', r.choice([0, 1, 3, 10, 100, 0x10000, 0x10001, r.getrandbits(32)]))


def particle_env(r, regions):
    """Particle's script position, ground, offsets and Spawn's set-up (see the header); returns the effect. (With the script missing the
    original's x, y and z are its stack's leftovers: the reference is always one of the list's scripts.)"""
    thing = regions[THING]
    table = regions[PTABLE]
    put(thing, 0xc, 'I', r.choice([1, 3] + ([2] if struct.unpack_from('<I', table, 8)[0] else [])))
    ground = terrainheight.case(r, ['Height'], base=GROUND)
    regions.update(ground.regions)
    px, pz = (struct.unpack('<f', struct.pack('<I', a))[0] for a in ground.args)
    small = [0, 0, 1, -1, 2, 5, -3, 100]
    gx, gz = (r.choice(small) if r.random() < 0.9 else r.getrandbits(32) - (1 << 31) for _ in range(2))
    regions[0x785584] = struct.pack('<ii', gx, gz)
    scripts = regions[PSCRIPTS]
    for k in range(3):
        for off, v in ((0x88, px - 10.0 * gx), (0x90, pz - 10.0 * gz)):
            if r.random() < 0.9 and v == v and abs(v) < 3e38:
                put(scripts, 0x120 * k + off, 'f', v)
            elif r.random() < 0.5:
                put(scripts, 0x120 * k + off, 'f', r.choice([0.0, -0.0, 1e30, -1e30, 2.5e6, float('nan'), float('inf')]))
    spawn = particles.spawn(r)
    regions.update(spawn.regions)
    return spawn.args[0]


def case(r):
    op = r.deal(OPS)
    regions = {}
    sprite_env(r, regions)
    if op in ('Particle', 'SpeedParticle'):
        while regions[0x7b49f0][0x24] != 1:   # (the list ready: GetPosition's x, y and z are set)
            regions = {}
            sprite_env(r, regions)
    if op == 'CentreOnSprite':
        centre_env(r, regions)
        return Call(CALLS[op], [r.choice([THING] * 9 + [0])], regions, ret='v')
    if op in ('UpdateSpeed', 'TickSpeed'):
        speed_env(r, regions)
        if op == 'TickSpeed':
            thing = regions[THING]
            ticks = r.choice([0, 1, 12, 13, 100, 0xfffffff3, r.getrandbits(32)])
            if r.random() < 0.6:
                ticks = (ticks & ~3) | (thing[0] & 3)
            other = ticks if r.random() < 0.8 else r.getrandbits(32)
            put(thing, 0xbc, 'I', (other + r.choice([-13, -12, -11, 0, 5, r.getrandbits(32)])) & 0xffffffff)
            regions[TICKS] = struct.pack('<I', ticks)
            regions[OTHER_TICKS] = struct.pack('<I', other)
            regions[0x80239c] = struct.pack('<I', TICKS - 0x1da70c)
            regions[HOLDER] = struct.pack('<I', OTHER_TICKS - 0x1da70c)
            regions[0x7cf6e0] = put(bytearray(r.bytes(0x10)), 0xc, 'I', HOLDER)
        return Call(CALLS[op], [THING], regions, conv='t', ret='v')
    if op == 'Particle':
        return Call(CALLS[op], [THING, particle_env(r, regions)], regions, conv='t', ret='v')
    if op == 'SpeedParticle':
        regions[THING] = regions[THING] + bytearray(r.bytes(0x1a0))
        regions[THING][0x3a0] = r.choice([0, 1, 0x80])
        speed_env(r, regions)
        particle_env(r, regions)
        return Call(CALLS[op], [THING], regions, conv='t', ret='v')
    if op == 'PlaceAll':
        thing = regions[THING]
        handles = r.sample(range(1, 10), r.randrange(1, 6))
        mine = r.choice(handles)
        thing[2] = r.choice(PEOPLE + PEOPLE + [0, 2, 3, r.getrandbits(8)])
        thing[3] = r.choice([0, 0, 0, 1])
        table = bytearray(r.bytes(20 * 10))
        for i, h in enumerate(handles):
            if h == mine:
                at = THING
            else:
                at = OTHERS + 0x40 * i
                other = bytearray(r.bytes(0x40))
                other[2] = r.choice(PEOPLE + [0, 2, 3, 9, 0x11, 0x13, r.getrandbits(8)])
                other[3] = r.randrange(1, 256) if other[2] in PEOPLE else r.choice([0, 1])
                put(other, 0, 'H', h)
                regions[at] = other
            if h == mine:
                put(thing, 0, 'H', h)
            nxt = 0x7cfb90 + 20 * handles[i + 1] if i + 1 < len(handles) else 0
            put(table, 20 * h, 'I', at)
            put(table, 20 * h + 4, 'H', h)
            put(table, 20 * h + 8, 'I', nxt)
        put(table, 0, 'I', r.choice([0, r.getrandbits(32)]))
        regions[0x7cfb90] = table
        regions[0x7cf56c] = struct.pack('<I', r.choice([0x7cfb90 + 20 * handles[0]] * 9 + [0]))
    t = r.choice([0.0, -0.0, 0.25, 0.35, 0.5, 1.0, 1.35, 1.5, 2.0, 2.5, -0.5, -1.0, 1e-40, float('nan'), float('inf'), float('-inf'),
                  r.uniform(-1.5, 2.5), r.uniform(0, 1), r.uniform(0, 1), struct.unpack('<f', r.bytes(4))[0]])
    return Call(CALLS[op], [THING, fb(t)], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
