# The online mail's files (src/game/online/mailbox) against the original: the outbox written (0x5cb720: three lists of numbers, each ended by a line feed) and a message read (0x5cba70: three numbers, two texts, the rest a list of lines), the file stream's
# parts recorders (the object is on each side's own stack: this logged as 0 or its vtable); the opening, the writing and the reading of a line recorders of the bench, the strings, the sprintf and the lists' cursor real. And 0x5cde40, the ios's destructor.
# The line reader answers a stream of the bench's: its state (+4 of its ios) says when the lines end.
# usage: mailbox.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Save,Load,Ios)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, fill_this, global_heap, heap, log_fold, log_region, mask_this, put, set_return, strings, this_first  # noqa: E402

INIT, OSTREAM, ISTREAM, FILECTOR, OPEN, STATE, ODTOR, IDTOR, WIDEN, GETLINE, WRITE, FCLOSE, FILEINIT, FILECLOSE, FILEDTOR, IOSDTOR, ATOI, ERASE, INSERT = range(19)
B = Bench([(0x5cb720, 0x5cba47), (0x5cba70, 0x5cbf39), (0x5cde40, 0x5cde4c), (0x45b340, 0x45b9a0), (0x4f1340, 0x4f1420), (0x5ce910, 0x5ce920)] + gamestring.B_RANGES,
          hooks={0x5cfd80: _recorder(INIT, 0), 0x5cf900: _recorder(OSTREAM, 4), 0x5cfd90: _recorder(ISTREAM, 3), 0x5d06c0: _recorder(FILECTOR, 1), 0x5d08e0: _recorder(OPEN, 2), 0x4f0110: _recorder(STATE, 2),
                 0x5cf630: _recorder(ODTOR, 0), 0x5ce920: _recorder(IDTOR, 0), 0x5cf9c0: _recorder(WIDEN, 1), 0x5cfe40: _recorder(GETLINE, 3), 0x5d0e70: _recorder(WRITE, 2, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True),
                 0x5d0a90: _recorder(FILEINIT, 2), 0x5d09b0: _recorder(FILECLOSE, 0), 0x5cec80: _recorder(FILEDTOR, 0), 0x693830: _recorder(IOSDTOR, 0), 0x67a520: _recorder(ATOI, 1, cdecl=True),
                 0x5cdf70: _recorder(ERASE, 2), 0x5d0330: _recorder(INSERT, 3)})
SELF, LISTS, STREAMS, TABLES, TEXTS = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x500, RAM + 0x1000
GOOD, BAD, VBT = STREAMS, STREAMS + 0x40, STREAMS + 0x80


def base(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 49 + [2]))}
    for k in (INIT, FILECTOR, OPEN, ODTOR, IDTOR, WIDEN, GETLINE, FILEINIT, FILECLOSE, FILEDTOR, IOSDTOR):
        mask_this(log, k)
    for k in (OSTREAM, ISTREAM, STATE):
        this_first(log, k)
    strings(log, OSTREAM, 0, out=1)
    strings(log, ISTREAM, 0, out=1)
    set_return(log, WIDEN, 10)
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))
    return log, regions


def folder(r, self, regions):
    """The std::string at self + 0, {allocator byte, pointer, length, reserve}: the block starts with its count."""
    text = r.choice([b'mail', b'c:\\m', b''])
    regions[TEXTS] = b'\0' + text + bytes(4)
    put(self, 4, 'III', TEXTS + 1 if text else 0, len(text), len(text))


def opened(r, log):
    set_return(log, OPEN, r.choice([1, 1, 1, 0]))
    fill_this(log, OPEN, r.choice([bytes(4), b'\1\0\0\0', b'\1\0\0\0', b'\1\0\0\0']), 0x50, which=2)   # (the file's pointer, set by the open)
    strings(log, OPEN, 1)


def save(r):
    log, regions = base(r)
    self = bytearray(r.bytes(0x90))
    folder(r, self, regions)
    for which, offset in enumerate((0x5c, 0x74, 0x68)):
        head = LISTS + 0x80 * which
        count = r.choice([0, 1, 2, 3])
        nodes = bytearray(0x80)
        for i in range(count + 1):
            put(nodes, 0x10 * i, 'I', head + 0x10 * (i + 1) if i < count else head)
            put(nodes, 0x10 * i + 8, 'I', r.getrandbits(32))
        regions[head] = nodes
        put(self, offset, 'I', head)
    regions[SELF] = self
    opened(r, log)
    strings(log, WRITE, 0b10, out=1)
    set_return(log, FCLOSE, r.choice([0, 0, 0, 0xffffffff]))
    return Call(0x5cb720, [SELF], regions, conv='t', ret='b')


def load(r):
    log, regions = base(r)
    self = bytearray(r.bytes(0x60))
    folder(r, self, regions)
    for at in (0x10, 0x20, 0x3c):
        put(self, at + 4, 'III', 0, 0, 0)
    head = LISTS
    count = r.choice([0, 1, 2])
    nodes = bytearray(0x80)
    for i in range(count + 1):
        put(nodes, 0x10 * i, 'I', head + 0x10 * (i + 1) if i < count else head)
    regions[head] = nodes
    put(self, 0x34, 'I', head)
    regions[SELF] = self
    opened(r, log)
    # The line reader: its text per call, its answer a stream whose ios is good or bad.
    lines = [b'12', b'-3', b'400', b'title', b'from'] + [r.choice([b'one', b'', b'three']) for _ in range(2)]
    fill(log, GETLINE, 0, b''.join(line.ljust(0x10, b'\0') for line in lines), records=7, which=3)
    strings(log, GETLINE, 0, out=1)
    streams = bytearray(0xa0)
    put(streams, 0, 'I', VBT)
    put(streams, 0x40, 'I', VBT)
    put(streams, 0x80 + 4, 'I', 8)
    put(streams, 8 + 4, 'I', r.choice([0, 1, 8]))
    put(streams, 0x48 + 4, 'I', r.choice([2, 4, 6]))
    regions[STREAMS] = streams
    answer = [GOOD] * 5 + [GOOD] * r.choice([0, 1, 2]) + [BAD] * 8
    data = bytearray(0x100)
    struct.pack_into('<%dI' % len(answer), data, 0, *answer)
    regions[TABLES] = data
    answers(log, GETLINE, TABLES)
    strings(log, ATOI, 1)
    strings(log, INSERT, 0, out=0b001)
    blob(log, INSERT, 2, 0b100)
    strings(log, ERASE, 0, out=1)
    set_return(log, FILECLOSE, r.choice([1, 1, 0]))
    return Call(0x5cba70, [SELF, r.getrandbits(32)], regions, conv='t', ret='b')


def ios(r):
    log, regions = base(r)
    regions[SELF] = r.bytes(0x20)
    return Call(0x5cde40, [SELF], regions, conv='t', ret='v')


OPS = {'Save': save, 'Load': load, 'Ios': ios}


def case(r):
    names = os.environ.get('OPS', 'Save,Load,Load,Ios').split(',')
    return OPS[r.choice(names)](r)


if __name__ == '__main__':
    B.main(case)
