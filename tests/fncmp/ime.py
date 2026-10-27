# The IME support (src/game/core/ime: 0x478b60, 0x47a960, 0x47a980, 0x47ae40, 0x47b300, 0x47c700,
# 0x47c720, 0x47c910, 0x47c940, 0x47db00, and the host 0x480e70) and the wide string's set length (core/wtraits: 0x47ba30)
# against the originals: the constructor on random bytes at addresses with different top bytes
# (the allocator bytes copy it, sic); the vector helpers overlapping, to null, empty; the sizes
# of vectors null, empty, inverted. The host's window check (0x480f90) and the text window's length (ui/window705a50: 0x66852b): texts of
# 0-40 characters or none, limits about them. The double-byte character 0x479260 (Char): the imports and the IME's handler as recorders.
# The input context's get / release (0x4791a0 / 0x478d20): ImmGetContext / ImmReleaseContext as recorders, a context or none.
# The destructors (0x478be0 / 0x478bc0, the heap region): entries with 0-2 strings each, the wide strings sole, shared or frozen, or none.
# The host's (0x480960, HostDtor): that IME at +0x1c, the vector +0x190 of 0-4 such strings or none.
# usage: ime.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, blob, heap, fill, import_table, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x478b60, 0x478bc0), (0x47c910, 0x47c93e), (0x47c940, 0x47c963), (0x47db00, 0x47db11),
           (0x47c720, 0x47c733), (0x47c700, 0x47c713), (0x47ae40, 0x47ae53), (0x47b300, 0x47b313),
           (0x47a960, 0x47a975), (0x47a980, 0x47a99f), (0x47ba30, 0x47ba43), (0x480e70, 0x480eb8),
           (0x480f90, 0x480fce), (0x66852b, 0x668564), (0x67acd0, 0x67ad00), (0x479260, 0x47937b),
           (0x4791a0, 0x4791b8), (0x478d20, 0x478d31), (0x6b559a, 0x6b55ac), (0x478bc0, 0x478bde), (0x478be0, 0x478cea), (0x480960, 0x4809e5), (0x4f23f0, 0x4f2450),
           (0x47aba0, 0x47ac01)])
CONTEXT_IMPORTS = {'ImmGetContext': (0, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (1, 0x6fd080, 2, 0, 0),
                   'ImmDestroyContext': (2, 0x6fd07c, 1, 0, 0)}
ENTRIES, STRINGS, TEXTS, HOST = RAM + 0x400, RAM + 0x800, RAM + 0xc00, RAM + 0x1000
COUNTS = [0, 1, 2, 5, 0x7f, 0xfe, 0xff]
CHAR_IMPORTS = {'IsDBCSLeadByte': (0, 0x6fd154, 1, 0, 0), 'PeekMessageA': (1, 0x6fd354, 5, 0, 1),
                'MultiByteToWideChar': (2, 0x6fd174, 6, 0, 0x10), 'DefWindowProcA': (3, 0x6fd340, 4, 0, 0)}
SIZES = [0x47c720, 0x47c700, 0x47ae40, 0x47b300]


def case(r):
    op = r.choice(OPS)
    mem = r.bytes(0x1000)
    if op == 'Ctor':
        at = r.choice([RAM, RAM + 0x1000 * r.randrange(16) + 4 * r.randrange(16), 0x7a0000 + 0x100 * r.randrange(8)])
        return Call(0x478b60, [at], {at: r.bytes(0x70)}, conv='t', ret='a')
    if op == 'Host':
        at = r.choice([RAM, RAM + 0x1000 * r.randrange(16) + 4 * r.randrange(16), 0x7a0000 + 0x100 * r.randrange(8)])
        return Call(0x480e70, [at], {at: r.bytes(0x1a0)}, conv='t', ret='a')
    if op in ('TextLength', 'OnWindow'):   # the text window: its text +0x138 (0-40 characters) or none, its limit +0x13c about them
        window = bytearray(r.bytes(0x140))
        n = r.choice([0, 1, 5, 12, 13, 40])
        put(window, 0x138, 'IH', r.choice([RAM + 0x800] * 5 + [0]), r.choice([0, n, n + 1, 12, 500, 0x7fff, 0x8000, r.getrandbits(16)]))
        regions = {RAM + 0x400: window, RAM + 0x800: bytes(r.randrange(1, 256) for _ in range(2 * n)) + bytes(2) + r.bytes(0x10)}
        if op == 'TextLength':
            return Call(0x66852b, [RAM + 0x400], regions, conv='t', ret='w')
        host = bytearray(r.bytes(0x1b0))
        put(host, 4, 'I', r.choice([RAM + 0x400] * 3 + [0, RAM + 0x500]))
        regions[RAM] = host
        if r.random() < 0.5:   # the IME's cap (0x6feee0, 12 in the game) raised so that the room's sign shows
            regions[0x6feee0] = struct.pack('<I', r.choice([0x7fffffff, 0xffffffff, 0x10000]))
        return Call(0x480f90, [RAM, RAM + 0x400], regions, conv='t', ret='v')
    if op == 'Char':   # 0x479260: the imports and the IME's handler +0x50 (or none) recorders; the trail byte in the peeked MSG's wParam
        log = log_region()
        regions = {LOG: log, IMPORT_TABLE: import_table(log, CHAR_IMPORTS)}
        set_return(log, 0, r.choice([0, 1, 1, 1, 0x80, r.getrandbits(32)]))   # IsDBCSLeadByte
        set_return(log, 1, r.choice([0, 1, 1, 1, 1, 2, r.getrandbits(32)]))   # PeekMessageA (fails all 10 times or succeeds at once)
        set_return(log, 2, r.choice([1, 1, 1, 0, 2, r.getrandbits(32)]))   # MultiByteToWideChar
        fill(log, 1, 0, r.bytes(0x1c))
        fill(log, 2, 4, r.choice([r.bytes(2), b'\xff\xff', b'\0\0']), which=1)
        blob(log, 2, 2, 1)   # (the two bytes' dword on the stack)
        ime = bytearray(r.bytes(0x60))
        put(ime, 0x50, 'I', r.choice([0, recorder(4, 4, com=True)]))
        regions[RAM] = ime
        character = r.choice([r.getrandbits(32), r.getrandbits(8), 0x81, 0xe0, 0])
        return Call(0x479260, [RAM, r.getrandbits(32), character, r.getrandbits(32)], regions, conv='t', ret='a')
    if op in ('GetContext', 'ReleaseContext'):   # 0x4791a0 / 0x478d20 through the IMM32 thunks: the imports as recorders
        log = log_region()
        regions = {LOG: log, IMPORT_TABLE: import_table(log, CONTEXT_IMPORTS), RAM: mem}
        if r.random() < 0.7:
            set_return(log, r.choice([0, 1]), r.choice([0, 1, r.getrandbits(32)]))
        if op == 'GetContext':
            return Call(0x4791a0, [RAM + 0x100, r.choice([0, r.getrandbits(32)])], regions, conv='t', ret='a')
        put(mem, 0x104, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        return Call(0x478d20, [RAM + 0x100], regions, conv='t', ret='a')
    if op in ('Dtor', 'DelDtor', 'HostDtor'):   # 0x478be0 / 0x478bc0: entries with 0-2 strings each, the wide strings sole, shared or frozen, or none
        log = log_region()
        ime = bytearray(r.bytes(0x60))
        entries, strings, texts = bytearray(r.bytes(0x80)), bytearray(r.bytes(0x180)), bytearray(r.bytes(0x200))
        regions = {LOG: log, IMPORT_TABLE: import_table(log, CONTEXT_IMPORTS), HEAP: heap(), RAM: ime, ENTRIES: entries,
                   STRINGS: strings, TEXTS: texts}
        put(ime, 0x5c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        texts_used = 0

        def text():   # the characters of a string (or none), the count byte before them
            nonlocal texts_used
            texts[0x20 * texts_used + 1] = r.choice(COUNTS)
            texts_used += 1
            return r.choice([0, TEXTS + 0x20 * texts_used - 0x1e, TEXTS + 0x20 * texts_used - 0x1e])
        n = r.choice([0, 0, 1, 2, 3])
        for i in range(n):
            m = r.choice([0, 1, 2])
            first = STRINGS + 0x40 * i if m or r.random() < 0.5 else 0
            put(entries, 0x24 * i + 0x14, 'II', first, first + 0x10 * m if first else 0)
            for j in range(m):
                put(strings, 0x40 * i + 0x10 * j + 4, 'I', text())
        first = ENTRIES if n or r.random() < 0.5 else 0
        put(ime, 0x3c, 'II', first, first + 0x24 * n if first else 0)
        for at in (0xc, 0x1c, 0x2c):
            put(ime, at, 'I', text())
        if op == 'Dtor':
            return Call(0x478be0, [RAM], regions, conv='t')
        if op == 'HostDtor':   # (the IME at HOST + 0x1c, the vector's strings at STRINGS + 0x100)
            host = bytearray(r.bytes(0x1a0))
            host[0x1c:0x7c] = ime
            n = r.choice([0, 0, 1, 2, 4])
            first = STRINGS + 0x100 if n or r.random() < 0.5 else 0
            put(host, 0x190, 'II', first, first + 0x10 * n if first else 0)
            for j in range(n):
                put(strings, 0x100 + 0x10 * j + 4, 'I', text())
            regions[HOST] = host
            del regions[RAM]
            return Call(0x480960, [HOST], regions, conv='t')
        return Call(0x478bc0, [RAM, r.choice([0, 1, 2, 3, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'Size':
        first = r.choice([0, RAM + 0x400])
        put(mem, 0x104, '3I', first, *((first + r.choice([0, 4, 16, 0x40, r.randrange(-0x40, 0x40)])) & 0xffffffff for _ in range(2)))
        return Call(r.choice(SIZES), [RAM + 0x100], {RAM: mem}, conv='t', ret='a')
    if op == 'Current':
        put(mem, 0x154, 'i', r.choice([-1, -1, -2, 0, 1, r.randrange(20), r.getrandbits(31), -r.getrandbits(31)]))
        return Call(0x47a960, [RAM + 0x100], {RAM: mem}, conv='t', ret='a')
    if op == 'Limit':
        return Call(0x47a980, [RAM + 0x100, r.choice([0, 11, 12, 13, r.getrandbits(32)])], {RAM: mem}, conv='t')
    if op == 'Construct':
        return Call(0x47db00, [r.choice([0, RAM + 0x100, RAM + 0x200]), RAM + r.choice([0x100, 0x200, 0x300])], {RAM: mem})
    if op == 'SetLength':
        n = r.choice([0, 1, r.randrange(0x100)])
        put(mem, 0x104, 'I', RAM + 0x200 + r.randrange(0x100))
        return Call(0x47ba30, [RAM + 0x100, n], {RAM: mem}, conv='t')
    n = r.choice([0, 1, 2, r.randrange(3, 20)])
    src = RAM + 0x100 + 4 * r.randrange(0x10)
    dst = r.choice([src + 4 * r.randrange(-12, 13), RAM + 0x800, 0])
    if not dst:  # past a null first slot it would write at 4 on
        n = min(n, 1)
    if op == 'Copy':
        return Call(0x47c910, [r.getrandbits(32), src, src + 4 * n, dst], {RAM: mem}, conv='t', ret='a')
    value = r.choice([src, dst + 4 * r.randrange(n + 1) if dst else src])
    return Call(0x47c940, [r.getrandbits(32), dst, n, value], {RAM: mem}, conv='t')


OPS_ALL = ['Ctor', 'Copy', 'Fill', 'Size', 'Current', 'Limit', 'Construct', 'SetLength', 'Host', 'TextLength', 'OnWindow', 'Char', 'GetContext', 'ReleaseContext', 'Dtor', 'DelDtor', 'HostDtor']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
