# The decode dispatchers of src/game/gfx/wavelet (0x592060 WaveletCodec_Decode, 0x5920f0 WaveletCodec_DecodeScaled) against the originals: the full codec 0x54 bytes below the
# interface is a recorder (the log shows the slot and the forwarded arguments), versions 0x11, 0x12, 0x13, 0x14 and others; and the virtual base destructor 0x5919a0 (the vtable
# pointers it writes, the five blocks it frees, the heap's record of them).
# usage: wavelet_decode.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Decode,Scaled,Dtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x592060, 0x5920f0), (0x5920f0, 0x592150), (0x5919a0, 0x591a60), (0x67acb0, 0x67acb1)])
FULL, VT = RAM, RAM + 0x1000
CODEC = FULL + 0x54


def dtor_case(r):
    base = RAM + 0x100
    obj = bytearray(r.bytes(0x200))
    put(obj, 0x100 - 0x50, 'I', VT)
    for i in range(1, 6):
        put(obj, 0x100 - 0x10 * i + 8, 'I', r.choice([0, 0, RAM + 0x1000 + 0x40 * i, r.getrandbits(32)]))
    regions = {RAM: obj, VT: put(bytearray(r.bytes(0x10)), 4, 'I', r.choice([0x50, 0x54, 0x58, 0x5c, 0x60, 0x70, 0x90])), HEAP: heap()}
    return Call(0x5919a0, [base], regions, conv='t', ret='v')


def case(r):
    name = r.choice(OPS)
    if name == 'Dtor':
        return dtor_case(r)
    version = r.choice([0x11, 0x12, 0x13, 0x14, 0, -1, 0x7fffffff, 0x80000000, 0x12, 0x13])
    regions = {FULL: struct.pack('<I', VT) + r.bytes(0x60), VT: vtable({0: 10, 1: 13, 2: 10}), LOG: log_region()}
    args = [r.getrandbits(32) for _ in range(13 if name == 'Scaled' else 10)]
    scaled = name == 'Scaled'   # (the result is the recorder's hash of all the forwarded arguments; the scaled one leaves EAX alone below version 0x13)
    return Call(0x5920f0 if scaled else 0x592060, [CODEC, version & 0xffffffff] + args, regions, conv='t', ret='a' if not scaled or 0x12 < version < 0x80000000 else 'v')


OPS = [k for k in os.environ.get('OPS', 'Decode,Scaled,Dtor').split(',') if k]

if __name__ == '__main__':
    B.main(case)
