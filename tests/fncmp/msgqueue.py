# The GUI library's message queue (src/game/ui/msgqueue: 0x669735, 0x6697e3, 0x6698a4, 0x6698e6) against the originals: a queue of 1-8
# entries (each free or not), its fill index anywhere in them, its count and the four values random; the window now and then 0. The free:
# entries set or not (the heap region given: its free count and hash). Contains: 0-4 word pairs (then the end), the pair asked for one of
# them or random (upper halves random). DispatchOne: the entries' windows among 4 (flags bit 0 and the handler, a logger, set or not),
# their messages mostly 0x1000a-0x1000c, now and then a value 0x80808080, the pairs [0xfaa578] and the windows [0xfaa57c], [0xfaa5dc],
# [0xfaa5ec] set or not, the pair asked for in the list half the time; the entries now and then missing. Init: the entries set or not,
# the capacity 0-8, the allocation failing now and then. Peek / Cancel / Clear: the queue as for Push, now and then a value 0x80808080,
# the windows among two (and 0), the entries now and then missing.
# Pump (0x658e42): DispatchOne's queue as [0xfaa5f0] (or 0), the hook [0xfaa5fc] fncmp's logger or 0, `most` 0 to past the capacity or
# negative.
# PumpSecond (0x658f0b): Pump's, the queue as [0xfaa5f4] (or 0).
# Delete (0x6596b0): Free's, the flags 0-3 or any (the block freed with bit 0: the heap's hash of the frees).
# usage: msgqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Free,Push,Contains,DispatchOne,Init,Peek,Cancel,Clear,Pump)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put  # noqa: E402

CALLS = {'Delete': 0x6596b0, 'Free': 0x669735, 'Push': 0x6697e3, 'Contains': 0x6698a4, 'DispatchOne': 0x6698e6, 'Init': 0x669767, 'Peek': 0x669ad7,
         'Cancel': 0x669b3e, 'Clear': 0x669bdb, 'Pump': 0x658e42, 'PumpSecond': 0x658f0b}
B = Bench([(0x6596b0, 0x6596de), (0x669735, 0x669767), (0x6697e3, 0x6698a4), (0x6698a4, 0x6698e6), (0x6698e6, 0x669ad7), (0x65d57c, 0x65d58d),
           (0x65f697, 0x65f6d1), (0x669767, 0x6697e3), (0x669ad7, 0x669bdb), (0x669bdb, 0x669c30), (0x67fff0, 0x680048), (0x658e42, 0x658f0b), (0x658f0b, 0x658f4b)])
QUEUE, ENTRIES, WINDOWS, PAIRS = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x1000
WINDOW = 0x120


def queue(r, regions):
    cap = r.choice([1, 2, 3, 4, 8])
    entries = bytearray(r.bytes(16 * cap))
    for k in range(cap):
        if r.random() < 0.5:
            put(entries, 16 * k, 'I', 0)
    regions[ENTRIES] = entries
    q = bytearray(r.bytes(0x14))
    put(q, 0, 'I', ENTRIES)
    put(q, 4, 'I', cap)
    put(q, 8, 'I', r.randrange(cap))
    put(q, 0xc, 'I', r.choice([0, 0, cap - 1, r.randrange(cap)]))
    regions[QUEUE] = q
    return cap


def pairs(r, regions):
    """0-4 word pairs (the seconds mostly bytes, never -1) and the end (a pair whose second is -1). Returns the pairs."""
    words = [(r.getrandbits(16), r.choice([r.getrandbits(8), r.getrandbits(8), r.getrandbits(15)])) for _ in range(r.choice([0, 1, 2, 4]))]
    regions[PAIRS] = b''.join(struct.pack('<HH', a, b) for a, b in words) + struct.pack('<Hh', r.getrandbits(16), -1)
    return words


def case(r):
    op = r.deal(OPS)
    regions = {}
    cap = queue(r, regions)
    if op in ('Free', 'Delete'):   # (Delete: Free's, the flags bit 0 either way)
        regions[HEAP] = heap(0x20)
        put(regions[QUEUE], 0, 'I', r.choice([HEAP + 0x10, HEAP + 0x10, 0]))
        if op == 'Delete':
            return Call(CALLS[op], [QUEUE, r.choice([0, 1, 2, 3, 0xffffffff])], regions, conv='t', ret='a')
        return Call(CALLS[op], [QUEUE], regions, conv='t')
    if op == 'Contains':
        words = pairs(r, regions)
        a, b = r.getrandbits(32), r.getrandbits(32)
        if words and r.random() < 0.6:
            wa, wb = r.choice(words)
            a = a & 0xffff0000 | wa
            b = b & 0xffff0000 | (wb if r.random() < 0.8 else r.getrandbits(16))
        return Call(CALLS[op], [PAIRS, a, b], regions, ret='w')
    if op in ('DispatchOne', 'Pump', 'PumpSecond'):
        regions[LOG] = log_region()
        wins = [WINDOWS + WINDOW * k for k in range(4)]
        for w in wins:
            win = bytearray(r.bytes(WINDOW))
            put(win, 0x48, 'I', win[0x48] & 0xfe | (r.random() < 0.7))
            put(win, 0x114, 'I', r.choice([CDECL_LOG4] * 4 + [0]))
            regions[w] = win
        words = pairs(r, regions)
        for k in range(cap):
            entry = regions[ENTRIES]
            if struct.unpack_from('<I', entry, 16 * k)[0] == 0 and r.random() < 0.8:
                continue
            message = r.choice([0x1000a, 0x1000b, 0x1000c] * 3 + [0x10001, 1, r.getrandbits(32), 0x80808080])
            a, b = [r.choice([r.getrandbits(32)] * 12 + [0x80808080]) for _ in range(2)]
            if words and r.random() < 0.5:
                wa, wb = r.choice(words)
                a = a & 0xffff0000 | wa if a != 0x80808080 else a
                b = b & 0xff00ffff | (wb & 0xff) << 16 if b != 0x80808080 else b
            put(entry, 16 * k, '4I', r.choice(wins), message, a, b)
        regions[0xfaa578] = struct.pack('<II', r.choice([PAIRS, PAIRS, 0]), r.choice(wins[:2] + [0]))
        regions[0xfaa5dc] = struct.pack('<I', r.choice(wins[1:3] + [0]))
        regions[0xfaa5ec] = struct.pack('<I', r.choice(wins[2:] + [0]))
        if r.random() < 0.08:
            put(regions[QUEUE], 0, 'I', 0)
        if op == 'PumpSecond':
            regions[0xfaa5f4] = struct.pack('<I', QUEUE if r.random() < 0.9 else 0)
            return Call(CALLS[op], [r.choice([0, 1, 2, 3, cap, cap + 1, 100, -1, 0x80000000])], regions, ret='a')
        if op == 'Pump':
            regions[0xfaa5f0] = struct.pack('<I', QUEUE if r.random() < 0.9 else 0)
            regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
            return Call(CALLS[op], [r.choice([0, 1, 2, 3, cap, cap + 1, 100, -1, 0x80000000])], regions, ret='a')
        return Call(CALLS[op], [QUEUE], regions, conv='t', ret='w')
    if op == 'Init':
        regions[HEAP] = heap(0x100, r.choice([0] * 6 + [1]))
        put(regions[QUEUE], 0, 'I', r.choice([0] * 6 + [ENTRIES]))
        return Call(CALLS[op], [QUEUE, r.choice([0, 1, 2, 3, 4, 8])], regions, conv='t', ret='w')
    if op in ('Peek', 'Cancel', 'Clear'):
        entry = regions[ENTRIES]
        for k in range(cap):
            if struct.unpack_from('<I', entry, 16 * k)[0]:
                put(entry, 16 * k, 'I', r.choice([0x1234, 0x5678, 0x5678, r.getrandbits(32)]))
                if r.random() < 0.3:
                    put(entry, 16 * k + r.choice([4, 8, 0xc]), 'I', 0x80808080)
        if r.random() < 0.1:
            put(regions[QUEUE], 0, 'I', 0)
        if op == 'Cancel':
            return Call(CALLS[op], [QUEUE, r.choice([0x1234, 0x5678, 0])], regions, conv='t', ret='w')
        return Call(CALLS[op], [QUEUE], regions, conv='t', ret='a' if op == 'Peek' else 'v')
    window = r.choice([0x1234, r.getrandbits(32), r.getrandbits(32), 0])
    return Call(CALLS[op], [QUEUE, window] + [r.getrandbits(32) for _ in range(3)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
