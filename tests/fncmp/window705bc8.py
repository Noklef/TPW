# The GUI library's kind 11 window (vtable 0x705bc8, two ranges) against the originals (src/game/ui/window705bc8: 0x66cc90, 0x66cd05).
# FirstToSecond / SecondToFirst: the ranges' ends small or any (the second's mostly 0 to 1 or small), the spans their differences (now and
# then any; never 0, nor -1 under a dividend of -0x80000000: a fault in both), the value +0x144 / +0x158 an end of its range, in it or any.
# Compare (0x66cd83): the two windows' slot 3 recorders (each its own) answering the same mostly, the values +0x158 any. SetValue (0x66cdc4):
# SecondToFirst's ranges, the value an end of the second range, past it, the current +0x158 or any; the object +0x9c a recorder (slot 3,
# its event hashed by its word +8) or none, the parent +0x60 a window (byte +4 mostly clear) or none, window.py's queues. SetValueSync
# (0x66cc4d): SetValue's. SetFirst (0x66cea9): SetValue's, the value an end of the first range, past it, the current +0x144 or any (the
# spans never -1: no overflowing quotient). Drag (0x66cf7d): SetFirst's, the drag +0x164 on mostly, the y the word +0x160, near it or any.
# SetDrag (0x66cf17): on 0, 1, only its high half or any; the capture [0xfaa5e8] the window, another or none (no root [0xfaa5d8]: the
# hover update stops at once). OnMessage (0x66d001): Drag's and SetDrag's, the active window [0xfaa5dc] it, another or none, the
# messages 0x10003-0x10005 or Window_DefaultProc's harmless ones, a 0 now and then; the buttons +0x11c 0 and the manager [0x77c48c] a
# recorder (slot 8).
# usage: window705bc8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FirstToSecond,SecondToFirst,Compare,SetValue,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import RANGES, queues  # noqa: E402

CALLS = {'FirstToSecond': 0x66cc90, 'SecondToFirst': 0x66cd05, 'Compare': 0x66cd83, 'SetValue': 0x66cdc4, 'SetValueSync': 0x66cc4d,
         'SetFirst': 0x66cea9, 'SetDrag': 0x66cf17, 'Drag': 0x66cf7d, 'OnMessage': 0x66d001}
B = Bench(sorted(set(RANGES + [(0x66cc90, 0x66cd05), (0x66cd05, 0x66cd83), (0x66cd83, 0x66cdc4), (0x66cdc4, 0x66cea9), (0x66cc4d, 0x66cc7c),
                               (0x66cea9, 0x66cf17), (0x66cf17, 0x66cf7d), (0x66cf7d, 0x66cfff), (0x66d001, 0x66d08e), (0x65e549, 0x65e56e),
                               (0x65e56e, 0x65e59b), (0x65dd18, 0x65dd5f), (0x65f6d1, 0x65fba8)])))
WINDOW, OTHER, VT, VT2, OBJ, PARENT = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x480, RAM + 0x500, RAM + 0x600
MGR, MGRVT = RAM + 0x700, RAM + 0x740


def s32(v):
    return (v + 0x80000000 & 0xffffffff) - 0x80000000


def rng(r, big):
    low = r.choice([0, 0, 1, -5, r.randrange(-1000, 1000), s32(r.getrandbits(32))])
    high = r.choice([low + big, low + r.randrange(1, 50), low + 1, s32(r.getrandbits(32))])
    span = r.choice([high - low] * 6 + [r.randrange(1, 100), -3, s32(r.getrandbits(32))])
    return s32(low), s32(high), s32(span) or 1


def compare(r):
    log = log_region()
    vt2 = vtable({})
    put(vt2, 0xc, 'I', recorder(5, 0))
    regions = {LOG: log, VT: vtable({}), VT2: vt2}
    regions[WINDOW] = put(bytearray(r.bytes(0x170)), 0, 'I', VT)
    regions[OTHER] = put(bytearray(r.bytes(0x170)), 0, 'I', VT2)
    mine = r.choice([0, 1, 11, r.getrandbits(32)])
    set_return(log, 3, mine)
    set_return(log, 5, r.choice([mine] * 4 + [mine ^ 1, r.getrandbits(32)]))
    return Call(0x66cd83, [WINDOW, OTHER], regions, conv='t', ret='a')


def set_value(r, op='SetValue'):
    log = log_region()
    regions = {LOG: log, VT: vtable({3: 2})}
    regions[OBJ] = put(bytearray(r.bytes(0x20)), 0, 'I', VT)
    w = bytearray(r.bytes(0x170))
    lo1, hi1, span1 = rng(r, 0xfff)
    lo2, hi2, span2 = rng(r, 1)
    put(w, 0x138, 'iii', lo1, hi1, span1)
    put(w, 0x14c, 'iii', lo2, hi2, span2)
    current = r.choice([lo2, hi2, lo2 + r.randrange(-2, 50), s32(r.getrandbits(32))])
    put(w, 0x158, 'i', current)
    value = s32(r.choice([lo2, hi2, lo2 - 1, hi2 + 1, current, current, lo2 + r.randrange(-2, 50), r.getrandbits(32)]))
    held = min(value, hi2) if value > hi2 else max(value, lo2)
    if held not in (lo2, hi2) and span2 == -1 and s32(s32(held - lo2) * span1) == -0x80000000:
        put(w, 0x154, 'i', 1)   # (SecondToFirst's one overflowing quotient)
    put(w, 0x9c, 'I', r.choice([OBJ, OBJ, 0]))
    put(w, 0x60, 'I', r.choice([PARENT, PARENT, 0]))
    regions[PARENT] = put(bytearray(r.bytes(0x20)), 4, 'B', r.choice([0, 0, 0, 1]))
    regions[WINDOW] = w
    queues(r, regions)
    blob_bytes(log, 3, 1, 0x300)
    if op in ('SetValue', 'SetValueSync'):
        return Call(CALLS[op], [WINDOW, value & 0xffffffff], regions, conv='t', ret='v' if op == 'SetValue' else 'a')
    for off in (0x140, 0x154):   # (no span -1: SetFirst's value in the second range is not worked out here)
        if struct.unpack_from('<i', w, off)[0] == -1:
            put(w, off, 'i', -2)
    first = r.choice([lo1, hi1, lo1 - 1, hi1 + 1, lo1 + r.randrange(0x1000), s32(r.getrandbits(32))])
    put(w, 0x144, 'i', s32(r.choice([first, first, r.getrandbits(32)])))
    if op == 'SetFirst':
        return Call(CALLS[op], [WINDOW, s32(first) & 0xffffffff], regions, conv='t', ret='v')
    put(w, 0x164, 'H', r.choice([1, 1, 1, 0, 0x8000, r.getrandbits(16)]))
    mark = r.choice([0, 5, 0xffff, 0x8000, r.getrandbits(16)])
    put(w, 0x160, 'H', mark)
    put(w, 0x11c, 'I', 0)
    y = r.choice([mark, mark + 1, mark - 1, mark + r.randrange(-40, 40), r.getrandbits(16)]) & 0xffff | r.getrandbits(16) << 16
    regions[0xfaa5d8] = bytearray(struct.pack('<5I', 0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([WINDOW, WINDOW, 0, PARENT])))
    if op == 'Drag':
        return Call(CALLS[op], [WINDOW, y], regions, conv='t', ret='v')
    if op == 'SetDrag':
        return Call(CALLS[op], [WINDOW, r.choice([0, 1, 1, 0x10000, r.getrandbits(32)]), y], regions, conv='t', ret='v')
    put(regions[0xfaa5d8], 4, 'I', r.choice([WINDOW, PARENT, 0]))
    regions[0x77c480] = put(bytearray(r.bytes(0x10)), 0xc, 'I', MGR)
    regions[MGRVT] = vtable({8: 2})
    regions[MGR] = struct.pack('<I', MGRVT)
    message = r.choice([0x10003, 0x10004, 0x10005] * 3 + [0x80000000 | r.getrandbits(31), 0x100, 0x1000d, 0x11006, 0x101, 0x15, 0x13])
    a = y if message == 0x10003 else r.choice([0, 0, 1, r.getrandbits(32)])
    return Call(CALLS[op], [WINDOW, message, a, y], regions, ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Compare':
        return compare(r)
    if op not in ('FirstToSecond', 'SecondToFirst'):
        return set_value(r, op)
    w = bytearray(r.bytes(0x160))
    lo1, hi1, span1 = rng(r, 0xfff)
    lo2, hi2, span2 = rng(r, 1)
    value = r.choice([lo1, hi1, lo1 + r.randrange(0x1000), s32(r.getrandbits(32))]) if op == 'FirstToSecond' else         r.choice([lo2, hi2, lo2 + r.randrange(-2, 50), s32(r.getrandbits(32))])
    put(w, 0x138, 'iii', lo1, hi1, span1)
    put(w, 0x14c, 'iii', lo2, hi2, span2)
    put(w, 0x144 if op == 'FirstToSecond' else 0x158, 'i', s32(value))
    if op == 'FirstToSecond':   # (the product wraps in both; only its quotient by -1 overflows)
        product = s32(span2 * s32(value))
        if span1 == -1 and product == -0x80000000:
            put(w, 0x140, 'i', 1)
    else:
        product = s32(s32(s32(value) - lo2) * span1)
        if span2 == -1 and product == -0x80000000:
            put(w, 0x154, 'i', 1)
    return Call(CALLS[op], [WINDOW], {WINDOW: w}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
