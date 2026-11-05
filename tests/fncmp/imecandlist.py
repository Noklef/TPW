# The IME's candidate lists read (src/game/core/imeinput: Ime_ReadCandidates 0x47a0c0, Ime_OpenCandidates 0x479d80, Ime_ChangeCandidates 0x479f10) against the originals: the lists' vectors and wide strings real, the imports recorders (the candidate list and the widened text filled, their answers dealt from tables). A blank list's +4, +8, +0xc and +0x20 are unset locals of the original's: left out where the two callers add blanks (0x47a0c0's writes there are seen in its own cases).
# usage: imecandlist.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, answers, fill, heap, import_table, log_region, put, set_return  # noqa: E402

LIST, WIDEN, GETCTX, RELEASE = range(40, 44)
IMPORTS = {'ImmGetCandidateListA': (LIST, 0x6fd078, 4, 0, 0), 'MultiByteToWideChar': (WIDEN, 0x6fd174, 6, 1 << 2, 0), 'ImmGetContext': (GETCTX, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (RELEASE, 0x6fd080, 2, 0, 0)}
B = Bench([(0x47a0c0, 0x47a39f), (0x479d80, 0x479f0f), (0x479f10, 0x47a0b1), (0x6b5570, 0x6b55be), (0x47ae40, 0x47dd50), (0x479150, 0x479260), (0x479490, 0x4796a8), (0x45b380, 0x45b9a0), (0x405000, 0x406000), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67a400, 0x690000), (0x6918f0, 0x691cd0)])
IME, CONTEXT, ENTRIES, STRINGS, LIST_T, WIDEN_T = RAM, RAM + 0x180, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100
BLANK_FIELDS = (4, 8, 0xc, 0x20)


def table(values, rest):
    return struct.pack('<48i', *(values + [rest] * (48 - len(values))))


def candidates(r, count):
    """A CANDIDATELIST of `count` strings, at most 0x70 bytes."""
    texts = [bytes(r.randrange(1, 0x100) for _ in range(r.randrange(0, 6))) + b'\0' for _ in range(count)]
    out = bytearray(struct.pack('<6I', 0x70, r.getrandbits(32), count, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)))
    at = 0x18 + 4 * count
    for t in texts:
        out += struct.pack('<I', at)
        at += len(t)
    return bytes(out) + b''.join(texts)


def entries(r, n):
    """n lists at ENTRIES, each with 0 to 2 empty strings; the strings' vectors with room or not."""
    data = bytearray(r.bytes(0x24 * max(n, 1)))
    strings = bytearray(r.bytes(0x60 * max(n, 1)))
    for i in range(n):
        k = r.choice([0, 0, 1, 2])
        at = STRINGS + 0x60 * i
        room = r.choice([k, 4])
        data[i * 0x24 + 0x10] = 0
        put(data, i * 0x24 + 0x14, '3I', at if k or room else 0, at + 0x10 * k if k or room else 0, at + 0x10 * room if k or room else 0)
        for j in range(k):
            put(strings, 0x60 * i + 0x10 * j + 4, '3I', 0, 0, 0)
    return data, strings


def case(r):
    log = log_region()
    imports = import_table(log, IMPORTS)
    answers(log, LIST, LIST_T)
    answers(log, WIDEN, WIDEN_T)
    set_return(log, GETCTX, r.choice([0, 0x1000, 0x1000, 0x1000]))
    set_return(log, RELEASE, r.getrandbits(32))
    which = r.random()
    # (a string's byte +0 is the top byte of the context's address, and the two callers' context is on the stack: no strings in their cases)
    fill(log, LIST, 2, candidates(r, r.choice([0, 1, 1, 2, 3]) if which < 0.5 else 0), which=0)
    fill(log, WIDEN, 4, struct.pack('<16H', *[r.randrange(1, 0x10000) for _ in range(16)]), which=1)
    size = r.choice([0, 0x70, 0x70, 0x70, 0x70])
    widen = [v for _ in range(8) for v in (r.choice([0, 1, 4, 9, 9]), r.choice([0, 1, 1, 1]))]
    ime = bytearray(r.bytes(0x70))
    regions = {LOG: log, IMPORT_TABLE: imports, 0x78a374: struct.pack('<2H', r.randrange(1, 0x10000), 0) if r.random() < 0.5 else bytes(2),
               LIST_T: table([v for _ in range(16) for v in (size, 0)], 0), WIDEN_T: table(widen, 1), CONTEXT: struct.pack('<2I', r.getrandbits(32), r.getrandbits(32))}
    put(ime, 0x54, 'I', r.getrandbits(32))
    if which < 0.5:
        n = r.choice([1, 2, 4])
        data, strings = entries(r, n)
        put(ime, 0x3c, '3I', ENTRIES, ENTRIES + n * 0x24, ENTRIES + n * 0x24)
        regions.update({IME: ime, ENTRIES: data, STRINGS: strings, HEAP: heap(0x4000, r.choice([0] * 14 + [1, 2, 3]))})
        return Call(0x47a0c0, [IME, CONTEXT, r.randrange(n)], regions, conv='t', ret='a')
    n = r.choice([0, 1, 2, 31, 32, 33])
    data, strings = entries(r, n)
    room = n < 32 and r.random() < 0.5
    capacity = 40 if room else n
    data += bytes(0x24 * 40 - len(data) if len(data) < 0x24 * 40 else 0)
    put(ime, 0x3c, '3I', ENTRIES if n else 0, ENTRIES + n * 0x24 if n else 0, ENTRIES + capacity * 0x24 if n else 0)
    base = ENTRIES if room and n else HEAP + 0x10
    # (a blank list added in place or, the first thing allocated, in a new block)
    ignore = [(base + i * 0x24 + f, 4) for i in range(n, 32) for f in BLANK_FIELDS]
    mask = r.choice([0, 1, 0x80000001, 1 << r.randrange(32), r.getrandbits(32) & r.getrandbits(32) & r.getrandbits(32)])
    regions.update({IME: ime, ENTRIES: data, STRINGS: strings, HEAP: heap(0x10000)})
    return Call(r.choice([0x479d80, 0x479f10]), [IME, r.getrandbits(32), mask], regions, conv='t', ret='v', ignore=ignore)


if __name__ == '__main__':
    B.main(case)
