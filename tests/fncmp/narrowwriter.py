# The narrow-text writer (src/game/core/narrowwriter) against the original. A writer at W with the game's vtable (0x70a090: slot 2 its
# PutChar, run, so the slots that write through it are compared through the buffer), the cursor in a buffer at BUF (room for 0-0x20
# bytes, or the end any) or null for Finish, the full byte 0 mostly (else 1 or any), the range table at TABLE or none: an object (a
# recorder vtable, its slot 0 Finish's release) whose +8 is a chain of three segments (methods.py's Range op's: first 3, 10 or 20, up to
# four more, the links any of the three or none; the values random).
# Finish (0x6afe40), Dtor (0x6ad030) and Delete (0x6ad090: flags any, Mem_Free hooked); Char (0x6afd40: any character, its high byte 0 now and then); WideChar (0x6afe10: a key mostly in the segments,
# its high half any now and then); Wide (0x6afd90: 0-12 characters, none 0 but the terminator, count -1 or 0..the length).
# usage: narrowwriter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, vtable  # noqa: E402

B = Bench([(0x6afe40, 0x6afe80), (0x6afd40, 0x6afd84), (0x6afe10, 0x6afe3b), (0x6afd90, 0x6afe0b), (0x6ad8f0, 0x6ad946), (0x6ad030, 0x6ad086),
           (0x6ad090, 0x6ad0ae)])
W, BUF, TABLE, SEGS, HVT, TEXT = RAM, RAM + 0x104, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500


def key(r):
    return r.choice([0, 3, 5, 9, 10, 11, 14, 15, 20, 22, 25, r.randrange(1, 40), r.getrandbits(16)])


def world(r, cursor_null=False):
    regions = {LOG: log_region(), HVT: vtable({})}
    room = r.choice([0, 1, 2, 5, 0x20, r.randrange(0x21)])
    cursor = 0 if cursor_null and r.random() < 0.2 else BUF + r.randrange(2)
    end = cursor + room if r.random() < 0.9 else r.getrandbits(32)
    w = bytearray(r.bytes(0x14))
    struct.pack_into('<IIII', w, 0, 0x70a090, cursor, end, TABLE if r.random() < 0.92 else 0)
    w[0x10] = 0 if r.random() < 0.85 else r.choice([1, r.getrandbits(8)])
    regions[W] = w
    regions[BUF - 4] = r.bytes(0x50)
    segs = bytearray(r.bytes(0xc0))
    for at, lo in ((0, 10), (0x40, 3), (0x80, 20)):
        struct.pack_into('<HHHH', segs, at, lo, r.choice([0, 0x40, 0x80]) if at < 0x80 else 0, lo + r.randrange(5),
                         r.choice([0, 0x40, 0x80]) if at < 0x80 else 0)
    regions[SEGS] = segs
    regions[TABLE] = struct.pack('<III', HVT, r.getrandbits(32), SEGS + r.choice([0, 0, 0x40]))
    return regions


def finish(r):
    return Call(0x6afe40, [W], world(r, True), conv='t', ret='v')


def dtor(r):
    return Call(0x6ad030, [W], world(r, True), conv='t', ret='v')


def delete(r):
    regions = world(r, True)
    regions[HEAP] = heap(0x100)   # (Mem_Free's count and hash)
    return Call(0x6ad090, [W, r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


def char(r):
    c = r.getrandbits(16)
    if r.random() < 0.3:
        c &= 0xff
    return Call(0x6afd40, [W, c if r.random() < 0.9 else c | r.getrandbits(32) & 0xffff0000], world(r), conv='t', ret='v')


def wide_char(r):
    k = key(r)
    return Call(0x6afe10, [W, k if r.random() < 0.9 else k | r.getrandbits(32) & 0xffff0000], world(r), conv='t', ret='v')


def wide(r):
    regions = world(r)
    cells = [key(r) or 1 for _ in range(r.choice([0, 1, 3, 12, r.randrange(13)]))]
    if r.random() < 0.5:
        regions[TEXT] = struct.pack('<%dH' % len(cells), *cells) + b'\0\0' + r.bytes(4)
        return Call(0x6afd90, [W, TEXT, 0xffffffff], regions, conv='t', ret='v')
    regions[TEXT] = struct.pack('<%dH' % len(cells), *cells) + r.bytes(4)
    return Call(0x6afd90, [W, TEXT, r.randint(0, len(cells))], regions, conv='t', ret='v')


CALLS = {'Finish': finish, 'Dtor': dtor, 'Delete': delete, 'Char': char, 'WideChar': wide_char, 'Wide': wide}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    return CALLS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
