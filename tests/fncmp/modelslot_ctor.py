# The model slot's element constructor of src/game/model/slots (0x429a60) against the original: a slot of random bytes; the part at +0x48 with virtual
# bases is built through the vbtables in the image, and the scene flags at 0x8bd508 are touched twice (SceneFlags_Touch 0x5836b0 run, its stamp from 0x8bcc40).
# usage: modelslot_ctor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x429a60, 0x429b4a), (0x5836b0, 0x5836c2)])


def case(r):
    regions = {RAM: r.bytes(0xb0), 0x8bd508: r.bytes(8), 0x8bcc40: r.bytes(4)}
    return Call(0x429a60, [RAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Ctor').split(',') if k]

if __name__ == '__main__':
    B.main(case)
