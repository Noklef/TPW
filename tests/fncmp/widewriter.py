# The wide-text writer (src/game/core/widewriter) against the original. A writer at W: the cursor null or in a buffer at BUF (cells
# 0-8, the end at the cursor now and then, else any cell), the full byte 0, 1 or any, a holder at HOLD (a recorder vtable, slot 0 no
# arguments) or null. The buffer starts two cells before BUF (the cell before the cursor).
# Finish (0x6b00e0): as above. Dtor (0x6ad0b0) and Delete (0x6ad110: flags any, Mem_Free hooked) as Finish.
# Char / Narrow (0x6aff70 / 0x6afe80; lead bytes near the table's and the bytes after them high now and then, so that their codes
# come out small): the writer's vtable the game's (0x70a0c0: slot 4 Obj_PutWord, the words written to the buffer
# until the end; the pushed word's high half is the register's leftovers, so it is not logged), its cursor in the buffer (0-0x20 cells
# before the end, or at it, or full); the code table at TABLE or none: the lead byte 0, 1, 0x80-0xff or any, the count 0-0x400 (codes
# random), the codes at CODES. Char: a random character (low byte 0 now and then). Narrow: 0-12 bytes (lead bytes followed by another;
# the count form never ends on a lead byte: the original runs past the end), count -1 (the text terminated) or the length.
# Wide (0x6affe0): the writer's cursor as for Char, 0-12 cells (none 0 but the terminator), count -1 or 0..the length.
# usage: widewriter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, vtable  # noqa: E402

B = Bench([(0x6b00e0, 0x6b0122), (0x6afe80, 0x6aff70), (0x6aff70, 0x6affdf), (0x6affe0, 0x6b0095), (0x6b00a0, 0x6b00da), (0x6ad0b0, 0x6ad106),
           (0x6ad110, 0x6ad12e)])
W, BUF, HOLD, HVT = RAM, RAM + 0x104, RAM + 0x200, RAM + 0x300
TABLE, TEXT, CODES = RAM + 0x400, RAM + 0x480, RAM + 0x1000


def finish_world(r):
    log = log_region()
    k = r.randrange(9)
    cursor = r.choice([BUF + 2 * k] * 6 + [0])
    end = r.choice([cursor, cursor, BUF + 2 * r.randrange(9), r.getrandbits(32)])
    w = bytearray(r.bytes(0x14))
    struct.pack_into('<III', w, 4, cursor, end, r.choice([HOLD, HOLD, 0]))
    w[0x10] = r.choice([0, 0, 1, r.getrandbits(8)])
    return {W: w, BUF - 4: r.bytes(0x20), HOLD: struct.pack('<I', HVT) + r.bytes(4), HVT: vtable({}), LOG: log}


def finish(r):
    return Call(0x6b00e0, [W], finish_world(r), conv='t', ret='v')


def dtor(r):
    return Call(0x6ad0b0, [W], finish_world(r), conv='t', ret='v')


def delete(r):
    regions = finish_world(r)
    regions[HEAP] = heap(0x100)   # (Mem_Free's count and hash)
    return Call(0x6ad110, [W, r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


def writer(r, regions, table):
    """The writer at W (the game's vtable) writing into the buffer at BUF: room for 0-0x20 cells, or none, or full."""
    room = r.choice([0, 0, 1, 2, 5, 0x20, r.randrange(0x21)])
    w = bytearray(r.bytes(0x14))
    struct.pack_into('<III', w, 0, 0x70a0c0, BUF, BUF + 2 * room)
    struct.pack_into('<I', w, 0xc, table)
    w[0x10] = 0 if r.random() < 0.7 else r.choice([1, r.getrandbits(8)])
    regions[W] = w
    regions[BUF] = r.bytes(0x48)


def code_table(r, regions):
    if r.random() < 0.08:
        return 0, 0, 0
    lead = r.choice([0, 1, 0x80, 0x81, 0xa0, 0xff, r.getrandbits(8)])
    count = r.choice([0, 1, 0x40, 0x100, 0x400, r.randrange(0x401), r.randrange(1, 0x80), r.randrange(1, 0x80)])
    regions[TABLE] = struct.pack('<IIBBHI', r.getrandbits(32), r.getrandbits(32), lead, r.getrandbits(8), count, CODES) + r.bytes(8)
    regions[CODES] = r.bytes(2 * 0x400)
    return TABLE, lead, count


def pair(r, lead):
    """A lead byte (just above the table's, so that the code comes out small now and then) and the byte after it; none (an ordinary
    byte, None) for the lead 0xff."""
    if lead == 0xff:
        return r.randrange(1, 0x100), None
    b = r.choice([min(lead + 1 + r.randrange(3), 0xff), r.randrange(lead + 1, 0x100)])
    return b, r.choice([r.getrandbits(8), r.randrange(min(0x100 - lead, 0xff), 0x100)]) or 1


def char(r):
    regions = {}
    table, lead, _ = code_table(r, regions)
    writer(r, regions, table)
    c = r.getrandbits(16)
    if r.random() < 0.1:
        c &= 0xff00
    elif r.random() < 0.5:
        b, after = pair(r, lead)
        c = b | (after or r.getrandbits(8)) << 8
    return Call(0x6aff70, [W, c if r.random() < 0.9 else c | r.getrandbits(32) & 0xffff0000], regions, conv='t', ret='v')


def narrow(r):
    regions = {}
    table, lead, count = code_table(r, regions)
    writer(r, regions, table)
    text = bytearray()
    for _ in range(r.choice([0, 1, 3, 12, r.randrange(13)])):
        b = r.choice([r.randrange(1, 0x80), r.randrange(1, 0x100), 0] + [count + 1] * (count < lead))   # (the code the count: none)
        if b - 1 >= lead or not b:
            b, after = pair(r, lead)
            text += bytes([b] if after is None else [b, after])
        else:
            text.append(b)
    if r.random() < 0.5:
        regions[TEXT] = bytes(text) + b'\0' + r.bytes(3)
        return Call(0x6afe80, [W, TEXT, 0xffffffff], regions, conv='t', ret='v')
    regions[TEXT] = bytes(text) + r.bytes(4)
    return Call(0x6afe80, [W, TEXT, len(text)], regions, conv='t', ret='v')


def wide(r):
    regions = {}
    writer(r, regions, r.getrandbits(32))
    cells = [r.randrange(1, 0x10000) for _ in range(r.choice([0, 1, 3, 12, r.randrange(13)]))]
    if r.random() < 0.5:
        regions[TEXT] = struct.pack('<%dH' % len(cells), *cells) + b'\0\0' + r.bytes(4)
        return Call(0x6affe0, [W, TEXT, 0xffffffff], regions, conv='t', ret='v')
    regions[TEXT] = struct.pack('<%dH' % len(cells), *cells) + r.bytes(4)
    return Call(0x6affe0, [W, TEXT, r.randint(0, len(cells))], regions, conv='t', ret='v')


CALLS = {'Finish': finish, 'Dtor': dtor, 'Delete': delete, 'Char': char, 'Narrow': narrow, 'Wide': wide}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    return CALLS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
