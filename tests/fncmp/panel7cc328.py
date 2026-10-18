# The 0x7cc328 panel (src/game/ui/panel7cc328: 0x4b4670) against the original: any object bytes, the script slot at +0x24 0, small or
# any; the sprite scripts not ready, so the kill (SpriteScript_Kill) does nothing.
# usage: panel7cc328.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Kill24)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Kill24': 0x4b4670}
B = Bench([(0x4b4670, 0x4b4695), (0x475550, 0x47558b)])


def case(r):
    op = r.deal(OPS)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x24, 'I', r.choice([0, 0, 1, 2, 3, 0xffffffff, r.randrange(8)]))
    return Call(CALLS[op], [RAM], {RAM: obj, 0x7b49f0: bytearray(16)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
