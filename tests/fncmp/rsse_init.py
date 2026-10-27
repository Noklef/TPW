# RSSE_Init (0x551600) and the script event name lookup (0x468600) of src/game/rsse/rsse_vm against the originals: the opcode name table (0x765280) pointing at strings
# of every length and sign (and empty strings ending it), the initialised flag set or clear, globals of random content; scripts with twelve event tables of up to three
# entries, keys that match (or not) and outputs given or null; a timer's start (0x557160) on a vm of random content, the game clock paused; an
# object's vector read through a frame's chain (0x556a80); the first finished entry taken from up to five, or a negative count (0x557110).
# A walker started (0x556f40) on ridebounds.py's pose() world: two parts' places, the slots free or not, the game clock paused.
# A walker moved (0x5580a0) on that world, heightat.py's floor and personscripts.py's person: see step_case().
# usage: rsse_init.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Event,Timer,GetVec,TakeDone,Walk,Step)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, RAM, blob, global_heap, heap, log_region, put, set_return, sine_table, vtable  # noqa: E402
from particles import RANGES as PARTICLE_RANGES  # noqa: E402

CALLS = {'Init': 0x551600, 'Event': 0x468600, 'Timer': 0x557160, 'GetVec': 0x556a80, 'TakeDone': 0x557110, 'Walk': 0x556f40, 'Step': 0x5580a0,
         'Back': 0x5571a0, 'Walkers': 0x557d80, 'Trigger': 0x5573d0, 'Node': 0x557970, 'All': 0x557ab0}
B = Bench([(0x551600, 0x5516a3), (0x468600, 0x468708), (0x557160, 0x557197), (0x556a80, 0x556aac), (0x557110, 0x55715b),(0x402d70, 0x402d75), (0x402f10, 0x402f2d), (0x4031e0, 0x4031ef), (0x5da3c0, 0x5da3c1),
           (0x556f40, 0x557108), (0x466b70, 0x466c7f), (0x556af0, 0x556b8c), (0x556b90, 0x556ddb), (0x44b220, 0x44b267), (0x67a830, 0x67a860),
           # (Step's: the function, heightat.py's and personscripts.py's ranges)
           (0x5580a0, 0x5584a8), (0x451290, 0x45138d), (0x451390, 0x451634), (0x4511a0, 0x451286), (0x4527f0, 0x452a70), (0x67a400, 0x690000),
           (0x4f9e60, 0x4f9ed0), (0x4d4100, 0x4d4134), (0x4d4140, 0x4d416d), (0x4217f0, 0x421800), (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e90),
           (0x5f5fa0, 0x5f5fc0), (0x402d70, 0x403220), (0x4033a0, 0x4033f0), (0x4f9ed0, 0x4f9efc), (0x4d4170, 0x4d4186), (0x475550, 0x47558b),
           (0x4fdc90, 0x4fdcb6), (0x4fdc60, 0x4fdc90), (0x4fe950, 0x4fe977), (0x475010, 0x476a30), (0x540b90, 0x540da0), (0x541f60, 0x541fb0),
           (0x50c0d0, 0x50c0f2), (0x50bab0, 0x50bac4), (0x50be40, 0x50be7c), (0x4fe900, 0x4fe944),
           (0x5571a0, 0x557395), (0x557d80, 0x5580a0), (0x44b410, 0x44b4d0), (0x44b4c0, 0x44b520), (0x450ea0, 0x4512a0), (0x475a10, 0x475b90),
           (0x4f9e60, 0x4f9f10), (0x4fcac0, 0x4fcb20), (0x466b70, 0x466c90), (0x475550, 0x4755a0), (0x5f5680, 0x5f56a0),
           (0x5573d0, 0x557a4c), (0x412e90, 0x412eea), (0x4145d0, 0x4145e0), (0x4145e0, 0x4145f0), (0x51bfc0, 0x51c048),
           (0x557ab0, 0x557d80), (0x435710, 0x435740)] + PARTICLE_RANGES)
STRINGS, SCRIPT, KEYS, TABLES, OUT = RAM + 0x100, RAM + 0x1000, RAM + 0x1400, RAM + 0x1500, RAM + 0x1800


def init_case(r):
    n = r.choice([0, 1, 2, 3, 6, 12])
    text = bytearray()
    table = bytearray(4 * (n + 2))
    for k in range(n):
        s = bytes(r.choice([1, 2, 7, 0x41, 0x7f, 0x80, 0xff, r.randrange(1, 256)]) for _ in range(r.choice([0, 1, 2, 5, 9, 20])))
        put(table, 4 * k, 'I', STRINGS + len(text))
        text += s + b'\0'
    put(table, 4 * n, 'I', RAM)             # a pointer to an empty string ends the table
    if n and r.random() < 0.15:
        put(table, 0, 'I', RAM)             # an empty first name: nothing is summed
    regions = {0x765280: table, STRINGS: bytes(text) + b'\0', RAM: bytes(8), 0x879190: r.bytes(0x30)}
    if r.random() < 0.3:
        put(regions[0x879190], 0x10, 'I', 0)  # 0x8791a0 clear
    return Call(CALLS['Init'], [], regions, conv='c', ret='v')


def event_case(r):
    script = bytearray(r.bytes(0xb0))
    keys = [r.choice([1, 2, 3, 0x1234, r.getrandbits(32)]) for _ in range(4)]
    put(script, 0xac, 'I', KEYS)
    tables = bytearray(0x400)
    top = 0
    for g in range(12):
        n = r.choice([0, 0, 1, 2, 3])
        put(script, 0x1c + 12 * g, 'I', n)
        put(script, 0x1c + 12 * g + 4, 'I', TABLES + top)
        for e in range(n):
            put(tables, top + 8 * e, 'II', r.getrandbits(32), r.choice(keys + [7]))
        top += 8 * n
    regions = {SCRIPT: script, KEYS: bytes().join(k.to_bytes(4, 'little') for k in keys), TABLES: tables, OUT: r.bytes(0x10)}
    outs = [OUT + 4 * k if r.random() < 0.8 else 0 for k in range(3)]
    return Call(CALLS['Event'], [SCRIPT, r.randrange(4)] + outs, regions, conv='c', ret='a')


def timer_case(r):
    # the game clock (0x785970) paused, so its time is its +0x3c plus +0x48
    clock = bytearray(r.bytes(0x80))
    put(clock, 0x38, 'I', r.choice([1, 2, 0x80000000]))
    vm = RAM + 0x2000
    args = [r.choice([0, 1000, 5000, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32)]
    return Call(CALLS['Timer'], [vm] + args, {0x785970: clock, vm: r.bytes(0x100)}, conv='c', ret='a')


def vector_case(r):
    obj = bytearray(r.bytes(0x400))  # the frame: +8 -> RAM + 0x100, its +4 -> RAM + 0x200, its +0x78 -> RAM + 0x300
    put(obj, 8, 'I', RAM + 0x100)
    put(obj, 0x104, 'I', RAM + 0x200)
    put(obj, 0x278, 'I', RAM + 0x300)
    return Call(CALLS['GetVec'], [RAM, RAM + 0x3f0, RAM + 0x3f4, RAM + 0x3f8], {RAM: obj}, ret='a')


def finished_case(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0x7c, 'i', r.choice([0, 1, 3, 5, -1]))
    put(obj, 0x2c, 'I', RAM + 0x200)
    for i in range(6):
        put(obj, 0x200 + 0x20 * i + 0x18, 'H', r.choice([0, 4, 4, 1]))
        put(obj, 0x200 + 0x20 * i + 0x10, 'I', r.getrandbits(32))
    return Call(CALLS['TakeDone'], [RAM], {RAM: obj}, ret='a')


def walk_case(r):
    """0x556f40 (cdecl; vm, value, from, to, a, b, kind, last): the vm is pose()'s object (its ride, parts and nodes), its walker slots
    (+0x2c, +0x7c of them: up to 5, or a negative count) free or not (the busy word's high byte alone now and then); the keys the parts'
    own (their masks given 0x880, or now and then 0x80 or 0x800 alone, one key on two parts now and then, each key dealt one its lookup's
    mask finds) or -1: a key the ride has no part for leaves the original's position locals unwritten (uninitialised stack, sic), so
    none is dealt; kind 4 now and then; the game clock (0x785970) paused, as timer_case() has it."""
    from ridebounds import pose
    c = pose(r)
    ram = bytearray(c.regions[RAM])
    vm = c.args[0]
    walkers = RAM + 0x2000
    put(ram, vm - RAM + 0x2c, 'I', walkers)
    put(ram, vm - RAM + 0x7c, 'i', r.choice([0, 1, 3, 5, 5, -1]))
    slots = bytearray(r.bytes(0xc0))
    for i in range(6):
        put(slots, 0x20 * i + 0x18, 'H', r.choice([0, 1, 1, 4, 0x100, 0x400]))
    n = int.from_bytes(ram[0x268:0x26a], 'little')   # (the model's part count, MODEL + 0x48)
    if n >= 2 and r.random() < 0.4:   # (a key twice: the first lookup's mask may find one part, the other mask the other)
        put(ram, 0x600 + 0x14 + 4, 'I', int.from_bytes(ram[0x604:0x608], 'little'))
    keys = [int.from_bytes(ram[0x600 + 0x14 * i + 4:0x600 + 0x14 * i + 8], 'little') for i in range(min(n, 4))]
    alone = r.random() < 0.5
    bits = [r.choice([0x880, 0x80, 0x800]) if alone else 0x880 for _ in range(min(n, 4))]
    for i in range(min(n, 4)):   # (masks 0x880, or now and then 0x80 or 0x800 alone; the keys dealt are found under the lookup's mask)
        put(ram, 0x600 + 0x14 * i, 'I', int.from_bytes(ram[0x600 + 0x14 * i:0x604 + 0x14 * i], 'little') & ~0x880 | bits[i])
    kind = r.choice([4, 4, 0, 1, r.getrandbits(32)])
    key = lambda m: r.choice([k for k, b in zip(keys, bits) if b & m] + [0xffffffff])  # noqa: E731
    clock = bytearray(r.bytes(0x80))
    put(clock, 0x38, 'I', r.choice([1, 2, 0x80000000]))
    regions = {RAM: bytes(ram), walkers: bytes(slots), 0x7a4610: c.regions[0x7a4610], 0x785970: clock}
    args = [vm, r.getrandbits(32), key(0x800), key(0x80 if kind == 4 else 0x800), r.getrandbits(32), r.getrandbits(32), kind, r.getrandbits(32)]
    return Call(CALLS['Walk'], args, regions, conv='c', ret='a')


STEP_POSE, STEP_GROUND, STEP_SCRIPTS = RAM + 0x30000, RAM + 0x20000, RAM + 0x40000   # (the worlds side by side, clear of heightat's at RAM)
PERSON, SLOT = RAM + 0x50000, RAM + 0x50800


def step_case(r):
    """0x5580a0 (cdecl; vm, slot, from, to, mode, t): pose()'s world at STEP_POSE (its object the vm: +0x80 a time near the clock's or
    any, +0x84 zero (no wobble) or not, +0x88 / +0x8a / +0xc0 any), the keys found under each lookup's mask (0x80 for mode 1 / 2, as
    walk_case() deals them); heightat.view()'s floor (its ground at STEP_GROUND) or y between the parts (the slot's +0x1a bit 0); the
    person (the things' entry, 0x7cfb90 + 20 * the slot's +0x10) with script_list()'s scripts for the entry point Place is given; the
    slot's +0x18 2 mostly (a word: 0x102 too), its +0x16 each kind; t any (past 1000, negative); the game clock paused; the sine table 4096 entries or 64."""
    from ridebounds import pose
    import heightat
    import personscripts
    import terrainheight
    c = pose(r, STEP_POSE)
    ram = bytearray(c.regions[STEP_POSE])
    vm = c.args[0]
    n = int.from_bytes(ram[0x268:0x26a], 'little')
    if n >= 2 and r.random() < 0.4:
        put(ram, 0x600 + 0x14 + 4, 'I', int.from_bytes(ram[0x604:0x608], 'little'))
    keys = [int.from_bytes(ram[0x600 + 0x14 * i + 4:0x600 + 0x14 * i + 8], 'little') for i in range(min(n, 4))]
    alone = r.random() < 0.5
    bits = [r.choice([0x880, 0x80, 0x800]) if alone else 0x880 for _ in range(min(n, 4))]
    for i in range(min(n, 4)):
        put(ram, 0x600 + 0x14 * i, 'I', int.from_bytes(ram[0x600 + 0x14 * i:0x604 + 0x14 * i], 'little') & ~0x880 | bits[i])
    key = lambda m: r.choice([k for k, b in zip(keys, bits) if b & m] + [0xffffffff])  # noqa: E731
    mode = r.choice([0, 1, 2, 1, 2, r.getrandbits(32)])
    clock = bytearray(r.bytes(0x80))
    put(clock, 0x38, 'I', r.choice([1, 2, 0x80000000]))
    put(clock, 0x3c, 'I', r.choice([0, 1000, 123456, r.getrandbits(31), r.getrandbits(32)]))
    put(clock, 0x48, 'I', r.choice([0, 0, 50, r.getrandbits(32)]))
    now = (int.from_bytes(clock[0x3c:0x40], 'little') + int.from_bytes(clock[0x48:0x4c], 'little')) & 0xffffffff
    o = vm - STEP_POSE
    put(ram, o + 0x80, 'I', (now - r.choice([0, 1, 250, 999, r.randrange(5000), r.getrandbits(32)])) & 0xffffffff)
    put(ram, o + 0x84, 'I', r.choice([0, 0, 0, 1000, 2000, 1, 0xffffffff, 500, r.getrandbits(32)]))
    put(ram, o + 0x88, 'HH', r.choice([0, 1000, r.getrandbits(16)]), r.choice([0, 1000, r.getrandbits(16)]))
    put(ram, o + 0xc0, 'H', r.choice([0, 50, 0xffce, r.getrandbits(16)]))
    v = heightat.view(r, STEP_GROUND)
    regions = dict(v.regions)
    regions.pop(heightat.OUT, None)
    if 0x7a0854 not in regions:   # (Place's ground)
        regions.update(terrainheight.case(r, ['Height'], STEP_GROUND).regions)
    slot = bytearray(r.bytes(0x20))
    h = r.choice([0, 1, 2, 3, 5, 6, 7, r.randrange(16), r.randrange(64)])
    put(slot, 0x10, 'I', h | r.choice([0, 0, r.getrandbits(16) << 16]))
    put(slot, 0x16, 'HH', r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(16)]), r.choice([2, 2, 2, 2, 0, 1, 0x102, 0xff02, r.getrandbits(16)]))
    slot[0x1a] = slot[0x1a] & ~1 | r.choice([0, 1])
    wobble = int.from_bytes(ram[o + 0x84:o + 0x88], 'little') != 0
    kind = int.from_bytes(slot[0x16:0x18], 'little')
    if int.from_bytes(slot[0x18:0x1a], 'little') != 2:
        anim = 1
    elif wobble:
        anim = 2
    else:
        anim = {3: 3, 5: 8, 6: 5}.get(kind)
    person = bytearray(r.bytes(0x400))
    for off in (0x30, 0x210):   # (Person_ReleaseScripts' slots)
        put(person, off, 'I', r.choice([0, 0, 1, 2, 7]))
    person[2] = r.choice([0, 1, 1, 2])
    personscripts.script_list(r, regions, person, anim, STEP_SCRIPTS)
    regions[PERSON] = person
    things = bytearray(r.bytes(20 * (h + 1)))
    put(things, 20 * h, 'I', PERSON)
    regions[0x7cfb90] = things
    regions[0x785970] = clock   # (paused, as script_list's)
    regions[0x7b095c] = sine_table(r.choice([4096, 4096, 64]))
    regions[STEP_POSE] = bytes(ram)
    regions[0x7a4610] = c.regions[0x7a4610]
    regions[SLOT] = slot
    t = r.choice([0, 1000, 1001, 500, r.randrange(1001), r.randrange(1001), -1, -1000, 0x7fffffff, r.getrandbits(32)])
    args = [vm, SLOT, key(0x80 if mode == 1 else 0x800), key(0x80 if mode == 2 else 0x800), mode, t & 0xffffffff]
    return Call(CALLS['Step'], args, regions, conv='c', ret='v')


WALK_SLOTS = RAM + 0x58000


def walkers_case(r, op):
    """0x5571a0 (cdecl; vm, value) / 0x557d80 (cdecl; vm): step_case()'s world (its paused clock, person, scripts and ground; the y
    between the parts, each slot's byte +0x1a bit 0, since 0x557d80 makes the ride's model the view: picked without filling, its +0x30
    given 0x80 and the part states' flag 8 cleared), the vm's walker slots (+0x2c: up to 5 at WALK_SLOTS, or a negative count) each
    busy (+0x18 1-4, or 0, 0x100, 0x102) or not, of each kind (+0x16: 2 and 4 most, 1, 3, 5, 6, 0, any), +0x10 the person's handle
    (its high half any; a value two slots share now and then), the keys +0 / +6 found under 0x800 and +2 / +4 under 0x80 for kind 4
    (else 0x800; most often a key only parts of that one mask have, so that a lookup under the other finds nothing), or -1 (the
    ride's centre; not for a facing slot's +2, whose direction it leaves unwritten), +8 / +0xc the leg's start and end about the
    clock's time (never equal: the original divides by their difference; a leg part way through often); the wobble's end (+0x80 +
    +0x84) the clock's time or one either side now and then; the part states' scripts -1 or small slots; Particle_Spawn's world as
    particles.spawn() has it."""
    from ridebounds import MODEL, pose
    import particles
    import personscripts
    import terrainheight
    c = pose(r, STEP_POSE)
    ram = bytearray(c.regions[STEP_POSE])
    vm = c.args[0]
    o = vm - STEP_POSE
    n = int.from_bytes(ram[0x268:0x26a], 'little')
    if n >= 2 and r.random() < 0.4:
        put(ram, 0x600 + 0x14 + 4, 'I', int.from_bytes(ram[0x604:0x608], 'little'))
    keys = [int.from_bytes(ram[0x600 + 0x14 * i + 4:0x600 + 0x14 * i + 8], 'little') for i in range(min(n, 4))]
    alone = r.random() < 0.5
    bits = [r.choice([0x880, 0x80, 0x800]) if alone else 0x880 for _ in range(min(n, 4))]
    for i in range(min(n, 4)):
        put(ram, 0x600 + 0x14 * i, 'I', int.from_bytes(ram[0x600 + 0x14 * i:0x604 + 0x14 * i], 'little') & ~0x880 | bits[i])
        put(ram, 0x680 + 0x14 * i + 0xc, 'i', r.choice([-1, -1, 0, 1, 3, r.randrange(64)]))
    found = lambda m: [k for k, b in zip(keys, bits) if b & m]  # noqa: E731
    only = lambda m: [k for k, b in zip(keys, bits) if b == m and k not in [k2 for k2, b2 in zip(keys, bits) if b2 != m]]  # noqa: E731
    ram[MODEL - RAM + 0x30] |= 0x80
    ram[0x6e0] &= ~8
    put(ram, 0x6e8, 'I', r.choice([0, 1, 2, 5, r.getrandbits(32)]))   # (the part states' active count)
    clock = bytearray(r.bytes(0x80))
    put(clock, 0x38, 'I', r.choice([1, 2, 0x80000000]))
    put(clock, 0x3c, 'I', r.choice([0, 1000, 123456, r.getrandbits(31), r.getrandbits(32)]))
    put(clock, 0x48, 'I', r.choice([0, 0, 50, r.getrandbits(32)]))
    now = (int.from_bytes(clock[0x3c:0x40], 'little') + int.from_bytes(clock[0x48:0x4c], 'little')) & 0xffffffff
    put(ram, o + 0x2c, 'I', WALK_SLOTS)
    put(ram, o + 0x7c, 'i', r.choice([0, 1, 3, 5, 5, 5, -1]))
    ago = r.choice([0, 1, 250, 999, r.randrange(5000), r.getrandbits(32)])
    put(ram, o + 0x80, 'I', (now - ago) & 0xffffffff)
    put(ram, o + 0x84, 'I', r.choice([0, 0, 0, 1000, 2000, 1, 0xffffffff, 500, r.getrandbits(32), ago, ago, ago + 1, ago - 1]) & 0xffffffff)
    put(ram, o + 0x88, 'HH', r.choice([0, 1000, r.getrandbits(16)]), r.choice([0, 1000, r.getrandbits(16)]))
    put(ram, o + 0xc0, 'H', r.choice([0, 50, 0xffce, r.getrandbits(16)]))
    h = r.choice([0, 1, 2, 3, 5, 7, r.randrange(16)])
    slots = bytearray(r.bytes(0xc0))
    for i in range(6):
        s = 0x20 * i
        kind = r.choice([2, 2, 4, 4, 1, 3, 5, 6, 0, r.getrandbits(16)])
        busy = r.choice([1, 2, 3, 4, 1, 2, 3, 4, 0, 0x100, 0x102])
        near = lambda m: (only(m) if only(m) and r.random() < 0.6 else found(m)) + [0xffff]  # noqa: E731
        m = 0x80 if kind == 4 else 0x800
        to = found(m) if busy == 2 and kind not in (1, 2, 4) else near(m)
        if not to:
            busy, to = 4, [0xffff]
        put(slots, s, 'HHHH', r.choice(near(0x800)), r.choice(to), r.choice(near(m)), r.choice(near(0x800)))
        start = (now - r.choice([0, 1, 300, 999, 1000, 5000, r.randrange(2000), r.randrange(2000), r.getrandbits(32)])) & 0xffffffff
        put(slots, s + 8, 'II', start, (start + r.choice([1, 100, 1000, 2000, 50, 3000, 3000, r.getrandbits(16) + 1, r.getrandbits(32) | 1])) & 0xffffffff)
        put(slots, s + 0x10, 'I', h | r.choice([0, 0, r.getrandbits(16) << 16]))
        put(slots, s + 0x16, 'HH', kind, busy)
        slots[s + 0x1a] |= 1
    if r.random() < 0.3:
        a, b = r.sample(range(6), 2)
        slots[0x20 * b + 0x10:0x20 * b + 0x14] = slots[0x20 * a + 0x10:0x20 * a + 0x14]
    regions = dict(particles.spawn(r).regions)
    regions.update(terrainheight.case(r, ['Height'], STEP_GROUND).regions)
    person = bytearray(r.bytes(0x400))
    for off in (0x30, 0x210):   # (Person_ReleaseScripts' slots)
        put(person, off, 'I', r.choice([0, 0, 1, 2, 7]))
    person[2] = r.choice([0, 1, 1, 2])
    personscripts.script_list(r, regions, person, None, STEP_SCRIPTS)
    if 0x763f88 not in regions:   # (Thing_SpriteChoice's folder 3 for PartStates_Activate)
        regions[0x763f88] = put(bytearray(r.bytes(14 * 12)), 8 + 12 * 3, 'I', r.randrange(1, 8))
    regions[PERSON] = person
    things = bytearray(r.bytes(20 * (h + 1)))
    put(things, 20 * h, 'I', PERSON)
    regions[0x7cfb90] = things
    regions[0x785970] = clock
    regions[0x7b095c] = sine_table(r.choice([4096, 4096, 64]))
    regions[0x79fb10] = r.bytes(0x10)   # (Model_BeginView's view and floor mode)
    regions[STEP_POSE] = bytes(ram)
    regions[0x7a4610] = c.regions[0x7a4610]
    regions[WALK_SLOTS] = bytes(slots)
    if op == 'Back':
        value = r.choice([int.from_bytes(slots[0x20 * i + 0x10:0x20 * i + 0x14], 'little') for i in range(6)] + [r.getrandbits(32)])
        return Call(CALLS['Back'], [vm, value], regions, conv='c', ret='v')
    return Call(CALLS['Walkers'], [vm], regions, conv='c', ret='v')


TMANAGER, TVT, TTHING, TDEFS = RAM + 0x60000, RAM + 0x60100, RAM + 0x60200, RAM + 0x61000


def trigger_case(r):
    """0x5573d0 (cdecl; vm, kind, key, what, extra): pose()'s world (at STEP_POSE), its parts' masks given 0x100, 0x200, both or neither
    (the effects look under 0x100, the sounds under 0x200), the key a part's, -1 (the ride's centre) or one the ride has no part for;
    kinds 0-11 (1 and 2 most) and any; what an effect (0x13, 0x27, any below 0x69, or with bit 0x8000 an index 0-4 of the thing's
    definition's +0x4e0 dwords), or a sound; extra 1000 or not; the vm's thing (+0xac) one whose type has a definition (0x7890a0, cached
    or not) with +0x4e0..+0x4f0 effects and +0x4f0 a category or 0; ridesound.py's sound globals and manager (its command logged);
    particles.spawn()'s world."""
    import particles
    from ridebounds import pose
    c = pose(r, STEP_POSE)
    ram = bytearray(c.regions[STEP_POSE])
    vm = c.args[0]
    n = int.from_bytes(ram[0x268:0x26a], 'little')
    keys = [int.from_bytes(ram[0x600 + 0x14 * i + 4:0x600 + 0x14 * i + 8], 'little') for i in range(min(n, 4))]
    for i in range(min(n, 4)):
        put(ram, 0x600 + 0x14 * i, 'I', int.from_bytes(ram[0x600 + 0x14 * i:0x604 + 0x14 * i], 'little') & ~0x300 | r.choice([0x100, 0x200, 0x300, 0]))
    h = r.choice([1, 2])
    put(ram, vm - STEP_POSE + 0xac, 'H', h)
    kind = r.choice([1, 1, 2, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 0, 11, r.getrandbits(32)])
    if kind in (1, 2):
        what = r.choice([0x13, 0x27, r.randrange(0x69), 0x8000 | r.randrange(5), 0xffff8000 | r.randrange(5), 0x8000 | r.randrange(5)])
    else:
        what = r.choice([0, 1, 0x1d, 0xbb, r.getrandbits(32)])
    extra = r.choice([1000, 1000, 0, 1, 0x10, r.getrandbits(32)])
    key = r.choice(keys * 3 + [0xffffffff, 7, r.getrandbits(32)]) if keys else r.choice([0xffffffff, 0xffffffff, 3])
    regions = dict(particles.spawn(r).regions)
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1]))
    put(g, 0xc, 'I', TMANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.85:
        set_return(log, 2, r.getrandbits(32))
    things = bytearray(r.bytes(60))
    put(things, 20 * h, 'I', TTHING)
    thing = bytearray(r.bytes(0x20))
    type_ = r.getrandbits(16)
    put(thing, 0xe, 'H', type_)
    defs = bytearray(r.bytes(0x738))
    put(defs, 0, 'I', TDEFS)
    put(defs, 0x528, 'i', 1)
    if r.random() < 0.3:
        put(defs, 0x730, 'HxxI', type_, TDEFS)
    block = bytearray(r.bytes(0x500))
    put(block, 0, 'H', type_)
    for k in range(4):
        put(block, 0x4e0 + 4 * k, 'I', r.choice([0x13, 0x27, r.randrange(0x69), r.getrandbits(32)]))
    put(block, 0x4f0, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))
    regions.update({STEP_POSE: bytes(ram), 0x7a4610: c.regions[0x7a4610], LOG: log, 0x802bc0: g, 0x8791a0: r.bytes(4),
                    TMANAGER: put(bytearray(r.bytes(0x10)), 0, 'I', TVT), TVT: vtable({2: 2}, 4), 0x803a20: r.bytes(0x20),
                    0x7cfb90: things, TTHING: thing, 0x7890a0: defs, TDEFS: block})
    return Call(CALLS['Trigger'], [vm, kind & 0xffffffff, key, what & 0xffffffff, extra], regions, conv='c', ret='a')


TNODES = RAM + 0x62000


def node_case(r):
    """0x557970 (cdecl; vm, node, what, extra, value): trigger_case()'s world and arguments, the kind and key in a node (of three
    0x20 apart at TNODES, the middle one passed) linked to the others or not, the vm's first node +0xb0 it, another or none; the node
    count [0x8791b4] any; the node freed to global_heap()'s GlobalFree (its pointer hashed)."""
    c = trigger_case(r)
    vm, kind, key, what, extra = c.args
    nodes = bytearray(r.bytes(0x60))
    node = TNODES + 0x20
    put(nodes, 0x28, 'I', kind)
    put(nodes, 0x30, 'I', key)
    put(nodes, 0x20, 'II', r.choice([0, TNODES + 0x40]), r.choice([0, TNODES]))
    ram = bytearray(c.regions[STEP_POSE])
    put(ram, vm - STEP_POSE + 0xb0, 'I', r.choice([node, node, TNODES, 0]))
    c.regions.update({STEP_POSE: bytes(ram), TNODES: nodes, 0x8791b4: r.bytes(4), HEAP: heap(0x100)})
    c.regions.update(global_heap())
    return Call(CALLS['Node'], [vm, node, what, extra, r.getrandbits(32)], c.regions, conv='c', ret='a')


RIDERS, RFLAGS = RAM + 0x59000, RAM + 0x59100


def all_case(r):
    """0x557ab0 (cdecl): walkers_case()'s world with its vm the one record of the ride list [0x8791b0] (the system [0x8791a0] on
    mostly), its riders (+0x28 at RIDERS, +0x64 of them: 0-4 or negative) each a handle dword (0 for none; the person's index, its
    high half any, so % 7 runs negative too), a key found under 0x800, the centre's or none, and a start about the clock's time; the
    record's +0x1c's +0x1c set or not, +0x6e and the bob rate +0xc0 any. With no ride (the record's index +0xc8 0) or a part without a
    node the walkers that would face their part (state 2, kinds other than 1, 2 and 4) are made kind 2: Ride_PartPlace then leaves
    the direction unwritten and 0x557d80 reads its stack's leftovers (sic), which differ under the two callers. Now and then a rider's
    handle is the person's at an index above 0xff (the things' entry at its low byte any), a start lies ahead of the clock (the time
    since it negative), or a start at the clock's time (the sine 0) with the root's y a whole number and the nodes' y that plus +0x6e
    (the bob equal to the node's y) or a NaN."""
    c = walkers_case(r, 'Walkers')
    vm = c.args[0]
    o = vm - STEP_POSE
    ram = bytearray(c.regions[STEP_POSE])
    n = int.from_bytes(ram[0x268:0x26a], 'little')
    found = [int.from_bytes(ram[0x600 + 0x14 * i + 4:0x600 + 0x14 * i + 8], 'little') for i in range(min(n, 4))
             if int.from_bytes(ram[0x600 + 0x14 * i:0x600 + 0x14 * i + 4], 'little') & 0x800]
    if int.from_bytes(ram[o + 0xc8:o + 0xcc], 'little') == 0 or \
            any(int.from_bytes(ram[0x680 + 0x14 * i + 4:0x680 + 0x14 * i + 8], 'little') == 0 for i in range(min(n, 4))):
        slots = bytearray(c.regions[WALK_SLOTS])
        for k in range(6):
            if int.from_bytes(slots[0x20 * k + 0x18:0x20 * k + 0x1a], 'little') == 2 and \
                    int.from_bytes(slots[0x20 * k + 0x16:0x20 * k + 0x18], 'little') not in (1, 2, 4):
                put(slots, 0x20 * k + 0x16, 'H', 2)
        c.regions[WALK_SLOTS] = bytes(slots)
    clock = c.regions[0x785970]
    now = (int.from_bytes(clock[0x3c:0x40], 'little') + int.from_bytes(clock[0x48:0x4c], 'little')) & 0xffffffff
    h = next(k for k in range(64) if int.from_bytes(c.regions[0x7cfb90][20 * k:20 * k + 4], 'little') == PERSON)
    riders = bytearray(r.bytes(0x40))
    high = 0x100 + r.choice([k for k in range(16) if k != h])   # (a handle above 0xff whose low byte's entry is not the person)
    things = bytearray(c.regions[0x7cfb90]) + bytearray(r.bytes(20 * (high + 1) - len(c.regions[0x7cfb90])))
    put(things, 20 * high, 'I', PERSON)
    c.regions[0x7cfb90] = bytes(things)
    level = r.random() < 0.25
    if level:
        y = r.choice([0, 1, 5, -3, 100])
        put(ram, 0x344, 'f', y)
        extra = r.choice([0, 1, 0xffff, 7])
        put(ram, o + 0x6e, 'H', extra)
        for i in range(4):
            want = y + (extra if extra < 0x8000 else extra - 0x10000)
            if want == 0 and r.random() < 0.5:
                put(ram, 0x700 + 0x40 * i + 0x34, 'I', 0x80000000)   # (-0: equal to the bob's +0, its bits not)
            elif r.random() < 0.8:
                put(ram, 0x700 + 0x40 * i + 0x34, 'f', want)
            else:
                put(ram, 0x700 + 0x40 * i + 0x34, 'I', r.choice([0x7fc00000, 0xffc00000, 0x7f800001]))
        if r.random() < 0.3:
            put(ram, 0x344, 'I', 0x7fc00000)
    for i in range(4):
        handle = 0 if r.random() < 0.15 else (high if r.random() < 0.2 else h) | r.choice([0, 0, r.getrandbits(16) << 16])
        key = r.choice(found) if found and r.random() < 0.8 else r.choice([0xffff, r.randrange(8)])
        start = (now - r.choice([0, 1, 999, 1000, r.randrange(100000), r.getrandbits(31), r.getrandbits(32)])) & 0xffffffff
        if level and r.random() < 0.8:
            start = now
        elif r.random() < 0.2:
            start = (now + r.choice([1, 500, 1001, r.randrange(1, 1 << 31)])) & 0xffffffff
        put(riders, 0x10 * i, 'IIII', handle, key, r.getrandbits(32), start)
    flags = bytearray(r.bytes(0x20))
    put(flags, 0x1c, 'I', r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    put(ram, o, 'I', 0)
    put(ram, o + 0x1c, 'I', RFLAGS)
    put(ram, o + 0x28, 'I', RIDERS)
    put(ram, o + 0x64, 'i', r.choice([1, 2, 3, 4, 4, 0, -1]))
    if not level:
        put(ram, o + 0x6e, 'H', r.choice([0, 1, 0xffff, r.getrandbits(16)]))
    put(ram, o + 0xc0, 'H', r.choice([0, 1, 50, 0xffce, r.getrandbits(16)]))
    c.regions.update({STEP_POSE: bytes(ram), RIDERS: riders, RFLAGS: flags, 0x8791a0: put(bytearray(4), 0, 'I', 0 if r.random() < 0.05 else 1),
                      0x8791b0: put(bytearray(4), 0, 'I', vm)})
    return Call(CALLS['All'], [], c.regions, conv='c', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'All':
        return all_case(r)
    if op == 'Trigger':
        return trigger_case(r)
    if op == 'Node':
        return node_case(r)
    if op in ('Back', 'Walkers'):
        return walkers_case(r, op)
    return {'Init': init_case, 'Event': event_case, 'Timer': timer_case, 'GetVec': vector_case, 'TakeDone': finished_case,
            'Walk': walk_case, 'Step': step_case}[op](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
