# The definitions reloaded (src/game/world/objectdefs: 0x413140 ObjectDefs_ReloadAll) against the original, each callee a recorder: the effects let go (k 1), the four buffers freed (k 2), the definition destroyed (k 9) and freed (cdecl k 6), the named list's find (k 4, its out filled or not),
# the folders' load (k 5), RemoveFontResourceA (stdcall k 8), File_Delete (cdecl k 7, writing its result), the fonts' slot 3 (k 3). 0-4 definitions, some null, the count about them or negative; 0-4 fonts; the finds and the loads answering 0 or more.
# usage: objectdefsreload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, recorder, strings, vtable  # noqa: E402

RELEASE, FOUR, SLOT, FIND, LOAD, FREE, DELETE, REMOVE, DTOR = 1, 2, 3, 4, 5, 6, 7, 8, 9
B = Bench([(0x413140, 0x413338)], hooks={0x4145f0: _recorder(RELEASE, 0), 0x413340: _recorder(FOUR, 0), 0x412cb0: _recorder(DTOR, 0), 0x421770: _recorder(FIND, 2), 0x413930: _recorder(LOAD, 1),
                                         0x67acb0: _recorder(FREE, 1, cdecl=True), 0x5f5d60: _recorder(DELETE, 2, cdecl=True)})
DEFS, FONTS, VT, FINDS, LOADS = RAM, RAM + 0x1000, RAM + 0x1200, RAM + 0x1300, RAM + 0x1400


def case(r):
    log = log_region()
    defs = bytearray(r.bytes(0x740))
    count = r.randrange(0, 5)
    for i in range(count):
        put(defs, 4 * i, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
    put(defs, 0x528, 'i', r.choice([count, count, count, 0, -1, -r.randrange(1 << 31)]))
    fonts = r.randrange(0, 5)
    vector = bytearray(r.bytes(0x14 * max(fonts, 1)))
    for i in range(fonts):
        put(vector, 0x14 * i, 'I', VT)
    answers(log, FIND, FINDS)
    answers(log, LOAD, LOADS)
    strings(log, FIND, 0, out=1 << 1)
    if r.random() < 0.7:
        fill(log, FIND, 1, r.bytes(0x14), records=5)
    strings(log, DELETE, 0, out=1 << 0)
    fill(log, DELETE, 0, r.bytes(0x14), which=1, records=5)
    regions = {LOG: log, DEFS: defs, FONTS: vector, VT: vtable({SLOT: 1}), FINDS: struct.pack('<40I', *[r.choice([1, 1, 1, 1, 0, 2]) for _ in range(40)]), LOADS: struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 1, 0, 0x100]) for _ in range(40)]),
               0x786b9c: r.bytes(4), 0x789c1c: struct.pack('<II', FONTS, FONTS + 0x14 * fonts), 0x6fd058: struct.pack('<I', recorder(REMOVE, 1, True))}
    return Call(0x413140, [DEFS, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
