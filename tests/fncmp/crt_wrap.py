# The C library's thin wrappers (src/game/core/crt): atoi, isalpha, isdigit, isspace, isalnum, iswspace, atof, strncpy against the originals, with crt.py's
# inputs; the ports here, not the driver's stand-ins (crt.py checks those).
# Bench limit: operator new and delete (0x67acc0, 0x67acb0) are the driver's and emulator's heap on both sides, so not here.
# usage: crt_wrap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import crt  # noqa: E402
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x67a400, 0x6902c0)])
KINDS = ['atoi', 'strncpy', 'iswspace', 'isalpha', 'isdigit', 'isspace', 'isalnum', 'atof']


def case(r):
    op = r.choice(OPS)
    if op in crt.CLASSES and r.random() < 0.4:   # (MB_CUR_MAX 2: _isctype, for -1 to 0xff only: past that it asks Windows)
        c = r.choice([r.randrange(-1, 256), r.choice(b' \t\n09azAZ_.')])
        return Call(crt.CLASSES[op], [c & 0xffffffff], {0x77e7dc: struct.pack('<i', 2)}, ret='a')
    crt.OPS = [op]
    return crt.case(r)


OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
