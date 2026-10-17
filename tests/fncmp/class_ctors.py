# The constructors of src/game/core/initializers that run other constructors (0x6181c0 Class7037e8_Ctor, 0x47eab0 Class6fefc8_Ctor, 0x6175c0 Class703710_Ctor) against the originals: an object of random bytes; the
# nested constructors (0x632380, 0x661870, 0x5fa680) are run, not stubbed.
# usage: class_ctors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor7037e8,Ctor6fefc8,Ctor703710)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Ctor7037e8': 0x6181c0, 'Ctor6fefc8': 0x47eab0, 'Ctor703710': 0x6175c0}
B = Bench([(0x6181c0, 0x618250), (0x632380, 0x6323d0), (0x47eab0, 0x47eb30), (0x661870, 0x6618b6), (0x5fa680, 0x5fa6b0), (0x6175c0, 0x617600)])


def case(r):
    return Call(CALLS[r.choice(OPS)], [RAM], {RAM: r.bytes(0x80)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
