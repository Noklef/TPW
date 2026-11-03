# A channel voice's resume by its handle (src/game/core/channelvoice: 0x6c5b20) against the original: the real semaphore lock over its import recorders, the pool's find, the two events and the owner's object's method recorders.
# usage: voiceresume.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, import_table, log_region, recorder, set_return  # noqa: E402

WAIT, RELEASE, RESET, SET, METHOD = 1, 2, 3, 4, 5
FIND = 15
B = Bench([(0x6c5b20, 0x6c5bdf), (0x6307f0, 0x630880)], hooks={0x6baa60: _recorder(FIND, 1)})
SELF = RAM + 0x1000
OWNER = RAM + 0x1100
OUT = RAM + 0x1200
OBJECT = RAM + 0x1300
OBJECT_VT = RAM + 0x1400
VOICE = RAM + 0x1500
EVENT = RAM + 0x1600


def case(r):
    log = log_region()
    table = import_table(log, {'Wait': (WAIT, 0x6fd128, 2, 0, 0), 'Release': (RELEASE, 0x6fd170, 3, 0, 0), 'Reset': (RESET, 0x6fd168, 1, 0, 0), 'Set': (SET, 0x6fd124, 1, 0, 0)})
    set_return(log, WAIT, r.choice([0, 0, 0, 0x102]))
    set_return(log, RESET, r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    set_return(log, SET, r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    set_return(log, FIND, r.choice([VOICE, VOICE, VOICE, VOICE, 0]))
    blob(log, FIND, 0, 1)
    self_ = bytearray(r.randbytes(0x10))
    struct.pack_into('<I', self_, 4, r.choice([r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), 0]))
    owner = bytearray(r.randbytes(0x20))
    struct.pack_into('<2I', owner, 0x14, RAM + 0x1700, OBJECT)
    voice = bytearray(r.randbytes(0x80))
    struct.pack_into('<I', voice, 0x70, EVENT)
    vt = bytearray(0x28)
    struct.pack_into('<I', vt, 0x24, recorder(METHOD, 2))
    regions = {LOG: log, IMPORT_TABLE: table, SELF: self_, OWNER: owner, OUT: r.randbytes(4), OBJECT: struct.pack('<I', OBJECT_VT), OBJECT_VT: vt, VOICE: voice, EVENT: r.randbytes(4), 0xfb205c: r.randbytes(4)}
    return Call(0x6c5b20, [SELF, OUT, OWNER], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
