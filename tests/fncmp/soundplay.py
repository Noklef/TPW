# The sound play commands (src/game/core/soundplay: 0x51bfc0, 0x51c130, 0x485aa0, 0x486ae0, and the kids' sounds 0x551560, 0x551320 over
# the first) against the originals. The sound globals (on
# 0x802bc8, started 0x802bd4 each 0 or not, the manager 0x802bcc) and the category gSndCatUi 0x803a2c; the manager's vtable +8 a recorder
# logging the command as a hash of its 7 dwords (its result now and then set); the sound 0 or not, the position and range ints small,
# large (rounded as floats) or negative.
# usage: soundplay.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=PlayAt,PlayAtRange,UiPlay,UiPlay1d,KidPlay,KidPlayGraded)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'PlayAt': 0x51bfc0, 'PlayAtRange': 0x51c130, 'UiPlay': 0x485aa0, 'UiPlay1d': 0x486ae0, 'KidPlay': 0x551560,
         'KidPlayGraded': 0x551320}
B = Bench([(0x51bfc0, 0x51c048), (0x51c130, 0x51c1b8), (0x485aa0, 0x485abd), (0x486ae0, 0x486af9), (0x551320, 0x551560),
           (0x551560, 0x551600)])
MANAGER, VT = RAM + 0x100, RAM + 0x200


def coord(r):
    return r.choice([0, 1, -1, r.randrange(-2000, 2000), r.getrandbits(32) - (1 << 31), (1 << 24) + 1 + 2 * r.getrandbits(8)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.6:
        set_return(log, 2, r.getrandbits(32))
    regions = {LOG: log, 0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4),
               0x803a2c: r.bytes(4)}
    sound = r.choice([0, 1, 0x1d, r.getrandbits(16), r.getrandbits(32)])
    if op == 'UiPlay':
        return Call(CALLS[op], [sound], regions, ret='a')
    if op == 'UiPlay1d':
        return Call(CALLS[op], [], regions, ret='a')
    if op in ('KidPlay', 'KidPlayGraded'):  # (the kids' category at 0x803a24; the level and the grade's parts around the thresholds)
        regions[0x803a24] = r.bytes(4)
        level = r.choice([0, 1, 2, 3, 4, 7, 8, 9, 100, -1, -5, r.getrandbits(32)]) & 0xffffffff
        args = [level] + ([r.choice([0, 1, 49, 50, 75, 149, 150, 199, 1000, -1, -50, -151, r.getrandbits(32)]) & 0xffffffff
                           for _ in range(2)] if op == 'KidPlayGraded' else []) + [coord(r), coord(r), coord(r)]
        return Call(CALLS[op], args, regions, ret='a')
    args = [r.getrandbits(32), r.getrandbits(32), sound, coord(r), coord(r), coord(r)] + ([coord(r)] if op == 'PlayAtRange' else [])
    return Call(CALLS[op], args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
