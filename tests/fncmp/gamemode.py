# The game mode changed (src/game/world/gamemode: GameMode_Set 0x52f200, GameMode_SetOnly 0x52f580) against the originals: the mode 0, 3,
# 0x15, 0x16, 1, 0x35 or another, the current one [0x81ae2c] the same or another, record 0, 1, 0x100 (a clear low byte) or any; the global
# recorder 0x785a08 as recorded_actions.py's (recording or not, its first stream a record stream); the advisor as advisor.py's PostId (its
# table, scoring and queue; the message's unwritten stack bytes masked the same) for mode 3; mousemode.py's Holder scene for mode 0 (the
# current mouse mode a recorder object or none, the holder [0x7b05e8] set or not, a heap whose allocations fail now and then, the mode
# setting 0xfb3b7c 1 with its guard set so that the CMMOnline base is replaced by a CMMOnline: the base's own Assign 0x46c6b0 is not in the
# bench), whose cursor windows serve GameCursor_Set too; the tool globals of mode 0 dwords of their own. Agent B's unported 0x59aa70 is
# thiscall recorder 13 of 1, 0x4474d0 cdecl recorder 14 of 2, 0x446050 cdecl recorder 15 of none (driver.cpp's kGameModeEnv).
# usage: gamemode.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,SetOnly)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import mousemode  # noqa: E402
from fncmp import HEAP, LOG, RAM, STUBS, Bench, Call, _cdecl_log, _recorder, heap, log_region, put, set_return  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


ADVISOR_RANGES = [(0x5997b0, 0x5997f8), (0x429fe0, 0x42a010), (0x44b2e0, 0x44b3d0), (0x474070, 0x4740e0), (0x5442e0, 0x544340), (0x5989c0, 0x598ae0),
                  (0x5ed580, 0x5ed5b0), (0x59a940, 0x59aa63), (0x59abc0, 0x59ae1c), (0x59c2b0, 0x59c680), (0x41a980, 0x41a986),
                  (0x41a990, 0x41a9a7), (0x409cd0, 0x409cdd), (0x59b590, 0x59b611), (0x59bf20, 0x59c093), (0x5194d0, 0x51950d)]
MODE_RANGES = [(0x40bed0, 0x40bf65), (0x46c460, 0x46c480), (0x46c6a0, 0x46c6b0), (0x46c710, 0x46c72e), (0x46ced0, 0x46ced7), (0x46cff0, 0x46d000),
               (0x46ce80, 0x46ceca), (0x402d70, 0x403220), (0x4033a0, 0x4033f0), (0x475550, 0x4755a0), (0x4758f0, 0x475b90), (0x540c60, 0x540da0),
               (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x4814a0, 0x4814e3), (0x4a2aa0, 0x4a2abf), (0x6590f7, 0x6591d4),
               (0x65f11d, 0x65f14e), (0x66190a, 0x66191b), (0x661d96, 0x661dca), (0x46c350, 0x46c448), (0x46d550, 0x46d58b), (0x550ca0, 0x550d90),
               (0x46cfb0, 0x46cfc0), (0x40f3a0, 0x40f3a6), (0x5d1750, 0x5d1751), (0x5d1410, 0x5d1416)]
B = Bench(merged(ADVISOR_RANGES + MODE_RANGES + [(0x52f200, 0x52f83b), (0x403d00, 0x403d56), (0x403be0, 0x403f00), (0x527940, 0x52794a),
                                                 (0x486b00, 0x486b3a), (0x67a400, 0x6902c0), (0x5da3c0, 0x5da3c1)]),
          hooks={0x67a7d0: _cdecl_log, 0x59aa70: _recorder(13, 1), 0x4474d0: _recorder(14, 2, cdecl=True), 0x446050: _recorder(15, 0, cdecl=True)})
CALLS = {'Set': 0x52f200, 'SetOnly': 0x52f580}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
VT, BUF = RAM + 0x20000, RAM + 0x20100
MODES = [0, 0, 0, 0, 3, 3, 3, 0x15, 0x16, 1, 1, 0x35, 2, 0xb, 0x10]


def case(r):
    op = r.deal(OPS)
    mode = r.choice(MODES)
    adv = advisor.queue(r, 'PostId')
    scene = mousemode.set_mode(r, True)
    regions = {**adv.regions, **scene.regions}
    regions[HEAP] = heap(fail=r.choice([0] * 6 + [1, 2, 3, 4]))
    log = log_region()
    set_return(log, 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
    set_return(log, 9, r.choice([1, 1, 1, 0, 7, r.getrandbits(32)]))
    regions[LOG] = log
    regions[0x7b05e8] = struct.pack('<I', r.choice([0, 0, RAM + 0x8300]))
    if mode == 0 or r.random() < 0.5:   # (the CMMOnline base always replaced)
        regions[0xfb3518] = bytes([regions[0xfb3518][0] | 1])
        regions[0xfb3b7c] = struct.pack('<I', 1)
    rec = bytearray(r.bytes(0x30))   # (recorded_actions.py's recorder: its flags, its first stream a record stream)
    struct.pack_into('<2I', rec, 0, r.choice([0, 1, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    struct.pack_into('<3I', rec, 0xc, VT, BUF, r.choice([0, 1, 2, 3, 4, 0x20, 0x20, 0x20, 0x20]))
    struct.pack_into('<I', rec, 0x24, r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    regions.update({0x785a08: rec, VT: put(bytearray(r.bytes(0x10)), 0xc, 'I', STUBS + 0x10), BUF: r.bytes(0x20)})
    current = mode if r.random() < 0.3 else r.choice([0, 0, 3, 0x15, 0x16, 1, 2, 0x35])
    regions.update({0x81ae2c: struct.pack('<I', current), 0x81af64: r.bytes(4), 0x81ede4: r.bytes(8), 0x818694: r.bytes(2), 0x818684: r.bytes(2),
                    0x763b00: r.bytes(8), 0x7639a0: r.bytes(0x18), 0x81b604: r.bytes(4), 0x81d7a4: r.bytes(4), 0x816d48: r.bytes(4),
                    0x81b134: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])),
                    0x8186ac: struct.pack('<I', r.choice([0, RAM + 0x21000, r.getrandbits(32)])), RAM + 0x21000: r.bytes(0x40)})
    record = r.choice([0, 1, 1, 0x100, r.getrandbits(32)])
    return Call(CALLS[op], [mode, record], regions, masks=adv.masks)


if __name__ == '__main__':
    B.main(case)
