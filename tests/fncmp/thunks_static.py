# The static-initialiser and destructor stubs and the other jumping entry points of src/game/core/thunks (MOV ECX, a global; JMP a ported constructor or destructor, a direct JMP, this raised or lowered first)
# against the originals: the global object (or an object behind this) of random bytes (many dwords pointing into a second block, so that chains of pointers can be followed), a heap.
# Stubs whose targets need more get it: the string list destructors real lists; the handle and file stream destructors linked reference-counted objects with
# InterlockedDecrement's import slot and their vtables recorders; the semaphore constructor CreateSemaphoreA's slot a recorder; the screen surfaces destructor COM
# surfaces; the time functions the clock, timeGetTime a recorder and fncmp.COUNTER for the performance counter; the system information constructor CpuInfo_Ctor
# (CPUID) as stub_mark in both (driver.cpp's Drives env); the canvas constructor the hooked SurfaceInfo_Ctor (driver.cpp's Canvas env).
# usage: thunks_static.py [CASES [SEED]] [--wild|--both] [--show N]   (env ONLY=409240,... picks stubs)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import COUNTER, LOG, Bench, Call, HEAP, RAM, heap, counter, log_region, put, recorder, set_return, vtable  # noqa: E402

# (address, form, the global object, returns)
FUNCS = [
    (0x409240, 'self', None, 'v'),
    (0x42a000, 'adjust', None, 'a'),
    (0x42a030, 'adjust', None, 'a'),
    (0x42a040, 'adjust', None, 'a'),
    (0x45f940, 'self', None, 'v'),
    (0x492460, 'global', 0x7ca0c0, 'a'),
    (0x492740, 'global', 0x7ca0e8, 'a'),
    (0x49a0a0, 'global', 0x7ca250, 'a'),
    (0x4a46f0, 'global', 0x7cb318, 'a'),
    (0x4c07c0, 'global', 0x7cc988, 'v'),
    (0x51af80, 'global', 0x8023a8, 'a'),
    (0x5ac500, 'global', 0xf7b560, 'v'),
    (0x5c4c50, 'global', 0xf816c0, 'v'),
    (0x5c74a0, 'self', None, 'v'),
    (0x5f5560, 'global', 0xfa6e40, 'a'),
    (0x5fcce0, 'cdecl', None, 'v'),
    (0x5fcd30, 'cdecl', None, 'v'),
    (0x5fcd80, 'cdecl', None, 'v'),
    (0x5fcdd0, 'cdecl', None, 'v'),
    (0x5fce20, 'cdecl', None, 'v'),
    (0x5fce70, 'cdecl', None, 'v'),
    (0x5fcec0, 'cdecl', None, 'v'),
    (0x5fcf10, 'cdecl', None, 'v'),
    (0x5fcf60, 'cdecl', None, 'v'),
    (0x5fcfb0, 'cdecl', None, 'v'),
    (0x5fd000, 'cdecl', None, 'v'),
    (0x617360, 'cdecl', None, 'v'),
    (0x617380, 'cdecl', None, 'v'),
    (0x6173a0, 'cdecl', None, 'v'),
    (0x6173c0, 'cdecl', None, 'v'),
    (0x6300b0, 'self', None, 'v'),
    (0x6b0160, 'global', 0xfb1efc, 'v'),
    (0x6b01a0, 'global', 0xfb1ef0, 'v'),
    (0x6b01e0, 'global', 0xfb1ef4, 'v'),
    (0x6b0220, 'global', 0xfb1ef8, 'v'),
    (0x6c1230, 'self', None, 'v'),
    (0x489c80, 'timer', 'ms', 'a'),
    (0x5f5f80, 'timer', 'init', 'v'),
    (0x5f8a40, 'global', 0xfa7620, 'a'),
    (0x618ca0, 'global', 0xfa8258, 'a'),
    (0x41cc10, 'handle', 0, 'v'),
    (0x6bf320, 'handle', 4, 'v'),
    (0x532660, 'stream', 0x81b0e8, 'v'),
    (0x6c5ae0, 'semaphore', 0xfb205c, 'a'),
    (0x47e790, 'surfaces', 0x7b4ac0, 'v'),
    (0x45d810, 'list', 0xfb34e0, 'v'),
    (0x6d8990, 'list', 0xfb34e0, 'v'),
]
B = Bench([
    (0x5da3c0, 0x5da3c1), (0x507080, 0x5070c0), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b0d0), (0x67b050, 0x67b0d0),
    (0x5f5e80, 0x5f5f20),
    (0x618d30, 0x618d90),
    (0x5f71e0, 0x5f7220),
    (0x669fee, 0x66a012),
    (0x5f5f10, 0x5f5f90),
    (0x409240, 0x409250),
    (0x414d30, 0x414d50),
    (0x41cc10, 0x41cc20),
    (0x42a000, 0x42a010),
    (0x42a010, 0x42a040),
    (0x42a040, 0x42a050),
    (0x42a050, 0x42a080),
    (0x45d810, 0x45d820),
    (0x45d820, 0x45d880),
    (0x45f940, 0x45f950),
    (0x47e3e0, 0x47e470),
    (0x47e790, 0x47e7a0),
    (0x47faa0, 0x47fb20),
    (0x489c80, 0x489c90),
    (0x492060, 0x492080),
    (0x492460, 0x492470),
    (0x492740, 0x492750),
    (0x49a0a0, 0x49a0b0),
    (0x4a46f0, 0x4a4700),
    (0x4c07c0, 0x4c07d0),
    (0x507840, 0x507860),
    (0x51af80, 0x51af90),
    (0x532660, 0x532670),
    (0x55e470, 0x55e580),
    (0x5ac500, 0x5ac510),
    (0x5ac590, 0x5ac5e0),
    (0x5be0e0, 0x5be1d0),
    (0x5c4c50, 0x5c4c60),
    (0x5c4c60, 0x5c4cf0),
    (0x5c74a0, 0x5c74b0),
    (0x5d1410, 0x5d1430),
    (0x5f5560, 0x5f5570),
    (0x5f5f80, 0x5f5f90),
    (0x5f5f90, 0x5f5fc0),
    (0x5f61e0, 0x5f6210),
    (0x5f6a60, 0x5f6a90),
    (0x5f82a0, 0x5f8310),
    (0x5f8420, 0x5f8460),
    (0x5f8790, 0x5f87c0),
    (0x5f8a40, 0x5f8a50),
    (0x5fa680, 0x5fa6b0),
    (0x5fcce0, 0x5fccf0),
    (0x5fccf0, 0x5fce80),
    (0x5fce80, 0x5fd060),
    (0x617360, 0x617370),
    (0x617370, 0x617430),
    (0x6175c0, 0x617600),
    (0x6181c0, 0x618250),
    (0x618ca0, 0x618cb0),
    (0x619800, 0x619870),
    (0x6300b0, 0x6300c0),
    (0x6307c0, 0x6307f0),
    (0x630c30, 0x630c60),
    (0x632380, 0x6323d0),
    (0x67afa0, 0x67b0d0),
    (0x6af760, 0x6af780),
    (0x6b0160, 0x6b0170),
    (0x6b01a0, 0x6b01b0),
    (0x6b01e0, 0x6b01f0),
    (0x6b0220, 0x6b0230),
    (0x6b0750, 0x6b0770),
    (0x6b1e20, 0x6b1e40),
    (0x6b2a50, 0x6b2a70),
    (0x6b3c40, 0x6b4ab0),
    (0x6bf320, 0x6bf330),
    (0x6c1230, 0x6c1240),
    (0x6c5ae0, 0x6c5af0),
    (0x6d8990, 0x6d89a0)
], hooks={0x55e470: lambda e: e.write(e.r['ECX'], struct.pack('<I', 0x6fd800))})   # CpuInfo_Ctor (CPUID): driver.cpp's stub_mark, which its Drives env has
PICK = [f for f in FUNCS if not os.environ.get('ONLY') or f[0] in [int(x, 16) for x in os.environ['ONLY'].split(',')]]
THIS, POINTEE = RAM + 0x300, RAM + 0x1000
OBJECTS, VTABLE, COM_VTABLE = RAM + 0x2000, RAM + 0x3000, RAM + 0x3100
DECREMENT, CREATE_SEMAPHORE = 0x6fd12c, 0x6fd16c   # the import table's InterlockedDecrement and CreateSemaphoreA
IMPORT = 40   # the recorder slot an import logs as (clear of the vtables' 32)
CLOCK, TIME_GET_TIME, QUERY_FREQUENCY, QUERY_COUNTER = 0xfa71e0, 0x6fd3ec, 0x6fd0ac, 0x6fd0cc   # the game's Timer; timeGetTime, QueryPerformanceFrequency / Counter's import slots


def mix(r, n):
    """n dwords: a third are 0, a third point into POINTEE's block, the rest are random with the top bit set (so that they fault in both runs, not read the exe's data in the emulator only)."""
    return b''.join((0 if r.random() < 0.33 else POINTEE + 4 * r.randrange(0x100) if r.random() < 0.5 else r.getrandbits(32) | 0x80000000).to_bytes(4, 'little') for _ in range(n))



def string_list(r, regions):
    """A StringList (vtable, head, tail, count) whose head runs through 0..6 nodes of 0x1c bytes in POINTEE's block (a String of random words, then next at +0x14, prev), ending in 0
    or, now and then, a wild pointer with the top bit set."""
    count = r.choice([0, 1, 1, 2, 3, 4, 6])
    links = [POINTEE + 0x1c * k for k in r.sample(range(0x24), count)] + [0 if r.random() < 0.9 else r.getrandbits(32) | 0x80000000]
    block = bytearray(regions[POINTEE])
    for here, nxt in zip(links, links[1:]):
        put(block, here - POINTEE + 0x14, 'I', nxt)
    regions[POINTEE] = block
    return b''.join(x.to_bytes(4, 'little') for x in [r.getrandbits(32), links[0], r.getrandbits(32), count])



def ref_objects(r, regions):
    """Up to three reference-counted objects 0x40 apart at OBJECTS (RefObject: link +4, refs +8; RefObject2: link +4, refs +0xc), each linked to the next or not, their vtables
    recorders (the teardown, the deleting destructor with its flag), InterlockedDecrement a recorder answering a count from 2 down to -1 (or its hash)."""
    block = bytearray(r.bytes(0xc0))
    for k in range(3):
        put(block, 0x40 * k, 'I', VTABLE)
        put(block, 0x40 * k + 4, 'I', OBJECTS + 0x40 * (k + 1) if k < 2 and r.random() < 0.5 else 0)
    regions[OBJECTS] = block
    regions[VTABLE] = vtable({0: 1})
    regions[DECREMENT] = struct.pack('<I', recorder(IMPORT, 1, True))
    log = regions[LOG] = log_region()
    if r.random() < 0.8:
        set_return(log, IMPORT, r.choice([2, 1, 0, 0, 0, -1]))
    return OBJECTS if r.random() < 0.8 else 0


def case(r):
    address, form, imm, ret = r.choice(PICK)
    regions = {HEAP: heap(), POINTEE: r.bytes(0x400)}
    if form == 'handle':   # a Handle (vtable, object) at this (+ the stub's adjustment)
        obj = bytearray(r.bytes(0x400))
        put(obj, 0x300 + imm + 4, 'I', ref_objects(r, regions))
        regions[RAM] = obj
        return Call(address, [THIS], regions, conv='t', ret=ret)
    if form == 'stream':   # a StreamBase (vtable, source)
        regions[imm] = struct.pack('<2I', r.getrandbits(32), ref_objects(r, regions))
        return Call(address, [], regions, conv='c', ret=ret)
    if form == 'semaphore':   # CreateSemaphoreA a recorder (its four arguments logged, its hash the handle)
        regions[imm] = r.bytes(4)
        regions[CREATE_SEMAPHORE] = struct.pack('<I', recorder(IMPORT, 4, True))
        regions[LOG] = log_region()
        return Call(address, [], regions, conv='c', ret=ret)
    if form == 'surfaces':   # the two COM surfaces at +0xac and +0xb0 (or none), Release a recorder
        obj = bytearray(r.bytes(0xb4))
        put(obj, 0xac, 'I', OBJECTS if r.random() < 0.7 else 0)
        put(obj, 0xb0, 'I', OBJECTS + 0x40 if r.random() < 0.7 else 0)
        regions[imm] = obj
        regions[OBJECTS] = struct.pack('<I', COM_VTABLE) + bytes(0x3c) + struct.pack('<I', COM_VTABLE)
        regions[COM_VTABLE] = vtable({}, com={2: 1})
        regions[LOG] = log_region()
        return Call(address, [], regions, conv='c', ret=ret)
    if form == 'timer':   # the clock (enabled, ticks a ms, base ms, pad, start), timeGetTime a recorder, the performance counter COUNTER
        log = regions[LOG] = log_region()
        if r.random() < 0.7:
            set_return(log, IMPORT, r.getrandbits(32))
        regions[TIME_GET_TIME] = struct.pack('<I', recorder(IMPORT, 0, True))
        regions[QUERY_FREQUENCY] = regions[QUERY_COUNTER] = struct.pack('<I', COUNTER)
        ticks = r.choice([1, 0xa, 0x3e8, 0xda7, r.randrange(1, 2 ** 25), r.getrandbits(32) | 1])
        start = r.getrandbits(64) >> r.choice([0, 1, 20, 40])
        if imm == 'ms':   # (now - start) / ticks kept within 31 bits: past that the original's IDIV faults where the port does not (see Timer_GetMs)
            now = start + r.randrange(-2 ** 30, 2 ** 30) * ticks + r.randrange(ticks)
        else:
            now = r.choice([0, 0x369e99, 0x1234de, 0x3e8, 0x7c, 0x1e848000, r.getrandbits(40), r.getrandbits(64)])
        counter(log, now, r.getrandbits(r.choice([8, 32, 64])), r.getrandbits(2) if r.random() < 0.4 else 3)
        regions[CLOCK] = struct.pack('<4IQ', r.choice([0, 1, 1, r.getrandbits(32)]), ticks, r.getrandbits(32), r.getrandbits(32), start % 2 ** 64)
        return Call(address, [], regions, conv='c', ret=ret)
    if form == 'list':
        regions[imm] = string_list(r, regions)
        return Call(address, [], regions, conv='c', ret=ret)
    if form == 'global':
        regions[imm] = mix(r, 0x800 if address == 0x618ca0 else 0x100)   # (the canvas is large and the native run maps just what is given)
        return Call(address, [], regions, conv='c', ret=ret)
    obj = bytearray(mix(r, 0x200))
    put(obj, 0x2fc, 'I', r.choice([0, 0, 4, 8, 0x10, 0x20, 0x40, 0x70]))   # the offset an adjusting thunk takes off this
    regions[RAM] = obj
    return Call(address, [] if form == 'cdecl' else [THIS], regions, conv='c' if form == 'cdecl' else 't', ret=ret)


if __name__ == '__main__':
    B.main(case)
