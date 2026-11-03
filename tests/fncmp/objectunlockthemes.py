# The six theme checks (src/game/world/objectunlock: ObjectUnlock_CheckThemes, 0x4d4bc0) against the original. Thiscall recorders (their own benches have them; driver.cpp's kObjectUnlockThemesEnv): GlobalSave_ThemeByte 10, GlobalSave_MarkTheme 11,
# ObjectUnlock_AdviseLevel 12, the table's getters 13 and 18, ParkAnalyser_Total 14, Things_AveragePersonStat19c 16 (st0), Money_Total 19, ParkAnalyser_TotalSince 20, FunnyTime_Elapsed 21 (its out the stack's, logged as 0; the answer the case's); cdecl
# Things_CountKind1OnPlainCell 15, Research_AllDone 17. Each figure over, at or under its limit [0x785790] to [0x7857a8]. GlobalSave_Instance and _alldiv run. The original's debug print (0x5af4a0 of the instance, the empty 0x5da3c0) is left out by the port.
# usage: objectunlockthemes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, set_float_return, set_return, strings  # noqa: E402

B = Bench([(0x4d4bc0, 0x4d4e4d), (0x409410, 0x409490), (0x5da3c0, 0x5da3c1), (0x67c320, 0x67c3e0)],
          hooks={0x5af7f0: _recorder(10, 1), 0x5af810: _recorder(11, 1), 0x4d4fa0: _recorder(12, 2), 0x519510: _recorder(13, 0), 0x4c94c0: _recorder(14, 0), 0x4c9130: _recorder(15, 0, cdecl=True), 0x4c7bb0: _recorder(16, 1),
                 0x4ca6b0: _recorder(17, 0, cdecl=True), 0x5195d0: _recorder(18, 0), 0x4d09f0: _recorder(19, 0), 0x4c94d0: _recorder(20, 1), 0x4f8800: _recorder(21, 1), 0x5af4a0: lambda e: None})
SELF, GAME, INFO, BYTES, MARKS, PEOPLE, TIME = RAM + 0x100, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300
MONTH = 0x1792f8648000


def limit(r):
    return r.choice([0, 1, 5, 100, r.randrange(-100, 100), r.getrandbits(31), -r.getrandbits(31)])


def near(r, x):
    return (x + r.choice([1, 1, 2, r.randrange(1, 100), 0, -1, r.randrange(-100, 0)])) & 0xffffffff


def table(values):
    return struct.pack('<40I', *(values + [0] * 40)[:40])


def case(r):
    log = log_region()
    limits = [limit(r) for _ in range(7)]
    answers(log, 10, BYTES)
    answers(log, 11, MARKS)
    answers(log, 15, PEOPLE)
    set_return(log, 14, near(r, limits[0]))
    set_float_return(log, 16, r.choice([limits[2] + 0.5, limits[2] + 0.5, float(limits[2]), limits[2] - 0.5, float('nan'), float('inf')]))
    set_return(log, 17, r.choice([0, 1, 1, 0x100, r.getrandbits(32)]))
    set_return(log, 13, r.getrandbits(32))
    set_return(log, 18, r.getrandbits(32))
    set_return(log, 19, near(r, limits[4]))
    set_return(log, 20, near(r, limits[5]))
    set_return(log, 21, TIME)
    strings(log, 21, 0, out=1)
    months = near(r, limits[6]) if r.random() < 0.8 else r.getrandbits(32)
    if months & 0x80000000:
        months -= 1 << 32
    ticks = (months * MONTH + r.choice([0, 0, 1, -1, r.randrange(MONTH)])) & 0xffffffffffffffff
    regions = {LOG: log, 0x80239c: struct.pack('<I', GAME), 0xf7c58c: struct.pack('<I', INFO), 0x785790: struct.pack('<7i', *limits),
               BYTES: table([r.choice([0, 0, 0, 1, 0x100]) for _ in range(6)]), MARKS: table([r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]) for _ in range(6)]),
               PEOPLE: table([near(r, limits[1]), near(r, limits[3])]), TIME: struct.pack('<Q', ticks)}
    return Call(0x4d4bc0, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
