# A scene's check of its theme's level (src/game/frontend/scenesteps: 0x5e1cc0) against the original. Recorders: String_CtorStr 1, GlobalSave_ThemeGet 2, String_Dtor 3, GlobalSave_Construct 4,
# GlobalSave_CountSetLevel 5, ThemeRecord_GlobalSamField14 6, Setting_ReadMode 19, crt_atexit 20, slot 0x44 17. The string is a local (its address not compared). The state, the holder, the guard, the
# mode setting, the save info there or not and the block for it failing, the record found or not, and the level and the record's field on each side of each other.
# usage: themecheck.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, heap, log_region, mask_this, put, set_return, strings, vtable  # noqa: E402

CTOR, GET, DTOR, MAKE, LEVEL, FIELD, READ, ATEXIT = 1, 2, 3, 4, 5, 6, 19, 20
B = Bench([(0x5e1cc0, 0x5e1e30), (0x67acc0, 0x67acc1)],
          hooks={0x5f60b0: _recorder(CTOR, 1), 0x5b09c0: _recorder(GET, 1), 0x5f61e0: _recorder(DTOR, 0), 0x5aef50: _recorder(MAKE, 0), 0x5af680: _recorder(LEVEL, 0), 0x5b11a0: _recorder(FIELD, 0),
                 0x550ca0: _recorder(READ, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
THIS, HOLDER, VT, SAVE, RECORD, RECORDS, LEVELS, FIELDS = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x300, RAM + 0x380, RAM + 0x400, RAM + 0x500, RAM + 0x600
NUMBERS = [-1, 0, 0, 1, 1, 2, 5, 0x7fffffff, -0x80000000]


def case(r):
    log = log_region()
    mask_this(log, CTOR)
    mask_this(log, DTOR)
    strings(log, GET, 0, out=1)
    set_return(log, MAKE, SAVE)
    answers(log, GET, RECORDS)
    answers(log, LEVEL, LEVELS)
    answers(log, FIELD, FIELDS)
    this = bytearray(r.bytes(0x40))
    put(this, 0, 'I', VT)
    put(this, 0xc, 'I', r.choice([HOLDER] * 9 + [0]))
    put(this, 0x14, 'I', r.choice([0] * 9 + [2, r.getrandbits(32)]))
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1, 2])), THIS: this, HOLDER: r.bytes(0x40), VT: vtable({17: 0}), SAVE: r.bytes(0x10), RECORD: r.bytes(0x10),
               0x00fb2758: bytes([r.choice([0, 1, 0xfe, 0xff])]), 0x00fb3b7c: struct.pack('<I', r.choice([0, 1, 1, 1, 2, r.getrandbits(32)])), 0x00f7c58c: struct.pack('<I', r.choice([0, 0, SAVE])),
               RECORDS: struct.pack('<48I', *[r.choice([0, RECORD, RECORD, RECORD]) for _ in range(48)]),
               LEVELS: b''.join(struct.pack('<i', r.choice(NUMBERS)) for _ in range(48)), FIELDS: b''.join(struct.pack('<i', r.choice(NUMBERS)) for _ in range(48))}
    return Call(0x5e1cc0, [THIS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
