# The client's message stored in the mailbox (src/game/online/mailbox, 0x5ca010) against the original. The save (0x5cbf40) a recorder; the strings, the list's erase and the push real.
# usage: mailboxstore.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_fold, log_region, put, set_return  # noqa: E402

SAVE = 23
B = Bench([(0x5ca010, 0x5ca21c), (0x5ca220, 0x5ca31a), (0x5cdf00, 0x5cdf70), (0x5cdf70, 0x5cdfe0), (0x45b340, 0x45b9a0), (0x4f1340, 0x4f1420), (0x4a9420, 0x4a953c)] + gamestring.B_RANGES, hooks={0x5cbf40: _recorder(SAVE, 0)})
CLIENT, MAILBOX, LISTS, TEXTS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x800


def text_at(block, offset, regions, at, text):
    regions[at] = b'\0' + text + b'\0'
    put(block, offset + 4, 'III', at + 1 if text else 0, len(text), len(text))


def string_list(r, regions, block, at, nodes_at, texts_at):
    count = r.choice([0, 1, 2, 3])
    nodes = bytearray(0x20 * (count + 1))
    for i in range(count + 1):
        put(nodes, 0x20 * i, 'II', nodes_at + 0x20 * ((i + 1) % (count + 1)), nodes_at + 0x20 * ((i + count) % (count + 1)))
        if i:
            word = r.choice([b'one', b'', b'three'])
            where = texts_at + 0x20 * i
            regions[where] = b'\0' + word + b'\0'
            put(nodes, 0x20 * i + 0xc, 'III', where + 1 if word else 0, len(word), len(word))
    regions[nodes_at] = nodes
    put(block, at + 4, 'II', nodes_at, count)


def case(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 49 + [2, 4]))}
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))
    client = bytearray(r.bytes(0x94))
    text_at(client, 0, regions, TEXTS, r.choice([b'', b'from', b'a sender longer than any reserve']))
    text_at(client, 0x1c, regions, TEXTS + 0x40, r.choice([b'', b'second', b'a second text longer than any reserve']))
    text_at(client, 0x2c, regions, TEXTS + 0x80, r.choice([b'', b'pic.jpg', b'a picture path longer than any reserve']))
    string_list(r, regions, client, 0x10, LISTS, TEXTS + 0x100)
    regions[CLIENT] = client
    mailbox = bytearray(r.bytes(0x4c))
    for at, base, words in ((0x10, 0x200, [b'', b'old']), (0x20, 0x240, [b'', b'old', b'older']), (0x3c, 0x280, [b'', b'old.jpg'])):
        word = r.choice(words)
        count = r.choice([0, 0, 2, 0xff]) if word else 0
        regions[TEXTS + base] = bytes([count]) + word + b'\0'
        put(mailbox, at + 4, 'III', TEXTS + base + 1 if word else 0, len(word), r.choice([len(word), 0x40]) if word else 0)
    string_list(r, regions, mailbox, 0x30, LISTS + 0x100, TEXTS + 0x300)
    regions[MAILBOX] = mailbox
    set_return(log, SAVE, r.choice([0, 1]))
    return Call(0x5ca010, [CLIENT, r.choice([MAILBOX] * 9 + [0])], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
