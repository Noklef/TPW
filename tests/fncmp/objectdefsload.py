# One object definition loaded (src/game/world/objectdefs: 0x413c10) against the original. The folder ending in the suffix
# String or not; the list full now and then; each open a recorder (logging the stream as 0, answering yes or no), the balance reads recorders logging the name's text and what they read into (by its first dword, the stack's
# balance file by the vtable set), failing at any of their calls; the setting's mode made at its first use or there; parts, emt and language found or not; Ride_LoadResource a recorder (its text arguments logged as text) failing at any of its calls;
# kind 3 or not.
# usage: objectdefsload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, fail_mask, fill_this, heap, log_fold, log_region, put, set_return, strings, this_first, vtable  # noqa: E402

# Thiscall recorders (address: k, stack arguments).
TH = {0x401700: (7, 3), 0x405710: (8, 3), 0x412c20: (9, 0), 0x401030: (10, 0), 0x401250: (11, 0), 0x41e660: (26, 2), 0x41e720: (13, 2), 0x41f170: (14, 1), 0x41f190: (15, 1), 0x41f260: (16, 1), 0x41f330: (17, 1), 0x41f350: (18, 1),
      0x41f420: (19, 0), 0x41f630: (20, 0), 0x414420: (21, 1), 0x413410: (22, 1), 0x413600: (23, 1), 0x412fb0: (24, 3), 0x550ca0: (25, 0)}
# Cdecl recorders.
CD = {0x4629d0: (12, 11), 0x42fad0: (27, 3), 0x51b4f0: (28, 2), 0x67a7d0: (29, 1)}


def _swprintf(e):
    """The C library's swprintf (0x67af00) of its one format here, L"%S" (the original's goes through the system's MultiByteToWideChar): the text widened."""
    sp = e.r['ESP']
    to, text = e.u32(sp), e.u32(sp + 8)
    n = 0
    while e.read(text + n, 1)[0]:
        n += 1
    e.write(to, b''.join(bytes([c, 0]) for c in e.read(text, n)) + b'\0\0')
    e.r['EAX'] = n


B = Bench([(0x413c10, 0x4143e5), (0x5c5480, 0x5c5510), (0x424cf0, 0x424d10), (0x424d30, 0x424db0), (0x5f5fc0, 0x5f7f00), (0x5da3c0, 0x5da3c1), (0x409cd0, 0x409cdc), (0x67a400, 0x6902c0)],
          hooks={0x67af00: _swprintf, **{a: _recorder(k, n) for a, (k, n) in TH.items()}, **{a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()}})
DEFS, ENTRY, OPENER, SOURCE, VT = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1300
DEF, TEXTS = RAM + 0x2000, RAM + 0x3000
STRING_VT = 0x703090


class Texts:
    """C strings in one region."""
    def __init__(self):
        self.buf = bytearray()

    def add(self, text):
        at = TEXTS + len(self.buf)
        self.buf += text + b'\0'
        return at


def string(texts, text):
    return struct.pack('<5I', STRING_VT, len(text), texts.add(text), (len(text) & ~7) + 8, 8)


def load(r):
    log = log_region()
    log_fold(log)
    texts = Texts()
    suffix = r.choice([b'.wad', b'.WAD', b'.w', b''])
    folder = r.choice([b'Data\\Rides\\Big', b'R', b'', b'Data\\Rides\\Big_1\\']) + r.choice([suffix, suffix, b'.wad', b'.wa', b''])
    source = bytearray(r.bytes(0x40))
    name = r.choice([b'Big', b'R1', b'', b'Ghost Train'])
    put(source, 0xc, 'I', texts.add(name))
    source[0x18:0x2c] = string(texts, folder)
    put(source, 0x2c, 'I', VT)
    set_return(log, 6, texts.add(b'Data\\Theme'))
    d = bytearray(r.bytes(0x510))
    put(d, 0x38, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(d, 0x84, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(d, 0x9c, 'I', r.choice([3, 3, 3, 0, 1, 2, r.getrandbits(32)]))
    put(d, 0x264, 'i', r.choice([0, 0, 1, 2, 3, 8, -1]))
    for i in range(8):
        put(d, 0x244 + 4 * i, 'I', texts.add(b'Layer%d' % i))
    for k in (14, 15, 16, 17, 18, 13, 26):
        set_return(log, k, r.choice([0, 0, 1, r.getrandbits(32)]))
        strings(log, k, 0, 1 << (1 if k in (13, 26) else 0))   # (the stream: a stack address)
    set_return(log, 19, r.choice([0, 1]))
    set_return(log, 20, r.choice([0, 1]))
    set_return(log, 9, DEF)
    set_return(log, 28, r.getrandbits(32))
    strings(log, 7, 1, 2)   # (the name's text; the stream)
    this_first(log, 7)
    fail_mask(log, 7, r.choice([0xffffffff] * 5 + [0, 1, 3, 7, 0xfffffffe, r.getrandbits(32)]))
    strings(log, 8, 4)
    strings(log, 12, 3)
    fail_mask(log, 12, r.choice([0xffffffff] * 5 + [0, 1, 0xfffffffe, r.getrandbits(32) | 1]))
    strings(log, 27, 2, 4)   # (the folder's text; the balance file: a stack address)
    strings(log, 28, 1)
    this_first(log, 10)
    this_first(log, 11)
    strings(log, 23, 0, 1)
    blob(log, 22, 0, 1)   # (the variable holding the definition)
    count = r.choice([0, 0, 1, 7, 0x149, 0x14a]) if r.randrange(8) == 0 else r.choice([0, 1, 7])
    defs = bytearray(r.bytes(0x600))
    put(defs, 0x528, 'I', count)
    mode = r.choice([1, 1, 2, 2, 0, 3])
    made = r.choice([0, 1, 1, 0xff, 0xfe])
    if not made & 1:
        fill_this(log, 25, struct.pack('<I', mode))
    regions = {LOG: log, VT: vtable({}), DEFS: defs, ENTRY: r.bytes(0x10), OPENER: r.bytes(0x10), SOURCE: source, DEF: d, HEAP: heap(0x4000, fail=r.choice([0] * 60 + [1, 2, 3, 4, 5])),
               0xfb3b78: struct.pack('<II', made, mode if made & 1 else r.choice([mode, r.getrandbits(32)]))}
    regions[0x7898c8] = string(texts, suffix)
    regions[0xf80be0] = string(texts, b'\\')
    for at, text in ((0x78a338, b'Online_'), (0x788df0, b'Easy_'), (0x78a0b0, b'coaster')):
        regions[at] = struct.pack('<I', texts.add(text))
    regions[TEXTS] = texts.buf
    return Call(0x413c10, [DEFS, ENTRY, OPENER, SOURCE], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(load)
