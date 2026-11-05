# The IME's composition strings (src/game/core/ime: ImeContext_ReadComposition 0x478860, ImeContext_SetComposition 0x4788c0, Ime_OnComposition 0x478d40) against the originals: the wide and narrow strings and the C library real, the imports recorders (their answers dealt from tables, their out buffers filled). The read answers at most 0x3f bytes: at 0x40 the original's terminator lands on its return address.
# usage: imecomposition.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, answers, blob_bytes, fill, heap, import_table, log_region, put, set_return  # noqa: E402
import gamestring  # noqa: E402

READ, WIDEN, WRITE, NARROW, GETCTX, RELEASE, NOTIFY, LAYOUT, DESCRIBE = range(40, 49)
IMPORTS = {'ImmGetCompositionStringA': (READ, 0x6fd064, 4, 0, 1 << 2), 'MultiByteToWideChar': (WIDEN, 0x6fd174, 6, 1 << 2, 0), 'ImmSetCompositionStringA': (WRITE, 0x6fd094, 6, 1 << 2 | 1 << 4, 0),
           'WideCharToMultiByte': (NARROW, 0x6fd1dc, 8, 0, 0), 'ImmGetContext': (GETCTX, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (RELEASE, 0x6fd080, 2, 0, 0), 'ImmNotifyIME': (NOTIFY, 0x6fd068, 4, 0, 0),
           'GetKeyboardLayout': (LAYOUT, 0x6fd304, 1, 0, 0), 'ImmGetDescriptionA': (DESCRIBE, 0x6fd088, 3, 0, 1 << 1)}
B = Bench(sorted(set(gamestring.B_RANGES + [(0x478860, 0x4788bb), (0x4788c0, 0x478b56), (0x478d20, 0x478d40), (0x478d40, 0x47914b), (0x479150, 0x479260), (0x6b5570, 0x6b55be), (0x45b380, 0x45b9a0), (0x47b1c0, 0x47b320), (0x405000, 0x406000), (0x67a400, 0x690000), (0x6918f0, 0x691cd0),
                                            (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x47ae60, 0x47b060), (0x47b460, 0x47b5b0)])))
IME, CONTEXT, OUT, BLOCK_A, BLOCK_B, TEXT_A, TEXT_B, READ_T, WIDEN_T, NARROW_T, CTX_T = RAM, RAM + 0x80, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00


def table(values, rest):
    return struct.pack('<48i', *(values + [rest] * (48 - len(values))))


def wide(r, n):
    return struct.pack('<%dH' % n, *[r.randrange(1, 0x10000) for _ in range(n)])


def string(r, ime, offset, at, n):
    """A wide string at ime + offset: null, its own, shared or frozen, in a block at `at`; returns the block."""
    how = r.choice(['null', 'own', 'own', 'shared', 'frozen'])
    if how == 'null':
        put(ime, offset, '3I', 0, 0, 0)
        return bytes(2)
    res = max(n, r.choice([31, 31, 63]))
    put(ime, offset, '3I', at + 2, n, res)
    block = bytearray(r.bytes(2)) + wide(r, n) + bytes(2)
    block[1] = {'own': 0, 'shared': r.choice([1, 2, 0xfd]), 'frozen': 0xff}[how]
    return block + bytes((res + 2) * 2 - len(block))


def case(r):
    log = log_region()
    imports = import_table(log, IMPORTS)
    answers(log, READ, READ_T)
    answers(log, WIDEN, WIDEN_T)
    answers(log, NARROW, NARROW_T)
    answers(log, GETCTX, CTX_T)
    set_return(log, RELEASE, r.getrandbits(32))
    fill(log, READ, 2, r.bytes(0x40), which=0)
    count = r.choice([0, 1, 5, 20, 31, 40])
    fill(log, WIDEN, 4, wide(r, min(count, 30)), which=1)
    blob_bytes(log, NARROW, 2, 0xffff)
    fill(log, NARROW, 4, bytes(r.randrange(1, 0x100) for _ in range(30)), which=2)
    fill(log, DESCRIBE, 1, r.choice([b'Microsoft IME 98\0', b'Microsoft IME 98 x\0', b'Microsoft IME 9\0', b'\0', r.bytes(0x20)]), which=3)
    size = r.choice([0, 1, 5, 20, 0x3f, r.randrange(0x40)])
    made = r.choice([0, 1, 5, 12, 31])
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 12 + [1, 2, 3])), 0x78a374: wide(r, r.choice([0, 0, 1, 3])) + bytes(2),
               READ_T: table([r.choice([size, size, size, -1]) for _ in range(4)], size), WIDEN_T: table([], count),
               NARROW_T: table([made, r.choice([made, made, 0, -1]), made, r.choice([made, made, 0, -1])], made),
               CTX_T: table([r.choice([0, 0x1000, 0x1000, 0x1000, 0x1000]) for _ in range(3)], 0x2000),
               CONTEXT: struct.pack('<2I', r.getrandbits(32), r.getrandbits(32)), OUT: r.bytes(0x100),
               TEXT_A: wide(r, r.choice([0, 1, 8])) + bytes(2), TEXT_B: wide(r, r.choice([0, 1, 8])) + bytes(2)}
    which = r.random()
    if which < 0.2:
        out = r.choice([0, OUT, OUT])
        return Call(0x478860, [CONTEXT, r.choice([8, 0x800, r.getrandbits(32)]), out, r.choice([0, 1, 20, r.getrandbits(32)])], regions, conv='t', ret='a')
    if which < 0.45:
        return Call(0x4788c0, [CONTEXT, r.choice([9, r.getrandbits(32)]), r.choice([0, TEXT_A, TEXT_A]), r.getrandbits(32), r.choice([0, TEXT_B, TEXT_B]), r.getrandbits(32)], regions, conv='t', ret='a')
    ime = bytearray(r.bytes(0x70))
    regions[BLOCK_A] = string(r, ime, 8 + 4, BLOCK_A, r.choice([0, 1, 5, 12, 13, 30, 31]))
    regions[BLOCK_B] = string(r, ime, 0x18 + 4, BLOCK_B, r.choice([0, 1, 5, 30]))
    put(ime, 0x58, 'I', r.choice([0, 1, 5, 12, 12, 100]))
    regions[IME] = ime
    l = r.choice([0, 0, 8, 0x800, 0x808, 8 | r.getrandbits(32) & ~0x800, 0x800 | r.getrandbits(32), r.getrandbits(32) & ~0x808 | 1])
    return Call(0x478d40, [IME, r.getrandbits(32), r.getrandbits(32), l], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
