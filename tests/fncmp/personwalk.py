# A person's step along its nav (0x4fa2a0 Sprite_WalkNav, src/game/world/person) against the original: thingenv's park of 1-4 things with
# its cells (chains as things_methods.py's SetPos has them), the person one of them; its nav (+0xd4) as navupdate.py's with no behaviours
# (the step is pos + vel cut to max_speed; the move test, the pointer at 0x762190, the cdecl logger: passable or not), its rows 0-3 and
# columns any (the cell kept or changed), the path done (+0x60 = 1) at times; +0x18c set at times; the kind +2 1, 7 or others, a guard's
# state +0x19c 0x12 or not; the speed word +0xc2 about the word 0x75c7f0; personscripts.py's script list behind the reference +0xc,
# its scripts at 0x74db60, 0x74dbc0, 0x74ef90 or others. The cseg angle routine (0x6e7074, not in the listing) is the two-argument
# logger on both sides (driver.cpp's Person_SetEnv), its answer the logger's hash (or set: 0, 0x10000 and up, any).
# usage: personwalk.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import fncmp  # noqa: E402
from fncmp import CDECL_LOG2, LOG, Bench, Call, RAM, log_region, put, set_return  # noqa: E402
from personscripts import PSCRIPTS, script_list  # noqa: E402
from thingenv import CELLS, POOL, THING, park  # noqa: E402


def merged(ranges):
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


NAV_RANGES = [(0x50f3b0, 0x50f5f5), (0x510150, 0x510156), (0x50dd80, 0x50dfd0), (0x50cba0, 0x50cdc0), (0x4d49a0, 0x4d49e0), (0x50e080, 0x50e0d4),
              (0x50ece0, 0x50ed20), (0x50f1a0, 0x50f202), (0x50f600, 0x50f6f9), (0x50f700, 0x50f84a), (0x50f850, 0x50f86f), (0x4d99c0, 0x4d99d3),
              (0x4d97b0, 0x4d97d9), (0x5104b0, 0x510502), (0x510870, 0x51089d), (0x4d8300, 0x4d8328), (0x50fd40, 0x50fde2)]
B = Bench(merged(NAV_RANGES + [(0x4217f0, 0x421810), (0x475c50, 0x475d00), (0x4d41d0, 0x4d4220), (0x4d6350, 0x4d6370), (0x4d91f0, 0x4d93c0),
                               (0x4fa2a0, 0x4fa470), (0x50b6a0, 0x50b790), (0x510150, 0x5101a0), (0x540c70, 0x540da0), (0x5da3c0, 0x5da3c1),
                               (0x4d7330, 0x4d73b4), (0x4d7790, 0x4d7880), (0x4e0220, 0x4e02f0), (0x4fa460, 0x4fa480), (0x50b350, 0x50b360),
                               (0x5054d0, 0x5056f0), (0x506580, 0x5065d0)]),
          hooks={0x6e7074: fncmp._cdecl_log2})
TARGET, LIST = RAM + 0x100, RAM + 0x200
CODES = [0x74db60, 0x74dbc0, 0x74ef90]


def near(r, scale):
    return r.choice([0, r.randrange(-scale, scale), r.randrange(-0x100, 0x100), r.randrange(-0x4000, 0x4000)])


def nav(r, mem, at):
    """navupdate.py's nav at mem + at: pos in rows 0-3, vel, max_force, max_speed, the target, the path's points, distances, flags"""
    cells = [r.randrange(256), r.choice([0, 1, 2, 3, r.randrange(4)])]
    pos = [(c << 16) + r.randrange(0x10000) for c in cells]
    if r.random() < 0.02:
        pos = [r.getrandbits(32) for _ in range(2)]
    put(mem, at + 4, 'I', r.randrange(0, 0x40000))
    put(mem, at + 8, 'II', *pos)
    vel = [r.choice([near(r, 0x10000), near(r, 0x40000), r.randrange(-0x20000, 0x20000)]) & 0xffffffff for _ in range(2)]
    if r.random() < 0.03:   # (no speed: Nav_Update's normalizing divides by zero)
        vel = [0, 0]
    if r.random() < 0.05:
        vel = [r.getrandbits(32) for _ in range(2)]
    put(mem, at + 0x10, 'II', *vel)
    put(mem, at + 0x18, 'I', r.choice([0x6666, 0x10000, 0x3333, r.randrange(0x100, 0x40000)]))
    put(mem, at + 0x1c, 'I', r.choice([0x3333, 0x10000, 0x8000, r.randrange(0x100, 0x40000)] * 6 + [r.getrandbits(32)]))   # (any: IntPair_Truncate's IDIV faults)
    put(mem, at + 0x30, 'I', TARGET)
    mem[at + 0x60] = r.choice([0, 0, 1, 1, 2])
    put(mem, at + 0x5c, 'I', r.randrange(5))
    for k in range(5):
        put(mem, at + 0x64 + 8 * k, 'II', *((p + r.randrange(-0x80000, 0x80000)) & 0xffffffff for p in pos))
    put(mem, at + 0xa4, 'II', r.randrange(0, 0x400000), r.randrange(0, 0x400000))
    put(mem, at + 0xa0, 'I', r.choice([0, r.randrange(0x1000, 0x2000000), 0x100000, -0x100000, -r.randrange(0x1000, 0x2000000)]) & 0xffffffff)   # (negative: progress past 1.0)
    put(mem, at + 0xac, 'I', r.choice([0, 0x10000, 0x8000, r.getrandbits(32)]))
    return pos


def job(r, regions, pool, person, handles, me, pos):
    """ToJob's surroundings: the world's tick (RAM + 0x1da70c: [0x80239c] is RAM); the job +0x21a mostly one of the park's handles (the
    handyman's own now and then); the job's entrance +0x36 the handyman's cell id (row << 7 + column + 1, from its bytes or its nav's
    point) mostly; its tracked handle +0x5e the handyman's, 0 or another, its stamp +0x60 about the tick (past 0x64 behind it: the
    tracked one asked, Person_HasWord: the kind +2 4 or 5 and the word +0x218 / +0x21a)"""
    tick = r.getrandbits(32)
    regions[RAM + 0x1da70c] = struct.pack('<I', tick)
    person[2] = r.choice([5] * 6 + [1, 4, 7, r.getrandbits(8)])
    j = r.randrange(len(handles))
    job = person if j == me else memoryview(pool)[THING * j:THING * (j + 1)]
    put(person, 0x21a, 'H', handles[j] if r.random() < 0.97 else r.choice([0, 12]))
    cols = [person[5], (pos[0] >> 16) & 0xff]
    rows = [person[7], (pos[1] >> 16) & 0xff]
    cell = ((r.choice(rows) << 7) + r.choice(cols) + 1) & 0xffff
    put(job, 0x36, 'H', r.choice([cell] * 4 + [(cell + 1) & 0xffff, (cell + 0x80) & 0xffff, r.getrandbits(16)]))
    put(job, 0x5e, 'H', r.choice([handles[me]] * 4 + [0, r.choice(handles), r.getrandbits(16)]))
    put(job, 0x60, 'I', (tick - r.choice([0, 0, 0x64, 0x65, 0x1000, r.getrandbits(32)])) & 0xffffffff)
    for t in range(len(handles)):
        thing = person if t == me else memoryview(pool)[THING * t:THING * (t + 1)]
        if r.random() < 0.5:
            put(thing, 0x218, 'HH', r.choice(handles + [0]), r.choice(handles + [0]))


def case(r):
    op = r.deal(OPS)
    count = r.choice([1, 2, 3, 4])
    regions, handles, things_ = park(r, count=count, defs=False, cells=True)
    pool = bytearray(regions[POOL])
    grid = bytearray(regions[CELLS])
    for i in range(count):   # (things_methods.py's SetPos: chains of the park's handles)
        put(pool, THING * i + 8, 'HH', r.choice(handles + [0]), r.choice(handles + [0]))
        pool[THING * i + 5], pool[THING * i + 7] = r.choice([0, 1, 127, 128, 255, r.randrange(256)]), r.randrange(4)
    for i in range(len(grid) // 0x44):
        put(grid, 0x44 * i + 0x24, 'H', r.choice(handles + [0]))
    me = r.randrange(count)
    p = THING * me
    person = bytearray(pool[p:p + THING])
    script_list(r, regions, person)
    scripts = regions[PSCRIPTS]
    for k in range(3):
        if r.random() < 0.5:
            put(scripts, 0x120 * k + 0xc, 'I', (r.choice(CODES) - 0x74dab8) // 4)
    pos = nav(r, person, 0xd4)
    if r.random() < 0.3:   # (the cell kept)
        person[5], person[7] = (pos[0] >> 16) & 0xff, (pos[1] >> 16) & 0xff
    person[2] = r.choice([1, 1, 7, 7, 7, 0, 2, 3, r.getrandbits(8)])
    put(person, 0x19c, 'I', r.choice([0x12, 0x12, 0, 5, 0x13, r.getrandbits(32)]))
    put(person, 0x18c, 'I', r.choice([0] * 8 + [1, r.getrandbits(32)]))
    default = r.choice([0, 0, 1, 0x19, 0x32, 0x7fff, 0xffff, r.getrandbits(16)])
    put(person, 0xc2, 'H', r.choice([default, default, (default + 1) & 0xffff, (default - 1) & 0xffff, 0, 0xffff, r.getrandbits(16)]))
    regions[0x75c7f0] = struct.pack('<H', default)
    if op == 'ToJob':
        job(r, regions, pool, person, handles, me, pos)
    pool[p:p + THING] = person
    regions[POOL], regions[CELLS] = pool, grid
    log = log_region()
    if r.random() < 0.3:   # (the move test's and the angle's answer)
        set_return(log, 0x3e, r.choice([0, 1, 0x10000, 0x12345, r.getrandbits(32)]))
    regions.update({TARGET: struct.pack('<II', *((v + near(r, 0x40000)) & 0xffffffff for v in pos)), LIST: bytes(8),
                    0x7cf078: struct.pack('<II', LIST, LIST), 0x762190: struct.pack('<I', CDECL_LOG2), LOG: log})
    if op == 'ToJob':
        return Call(0x4d7790, [things_[me]], regions, conv='t', ret='v')
    return Call(0x4fa2a0, [things_[me]], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Walk,ToJob').split(',') if k]

if __name__ == '__main__':
    B.main(case)
