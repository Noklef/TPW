# The figures of an object on the dialog [0x7cc1f8] (src/game/ui/dialog7cc1f8stats: Dialog7cc1f8_ShowFigures, 0x4abb00) against the original. Recorders (their own benches have them; driver.cpp's kDialog7cc1f8StatsEnv): ObjectDefs_Find 10, ObjectControls_GetWord18 11, Language_WideText 12, Window_NotifyImpl100 13 (its text on the
# stack: the number of three digits, or the text of three, hashed); the children's slots 7 and 9 vtable recorders. Window_FindChild runs. The text a format as it is.
# usage: dialog7cc1f8stats.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from crt import swprintf_hook  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4abb00, 0x4abc7c), (0x65d7bc, 0x65d840), (0x519510, 0x51954d), (0x5da3c0, 0x5da3c1)],
          hooks={0x412e90: _recorder(10, 1), 0x4ca8c0: _recorder(11, 1, True), 0x485b00: _recorder(12, 1, cdecl=True), 0x65f5b7: _recorder(13, 1), 0x67af00: swprintf_hook})
TOP, KIDS, PANEL_KIDS, VT, DEF, TEXT, ID, WORLD = RAM, RAM + 0x100, RAM + 0x400, RAM + 0xa00, RAM + 0xc00, RAM + 0x1000, RAM + 0x1100, RAM + 0x80000
IDS = (0x1f5, 0x1ee, 0x1f2, 0x1ef)


def chain(r, base, ids, kids=None):
    out = bytearray(r.bytes(0x100 * len(ids)))
    for k, ident in enumerate(ids):
        put(out, 0x100 * k, 'I', VT)
        put(out, 0x100 * k + 0x54, 'I', base + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(out, 0x100 * k + 0x8c, 'I', ident)
        put(out, 0x100 * k + 0x5c, 'I', kids if kids else 0)
    return out


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 1 << 16)   # (the id a word: the upper half the register's)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 1 << 16)
    set_return(log, 10, DEF)
    set_return(log, 11, r.choice([0, r.randrange(100, 1000), r.randrange(100, 1000)]))
    set_return(log, 12, TEXT)
    blob_bytes(log, 13, 0, 0xff)
    d = bytearray(r.bytes(0x200))
    put(d, 0x13c, 'I', r.choice([0, 1, 100, 0x200000, 0x400000, 0xffffffff, r.getrandbits(32), r.randrange(101)]))
    put(d, 0x1b0, 'I', r.choice([0, 1, 100, 0x200000, 0x400000, 0xffffffff, r.getrandbits(32), r.randrange(101)]))
    put(d, 0x19c, 'I', r.choice([0, 7, 123456, 0xffffffff, r.getrandbits(32)]))
    ids = [i for i in IDS if r.random() < 0.9]
    r.shuffle(ids)
    panel_ids = ids or [0]
    top = chain(r, KIDS, [0x1ed] if r.random() < 0.97 else [0x1ec], PANEL_KIDS)
    regions = {LOG: log, TOP: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KIDS: top, PANEL_KIDS: chain(r, PANEL_KIDS, panel_ids), VT: vtable({7: 1, 9: 1}), DEF: d, TEXT: struct.pack('<4H', 0x41, 0x42, 0x43, 0), ID: struct.pack('<h', r.choice([0, 1, 0x7fff, r.randrange(0x8000), r.randrange(-0x8000, 0) if r.random() < 0.2 else 5])),
               WORLD + 0x1da700: r.bytes(0x40), 0x7cf83c: struct.pack('<I', WORLD), 0x7cc1f8: struct.pack('<II', TOP, r.choice([0, 0, 0, 2, 2, 1, 3, r.getrandbits(32)])), 0x7ba058: r.bytes(0x40)}
    return Call(0x4abb00, [ID], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
