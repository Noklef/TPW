# The adjusting thunks of src/game/core/thunks (this lowered by the dword before it, then a jump to a ported function: 0x42a0c0, 0x42a0d0, ...) against the originals: an object
# whose preceding dword is a small offset, random bytes (many dwords pointing into the object), the globals the target touches random, random stack arguments.
# usage: thunks_vbase.py [CASES [SEED]] [--wild|--both] [--show N]   (env ONLY=42a0c0,... picks thunks)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

# (address, convention, stack arguments, globals the target touches, arguments used as pointers)
FUNCS = [
    (0x42a0c0, 't', 0, [0x879dec], ()),
    (0x42a0d0, 't', 0, [], ()),
    (0x42a0e0, 't', 0, [], ()),
    (0x491f80, 't', 0, [0x8bcc40, 0x8bcc44, 0x8c47a0], ()),
    (0x491f90, 't', 0, [], ()),
    (0x564b10, 't', 0, [], ()),
    (0x5650a0, 't', 0, [], ()),
    (0x569cf0, 't', 1, [], ()),
    (0x569d80, 't', 0, [], ()),
    (0x56a690, 't', 1, [0xfbe010, 0xfbe044], ()),
    (0x56a6a0, 't', 2, [0xfbe038, 0xfbe044], ()),
    (0x56a6d0, 't', 3, [0x8bcbd0, 0x8bcbd4], ()),
    (0x56a6e0, 't', 0, [0xfbe038, 0xfbe044], ()),
    (0x56a710, 't', 4, [0x8bcbd0, 0x8bcbd4], ()),
    (0x56a730, 't', 3, [0x7013b4, 0x879dac, 0x8bcbd0], (2,)),
    (0x5738a0, 't', 0, [], ()),
    (0x5738b0, 't', 0, [0x879a24], ()),
    (0x573ef0, 't', 0, [0x8bd520, 0x8c1620], ()),
    (0x573f00, 't', 0, [], ()),
    (0x57ac70, 't', 0, [], ()),
    (0x57ac80, 't', 0, [0x701a10, 0x701a60, 0x8bb628, 0x8bb62c], ()),
    (0x57ca70, 't', 0, [], ()),
    (0x58c2b0, 't', 0, [], ()),
    (0x58c2d0, 't', 0, [], ()),
    (0x58c320, 't', 0, [], ()),
    (0x58c8e0, 't', 4, [], ()),
    (0x58c8f0, 't', 0, [], ()),
    (0x58c900, 't', 0, [0x702248], ()),
    (0x58c910, 't', 0, [0x702248], ()),
    (0x58c920, 't', 0, [], ()),
    (0x58c940, 't', 0, [0x702248], ()),
    (0x5987c0, 't', 0, [], ()),
    (0x5987d0, 't', 0, [], ()),
]
B = Bench([
           (0x40f260, 0x40f280), (0x424ec0, 0x424ee0), (0x42a010, 0x42a040), (0x42a050, 0x42a080), (0x42a0c0, 0x42a100), (0x491f80, 0x491fa0),
           (0x491f90, 0x491fb0), (0x564a50, 0x564a70), (0x564b10, 0x564b30), (0x564ee0, 0x564f00), (0x5650a0, 0x5650c0), (0x569cf0, 0x569d20),
           (0x569d80, 0x569da0), (0x569f20, 0x569f40), (0x569f30, 0x56a040), (0x56a430, 0x56a450), (0x56a440, 0x56a460), (0x56a510, 0x56a550),
           (0x56a690, 0x56a6c0), (0x56a6d0, 0x56a6f0), (0x56a6e0, 0x56a700), (0x56a710, 0x56a750), (0x573860, 0x573880), (0x5738a0, 0x5738c0),
           (0x5738b0, 0x5738d0), (0x573e30, 0x573e80), (0x573ef0, 0x573f10), (0x57ac70, 0x57acb0), (0x57ca70, 0x57ca90), (0x58a550, 0x58a5d0),
           (0x58c2b0, 0x58c310), (0x58c320, 0x58c350), (0x58c8a0, 0x58c8e0), (0x58c900, 0x58c948), (0x591a60, 0x591a80), (0x591a70, 0x591a90),
           (0x5987c0, 0x5987e0), (0x5987d0, 0x5987f0), (0x5cec60, 0x5cec80), (0x5d1410, 0x5d1430), (0x5d60c0, 0x5d60e0), (0x6b71c0, 0x6b71e0),
           (0x6bd790, 0x6bd7b0), (0x573e70, 0x573f20), (0x58c3c0, 0x58c760), (0x58c8e0, 0x58c900), (0x579b80, 0x579c13), (0x58b300, 0x58b320),
           (0x67acb0, 0x67acb1),
])
THIS = RAM + 0x300
FLOAT = {0x58c940}   # the thunks whose target returns a float (ST0)
PICK = [f for f in FUNCS if not os.environ.get('ONLY') or f[0] in [int(x, 16) for x in os.environ['ONLY'].split(',')]]


def fixups(r, address, regions, args):
    """Shapes the objects the targets read through pointers, so that the comparison reaches their arithmetic."""
    obj = regions[RAM]
    if address in (0x58c2d0, 0x58c910, 0x58c8e0, 0x58c8f0):   # the virtual base offset behind the vbtable pointer at this - 0x3c / - 0x10, the two dwords it leads to
        at = 0x300 - struct.unpack_from('<I', obj, 0x2fc)[0] - (0x3c if address == 0x58c2d0 else 0x10)   # (this lowered by the offset the thunk takes off)
        put(obj, at, 'I', RAM + 0x220)
        put(regions[RAM], 0x224, 'I', r.choice([0, 4, 0x10, 0x20, 0x30]))
        if address == 0x58c8e0:   # pixels already: 0 (TexAtlas_Alloc is not reached); none at this - 0xc itself now and then, so a
            if r.random() < 0.5:   # thunk that took nothing off would go on to allocate
                put(obj, 0x2f4, 'I', 0)
            put(obj, at + 4, 'I', r.getrandbits(32) | 1)
        if address == 0x58c8f0:   # the pixels 0, tracked or not (the table at 0x87b1f8, its count 0-4 at 0x8bb214)
            pairs = [r.getrandbits(32) | 0x80000000 for _ in range(10)]
            put(obj, at + 4, 'I', r.choice([0, pairs[1], pairs[3], (pairs[5] - 4) & 0xffffffff, r.getrandbits(32)]))
            regions[0x87b1f8] = b''.join(p.to_bytes(4, 'little') for p in pairs)
            regions[0x8bb214] = struct.pack('<I', r.randrange(5))
            regions.update({HEAP: heap(0x100), **global_heap()})
    elif address == 0x573f00:             # a new class B surface on the heap, now and then not got
        regions[HEAP] = heap(0x100, r.choice([0, 0, 0, 1]))
    elif address == 0x491f80:             # the scene's table: a count below and above 0x80, the slots after it
        put(obj, 0x2ec, 'II', r.choice([0, 1, 0x1234, r.getrandbits(32)]), r.choice([0, 1, 0x1234, r.getrandbits(32)]))
        scene = RAM + 0x1000
        regions[scene] = put(bytearray(0x2500), 0x240c, 'I', r.choice([0, 1, 0x7f, 0x80, 0x81, 0xffffffff]))
        regions[0x8c47a0 & ~0xf] = put(bytearray(0x20), 0x8c47a0 & 0xf, 'I', scene)
        regions[0x8bcc40 & ~0xf] = bytes(r.choice([0, 1])) + bytes(0x1f) if False else r.bytes(0x20)
    elif address == 0x5738b0:             # the object at 0x879a24 with the word at +0x4c4
        scene = RAM + 0x1000
        regions[scene] = r.bytes(0x4c8)
        regions[0x879a24 & ~0xf] = put(bytearray(0x20), 0x879a24 & 0xf, 'I', scene)
    elif address == 0x56a730:             # x and y inside a small table of pointers; the stride and the table behind 0x879dac
        regions[0x8bcbd0 & ~0xf] = put(bytearray(0x20), 0x8bcbd0 & 0xf, 'I', r.choice([0, 1, 4, 8, 16]))
        holder = RAM + 0x1000
        regions[0x879dac & ~0xf] = put(bytearray(0x20), 0x879dac & 0xf, 'I', holder)
        regions[holder] = put(bytearray(0x20), 8, 'I', RAM + 0x1100)
        regions[RAM + 0x1100] = r.bytes(0x200)
        args[0], args[1] = r.randrange(8), r.randrange(8)


def case(r):
    address, conv, n, globs, ptr = r.choice(PICK)
    obj = bytearray(r.bytes(0x800))
    for k in range(0, 0x800, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x200 + 4 * r.randrange(0x80))
    put(obj, 0x2fc, 'I', r.choice([0, 0, 4, 8, 0x10, 0x20, 0x40, 0x70]))   # the offset the thunk takes off this
    regions = {RAM: obj}
    for g in globs:   # (the same mix, so that pointers read from a global stay inside the object)
        regions[g & ~0xf] = b''.join((RAM + 0x200 + 4 * r.randrange(0x80) if r.random() < 0.5 else r.getrandbits(32)).to_bytes(4, 'little') for _ in range(8))
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    fixups(r, address, regions, args)
    for i in ptr:
        args[i] = RAM + 0x200 + 0x40 * min(i, 3) + r.choice([0, 4, 0x20])
    if conv == 't':
        args = [THIS] + args
    return Call(address, args, regions, conv=conv, ret='d' if address in FLOAT else 'a')


if __name__ == '__main__':
    B.main(case)
