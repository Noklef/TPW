# The online mail's closing (src/game/online/mailbox) against the original: the client's destructor (0x5c9570), the mailbox's (0x5cb030). The winsock end a recorder (the import); all else real, down to the frees.
# usage: mailboxclose.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, global_heap, heap, log_fold, log_region, put, recorder  # noqa: E402

CLEANUP = 25
B = Bench([(0x5c9570, 0x5c9720), (0x5cb030, 0x5cb22d), (0x5c9970, 0x5c99f0), (0x657d02, 0x657d08), (0x5cdfe0, 0x5ce070), (0x5ce540, 0x5ce5d0), (0x5ce910, 0x5ce920), (0x634cc0, 0x634cf4), (0x5cde80, 0x5cdf00), (0x45b340, 0x45b9a0), (0x4f23f0, 0x4f2450), (0x5cdf70, 0x5cdfe0)] + gamestring.B_RANGES)
SELF, LISTS, CLIENT, ATTACHMENT, TEXTS = RAM, RAM + 0x400, RAM + 0xa00, RAM + 0xb00, RAM + 0x1000
LABEL = 0


def base(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    put(imports, 0x6fd430 - IMPORT_TABLE, 'I', recorder(CLEANUP, 0, True))
    return log, {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x2000, 0)}


def text_at(block, offset, regions, text, wide=False):
    """A string at block + offset, its text with a count byte (0, a shared count or 0xff) before it, its block allocated in the region."""
    global LABEL
    LABEL = (LABEL + 1) % 0x10
    at = TEXTS + 0x40 * LABEL
    count = 0 if not text else [0, 2, 0xff][len(text) % 3]
    regions[at] = bytes(2) + bytes([count]) + text + bytes(3)
    put(block, offset + 4, 'III', at + 3 if text else 0, len(text), len(text))


def string_list(r, regions, block, at, nodes_at, count, node=0x20):
    nodes = bytearray(node * (count + 1))
    for i in range(count + 1):
        put(nodes, node * i, 'II', nodes_at + node * ((i + 1) % (count + 1)), nodes_at + node * ((i + count) % (count + 1)))
    regions[nodes_at] = nodes
    put(block, at + 4, 'II', nodes_at, count)


def client(r, regions):
    block = bytearray(0x94)
    text_at(block, 0, regions, r.choice([b'', b'from']))
    string_list(r, regions, block, 0x10, LISTS, r.choice([0, 1, 3]))
    text_at(block, 0x1c, regions, r.choice([b'', b'second']))
    text_at(block, 0x2c, regions, r.choice([b'', b'third']))
    text_at(block, 0x44, regions, r.choice([b'', b'wide']))
    text_at(block, 0x54, regions, r.choice([b'', b'wider']))
    text_at(block, 0x74, regions, r.choice([b'', b'last']))
    if r.random() < 0.5:
        attachment = bytearray(0x24)
        regions[ATTACHMENT] = attachment + bytes(0x10)
        put(block, 0x68, 'I', ATTACHMENT)
    return block


def client_dtor(r):
    log, regions = base(r)
    regions[CLIENT] = client(r, regions)
    return Call(0x5c9570, [CLIENT], regions, conv='t')


def mailbox_dtor(r):
    log, regions = base(r)
    self = bytearray(0xa0)
    text_at(self, 0, regions, r.choice([b'', b'folder']))
    text_at(self, 0x10, regions, r.choice([b'', b'title']))
    text_at(self, 0x20, regions, r.choice([b'', b'from']))
    string_list(r, regions, self, 0x30, LISTS + 0x100, r.choice([0, 1, 2]))
    text_at(self, 0x3c, regions, r.choice([b'', b'pic']))
    string_list(r, regions, self, 0x58, LISTS + 0x200, r.choice([0, 1, 2]), 0x10)
    string_list(r, regions, self, 0x64, LISTS + 0x280, r.choice([0, 1, 2]), 0x10)
    string_list(r, regions, self, 0x70, LISTS + 0x300, r.choice([0, 1, 2]), 0x10)
    text_at(self, 0x80, regions, r.choice([b'', b'host']))
    if r.random() < 0.7:
        regions[CLIENT] = client(r, regions)
        put(self, 0x98, 'I', CLIENT)
    regions[SELF] = self
    return Call(0x5cb030, [SELF], regions, conv='t')


def case(r):
    return r.choice([client_dtor, mailbox_dtor])(r)


if __name__ == '__main__':
    B.main(case)
