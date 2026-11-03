# A message tag's window handler (src/game/ui/msgtags: 0x48dae0) against the original. Window_DwordA0, Window_FindChild, the two accessors and the GUI rectangle code (as particles.py's AtWindow) real, the other calls recorders numbered as MsgTagHandlerEnv from 10; slot 5 of the windows recorder 5.
# The tag among the ten records or not (then the word before the first read, sic); 0-3 tags after it; the tags' window [0x7cb2c4] with its child 0x2e or none.
# usage: msgtaghandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x48dae0, 0x48e0e5), (0x409cd0, 0x409cdd),(0x65e705, 0x65e72d), (0x65d55a, 0x65d58c), (0x65d7bc, 0x65d840), (0x659d00, 0x659d43), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aa90), (0x66bff7, 0x66c00b), (0x65d42e, 0x65d49e), (0x65a43e, 0x65a493)],
          hooks={0x48e0f0: _recorder(10, 3, cdecl=True), 0x65d9dd: _recorder(11, 1), 0x5222e0: _recorder(12, 1, cdecl=True), 0x51feb0: _recorder(13, 1, cdecl=True), 0x65f077: _recorder(14, 1), 0x65d865: _recorder(15, 3), 0x65cd95: _recorder(16, 4), 0x65c7fe: _recorder(17, 2), 0x65e72d: _recorder(18, 2), 0x65f538: _recorder(19, 0),
                 0x4a25f0: _recorder(20, 1, cdecl=True), 0x658ccc: _recorder(21, 4, cdecl=True), 0x521e60: _recorder(22, 4, cdecl=True), 0x65e6d5: _recorder(23, 2), 0x65d773: _recorder(24, 0), 0x51ff10: _recorder(25, 2, cdecl=True), 0x51fe30: _recorder(26, 4, cdecl=True), 0x486b90: _recorder(27, 2, cdecl=True), 0x485aa0: _recorder(28, 1, cdecl=True),
                 0x48d9c0: _recorder(29, 1, cdecl=True), 0x65f66e: _recorder(30, 3)})
WINDOW, TAGS, PARENT, KIDS, OTHER = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x8000


def window_bytes(r):
    w = bytearray(r.bytes(0x200))
    put(w, 0, 'I', VT)
    put(w, 0x54, 'I', 0)
    return w


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[VT] = put(vtable({}, size=0x10), 0x14, 'I', recorder(5, 1))
    w = window_bytes(r)
    for i in range(16):
        put(w, 0xa0 + 4 * i, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(w, 0xa0 + 6 * 4, 'I', r.choice([0x1b, 0x1b, 3, 0]))
    put(w, 0xa0 + 9 * 4, 'I', r.choice([0xffffffff, 0xffffffff, 5, 0, 0x7fffffff, 0x80000005, r.getrandbits(32)]))
    put(w, 0xa0 + 10 * 4, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    w[0x134], w[0x135] = r.choice([0, 1, 0xff]), r.choice([0, 1])
    for at in range(0xc, 0x14, 2):   # (the rectangle +8's words: small or any)
        put(w, at, 'h', r.choice([r.randrange(-0x800, 0x800), r.randrange(-0x8000, 0x8000)]))
    for at in range(0xf8, 0x100, 2):
        put(w, at, 'h', r.randrange(-0x10, 0x10))
    regions[WINDOW] = w
    put(w, 0x60, 'I', PARENT)
    after = r.randrange(4)
    put(w, 0x54, 'I', TAGS if after else 0)
    for i in range(after):
        t = window_bytes(r)
        put(t, 0x54, 'I', TAGS + 0x200 * (i + 1) if i < after - 1 else 0)
        t[0x135] = r.choice([0, 0, 1, 2])
        regions[TAGS + 0x200 * i] = t
    records = bytearray(r.bytes(0x2080))
    for i in range(10):
        put(records, i * 0x208, 'I', RAM + 0x4000 + i)
        put(records, i * 0x208 + 4, 'H', r.choice([0, 1, r.getrandbits(16)]))
    found = r.choice(list(range(10)) * 2 + [-1])
    if found >= 0:
        put(records, found * 0x208, 'I', WINDOW)
    if r.random() < 0.2 and found > 0:   # (and a later one: the first wins)
        put(records, (found + 1) % 10 * 0x208, 'I', WINDOW)
    regions[0x7c26b0] = records
    regions[0x7c24a8] = struct.pack('<I', RAM + 0x5000) + r.choice([bytes(2), b'\x01\x00'])   # (the word of "record -1")
    the_tags = window_bytes(r)
    ids = [0x2e] if r.random() < 0.8 else []
    if ids or r.random() < 0.5:
        first, _ = kids(r, regions, KIDS, ids)
        put(the_tags, 0x5c, 'I', first)
    else:
        put(the_tags, 0x5c, 'I', 0)
    regions[PARENT] = the_tags
    regions[0x7cb2c4] = struct.pack('<I', PARENT)
    regions[0x7cb2c8] = struct.pack('<I', r.choice([WINDOW, WINDOW, OTHER, 0]))
    regions[0x7ca09c] = struct.pack('<I', r.randrange(0, 12))
    regions[0x7ca090] = struct.pack('<I', r.choice([WINDOW, WINDOW, OTHER, 0]))
    regions[0x7ca094] = struct.pack('<I', r.choice([WINDOW, OTHER, 0]))
    set_return(log, 12, r.choice([0, 1, 1, r.getrandbits(32)]))
    set_return(log, 22, r.getrandbits(32))
    set_return(log, 24, r.choice([0, 0, 1]))
    set_return(log, 30, r.getrandbits(32))
    fill(log, 15, 1, struct.pack('<H', r.randrange(0x100)))
    strings(log, 15, 0, 1 << 1)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 1 << 17)   # (RunAction's a: a word over the stack)
    message = r.deal([5, 5, 5, 5, 6, 6, 0x10, 0x10, 0x10, 0x11, 0x11, 0x13, 0x13, 0x14, 0x10006, 0x10006, 0x10006, 4, 7, 0xf, 0x12, 0x15, 0x10005, 0x10007, r.getrandbits(32)])
    if message == 0x10:
        a = r.choice([0x80080, 0x80080, 0x80081, 0x80081, 0x80082, r.getrandbits(32)])
    else:
        a = r.choice([0, 0, 1, 1, 2, r.getrandbits(32)])
    return Call(0x48dae0, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
