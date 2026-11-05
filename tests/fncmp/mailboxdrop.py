# The online mail's first message dropped (src/game/online/mailbox) against the original: 0x5ccda0 (the number out of three lists, the next message read into a target), 0x5cd220 (two files removed), 0x5c99f0 (the attachment made).
# The lists' erase, the removal, the session's accessors, the wide assign and the attachment's constructor recorders; the strings, the sprintf, the wcslen and the cursor real.
# usage: mailboxdrop.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Drop,Delete,Attach)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_fold, log_region, put, set_return, strings  # noqa: E402

ERASE, REMOVE, SAVE, LOAD, INSTANCE, USER, GET34, GET38, STRING14, STRING28, PUSH, WIDE, CTOR = range(13)
B = Bench([(0x5ccda0, 0x5cd091), (0x5cd220, 0x5cd3fb), (0x5c99f0, 0x5c9a51), (0x45b340, 0x45b9a0), (0x4f1340, 0x4f1420), (0x5ce530, 0x5ce53e), (0x5ce910, 0x5ce920), (0x67acd0, 0x67acf0)] + gamestring.B_RANGES,
          hooks={0x634cc0: _recorder(ERASE, 2), 0x67e560: _recorder(REMOVE, 1, cdecl=True), 0x5cb720: _recorder(SAVE, 0), 0x5cba70: _recorder(LOAD, 1), 0x48b390: _recorder(INSTANCE, 0, cdecl=True),
                 0x5b64a0: _recorder(USER, 0), 0x41ce80: _recorder(GET34, 0), 0x41ce90: _recorder(GET38, 0), 0x5b6480: _recorder(STRING14, 0), 0x5b6490: _recorder(STRING28, 0), 0x4a8ad0: _recorder(PUSH, 1),
                 0x4051d0: _recorder(WIDE, 2), 0x5c9720: _recorder(CTOR, 1)})
SELF, LISTS, TARGET, TEXTS, SESSION = RAM, RAM + 0x200, RAM + 0x700, RAM + 0x1000, RAM + 0x1800


def base(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 49 + [2]))}
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))
    return log, regions


def folder(r, self, regions, at=TEXTS):
    """The std::string at self + 0, {allocator byte, pointer, length, reserve}: the block starts with its count."""
    text = r.choice([b'mail', b'c:\\m', b''])
    regions[at] = b'\0' + text + bytes(4)
    put(self, 4, 'III', at + 1 if text else 0, len(text), len(text))


def text_at(r, self, offset, regions, at, text):
    regions[at] = b'\0' + text + b'\0'
    put(self, offset + 4, 'III', at + 1 if text else 0, len(text), len(text))


def delete(r):
    log, regions = base(r)
    self = bytearray(r.bytes(0x20))
    folder(r, self, regions)
    regions[SELF] = self
    strings(log, REMOVE, 1)
    return Call(0x5cd220, [SELF, r.getrandbits(32)], regions, conv='t', ret='v')


def attach(r):
    log, regions = base(r)
    regions[TARGET] = r.bytes(0x70)
    regions[TEXTS] = b'pic.jpg\0'
    set_return(log, CTOR, r.choice([TARGET + 0x40, 0]))
    strings(log, CTOR, 1)
    return Call(0x5c99f0, [TARGET, TEXTS], regions, conv='t')


def drop(r):
    log, regions = base(r)
    self = bytearray(r.bytes(0x80))
    number = r.choice([5, 9, 0x7fffffff])
    head_at = {}
    for which, offset in enumerate((0x70, 0x64, 0x58)):
        head = LISTS + 0x100 * which
        count = r.choice([0, 1, 2, 3, 4])
        nodes = bytearray(0xc0)
        for i in range(count + 1):
            put(nodes, 0x10 * i, 'I', head + 0x10 * (i + 1) if i < count else head)
            put(nodes, 0x10 * i + 8, 'I', number if (i < count and r.random() < 0.4) else r.choice([1, 2, number + 1]))
        head_at[offset] = (head, nodes, count)
        regions[head] = nodes
        put(self, offset + 4, 'I', head)
        put(self, offset + 8, 'I', count)
    # The number taken: the first node of +0x64.
    head, nodes, count = head_at[0x64]
    put(nodes, 8, 'I', number)
    put(self, 0x6c, 'I', r.choice([0, 1, 2, count]))
    # The message's lines (a list at +0x30, its head +0x34): strings at a node's +8.
    lines = r.choice([0, 1, 2])
    nodes = bytearray(0x100)
    block = TEXTS + 0x100
    data = bytearray(0x100)
    for i in range(lines + 1):
        put(nodes, 0x20 * i, 'I', LISTS + 0x300 + 0x20 * (i + 1) if i < lines else LISTS + 0x300)
        if i < lines:
            text = r.choice([b'one', b'', b'three'])
            data[0x20 * i:0x20 * i + 1 + len(text)] = b'\0' + text
            put(nodes, 0x20 * i + 8 + 4, 'I', block + 0x20 * i + 1 if text else 0)
    regions[LISTS + 0x300] = nodes
    regions[block] = data
    put(self, 0x34, 'I', LISTS + 0x300)
    folder(r, self, regions)
    text_at(r, self, 0x20, regions, TEXTS + 0x40, r.choice([b'', b'second', b'a much longer second text, past the reserve']))
    text_at(r, self, 0x3c, regions, TEXTS + 0x80, r.choice([b'', b'pic.jpg']))
    regions[SELF] = self
    target = bytearray(r.bytes(0x70))
    text_at(r, target, 0, regions, TEXTS + 0x500, r.choice([b'', b'old']))
    text_at(r, target, 0x1c, regions, TEXTS + 0x540, r.choice([b'', b'old', b'older text']))
    regions[TARGET] = target
    set_return(log, LOAD, r.choice([1, 1, 1, 0]))
    # The session's: the user name's wide text, its low bytes taken (the high byte is not 0 or is), and two wide strings.
    name = r.choice(['tim', '', 'a\u0100b'])
    session = bytearray(0x100)
    wide = (name + '\0').encode('utf-16-le')
    session[0x40:0x40 + len(wide)] = wide
    first, second = r.choice(['', 'ab']), r.choice(['', 'xyz'])
    session[0x80:0x80 + 2 * len(first) + 2] = (first + '\0').encode('utf-16-le')
    session[0xc0:0xc0 + 2 * len(second) + 2] = (second + '\0').encode('utf-16-le')
    regions[SESSION] = session
    set_return(log, INSTANCE, SESSION)
    set_return(log, USER, SESSION + 0x40)
    set_return(log, GET34, r.getrandbits(32))
    set_return(log, GET38, r.getrandbits(32))
    set_return(log, STRING14, SESSION + 0x80)
    set_return(log, STRING28, SESSION + 0xc0)
    set_return(log, CTOR, r.choice([TARGET + 0x40, 0]))
    strings(log, PUSH, 1)
    strings(log, CTOR, 1)
    strings(log, REMOVE, 1)
    strings(log, ERASE, 0, out=1)
    return Call(0x5ccda0, [SELF, TARGET], regions, conv='t', ret='b')


OPS = {'Drop': drop, 'Delete': delete, 'Attach': attach}


def case(r):
    names = os.environ.get('OPS', 'Drop,Drop,Drop,Delete,Attach').split(',')
    return OPS[r.choice(names)](r)


if __name__ == '__main__':
    B.main(case)
