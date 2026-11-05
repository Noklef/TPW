# std::istreambuf_iterator<char> and <wchar_t>'s members (src/game/core/streambufiter) against the originals: operator*, equal, _Inc; underflow and uflow recorders in the streambuf's vtable, answering in turn.
# The pointer to the next character is null, waiting, or wraps with its count; 0xff bytes in the buffer make wide ends. Now and then one cell serves as the pointer and the count: the order of the reads shows.
# usage: streambufiter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, answers, log_region, vtable  # noqa: E402

B = Bench([(0x6962d0, 0x696326), (0x696330, 0x6963eb), (0x6963f0, 0x69644b), (0x69d040, 0x69d098), (0x69d0a0, 0x69d15f), (0x69d160, 0x69d1b9)])
IT, X, SB, VT, NEXT, LEFT, ANSWERS, BUF = RAM, RAM + 8, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x304, RAM + 0x400, RAM + 0x600
UNDERFLOW, UFLOW = 4, 5
KINDS = ['deref', 'equal', 'inc']


def iterator(r):
    return struct.pack('<I', r.choice([SB, SB, SB, 0])) + bytes([r.choice([0, 0, 0, 1, 2, 0x80]), r.randrange(256), r.randrange(256), r.randrange(256)])


def case(r):
    kind, wide = r.deal([(k, w) for k in KINDS for w in (0, 1)])
    log = log_region()
    answers(log, UNDERFLOW, ANSWERS)
    answers(log, UFLOW, ANSWERS)
    next_at = r.choice([0, BUF, BUF, BUF, BUF + 2, 0xfffffff0])
    left = 0x20 if next_at == 0xfffffff0 else r.choice([0, 1, 2, 0x10, 0xffffffff, 0xfffffffe, 0x7fffffff, 0x80000000, 0x80000001])
    sb = bytearray(r.bytes(0x30))
    struct.pack_into('<I', sb, 0, VT)
    struct.pack_into('<I', sb, 0x1c, NEXT)
    struct.pack_into('<I', sb, 0x2c, NEXT if next_at == BUF and r.random() < 0.1 else LEFT)
    regions = {LOG: log, IT: iterator(r), X: iterator(r), SB: sb, VT: vtable({}, 8), NEXT: struct.pack('<II', next_at, left),
               ANSWERS: struct.pack('<40I', *[r.choice([0xffffffff, 0xffffffff, 0x41, 0xff, 0xffff, 0x1ffff, 0x1ff, 0xffff0041, 0]) for _ in range(40)]),
               BUF - 8: bytes(r.choice([0xff, 0xff, 0x41, 0]) for _ in range(0x20))}
    if kind == 'deref':
        return Call(0x69d040 if wide else 0x6962d0, [IT], regions, conv='t', ret='a')
    if kind == 'equal':
        return Call(0x69d0a0 if wide else 0x696330, [IT, r.choice([X, X, X, IT])], regions, conv='t', ret='a')
    return Call(0x69d160 if wide else 0x6963f0, [IT], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
