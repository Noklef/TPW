# The creators of the engine's two singletons with a virtual base (src/game/core/vwrap: 0x569e10 for 0x879acc, 0x5666f0 for 0x879ac0) against the
# originals: there already (sometimes) or made on fncmp's heap, whose allocation fails now and then (the error then pushed onto the engine's list at
# 0x879ad0, the log 0x564650 fncmp's cdecl logger, as in enginecreate.py); the vbtables 0x701410 / 0x701378 as in the game, the vtables 0x7013c8 /
# 0x701330 recorders (slot 1 is called); the base constructor's driver quirk globals as in class701378.py.
# usage: singletons.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, heap, log_region, put, strings, vtable  # noqa: E402

B = Bench([(0x569e10, 0x569eed), (0x5666f0, 0x566792), (0x5651c0, 0x565220), (0x5667d0, 0x566850), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60),
           (0x67a400, 0x690000)], hooks={0x564650: _cdecl_log})
OPS = {'879acc': (0x569e10, 0xc, 0x38), '879ac0': (0x5666f0, 0, 0x5c)}


def case(r):
    address, slot, size = OPS[r.deal(list(OPS))]
    log = log_region()
    strings(log, 0x3f, 1)
    # 0x879ab0: the quirk flag 0x879ab4, the singletons 0x879ac0 / 0x879acc, the error list 0x879ad0 (count at +0x200)
    block = bytearray(r.bytes(0x224))
    put(block, 0x10, 'I', 0)
    put(block, 0x1c, 'I', 0)
    put(block, 0x10 + slot, 'I', RAM + 0x3000 if r.random() < 0.2 else 0)
    put(block, 0x220, 'i', r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = bytearray(r.bytes(0x20))
    put(flags, 0, 'I', struct.unpack_from('<I', flags)[0] & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0))
    put(flags, 0x1c, 'I', r.choice([1, 1, 0, 4]))
    regions = {0x879ab0: block, 0x879a20: put(bytearray(r.bytes(0x20)), 4, 'I', RAM + 0x2000),
               RAM + 0x2000: put(bytearray(r.bytes(0x500)), 0x428, 'I', r.choice([0x10de, 0x10de, 0x1002, 0])), 0x8bd508: flags,
               0x701410: struct.pack('<2I', 0, 8), 0x701378: struct.pack('<2I', 0, 0x2c), 0x7013c8: vtable({}, 2), 0x701330: vtable({}, 2),
               RAM + 0x3000: r.bytes(0x10), HEAP: heap(size + 0x40, 1 if r.random() < 0.3 else 0), LOG: log}
    return Call(address, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
