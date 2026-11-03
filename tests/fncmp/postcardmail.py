# The virtual postcard's mail text (src/game/online/postcardmail: PostcardMail_Build, 0x5ca320) against the original: short texts (some with line ends, some null), 0-3 receivers, wide or plain names, an attachment or none; the string
# at +0x74 empty, holding text or shared; the heap failing now and then is left to stdstring.py. The real std::string members and sprintf; GetTickCount a recorder.
# usage: postcardmail.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region, put, set_return  # noqa: E402

B = Bench([(0x5ca320, 0x5cadef), (0x45b340, 0x45b9a0), (0x4f1340, 0x4f1400), (0x67a400, 0x690000)])
MAIL, TEXTS, NODES, ATTACH, BLOCK = RAM, RAM + 0x200, RAM + 0x600, RAM + 0x700, RAM + 0x800
TEXT = 0x20
LETTERS = b'abcdefgh@.1234 '


def text(r, chars, k, line_ends=True):
    """A short text at slot k (now and then null); line ends in some."""
    if r.random() < 0.1:
        return 0
    n = r.choice([0, 1, r.randrange(2, TEXT - 2)])
    t = bytearray(r.choice(LETTERS) for _ in range(n))
    if line_ends and n and r.random() < 0.3:
        t[r.randrange(n)] = r.choice(b'\r\n')
    chars[k * TEXT:k * TEXT + n + 1] = t + b'\0'
    return TEXTS + k * TEXT


def wide(r, chars, k):
    """A wide name of a few characters, or a plain one (its loop stops at once), a byte over 0x7f now and then."""
    n = r.randrange(0, 6)
    high = r.random() < 0.2
    pairs = b''.join(bytes([r.randrange(0x80, 0x100) if high else r.choice(LETTERS), 0]) for _ in range(n))
    t = pairs + b'\0\0' if r.random() < 0.8 else bytes(r.choice(LETTERS) for _ in range(n)) + b'\0'
    chars[k * TEXT:k * TEXT + len(t)] = t
    return TEXTS + k * TEXT if r.random() < 0.9 else 0


def case(r):
    log = log_region()
    table = import_table(log, {'GetTickCount': (1, 0x6fd1d0, 0, 0, 0)})
    set_return(log, 1, r.getrandbits(32))
    chars = bytearray(0x400)
    mail = bytearray(r.bytes(0x90))
    put(mail, 4, 'I', text(r, chars, 0))
    put(mail, 0x20, 'I', text(r, chars, 1))
    put(mail, 0x3c, '2I', r.getrandbits(32), r.getrandbits(32))
    put(mail, 0x48, 'I', wide(r, chars, 2))
    put(mail, 0x4c, 'I', r.deal([0, 1, r.getrandbits(32)]))
    put(mail, 0x58, 'I', wide(r, chars, 3))
    put(mail, 0x5c, 'I', r.deal([0, 1, r.getrandbits(32)]))
    put(mail, 0x64, 'I', r.getrandbits(32))
    nodes = bytearray(r.bytes(0x100))
    count = r.deal([0, 1, 2, 3])
    links = [NODES + 0x20 * k for k in range(count + 1)]   # (links[0] the head)
    for k, at in enumerate(links):
        put(nodes, at - NODES, 'I', links[(k + 1) % len(links)])
        put(nodes, at - NODES + 0xc, 'I', text(r, chars, 4 + k))
    put(mail, 0x14, 'I', NODES)
    regions = {MAIL: mail, TEXTS: chars, NODES: nodes, HEAP: heap(), LOG: log, IMPORT_TABLE: table}
    attachment = r.deal([0, ATTACH, ATTACH])
    put(mail, 0x68, 'I', attachment)
    if attachment:
        block = bytearray(r.bytes(0x30))
        put(block, 4, 'I', text(r, chars, 8))
        put(block, 0x14, 'I', text(r, chars, 9))
        put(block, 0x20, 'I', text(r, chars, 10) if r.random() < 0.85 else 0)
        regions[ATTACH] = block
    kind = r.deal(['empty', 'empty', 'own', 'shared', 'frozen'])
    if kind == 'empty':
        put(mail, 0x78, '3I', 0, 0, 0)
    else:
        n = r.randrange(0, 20)
        block = bytearray(0x40)
        block[0] = {'own': 0, 'shared': r.choice([1, 2, 0xfe]), 'frozen': 0xff}[kind]
        block[1:1 + n] = bytes(r.choice(LETTERS) for _ in range(n))
        regions[BLOCK] = block
        put(mail, 0x78, '3I', BLOCK + 1, n, 0x3e)
    return Call(0x5ca320, [MAIL], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
