# The mail client's event (src/game/online/mailbox, 0x5c9a60 and 0x5c9a80) against the original: the sending's steps by the state +0x6c, the owner's next message, the callback +0x90. Recorders: the library's imports, the callback, the mail's body and the
# owner's next message; the strings, the erase of the list and the old attachment's destructor real.
# usage: mailclientevent.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_fold, log_region, put, recorder, set_return, strings  # noqa: E402

BUILD, DROP = 21, 22
NEXTLINE, TEXT, COMMAND, STOP, GO, CALLBACK = 25, 26, 27, 28, 29, 30
B = Bench([(0x5c9a60, 0x5c9a7a), (0x5c9a80, 0x5c9e5b), (0x5cdf70, 0x5cdfe0), (0x45b340, 0x45b9a0)] + gamestring.B_RANGES,
          hooks={0x5ca320: _recorder(BUILD, 0), 0x5ccda0: _recorder(DROP, 1)})
SELF, LISTS, OWNER, ATTACHMENT, TEXTS, LIBRARY = RAM, RAM + 0x200, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x1000


def text_at(self, offset, regions, at, text):
    regions[at] = b'\0' + text + b'\0'
    put(self, offset + 4, 'III', at + 1 if text else 0, len(text), len(text))


def case(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    put(imports, 0x6fd578 - IMPORT_TABLE, 'I', recorder(TEXT, 1))
    put(imports, 0x6fd570 - IMPORT_TABLE, 'I', recorder(NEXTLINE, 1))
    put(imports, 0x6fd56c - IMPORT_TABLE, 'I', recorder(COMMAND, 1))
    put(imports, 0x6fd568 - IMPORT_TABLE, 'I', recorder(STOP, 0))
    put(imports, 0x6fd564 - IMPORT_TABLE, 'I', recorder(GO, 0))
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x2000, r.choice([0] * 49 + [2]))}
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))
    self = bytearray(r.bytes(0x94))
    text_at(self, 0, regions, TEXTS, r.choice([b'', b'from', b'a much longer sender, past any reserve there is']))
    text_at(self, 0x74, regions, TEXTS + 0x80, r.choice([b'', b'body']))
    # The list of strings +0x10: its head +0x14, its cursor +0x88 on a node or on the head's next, each node's string at +8.
    count = r.choice([1, 1, 2, 3])
    nodes = bytearray(0x100)
    for i in range(count + 1):
        put(nodes, 0x20 * i, 'II', LISTS + 0x20 * ((i + 1) % (count + 1)), LISTS + 0x20 * ((i + count) % (count + 1)))
        if i:
            word = r.choice([b'one', b'', b'three'])
            at = TEXTS + 0x100 + 0x20 * i
            regions[at] = b'\0' + word + b'\0'
            put(nodes, 0x20 * i + 0xc, 'III', at + 1 if word else 0, len(word), len(word))
    regions[LISTS] = nodes
    put(self, 0x14, 'II', LISTS, count)
    put(self, 0x88, 'I', LISTS + 0x20 * r.randrange(1, count + 1))
    # The attachment, its strings empty.
    attachment = bytearray(0x24)
    put(attachment, 0x20, 'I', r.choice([0, ATTACHMENT + 0x40]))
    regions[ATTACHMENT] = attachment + bytes(0x40)
    put(self, 0x68, 'I', r.choice([0, ATTACHMENT]))
    put(self, 0x6c, 'I', r.choice([0, 1, 2, 3, 3, 3, 4]))
    put(self, 0x70, 'I', LIBRARY)
    self[0x84] = r.choice([0, 1])
    self[0x85] = r.choice([0, 0, 1])
    put(self, 0x8c, 'I', OWNER)
    put(self, 0x90, 'I', r.choice([0, recorder(CALLBACK, 2, cdecl=True), recorder(CALLBACK, 2, cdecl=True)]))
    regions[SELF] = self
    regions[LIBRARY] = bytes(0x10)
    regions[OWNER] = bytes(0x10)
    set_return(log, DROP, r.choice([0, 1]))
    for k in (TEXT, NEXTLINE, COMMAND):
        strings(log, k, 1)
    kind = r.choice([0x78] * 8 + [-5, -4, -4, 0, r.getrandbits(32)])
    code = r.choice([0xfa, 0xfa, 0xfa, 0xfa, 0xdd, 0xf9, 0xfb, 0x1a4, 0x1a5, 0x1a6, 0x1c2, 0x225, 0x226, 0x227, 0x228, 0x228, 0x229, r.getrandbits(32), r.getrandbits(32) | 0x80000000])
    if r.random() < 0.15:
        return Call(0x5c9a60, [kind, code, r.getrandbits(32), SELF], regions, conv='s')
    return Call(0x5c9a80, [SELF, kind, code, r.getrandbits(32)], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
