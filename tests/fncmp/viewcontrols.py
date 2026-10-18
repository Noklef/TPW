# The view objects' button steps of src/game/frontend/viewcontrols against the originals: an object of random bytes, its step (+0x14) and
# values (+0x48, +0x50) now and then ordinary floats.
# usage: viewcontrols.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lower48,Raise48,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Lower48': 0x5dd310, 'Raise48': 0x5dd320, 'Raise50': 0x5dd330, 'Lower50': 0x5dd340}
B = Bench([(0x5dd310, 0x5dd34a)])
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = r.deal(OPS)
    obj = bytearray(r.bytes(0x60))
    for off in (0x14, 0x48, 0x50):
        if r.random() < 0.7:
            put(obj, off, 'I', r.f())
    return Call(CALLS[op], [RAM], {RAM: obj}, conv='t')


if __name__ == '__main__':
    B.main(case)
