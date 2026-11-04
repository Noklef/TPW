# The second saved pairs made (src/game/core/vobj2: Object_MakeSavedPairs130 0x57b120) against the original: a count of none to seven with indices that repeat, either allocation failing, a set's init failing at any place, sets without a texture; the callees recorders, the sets' constructor and the array iterator the game's.
# usage: savedpairsmake.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, heap, log_region, set_return  # noqa: E402

INIT, TEXTURE, LOCK, RELEASE, FREE = 1, 2, 3, 4, 5
B = Bench([(0x57b120, 0x57b2d2), (0x575bd0, 0x575be5), (0x67a400, 0x690000)],
          hooks={0x575ee0: _recorder(INIT, 1), 0x575f90: _recorder(TEXTURE, 1), 0x590d40: _recorder(LOCK, 0), 0x57b2e0: _recorder(RELEASE, 0), 0x579b80: _recorder(FREE, 1, cdecl=True)})
OBJECT, MODEL, TABLE, TEXTURES, ANSWERS = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500


def case(r):
    log = log_region()
    count = r.choice([0, 1, 1, 2, 3, 4, 5, 6, 7])
    inits = [1 if r.random() < 0.9 else 0 for _ in range(40)]
    inits = [r.choice([1, 2, 0x100, 0x80000000]) if a else 0 for a in inits]
    textures = [r.choice([0, 0, 1, RAM + 0x900, RAM + 0x910]) for _ in range(40)]
    answers(log, INIT, ANSWERS)
    answers(log, TEXTURE, ANSWERS + 160)
    set_return(log, LOCK, r.getrandbits(32))
    model = bytearray(r.bytes(0x60))
    struct.pack_into('<I', model, 0x50, TABLE)
    obj = bytearray(r.bytes(0x140))
    struct.pack_into('<I', obj, 8, MODEL)
    indices = [r.randrange(8) for _ in range(count)]
    regions = {LOG: log, HEAP: heap(0x1000, fail=r.choice([0, 0, 0, 0, 0, 0, 0, 0, 1, 2])), OBJECT: bytes(obj), MODEL: bytes(model),
               TABLE: b''.join(struct.pack('<II', r.getrandbits(32), r.choice([0, RAM + 0xa00 + 0x10 * i])) for i in range(8)),
               TEXTURES: struct.pack('<8I', *(indices + [0] * (8 - count))), ANSWERS: struct.pack('<80I', *(inits + textures))}
    return Call(0x57b120, [OBJECT, TEXTURES, count], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
