# The IME host's candidate window (src/game/core/ime: the raw handler 0x480d10 on the host 0x7b4e38, its handler 0x4809f0, the result's
# insertion 0x480fd0, and the IME's candidate notes 0x47a830 / 0x47a6d0) against the originals. The host at 0x7b4e38: its text window +4
# window705a20.py's text_fixture() (SetSelection's: its text 0-20 characters or none, its size +0x13c about it), its text +0x1a8 unfetched
# (the words 0, 1: ImeHost_OnWindow's) or one of 0-30 characters, the window's cursor / anchor inside the host's text (beyond it the
# original reads stack garbage); the IME +0x1c's result (the string +8's characters +0xc) 0-10 characters or none, its window +0x4c and
# index +0x54 random, its description string +0x28 none or a block of its own, shared or frozen, its +0x168 / +0x16c / +0x184 matching the
# host's +0x14 / +0x10 or not. The imports recorders: ImmGetContext (a context or none), ImmNotifyIME, GetKeyboardLayout,
# ImmGetDescriptionA (filling a NUL-ended description), MultiByteToWideChar (answering from a table: its first call the length, 0-40;
# filling the second's out buffer), ImmReleaseContext, OutputDebugStringA. The candidate window's default handler +0x118 CDECL_LOG4.
# The messages 0x14, 0x400, 0x401, 0x405 and others, a and b about the host's.
# usage: imehost.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SelectCandidate,SetCandidatePage,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
_OPS = os.environ.pop('OPS', None)
import window705a20  # noqa: E402
if _OPS is not None:
    os.environ['OPS'] = _OPS
from fncmp import CDECL_LOG4, IMPORT_TABLE, RAM, Bench, Call, answers, fill, import_table, put, set_return  # noqa: E402
from window705a20 import SEL, SEL_TEXT, WIN_RANGES, text_fixture  # noqa: E402

CALLS = {'SelectCandidate': 0x47a830, 'SetCandidatePage': 0x47a6d0, 'InsertResult': 0x480fd0, 'OnMessage': 0x4809f0, 'WindowProc': 0x480d10}
B = Bench(sorted(set(WIN_RANGES + [(0x47a830, 0x47a878), (0x47a6d0, 0x47a830), (0x480fd0, 0x4810f6), (0x4809f0, 0x480af9),
                                   (0x480d10, 0x480d2f), (0x6b557c, 0x6b55b2), (0x47a9a0, 0x47a9b0), (0x47a980, 0x47a99f),
                                   (0x66852b, 0x668564), (0x67bed0, 0x67bf10), (0x65f66e, 0x65f697),
                                   (0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450),
                                   (0x4f6080, 0x4f60a0)])))
IMPORTS = {'ImmGetContext': (20, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (21, 0x6fd080, 2, 0, 0), 'ImmNotifyIME': (22, 0x6fd068, 4, 0, 0),
           'GetKeyboardLayout': (23, 0x6fd304, 1, 0, 0), 'ImmGetDescriptionA': (24, 0x6fd088, 3, 0, 1 << 1),
           'MultiByteToWideChar': (25, 0x6fd174, 6, 1 << 2, 1 << 4), 'OutputDebugStringA': (26, 0x6fd1e0, 1, 1 << 0, 0)}
HOST = 0x7b4e38
RESULT, NAME_BLOCK, WIDEN, CAND = RAM + 0x50000, RAM + 0x50100, RAM + 0x50200, RAM + 0x50400


def wide(r, n):
    return b''.join(struct.pack('<H', r.choice([r.randrange(1, 0x80), r.randrange(1, 0x10000)])) for _ in range(n))


def fixture(r):
    regions, log, n = text_fixture(r)
    regions[IMPORT_TABLE] = import_table(log, IMPORTS)
    w = regions[SEL]
    host = bytearray(r.bytes(0x3b0))
    put(host, 4, 'I', SEL)
    if r.random() < 0.5:   # unfetched: the window's text is taken (none: an empty one)
        put(host, 0x1a8, 'HH', 0, 1)
        length = n if struct.unpack_from('<I', w, 0x138)[0] else 0
    else:
        length = r.choice([0, 1, 4, 12, 30])
        host[0x1a8:0x1a8 + 2 * length + 2] = wide(r, length) + bytes(2)
    put(w, 0x13c, 'I', r.choice([0, 1, n, n, n + 1, n + 5, 0xff]))
    put(w, 0x14a, 'hh', r.randrange(0, length + 1), r.choice([-1, -1, r.randrange(0, length + 1)]))
    ime = 0x1c
    k = r.choice([0, 1, 3, 10])
    regions[RESULT] = wide(r, k) + bytes(2)
    put(host, ime + 0xc, 'I', r.choice([RESULT] * 5 + [0]))
    put(host, ime + 0x54, 'I', r.choice([0xffffffff, 0, 1, r.getrandbits(32)]))
    count = r.choice([0, 0x7f, 1, 0xff])
    block = bytearray(r.bytes(0x48))
    block[1] = count
    regions[NAME_BLOCK] = block
    put(host, ime + 0x28, 'IIII', r.getrandbits(32), r.choice([0, 0, NAME_BLOCK + 2]), r.randrange(0, 20), 31)
    page = r.choice([0, 1, 5, r.getrandbits(32)])
    put(host, 0x10, 'III', page, r.choice([0, 3, page]), r.choice([0, 9, r.getrandbits(32)]))
    put(host, ime + 0x168, 'II', r.choice([struct.unpack_from('<I', host, 0x14)[0], 0, 7]), r.choice([page, page + 1]))
    host[ime + 0x184] = r.choice([0, 1, 1])
    regions[HOST] = host
    cand = bytearray(r.bytes(0x120))
    put(cand, 0x118, 'I', CDECL_LOG4)
    regions[CAND] = cand
    set_return(log, 20, r.choice([0, 0x1234, 0x1234, r.getrandbits(32)]))
    set_return(log, 23, r.getrandbits(32))
    desc = bytes(r.randrange(0x20, 0x7f) for _ in range(r.choice([0, 3, 20]))) + bytes(1)
    fill(log, 24, 1, desc)
    regions[WIDEN] = struct.pack('<48I', r.choice([0, 1, 3, len(desc), 40]), *[r.choice([0, 1, len(desc)]) for _ in range(47)])
    answers(log, 25, WIDEN)
    fill(log, 25, 4, wide(r, r.choice([0, 2, len(desc) - 1 if len(desc) < 30 else 20])) + bytes(2), 1)
    return regions, host


def case(r):
    op = r.deal(OPS)
    regions, host = fixture(r)
    if op == 'SelectCandidate':
        return Call(CALLS[op], [HOST + 0x1c, r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'SetCandidatePage':
        first = r.choice([0, 1, 5, r.getrandbits(32)])
        return Call(CALLS[op], [HOST + 0x1c, first, r.choice([first, first + 9, r.getrandbits(32)]), r.getrandbits(32)], regions, conv='t',
                    ret='v')
    if op == 'InsertResult':
        return Call(CALLS[op], [HOST], regions, conv='t', ret='v')
    message = r.choice([0x14, 0x400, 0x401, 0x401, 0x405, 0x405, 0x13, 0x402, 0x404, r.getrandbits(32)])
    a = r.choice([struct.unpack_from('<I', host, 0x14)[0], 0, 2, r.getrandbits(32)])
    b = r.choice([struct.unpack_from('<I', host, 0x10)[0], 0, 4, r.getrandbits(32)])
    if op == 'OnMessage':
        return Call(CALLS[op], [HOST, CAND, message, a, b], regions, conv='t', ret='a')
    return Call(CALLS[op], [CAND, message, a, b], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
