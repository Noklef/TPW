# The renderer's device class constructors (src/game/render/device: 0x5651c0 Class701260_Ctor, 0x5667d0 Class701378_Ctor) against the originals:
# an object whose vtable gives the virtual base offset, with the flag set or clear; the driver quirk globals (the adapter's vendor 0x10de or
# another, the driver kind 1 or another) for the interface's constructor.
# usage: device.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor1260,Ctor1378)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5651c0, 0x565220), (0x5667d0, 0x566850)])
VT, OBJ = RAM + 0x1000, RAM


def quirk_globals(r):
    return {0x879a20: put(bytearray(r.bytes(0x20)), 4, 'I', RAM + 0x2000), 0x879ab0: r.bytes(0x10),
            0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 0, 4])), RAM + 0x2000: put(bytearray(r.bytes(0x500)), 0x428, 'I', r.choice([0x10de, 0x10de, 0x1002, 0]))}


def ctor1260(r):
    regions = quirk_globals(r)
    regions[OBJ] = r.bytes(0x400)
    return Call(0x5651c0, [OBJ], regions, conv='t', ret='a')


def ctor1378(r):
    offset = r.choice([0x2c, 0x30, 0x34, 0x40, 0x20])
    obj = bytearray(r.bytes(0x100))
    put(obj, 0, 'I', VT)
    regions = quirk_globals(r)
    regions.update({OBJ: obj, VT: put(bytearray(r.bytes(0x10)), 4, 'I', offset)})
    return Call(0x5667d0, [OBJ, r.choice([0, 1, 1, 0xffffff00, 7])], regions, conv='t', ret='a')


def case(r):
    return {'Ctor1260': ctor1260, 'Ctor1378': ctor1378}[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', 'Ctor1260,Ctor1378').split(',') if k]

if __name__ == '__main__':
    B.main(case)
