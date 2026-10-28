# The number text parts' SetLayouts (src/game/render/textparts: slot 3 of the vtables 0x6ff3b0, 0x6ff3e0, 0x6ff410, 0x6ff440 and 0x6ff4a0;
# 0x4903e0, 0x490650, 0x490940, 0x490b80, 0x490ff0) against the originals. rendererpart.py's TextLayout world (the part's recording vtable,
# its info and record); mostly the flag 0x80 or'd in with the info's dword +0 a number about each one's bounds (0, 0x18 / 0x30, 0xeeeeeee,
# negative, the extremes, any), else as TextLayout (0x48fc40, which they call). The formatter 0x6acd60 the log stub with its number
# argument hashed (text_args(); its buffer an out argument) and the 8 hex digits it writes what slot 9 (the recorder 9) is sent: hashed
# (blob_bytes(), its first character alone for an empty one).
# L350 / L470 / L500 (0x48ff70 / 0x490dc0 / 0x4918a0, slot 3 of the vtables 0x6ff350 / 0x6ff470 / 0x6ff500): the same, their numbers
# formatted with the CRT's swprintf (0x67af00: crt.py's swprintf_hook, the driver's stand-in natively); L350 with the flag 0x8 alone
# now and then (the percent of a 1024th, '%.2f %%'; else its flag 8 clear: with it the text's address would be logged), L470's 0 by
# the text 0x1c9 (no number argument), L500's 0x7fffffff as '-'. Their buffers hashed up to the terminator (swprint()'s length: the
# stack past it differs), but for L470's 0 (the formatter's buffer not hashed: text_args() takes 1-3 arguments, so the stub logs and
# writes nothing for that call's none; the id is compared) and L500's characters 8 and 9 (the buffer's address, sic: the port
# writes its own there as the original's record does).
# usage: textpartnumbers.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=L3b0,L3e0,L410,L440,L4a0,L350,L470,L500)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import rendererpart  # noqa: E402
from crt import swprintf_hook  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log4, blob_bytes, put, strings, text_args  # noqa: E402

CALLS = {'L3b0': 0x4903e0, 'L3e0': 0x490650, 'L410': 0x490940, 'L440': 0x490b80, 'L4a0': 0x490ff0, 'L350': 0x48ff70, 'L470': 0x490dc0,
         'L500': 0x4918a0}
B = Bench(dialog7cc5f4.merged(rendererpart.RANGES + [(0x4903e0, 0x4904b9), (0x490650, 0x4907ac), (0x490940, 0x4909ef), (0x490b80, 0x490c23),
                                                     (0x490ff0, 0x491166), (0x48ff70, 0x49003e), (0x490dc0, 0x490e53),
                                                     (0x4918a0, 0x49191e)]), hooks={0x6acd60: _cdecl_log4, 0x67af00: swprintf_hook})
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
INFO = RAM + 0x500   # (rendererpart.layout()'s)


def value(r):
    return r.choice([0, 0, 1, -1, 0x18, 0x19, 0x30, 0x31, 47, 48, 49, 72, 0xeeeeeee, 0xeeeeeef, -0xeeeeeee, -0xeeeeeef, 0x7fffffff, -0x7fffffff,
                     -0x80000000, r.randrange(-100, 101), r.randrange(-100000, 100001), r.getrandbits(32) - (1 << 31),
                     0xeeeeeee + r.randrange(-3, 4), -0xeeeeeee + r.randrange(-3, 4)])


def swprint(op, n, percent=False):
    """blob_bytes()'s mask for the text (with its terminator) an swprintf op makes of n"""
    if percent:
        f32 = lambda x: struct.unpack('<f', struct.pack('<f', x))[0]   # (the FPU's 24-bit precision)
        text = '%.2f %%' % f32(f32(n / 1024) * 100)
    elif op == 'L470' and not n:
        return 0
    elif op == 'L500' and n == 0x7fffffff:
        text = '-'
    else:
        text = '%d' % n
    return ((1 << 2 * len(text) + 2) - 1) & (~0xf0000 if op == 'L500' else -1)


def case(r):
    op = r.deal(OPS)
    c = rendererpart.layout(r, True)
    part, flags, info = c.args
    if r.random() < 0.8:
        n = value(r) if op != 'L500' or r.random() < 0.8 else 0x7fffffff
        if op == 'L350' and r.random() < 0.3:   # (the flag 0x8 alone: n a 1024th, 32 more than a multiple of 64 a tie at 2 places)
            n = r.choice([n, 32, -32, 96, 1024, 1, r.randrange(-5000, 5000)])
            put(c.regions[INFO], 0, 'i', n)
            log = c.regions[LOG]
            strings(log, 0x3d, 0, 2)
            blob_bytes(log, 9, 0, swprint(op, n, True))
            return Call(CALLS[op], [part, (flags & ~0x80) | 8, INFO], c.regions, conv='t', ret='v', cw=c.cw)
        put(c.regions[INFO], 0, 'i', n)
        flags, info = flags | 0x80, INFO
        log = c.regions[LOG]
        if op != 'L470' or n:
            text_args(log, 1)
        strings(log, 0x3d, 0, 2)
        blob_bytes(log, 9, 0, 3 if op == 'L410' and not n else swprint(op, n) if op in ('L350', 'L470', 'L500') else 0x3ffff)
    else:
        flags &= ~0x88 if op == 'L350' else ~0x80
    return Call(CALLS[op], [part, flags, info], c.regions, conv='t', ret='v', cw=c.cw)


if __name__ == '__main__':
    B.main(case)
