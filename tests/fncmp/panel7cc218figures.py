# Panel7cc218_RefreshFigures (src/game/ui/panel7cc218: 0x4ade40) and Object_Field48AsInt (src/game/world/object: 0x4dd6d0) against the original: every call a recorder numbered as Panel7cc218FiguresEnv from 10 (17 not used: the field is real),
# the circular buffer's two calls real, Window_FindChild answering objects in turn (a checked one 0 now and then) whose slot 7 is recorder 27.
# usage: panel7cc218figures.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4ade40, 0x4ae135), (0x4dd6d0, 0x4dd6d8), (0x495cf0, 0x495d56), (0x67a830, 0x67a860), (0x5da3c0, 0x5da3c1)],
          hooks={0x4dd4e0: _recorder(10, 0), 0x65d7d0: _recorder(11, 1), 0x4dd670: _recorder(12, 0), 0x6acd60: _recorder(13, 5, cdecl=True), 0x65f5b7: _recorder(14, 1), 0x4e0560: _recorder(15, 3), 0x4df640: _recorder(16, 3), 0x4e2400: _recorder(18, 0),
                 0x485f60: _recorder(19, 2, cdecl=True), 0x4ad4e0: _recorder(20, 0), 0x4af030: _recorder(21, 0), 0x4ae140: _recorder(22, 4, cdecl=True)})
SELF, CHILDREN, VT, OBJECTS, OBJECT = RAM, RAM + 0x100, RAM + 0x300, RAM + 0x1000, RAM + 0x2000
FLOATS = [0.0, 1.0, 99.5, 100.0, -3.5, 1e12, float('inf'), float('nan')]


def obj(r):
    o = bytearray(r.bytes(0x300))
    put(o, 0x44, 'f', r.choice(FLOATS))
    put(o, 0x48, 'f', r.choice(FLOATS))
    return o


def figures(r):
    log = log_region()
    regions = {LOG: log}
    regions[VT] = put(vtable({}, size=0x10), 0x1c, 'I', recorder(27, 1))
    regions[OBJECTS] = struct.pack('<I', VT) * 40 * 4
    kids = [OBJECTS + 0x10 * i for i in range(40)]
    for i in range(6):
        if r.random() < 0.12:
            kids[i] = 0
    regions[CHILDREN] = struct.pack('<40I', *kids)
    answers(log, 11, CHILDREN)
    o = obj(r)
    full = r.random() < 0.5
    held = r.randrange(0, 31)
    o[0x1a8 + 0x84] = r.choice([0, 1]) if not full else 1
    put(o, 0x1a8 + 0x7c, 'I', r.randrange(0, 30))
    put(o, 0x1a8 + 0x80, 'I', held)
    for i in range(30):
        put(o, 0x1a8 + 4 + 4 * i, 'I', r.choice([r.randrange(1000), r.getrandbits(32)]))
    regions[OBJECT] = o
    regions[0x7c2658] = struct.pack('<I', OBJECT)
    regions[0x78ba94] = r.bytes(4)
    me = bytearray(r.bytes(0x40))
    put(me, 0, 'I', VT)
    put(me, 0x14, 'I', r.choice([RAM + 0x800, RAM + 0x800, RAM + 0x800, 0]))
    put(me, 0x24, 'I', r.choice([RAM + 0x900, 0]))
    status = r.randrange(4)
    put(me, 8, 'I', r.choice([status, status, r.randrange(4)]))
    regions[SELF] = me
    set_return(log, 19, status)
    set_return(log, 12, r.choice([0, 7, -7 & 0xffffffff, 0x80000000, r.getrandbits(32)]))
    for k in (10, 13, 15, 16, 18):
        set_return(log, k, r.getrandbits(32))
    blob_bytes(log, 13, 4, 0xfffff)
    strings(log, 13, 0, 1 << 1)
    strings(log, 14, 0, 1 << 0)
    return Call(0x4ade40, [SELF], regions, conv='t')


def field(r):
    return Call(0x4dd6d0, [OBJECT], {OBJECT: obj(r)}, conv='t', ret='a')


def case(r):
    return r.deal([figures, figures, figures, figures, field])(r)


if __name__ == '__main__':
    B.main(case)
