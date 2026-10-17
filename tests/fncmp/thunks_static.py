# The static-initialiser and destructor stubs and the other jumping entry points of src/game/core/thunks (MOV ECX, a global; JMP a ported constructor or destructor, a direct JMP, this raised or lowered first)
# against the originals: the global object (or an object behind this) of random bytes (many dwords pointing into a second block, so that chains of pointers can be followed), a heap. Entries whose targets call imports (the handle destructors, the
# time functions, the file stream destructor, the semaphore constructor, the screen surfaces destructor), differ only by the emulator reading the exe's data where the driver faults (the string list destructor), need the game's colour environment (the canvas constructor) or run CPUID (the system information constructor) cannot run in the emulator and are not in the list.
# usage: thunks_static.py [CASES [SEED]] [--wild|--both] [--show N]   (env ONLY=409240,... picks stubs)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

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
])
PICK = [f for f in FUNCS if not os.environ.get('ONLY') or f[0] in [int(x, 16) for x in os.environ['ONLY'].split(',')]]
THIS, POINTEE = RAM + 0x300, RAM + 0x1000


def mix(r, n):
    """n dwords: a third are 0, a third point into POINTEE's block, the rest are random with the top bit set (so that they fault in both runs, not read the exe's data in the emulator only)."""
    return b''.join((0 if r.random() < 0.33 else POINTEE + 4 * r.randrange(0x100) if r.random() < 0.5 else r.getrandbits(32) | 0x80000000).to_bytes(4, 'little') for _ in range(n))


def case(r):
    address, form, imm, ret = r.choice(PICK)
    regions = {HEAP: heap(), POINTEE: r.bytes(0x400)}
    if form == 'global':
        regions[imm] = mix(r, 0x800 if address == 0x618ca0 else 0x100)   # (the canvas is large and the native run maps just what is given)
        return Call(address, [], regions, conv='c', ret=ret)
    obj = bytearray(mix(r, 0x200))
    put(obj, 0x2fc, 'I', r.choice([0, 0, 4, 8, 0x10, 0x20, 0x40, 0x70]))   # the offset an adjusting thunk takes off this
    regions[RAM] = obj
    return Call(address, [] if form == 'cdecl' else [THIS], regions, conv='c' if form == 'cdecl' else 't', ret=ret)


if __name__ == '__main__':
    B.main(case)
