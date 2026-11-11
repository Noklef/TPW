# Panel7cc408_ShowVisitor (src/game/ui/panel7cc408info, 0x4b72a0) and Panel7cc408_Reopen (0x4b7660; the panel's slots 5 and 6 vtable recorders) against the original. Recorders (Panel7cc408InfoEnv): Charset_FormatWide 18 with its argument hashed and, from the one call without one, 19; HoverLabel_Attach 20, Visitor_TimeInPark 21, FunnyTime_SpanOf 22 (it answers the span region), 0x4e2780 23 (agent A's; it fills the name),
# Window_Notify1 24, Records_DataIfTerminated 26, Language_Widen 25. The children's slots 7 and 9 and the panel's slot 0xb vtable recorders. Window_FindChild, __ftol and _alldiv real. The children by hand, one missing now and then (0x3733 and 0x3734, 0x3735 never: a fault in both).
# usage: panel7cc408info.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, blob_when, blob_when_inner, fill, log_region, put, set_return, strings, vtable  # noqa: E402

PLAIN = _recorder(19, 4, cdecl=True)
FULL = _recorder(18, 5, cdecl=True)


def format_hook(e):
    return (FULL if e.u32(e.r['ESP'] + 0xc) in (0x10, 0x1c8) else PLAIN)(e)   # (a called hook has no return address on the stack: the entry tells the two call sites apart)


B = Bench([(0x4b72a0, 0x4b759c), (0x4b7660, 0x4b771c),(0x65d7bc, 0x65d840), (0x67a830, 0x67a860), (0x67c320, 0x67c3ca), (0x5da3c0, 0x5da3c1)],
          hooks={0x6acd60: format_hook, 0x485d20: _recorder(20, 3, cdecl=True), 0x4fd950: _recorder(21, 0), 0x4f8730: _recorder(22, 2), 0x4e2780: _recorder(23, 2), 0x65d3a3: _recorder(24, 1), 0x6ace60: _recorder(26, 1), 0x485ac0: _recorder(25, 1, cdecl=True)})
SELF, WINDOW, PAGE, KIDS, VISITOR, VT, SPAN, WORLD, THINGS, WINDOW_KIDS, INNER = (RAM + 0x1000 * k for k in range(11))
PAGE_IDS = (0x3724, 0x372d, 0x372e, 0x372f, 0x3730, 0x3731, 0x3732)


def chain(r, base, ids):
    """the windows at base, 0x100 bytes each, with those ids and each the next's sibling"""
    kids = bytearray(r.bytes(0x100 * len(ids)))
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k, 'I', VT)
        put(kids, 0x100 * k + 0x54, 'I', base + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    return kids


def case(r):
    log = log_region()
    blob_bytes(log, 18, 4, 0xfffff)
    blob_when(log, 18, 3, 0x10, 0xfff, 0)
    blob_when_inner(log, 18, 3, 0x10, (0xc, 0x7fff), 1)
    strings(log, 22, 0, out=0b1)
    strings(log, 23, 0, out=0b1)
    fill(log, 23, 0, r.bytes(0x40))
    set_return(log, 21, r.getrandbits(32))
    set_return(log, 22, SPAN)
    set_return(log, 25, r.getrandbits(32))
    set_return(log, 26, r.getrandbits(32))
    ids = [k for k in PAGE_IDS if r.random() < 0.85] + [0x3733]
    r.shuffle(ids)
    outer = [0x3734, r.getrandbits(32) | 0x1000]
    r.shuffle(outer)
    visitor = bytearray(r.bytes(0x240))
    put(visitor, 0x19c, 'f', r.choice([0.0, 1.0, 50.5, 100.0, 255.0, 256.0, 300.25, -1.0, r.random() * 100, r.random() * 100]))
    put(visitor, 0x1d8, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(visitor, 0x1dc, 'H', r.randrange(4))
    put(visitor, 0x220, 'I', r.choice([0x10, 0x10, 0, 1, 0xf, 0x11, r.getrandbits(32)]))
    put(visitor, 0x30, 'I', r.choice([0, 1, 2, r.getrandbits(32)]))
    self_ = put(bytearray(r.bytes(0x40)), 0, 'I', VT)
    put(self_, 0x10, 'II', WINDOW, r.choice([PAGE, PAGE, PAGE, 0]))
    put(self_, 0x24, 'I', r.choice([0, 0, r.getrandbits(32)]))
    regions = {LOG: log, SELF: self_, WINDOW: put(bytearray(r.bytes(0x100)), 0x5c, 'I', WINDOW_KIDS), PAGE: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KIDS: chain(r, KIDS, ids), WINDOW_KIDS: chain(r, WINDOW_KIDS, outer),
               VISITOR: visitor, VT: vtable({5: 0, 6: 0, 7: 1, 9: 1, 0xb: 0}), SPAN: struct.pack('<II', r.getrandbits(32), r.choice([0, 1, 8, r.getrandbits(32)])), INNER: chain(r, INNER, [0x3735]),
               0x7c2658: struct.pack('<I', 0 if r.random() < 0.05 else VISITOR), 0x7cf83c: struct.pack('<I', WORLD), 0x78ba94: r.bytes(16), 0x7cfb90: r.bytes(0x50)}
    put(regions[WINDOW_KIDS], 0x100 * outer.index(0x3734) + 0x5c, 'I', INNER)
    return Call(r.deal([0x4b72a0, 0x4b7660]), [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
