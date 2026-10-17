# The constructors of src/game/core/initializers that run other constructors (0x6181c0 Class7037e8_Ctor, 0x47eab0 Class6fefc8_Ctor, 0x6175c0 Class703710_Ctor) against the originals: an object of random bytes; the
# nested constructors (0x632380, 0x661870, 0x5fa680) are run, not stubbed. Also the 0x6fefc8 destructor (0x47eb40): its surface released through a recorder
# (Surface_Release 0x5f9130 run), then the base's vtable (0x6618a6).
# usage: class_ctors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor7037e8,Ctor6fefc8,Ctor703710,Dtor6fefc8)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, log_region, put, vtable  # noqa: E402

CALLS = {'Ctor7037e8': 0x6181c0, 'Ctor6fefc8': 0x47eab0, 'Ctor703710': 0x6175c0, 'Dtor6fefc8': 0x47eb40}
B = Bench([(0x6181c0, 0x618250), (0x632380, 0x6323d0), (0x47eab0, 0x47eb30), (0x661870, 0x6618ba), (0x5fa680, 0x5fa6b0), (0x6175c0, 0x617600),
           (0x47eb40, 0x47eba9), (0x5f9130, 0x5f9151)])
SURFACE, VTC = RAM + 0x100, RAM + 0x200


def case(r):
    name = r.choice(OPS)
    if name == 'Dtor6fefc8':   # (byte +7 and the dword +0x64 decide whether its surface (+0x14; the IDirectDrawSurface at +0x38) is released)
        obj = bytearray(r.bytes(0x80))
        obj[7] = r.choice([0, 0, 1, 0x80])
        put(obj, 0x64, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(obj, 0x38, 'I', SURFACE if r.random() < 0.95 else 0)
        regions = {RAM: obj, SURFACE: struct.pack('<I', VTC) + r.bytes(0x10), VTC: vtable({}, com={2: 1}), LOG: log_region()}
        return Call(CALLS[name], [RAM], regions, conv='t', ret='a')
    return Call(CALLS[name], [RAM], {RAM: r.bytes(0x80)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
