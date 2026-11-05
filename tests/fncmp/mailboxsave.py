# A new message saved (src/game/online/mailbox, 0x5cbf40) against the original: its number from the last of the list +0x58, its date, two texts and lines written, its picture moved, its number added to the lists +0x58 and +0x64 (sorted and made unique), the outbox saved.
# The file stream's parts, the time, 0x634c60 and the import recorders (0x634bf0 too, whose nodes then stay); the strings, the sprintf, the list's sort parts, the erase 0x634cc0 and the vector iterators real.
# usage: mailboxsave.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill_this, global_heap, heap, log_fold, log_region, mask_this, put, recorder, set_return, strings, this_first  # noqa: E402

# (the k of each the driver's: 2 is the MailboxDropEnv's save, which this bench alone reaches, as it has no input stream)
INIT, OSTREAM, SAVE, FILECTOR, OPEN, STATE, ODTOR = range(7)
WRITE, FCLOSE, FILEINIT = 10, 11, 12
FILEDTOR, IOSDTOR = 14, 15
INSERTN, MOVE = 20, 21
NUMBERSDTOR, TIME, LOCALTIME = 29, 30, 31
B = Bench([(0x5cbf40, 0x5cc5db), (0x5cde40, 0x5cde4c), (0x634cc0, 0x634cf4), (0x45b340, 0x45b9a0), (0x4f1340, 0x4f1420), (0x5ce530, 0x5ce53e), (0x5d0510, 0x5d0590), (0x5d0590, 0x5d0690), (0x5d0690, 0x5d06c0), (0x5d03b0, 0x5d03e0), (0x5cf810, 0x5cf836), (0x47db00, 0x47db20), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b060)] + gamestring.B_RANGES,
          hooks={0x5cfd80: _recorder(INIT, 0), 0x5cf900: _recorder(OSTREAM, 4), 0x5d06c0: _recorder(FILECTOR, 1), 0x5d08e0: _recorder(OPEN, 2), 0x4f0110: _recorder(STATE, 2), 0x5cf630: _recorder(ODTOR, 0),
                 0x5d0e70: _recorder(WRITE, 2, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x5d0a90: _recorder(FILEINIT, 2), 0x693830: _recorder(IOSDTOR, 0), 0x5cb720: _recorder(SAVE, 0),
                 0x634c60: _recorder(INSERTN, 3), 0x634bf0: _recorder(NUMBERSDTOR, 0), 0x67d3f0: _recorder(TIME, 1, cdecl=True), 0x67d8e0: _recorder(LOCALTIME, 1, cdecl=True)})
SELF, LISTS, TM, TEXTS = RAM, RAM + 0x100, RAM + 0x800, RAM + 0x1000


def text_at(self, offset, regions, at, text):
    regions[at] = b'\0' + text + b'\0'
    put(self, offset + 4, 'III', at + 1 if text else 0, len(text), len(text))


def erase(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    nodes = bytearray(0x40)
    for i in range(3):
        put(nodes, 0x10 * i, 'II', LISTS + 0x10 * ((i + 1) % 3), LISTS + 0x10 * ((i + 2) % 3))
    self = bytearray(0x10)
    put(self, 4, 'II', LISTS, r.randrange(1, 9))
    return Call(0x634cc0, [SELF, SELF + 0x20, LISTS + 0x10 * r.randrange(1, 3)], {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x1000, 0), LISTS: nodes, SELF: self + bytes(0x10)}, conv='t')


def case(r):
    if r.random() < 0.1:
        return erase(r)
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    put(imports, 0x6fd11c - IMPORT_TABLE, 'I', recorder(MOVE, 2, True))
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x8000, r.choice([0] * 99 + [2]))}
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))
    self = bytearray(r.bytes(0x80))
    text_at(self, 0, regions, TEXTS, r.choice([b'mail', b'c:\\m', b'']))
    text_at(self, 0x10, regions, TEXTS + 0x40, r.choice([b'title', b'']))
    text_at(self, 0x20, regions, TEXTS + 0x80, r.choice([b'from', b'']))
    text_at(self, 0x3c, regions, TEXTS + 0xc0, r.choice([b'pic.jpg', b'']))
    # The list +0x58: the last node's number gives the new one; its count at +0x60.
    head = LISTS
    count = r.choice([0, 0, 1, 2, 3])
    nodes = bytearray(0x80)
    for i in range(count + 1):
        put(nodes, 0x10 * i, 'I', head + 0x10 * (i + 1) if i < count else head)
        put(nodes, 0x10 * i + 8, 'I', r.choice([1, 2, 7, 0x7fffffff]))
    regions[head] = nodes
    put(self, 0x5c, 'II', head, r.choice([count, count, 1 if not count else count]))
    # The list +0x64, real in the sort: a head (12 bytes), nodes in the heap's way, numbers maybe equal: the new one goes at the end.
    head = LISTS + 0x100
    count = r.choice([0, 1, 2, 3, 5, 9, 18])
    nodes = bytearray(0x200)
    for i in range(count + 1):
        put(nodes, 0x10 * i, 'I', head + 0x10 * (i + 1) if i < count else head)
        put(nodes, 0x10 * i + 4, 'I', head + 0x10 * (i - 1) if i else head + 0x10 * count)
        put(nodes, 0x10 * i + 8, 'I', r.choice([3, 3, 1, 8, 0, 0x80000000]))
    regions[head] = nodes
    put(self, 0x68, 'II', head, count)
    # The message's lines (a list at +0x30, its head +0x34).
    lines = r.choice([0, 1, 2])
    nodes = bytearray(0x80)
    data = bytearray(0x80)
    for i in range(lines + 1):
        put(nodes, 0x20 * i, 'I', LISTS + 0x400 + 0x20 * (i + 1) if i < lines else LISTS + 0x400)
        if i < lines:
            text = r.choice([b'one', b'', b'three'])
            data[0x20 * i:0x20 * i + 1 + len(text)] = b'\0' + text
            put(nodes, 0x20 * i + 0xc, 'I', TEXTS + 0x200 + 0x20 * i + 1 if text else 0)
    regions[LISTS + 0x400] = nodes
    regions[TEXTS + 0x200] = data
    put(self, 0x34, 'I', LISTS + 0x400)
    regions[SELF] = self
    # The time's: a tm whose +0xc, +0x10, +0x14 are the date.
    tm = bytearray(0x24)
    put(tm, 0xc, 'III', r.randrange(1, 32), r.randrange(0, 12), r.randrange(90, 130))
    regions[TM] = tm
    set_return(log, TIME, r.getrandbits(32))
    set_return(log, LOCALTIME, TM)
    blob(log, LOCALTIME, 0, 0b1)
    for k in (INIT, FILECTOR, OPEN, ODTOR, FILEINIT, FILEDTOR, IOSDTOR, NUMBERSDTOR):
        mask_this(log, k)
    for k in (OSTREAM, STATE):
        this_first(log, k)
    strings(log, OSTREAM, 0, out=1)
    set_return(log, OPEN, r.choice([1, 1, 1, 0]))
    fill_this(log, OPEN, r.choice([bytes(4), b'\1\0\0\0', b'\1\0\0\0', b'\1\0\0\0']), 0x50, which=2)   # (the file's pointer, set by the open)
    strings(log, OPEN, 1)
    strings(log, WRITE, 0b10, out=1)
    set_return(log, FCLOSE, r.choice([0, 0, 0, 0xffffffff]))
    set_return(log, WRITE, r.choice([RAM + 0x900]))
    strings(log, MOVE, 0b11)
    strings(log, INSERTN, 0, out=1)
    blob(log, INSERTN, 2, 0b1)
    return Call(0x5cbf40, [SELF], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
