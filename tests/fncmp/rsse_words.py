# The RSSE VM's code word fetch and operand value (src/game/rsse/rsse_vm: 0x557a70 RSSE_FetchWord, 0x5573a0 RSSE_OperandValue, cdecl) against
# the originals: a context whose pc is in the code, at its ends, past them or negative (the script killed: pc -10000, 0 returned) and whose length
# is now and then negative; operands tagged as variables (the top byte 0x40, the index small or 0x10000 on) or anything else (a 16-bit immediate, sign-extended:
# 0x4x top nibbles other than 0x40 among them).
# usage: rsse_words.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fetch,Operand)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x557a70, 0x557aa8), (0x5573a0, 0x5573c5), (0x5da3c0, 0x5da3c1)])
CTX, CODE, VARS = RAM, RAM + 0x100, RAM + 0x400
CALLS = {'Fetch': 0x557a70, 'Operand': 0x5573a0}


def case(r):
    op = r.deal(OPS)
    n = r.choice([0, 1, 4, 0x40])
    ctx = bytearray(r.bytes(0xa0))
    put(ctx, 0x18, '2I', CODE, VARS)
    put(ctx, 0x3c, 'i', r.choice([0, 1, n - 1, n, n + 1, -1, -10000, r.randrange(-0x80000000, 0x80000000), r.randrange(max(n, 1))]))
    put(ctx, 0x50, 'I', n if r.random() < 0.9 else r.choice([0xffffffff, 0x80000000]))
    regions = {CTX: ctx, CODE: r.bytes(0x100), VARS: r.bytes(0x100), VARS + 0x40000: r.bytes(0x100)}   # (indices 0x10000 on too)
    if op == 'Fetch':
        return Call(0x557a70, [CTX], regions, ret='a')
    word = r.choice([0x40000000 | r.randrange(0x40), 0x40010000 | r.randrange(0x40), r.getrandbits(32), r.choice([0x41, 0x48, 0x4f, 0xc0, 0x00, 0x80, 0x20, 0x10]) << 24 | r.getrandbits(24),
                     r.getrandbits(16), 0x8000, 0x7fff, 0xffff])
    return Call(0x5573a0, [CTX, word], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
