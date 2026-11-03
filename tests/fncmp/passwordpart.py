# The text part of a password field (src/game/render/passwordpart: PasswordPart_SetLayout, 0x4901d0) against the original. Recorder (driver.cpp's kPasswordPartEnv): TextPart_SetLayout 10 (the info's six dwords hashed, the +0x10 pointer by what it points to). wcscpy and _wcsset (0x6d76f0) run; the wide buffer 0x7ba058 compared.
# usage: passwordpart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put  # noqa: E402

B = Bench([(0x4901d0, 0x490250), (0x67bf10, 0x67bf3d), (0x6d76f0, 0x6d770e)], hooks={0x48fc40: _recorder(10, 2)})
PART, INFO, HOLDER, TEXT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    log = log_region()
    blob_bytes(log, 10, 1, 0x00f0ffff, inner=(0x10, 0xf))
    info = bytearray(r.bytes(0x20))
    put(info, 0x10, 'I', HOLDER)
    text = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(r.choice([0, 1, 2, 8, 40])))
    regions = {LOG: log, PART: r.bytes(16), INFO: info, HOLDER: struct.pack('<I', TEXT), TEXT: text + b'\0\0', 0x7ba058: r.bytes(0x60)}
    return Call(0x4901d0, [PART, r.choice([0, 1, 0x100, 0x100, 0x100, 0x1ff, 0xff, r.getrandbits(32)]), INFO], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
