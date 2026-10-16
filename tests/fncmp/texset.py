# The texture sets' constructor and getters (src/game/gfx/texset: 0x575bd0, 0x575f90, 0x575fa0)
# against the originals: any set bytes, textures 0 to 7 (and negative for the slot's address).
# usage: texset.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Texture,TextureSlot)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x575bd0, 0x575be5), (0x575f90, 0x575f9d), (0x575fa0, 0x575fad)])
CALLS = {'Ctor': 0x575bd0, 'Texture': 0x575f90, 'TextureSlot': 0x575fa0}
TEXTURES = RAM + 0x40


def case(r):
    op = r.choice(OPS)
    mem = bytearray(r.bytes(0x60))
    put(mem, 8, 'I', TEXTURES)
    if op == 'Ctor':
        return Call(0x575bd0, [RAM], {RAM: mem}, conv='t', ret='a')
    i = r.randrange(8) if op == 'Texture' else r.choice([r.randrange(8), -1, r.getrandbits(32)])
    return Call(CALLS[op], [RAM, i], {RAM: mem}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
