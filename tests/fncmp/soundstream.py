# The sound stream's chain (src/game/core/soundstream) against the originals: a stream at STREAM (a recorder vtable) whose +4 (next) is
# NEXT, now and then null or the stream itself, +8 (upstream) PREV, null or the stream, +0xc an id the find's argument matches now and
# then; the next and upstream recorder vtables too. The query's next answers its out (the local, the record's own stack slot: logged as
# 0, a fill writing it now and then) or a dword in RAM; the user queries (0x6c8e80, 0x6c7460, 0x6c8e00) likewise, over a record of random
# flags. The descriptions over the record; the notify stream's read (slot 7) and send (+0x18 a callback object, a recorder vtable, or
# null). The destructor over the heap (Mem_Free), any flags.
# usage: soundstream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_RETURNS, RAM, Bench, Call, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, convention, returns)
CALLS = {'dtor': (0x6bab30, 't', 'a'), 'setnext': (0x6c61e0, 't', 'a'), 'chain': (0x6c6210, 'c', 'a'), 'close': (0x6c6230, 't', 'v'),
         'unlink': (0x6c6300, 't', 'v'), 'find': (0x6c62e0, 't', 'a'), 'describe': (0x6c62c0, 't', 'v'), 'send': (0x6c6260, 't', 'v'),
         'rate': (0x6ba9a0, 't', 'a'), 'query': (0x6c6280, 't', 'a'), 'seektime': (0x6c6d70, 't', 'a'),
         'queryuser': (0x6c8e80, 't', 'a'), 'describeuser': (0x6c8ed0, 't', 'v'), 'query6c8': (0x6c7460, 't', 'a'),
         'describe6c8': (0x6c74b0, 't', 'v'), 'cbread': (0x6bab90, 't', 'a'), 'cbquery': (0x6c8e00, 't', 'a'), 'cbsend': (0x6c8e50, 't', 'v')}
RANGES = [(0x6bab30, 0x6bab50), (0x6ba9a0, 0x6ba9b0), (0x6c61e0, 0x6c632e), (0x6c6d70, 0x6c6e45), (0x6c8e00, 0x6c8eea), (0x6c7460, 0x6c74d6),
          (0x6bab90, 0x6babb8), (0x6c5090, 0x6c50a3), (0x6c50f0, 0x6c5103)]
B = Bench(RANGES)
STREAM, NEXT, PREV, VT, OUT, RECORD, WORD, CALLBACK, CALLBACK_VT = (RAM + 0x100 * i for i in range(9))
SLOTS = {0: 1, 1: 1, 4: 1, 5: 1, 7: 4, 9: 2, 11: 1, 12: 1}
QUERIES = ('query', 'queryuser', 'query6c8', 'cbquery')
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0] if abs(x) < 3e38 else x


def obj(r, nxt, prev, id_):
    o = bytearray(r.bytes(0x20))
    struct.pack_into('<IIII', o, 0, VT, nxt, prev, id_)
    struct.pack_into('<I', o, 0x18, r.choice([CALLBACK, CALLBACK, 0]))   # (the notify stream's callback)
    return o


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    id_ = r.getrandbits(32)
    stream = obj(r, r.choice([NEXT, NEXT, NEXT, 0, STREAM]), r.choice([PREV, PREV, 0, STREAM]), id_)
    regions = {STREAM: stream, NEXT: obj(r, r.choice([0, PREV]), r.choice([STREAM, 0]), r.getrandbits(32)),
               PREV: obj(r, r.choice([STREAM, NEXT]), 0, r.getrandbits(32)), VT: vtable(SLOTS), OUT: r.bytes(4), RECORD: r.bytes(0x60),
               WORD: r.bytes(4), LOG: log, CALLBACK: struct.pack('<I', CALLBACK_VT) + r.bytes(0xc), CALLBACK_VT: vtable({1: 3})}
    if r.getrandbits(1):
        for k in (0, 1, 4, 6, 9):
            set_return(log, k, r.getrandbits(32))
    if name == 'dtor':
        regions[HEAP] = heap(0x100)
        args = [r.choice([0, 1, 2, 3, r.getrandbits(32)])]
    elif name in ('setnext', 'chain'):
        args = [r.choice([NEXT, PREV, 0, STREAM])]
        if name == 'chain':
            args.append(STREAM)
    elif name == 'find':
        args = [r.choice([id_, id_, r.getrandbits(32)])]
    elif name in QUERIES:   # (the user queries: the answer's sign; the record's flags 0x800 / 0x1000 / 0x2000 any)
        strings(log, 9, 0, out=1)   # (the out: a local)
        if r.random() < 0.7:
            echo(log, 9, 0)
            if r.getrandbits(1):
                fill(log, 9, 0, struct.pack('<I', r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])))
        else:
            set_return(log, 9, WORD)
        args = [OUT, RECORD]
    elif name == 'seektime':   # (the rate, the channels and bits answers, the length; the time small, about a length's end or any)
        set_return(log, 6, r.choice([11025, 22050, 44100, 8000, r.randrange(1, 100000), r.getrandbits(32)]))
        set_return(log, 17, r.choice([1, 2, 2, 0x102, r.getrandbits(32)]))
        set_return(log, 16, r.choice([8, 0x10, 0x10, 0x110, r.getrandbits(32)]))
        length = r.choice([0, 0, r.randrange(1, 0x10000), r.randrange(0x10000, 0x1000000), r.getrandbits(32)])
        ms = r.choice([0, 1, r.randrange(1000), r.randrange(1, 100000), r.randrange(1 << 24, 1 << 25) | 1, r.getrandbits(32)])   # (odd past 2^24: not a float)
        if r.random() < 0.3:   # (the length about the offset: an estimate in floats, one either side)
            rate, ch, bits = (struct.unpack_from('<I', log, LOG_RETURNS + 8 * k + 4)[0] for k in (6, 17, 16))
            f = f32(f32(ms * f32(0.001)) * rate) * (2 if ch & 0xff == 2 else 1) * (2 if bits & 0xff == 0x10 else 1)
            offset = int(round(f)) & 0xffffffff if f < 2 ** 31 else 0x80000000
            offset += offset & 3
            length = (offset + r.choice([-1, 0, 0, 1])) & 0xffffffff
        set_return(log, 14, length)
        args = [OUT, ms]
    elif name in ('describe', 'send', 'cbsend'):
        args = [r.choice([RECORD, r.getrandbits(32)])]
    elif name in ('describeuser', 'describe6c8'):
        args = [RECORD]
    elif name == 'cbread':
        args = [r.getrandbits(32) for _ in range(4)]
    else:
        args = []
    return Call(address, ([] if conv == 'c' else [STREAM]) + args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
