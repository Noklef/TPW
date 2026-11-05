# The online mail's message sent (src/game/online/mailbox, 0x5cc8a0) against the original. Recorders: the message read, the session's accessors and constructor, the wide assign, the list's push, the client's constructor and connect, the attachment's
# constructor; the strings, the wcslen, the erase of the client's list and the old attachment's destructor real.
# The port's dword to the connect is logged as 0: its high word in the original is what EDX held, its low word goes to +0x90, which is compared.
# usage: mailboxsend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, global_heap, heap, log_fold, log_region, put, set_return, strings  # noqa: E402

LOAD, USER, GET34, GET38, STRING14, STRING28, CONSTRUCT, WIDE, ATTACH, CLIENT, CONNECT = 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
PUSHBACK = 24
B = Bench([(0x5cc8a0, 0x5ccd76), (0x5ccd80, 0x5ccd9e), (0x5c9970, 0x5c99f0), (0x5cdfe0, 0x5ce070), (0x45b340, 0x45b9a0), (0x5ce530, 0x5ce53e), (0x67acd0, 0x67acf0)] + gamestring.B_RANGES,
          hooks={0x5cba70: _recorder(LOAD, 1), 0x5b64a0: _recorder(USER, 0), 0x41ce80: _recorder(GET34, 0), 0x41ce90: _recorder(GET38, 0), 0x5b6480: _recorder(STRING14, 0), 0x5b6490: _recorder(STRING28, 0), 0x5b50b0: _recorder(CONSTRUCT, 0),
                 0x4051d0: _recorder(WIDE, 2), 0x5c93c0: _recorder(CLIENT, 0), 0x5c9f10: _recorder(CONNECT, 4), 0x5c9720: _recorder(ATTACH, 1), 0x5cdf00: _recorder(PUSHBACK, 1)})
SELF, LISTS, CLIENTAT, OLDATTACHMENT, TEXTS, SESSION = RAM, RAM + 0x200, RAM + 0x600, RAM + 0x700, RAM + 0x1000, RAM + 0x1800


def text_at(self, offset, regions, at, text):
    regions[at] = b'\0' + text + b'\0'
    put(self, offset + 4, 'III', at + 1 if text else 0, len(text), len(text))


def client_block(r, regions):
    """A client that sent before: its strings +0, +0x1c and +0x74, its list of strings +0x10 and maybe its old attachment."""
    block = bytearray(0x94)
    text_at(block, 0, regions, TEXTS + 0x300, r.choice([b'', b'old']))
    text_at(block, 0x1c, regions, TEXTS + 0x340, r.choice([b'', b'old', b'older text']))
    text_at(block, 0x74, regions, TEXTS + 0x380, r.choice([b'', b'old']))
    count = r.choice([0, 1, 2])
    nodes = bytearray(0x80)
    for i in range(count + 1):
        put(nodes, 0x20 * i, 'II', LISTS + 0x20 * ((i + 1) % (count + 1)), LISTS + 0x20 * ((i + count) % (count + 1)))
    regions[LISTS] = nodes
    put(block, 0x14, 'II', LISTS, count)
    if r.random() < 0.5:
        put(block, 0x68, 'I', OLDATTACHMENT)
        regions[OLDATTACHMENT] = bytes(0x24)
    regions[CLIENTAT] = block


def case(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 36 + [1, 2, 3, 4]))}
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))
    self = bytearray(r.bytes(0xa0))
    # The list +0x64 (its head +0x68, its count +0x6c): the first node's number is the message's.
    head = LISTS + 0x100
    nodes = bytearray(0x40)
    put(nodes, 0, 'I', head + 0x10)
    put(nodes, 0x10, 'I', head)
    put(nodes, 0x18, 'I', r.choice([5, 9]))
    regions[head] = nodes
    put(self, 0x68, 'II', head, r.choice([0, 1, 1, 1, 1, 1]))
    # Its lines.
    lines = r.choice([0, 1, 2])
    nodes = bytearray(0x80)
    data = bytearray(0x80)
    for i in range(lines + 1):
        put(nodes, 0x20 * i, 'I', LISTS + 0x180 + 0x20 * (i + 1) if i < lines else LISTS + 0x180)
        if i < lines:
            text = r.choice([b'one', b'', b'three'])
            data[0x20 * i:0x20 * i + 1 + len(text)] = b'\0' + text
            put(nodes, 0x20 * i + 0xc, 'I', TEXTS + 0x200 + 0x20 * i + 1 if text else 0)
    regions[LISTS + 0x180] = nodes
    regions[TEXTS + 0x200] = data
    put(self, 0x34, 'I', LISTS + 0x180)
    text_at(self, 0x20, regions, TEXTS, r.choice([b'', b'second', b'a much longer second text, past the reserve']))
    text_at(self, 0x3c, regions, TEXTS + 0x40, r.choice([b'', b'pic.jpg']))
    text_at(self, 0x80, regions, TEXTS + 0x80, r.choice([b'', b'old host']))
    regions[TEXTS + 0xc0] = r.choice([b'smtp\0', b'\0'])
    client_block(r, regions)
    put(self, 0x98, 'I', r.choice([CLIENTAT, CLIENTAT, CLIENTAT, 0]))
    regions[SELF] = self
    set_return(log, LOAD, r.choice([1, 1, 1, 0]))
    # The session: there, or made by the constructor's recorder.
    regions[0xf7d88c] = struct.pack('<I', r.choice([SESSION, SESSION, SESSION, 0]))
    session = bytearray(0x100)
    name = (r.choice(['tim', '', 'aĀb']) + '\0').encode('utf-16-le')
    session[0x40:0x40 + len(name)] = name
    first, second = r.choice(['', 'ab']), r.choice(['', 'xyz'])
    session[0x80:0x80 + 2 * len(first) + 2] = (first + '\0').encode('utf-16-le')
    session[0xc0:0xc0 + 2 * len(second) + 2] = (second + '\0').encode('utf-16-le')
    regions[SESSION] = session
    set_return(log, CONSTRUCT, SESSION)
    set_return(log, USER, SESSION + 0x40)
    set_return(log, GET34, r.getrandbits(32))
    set_return(log, GET38, r.getrandbits(32))
    set_return(log, STRING14, SESSION + 0x80)
    set_return(log, STRING28, SESSION + 0xc0)
    set_return(log, CLIENT, r.choice([CLIENTAT, CLIENTAT, 0]))
    set_return(log, ATTACH, r.choice([RAM + 0x800, RAM + 0x800, 0]))
    set_return(log, CONNECT, r.choice([0, 1, 7]))
    strings(log, CONNECT, 0b1, out=0b10)
    strings(log, ATTACH, 1)
    blob(log, PUSHBACK, 0, 0b100)
    return Call(0x5cc8a0, [SELF, TEXTS + 0xc0, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
