# The game panel's settings texts (src/game/ui/panel7cc218: Panel7cc218_ShowSettings, 0x4aec30) against the original (panel7cc218.py
# and panel7cc218choose.py bench the module's other functions). The panel's window +0x10 has the children 0x3e33 / 0x3e31 (plain
# windows) and 0x3e32 (window.py's shown_env() root, shown or hidden: Window_SetShown's surroundings) among 0-2 decoys in any order, one
# of them missing now and then (not checked: a fault in both); each one's object +0x9c (mostly) one whose slot 3 is the recorder 24, the
# text it is sent hashed (blob_inner_text(): the shared buffer 0x7ba058, random characters for the texts left as they are). The thing
# [0x7c2658] and its definition through ObjectDefs_Find's cache 0x7897d0: its track type +0x9c 2, 3 or another, its +0x70 0-4 or past;
# the settings +0x2c / +0x30 / +0x34 about 0 and 1, negative or any; the flags mostly one bit (the upper bits random). The formatter
# 0x6acd60 the log stub, its one argument hashed (text_args(), only for the texts' ids 0x1aa-0x1cf: the hidden or shown window's handler
# +0x114, CDECL_LOG4 too, takes none).
# usage: panel7cc218settings.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import window  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log4, blob_bytes, blob_inner_text, log_region, put, recorder, text_args, vtable  # noqa: E402

B = Bench(dialog7cc5f4.merged(window.RANGES + [
    (0x412e90, 0x412f00), (0x4a77a0, 0x4a77d0), (0x4aec30, 0x4af040), (0x4dd4e0, 0x4dd510), (0x6589f9, 0x658abb), (0x658ccc, 0x658daf),
    (0x659710, 0x659740), (0x65d773, 0x65d832), (0x65d90b, 0x65da9d), (0x65db25, 0x65dc00), (0x65f5b7, 0x65f60e), (0x6697e3, 0x6698b4)]),
    hooks={0x6acd60: _cdecl_log4})
PANEL, ROOT, KIDS, THING, DEF, TOBJ = RAM + 0x40000, RAM + 0x40100, RAM + 0x40200, RAM + 0x40800, RAM + 0x40900, RAM + 0x40a00
KID = 0xa0


def setting(r):
    return r.choice([0, 0, 1, 1, -1, 2, 5, -5, 100, -0x80000000, 0x7fffffff, r.getrandbits(32) - (1 << 31)])


def case(r):
    log = log_region()
    regions = {LOG: log}
    wins = window.shown_env(r, regions, log)
    shown = wins[0]   # (0x3e32: no parent, no siblings of its own)
    regions[TOBJ] = struct.pack('<I', TOBJ + 0x10) + bytes(0xc) + put(put(vtable({}, size=8), 0xc, 'I', recorder(24, 2)), 0x14, 'I', recorder(21, 1))
    blob_bytes(log, 24, 1, 0, (0x10, 0))
    blob_inner_text(log, 24, 0)
    text_args(log, 1, (0x1aa, 0x1cf))   # (the texts' ids: the windows' handlers, CDECL_LOG4 too, take no text argument)
    items = [(KIDS, 0x3e33), (KIDS + KID, 0x3e31), (shown, 0x3e32)]
    if r.random() < 0.03:
        items.remove(r.choice(items))
    items += [(KIDS + KID * (2 + k), r.choice([0, 0x3e30, 0x3e34, r.getrandbits(32)])) for k in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(items)
    for i, (at, ident) in enumerate(items):
        if at != shown:
            regions[at] = bytearray(r.bytes(KID))
        put(regions[at], 0x8c, 'I', ident)
        put(regions[at], 0x54, 'I', items[i + 1][0] if i + 1 < len(items) else 0)
        put(regions[at], 0x9c, 'I', TOBJ if r.random() < 0.9 else 0)
    regions[ROOT] = put(bytearray(r.bytes(KID)), 0x5c, 'I', items[0][0] if items else 0)
    chars = [r.choice(b'abc %') for _ in range(r.randrange(0x30))]
    regions[0x7ba058] = struct.pack('<%dH' % (len(chars) + 1), *chars, 0) + r.bytes(0x20)
    ident = r.getrandbits(16)
    regions[THING] = put(bytearray(r.bytes(0x20)), 0xe, 'H', ident)
    regions[0x7c2658] = struct.pack('<I', THING)
    regions[0x7897d0] = struct.pack('<HHI', ident, r.getrandbits(16), DEF)
    d = bytearray(r.bytes(0xa0))
    put(d, 0x70, 'I', r.choice([0, 1, 1, 2, 3, 4, 5, 0xffffffff, r.getrandbits(32)]))
    put(d, 0x9c, 'I', r.choice([2, 2, 3, 3, 0, 1, 4, r.getrandbits(32)]))
    regions[DEF] = d
    regions[PANEL] = put(put(bytearray(r.bytes(0x40)), 0x10, 'I', ROOT), 0x2c, 'iii', setting(r), setting(r), setting(r))
    if r.random() < 0.3:
        put(regions[PANEL], 0x34, 'i', 1)
    flags = r.choice([1, 2, 4, 4, 7, 3, 5, 6, 0]) | (r.getrandbits(29) << 3 if r.random() < 0.5 else 0)
    return Call(0x4aec30, [PANEL, flags], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
