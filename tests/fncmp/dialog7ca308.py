# The dialog 0x7ca308 (src/game/ui/dialog7ca308: 0x49ba00) against the original.
# ShowStaff (0x49ba00): the window [0x7ca308] with 0-3 other children ahead of the child 0x2484 (panel7cc328.py's child_window(): the saved
# rectangle's words and the depth small, wide or edges); the staff pool [0x7cf83c] of 0x20 entries (20 bytes), each valid (+0xa) or not,
# its type +0 0-4 mostly (5, -1 or any now and then) and its byte +9 small or any; the index one of them mostly (0x20 and past now and
# then); the cache [0x7523e8] / [0x7523ec] the entry's type / byte or not; the script [0x7ca310] 0, live or any; the sprite scripts as
# panel7cc328.py's script_env() builds them (spritescript.py's).
# FillList (0x49b5b0, cdecl (kind, clear)): the window [0x7ca308] (none now and then) with its list 0x248e among decoys (no children now and then), the staff pool as above; Window7059f0_Clear, AddItem, RefreshRows,
# StaffPoolEntry_FormatName and Staff_WageForRecord thiscall recorders 10-14 (driver.cpp's kDialog7ca308Env; their own benches have them), the rows logged by their wage and the name's first bytes, the name's buffer (the stack's) as 0.
# usage: dialog7ca308.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowStaff,FillList)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import panel7cc328  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, log_region, put, set_return, strings  # noqa: E402

CALLS = {'ShowStaff': 0x49ba00, 'FillList': 0x49b5b0}
B = Bench(sorted(set(panel7cc328.SHOW_RANGES + [(0x49ba00, 0x49bb8e), (0x507bd0, 0x507bf0), (0x49b5b0, 0x49b64d)])),
          hooks={0x6649d5: _recorder(10, 0), 0x66403b: _recorder(11, 4), 0x663324: _recorder(12, 0), 0x5074a0: _recorder(13, 2), 0x5077f0: _recorder(14, 0)})
WIN, KIDS, POOL = RAM + 0x60000, RAM + 0x60200, RAM + 0x60800


def staff_pool(r):
    pool = bytearray(r.bytes(20 * 0x20))
    for k in range(0x20):
        put(pool, 20 * k, 'I', r.choice([0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, r.getrandbits(32)]))
        pool[20 * k + 9] = r.choice([0, 1, 2, 3, r.randrange(8), r.getrandbits(8)])
        pool[20 * k + 0xa] = r.choice([1, 1, 1, 0, r.getrandbits(8)])
    return pool


def fill_list(r):
    log = log_region()
    blob_bytes(log, 11, 0, 0xf0, inner=(0, 0x7fff))
    strings(log, 13, 0, 0b1)
    fill(log, 13, 0, r.bytes(0x20))
    set_return(log, 14, r.getrandbits(32))
    regions = {LOG: log}
    panel7cc328.child_window(r, regions, WIN, KIDS, 0x248e)
    if r.random() < 0.06:   # (no list: no children at all, as child_window's chain may close on itself)
        put(regions[WIN], 0x5c, 'I', 0)
    pool = staff_pool(r)
    regions.update({POOL: pool, 0x7cf83c: struct.pack('<I', POOL)})
    shown = bytearray(r.bytes(0xc))
    put(shown, 0, 'I', r.choice([WIN] * 15 + [0]))
    regions[0x7ca308] = shown
    return Call(CALLS['FillList'], [r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]), r.choice([0, 1, 1, r.getrandbits(32)])], regions, ret='v')


def case(r):
    if r.deal(OPS) == 'FillList':
        return fill_list(r)
    regions, live = panel7cc328.script_env(r)
    panel7cc328.child_window(r, regions, WIN, KIDS, 0x2484)
    pool = staff_pool(r)
    index =r.choice([r.randrange(0x20)] * 8 + [0x20, 0x21, 0xffffffff, r.getrandbits(32)])
    cache = bytearray(r.bytes(8))
    if index < 0x20 and r.random() < 0.5:   # the entry's type, its byte or both kept
        same = r.random()
        put(cache, 0, 'I', struct.unpack_from('<I', pool, 20 * index)[0] if same < 0.7 else r.getrandbits(32))
        put(cache, 4, 'I', pool[20 * index + 9] if same < 0.4 or same >= 0.7 else pool[20 * index + 9] ^ 1)
    shown = bytearray(r.bytes(0xc))
    put(shown, 0, 'I', WIN)
    put(shown, 8, 'i', r.choice(live * 3 + [0, 0, 0, -1, r.randrange(13)]) if live else r.choice([0, 0, -1, r.randrange(13)]))
    regions.update({POOL: pool, 0x7cf83c: struct.pack('<I', POOL), 0x7523e8: cache, 0x7ca308: shown})
    return Call(CALLS['ShowStaff'], [index], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
