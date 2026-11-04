# The action replay's update (src/game/coaster/arreplay: 0x4041d0) against the original: the step (0x4045b0) a recorder answering in turn from a table, copying +0x2c to the wait (+0x30) as it goes (driver.cpp's arreplay_step_recorder) so a step
# can end the waiting; the streams a vtable's recorder, the calls recorders; the modes and waits.
# usage: arreplayupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, set_return, vtable  # noqa: E402

STEP, OBJECT, FIRST, CLOSE, FRAME, LAST = 1, 2, 3, 4, 5, 6   # (4: the streams' slot)
_record = _recorder(STEP, 0)


def step(e):   # (as the driver's arreplay_step_recorder)
    me = e.r['ECX']
    e.write(me + 0x30, struct.pack('<I', e.u32(me + 0x2c)))
    _record(e)


B = Bench([(0x4041d0, 0x4045b0), (0x5da3c0, 0x5da3c1)],
          hooks={0x4045b0: step, 0x5195d0: _recorder(OBJECT, 0), 0x527940: _recorder(FIRST, 1, cdecl=True), 0x5516b0: _recorder(FRAME, 0, cdecl=True), 0x439390: _recorder(LAST, 0, cdecl=True)})
SELF, VT, RESULT, ANSWERS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400
GAME = 0x800000   # (the game object: only its +0x1da70c read, for the debug print)


def case(r):
    log = log_region()
    me = bytearray(r.bytes(0x40))
    for at in (0xc, 0x14, 0x1c):
        put(me, at, 'I', VT)
    put(me, 4, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(me, 0x24, 'I', r.choice([1] * 15 + [0]))
    put(me, 0x28, 'i', r.choice([0, 0, 1, 1, 1, 2, 2, 3, -1]))
    put(me, 0x2c, 'I', r.choice([0, 0, 1, 3, 9]))
    put(me, 0x30, 'I', r.choice([0, 0, 0, 1, 2, 3, 4, 0xffffffff]))
    set_return(log, OBJECT, RESULT)
    table = [r.choice([1, 1, 1, 0]) for _ in range(6)] + [0] * 34
    answers(log, STEP, ANSWERS)
    regions = {LOG: log, SELF: me, VT: vtable({CLOSE: 2}), RESULT: r.bytes(0x118), ANSWERS: struct.pack('<40I', *table), 0x7cf83c: struct.pack('<I', GAME), GAME + 0x1da70c: r.bytes(4)}
    return Call(0x4041d0, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
