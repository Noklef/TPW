# The sound stream's chain (src/game/core/soundstream) against the originals: a stream at STREAM (a recorder vtable) whose +4 (next) is
# NEXT, now and then null or the stream itself, +8 (upstream) PREV, null or the stream, +0xc an id the find's argument matches now and
# then; the next and upstream recorder vtables too. The query's next answers its out (the local, the record's own stack slot: logged as
# 0, a fill writing it now and then) or a dword in RAM. The destructor over the heap (Mem_Free), any flags.
# usage: soundstream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, convention, returns)
CALLS = {'dtor': (0x6bab30, 't', 'a'), 'setnext': (0x6c61e0, 't', 'a'), 'chain': (0x6c6210, 'c', 'a'), 'close': (0x6c6230, 't', 'v'),
         'unlink': (0x6c6300, 't', 'v'), 'find': (0x6c62e0, 't', 'a'), 'describe': (0x6c62c0, 't', 'v'), 'send': (0x6c6260, 't', 'v'),
         'rate': (0x6ba9a0, 't', 'a'), 'query': (0x6c6280, 't', 'a')}
RANGES = [(0x6bab30, 0x6bab50), (0x6ba9a0, 0x6ba9b0), (0x6c61e0, 0x6c632e)]
B = Bench(RANGES)
STREAM, NEXT, PREV, VT, OUT, RECORD, WORD = (RAM + 0x100 * i for i in range(7))
SLOTS = {0: 1, 1: 1, 4: 1, 5: 1, 9: 2, 11: 1}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def obj(r, nxt, prev, id_):
    o = bytearray(r.bytes(0x18))
    struct.pack_into('<IIII', o, 0, VT, nxt, prev, id_)
    return o


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    id_ = r.getrandbits(32)
    stream = obj(r, r.choice([NEXT, NEXT, NEXT, 0, STREAM]), r.choice([PREV, PREV, 0, STREAM]), id_)
    regions = {STREAM: stream, NEXT: obj(r, r.choice([0, PREV]), r.choice([STREAM, 0]), r.getrandbits(32)),
               PREV: obj(r, r.choice([STREAM, NEXT]), 0, r.getrandbits(32)), VT: vtable(SLOTS), OUT: r.bytes(4), RECORD: r.bytes(0x48),
               WORD: r.bytes(4), LOG: log}
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
    elif name == 'query':
        strings(log, 9, 0, out=1)   # (the out: a local)
        if r.random() < 0.7:
            echo(log, 9, 0)
            if r.getrandbits(1):
                fill(log, 9, 0, r.bytes(4))
        else:
            set_return(log, 9, WORD)
        args = [OUT, RECORD]
    elif name in ('describe', 'send'):
        args = [r.choice([RECORD, r.getrandbits(32)])]
    else:
        args = []
    return Call(address, ([] if conv == 'c' else [STREAM]) + args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
