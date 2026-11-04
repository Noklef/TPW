# A coaster's contacts all tested (src/game/coaster/contacts: 0x439ab0) against the original: the four arrays' sizes, the nodes flagged or not, the later nodes and the partner's chain, the callees recorders; the frees' recorder keeps the control word, and the word at the end compares.
# usage: contactstestall.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put  # noqa: E402

CLEAR, PAIR, ALLOC, FREE = 3, 4, 5, 6
OWNER, NODES, TABLE, CW = RAM, RAM + 0x100, RAM + 0x900, RAM + 0x800
_free_record = _recorder(FREE, 1, cdecl=True)
TESTED, KEPT = 0x200000, 0x10000
OPS = ['mixed', 'mixed', 'mixed', 'tested', 'tested', 'kept', 'off', 'none', 'single', 'cut', 'flags change']


def _free(e):
    """Mem_Free's recorder (tools/fncmp/driver.cpp's contactsFreeStandIn, the same): the control word kept first."""
    e.write(CW, struct.pack('<I', e.cw))
    _free_record(e)


B = Bench([(0x439ab0, 0x439c40)], hooks={0x43ad50: _recorder(CLEAR, 2, cdecl=True), 0x439c40: _recorder(PAIR, 6, cdecl=True), 0x45b9d0: _recorder(ALLOC, 2, cdecl=True), 0x45ba00: _free})


def node(i):
    return NODES + 0x80 * i


def case(r):
    op = r.deal(OPS)
    log = log_region()
    count = 0 if op == 'none' else 1 if op == 'single' else r.randrange(2, 6)
    nodes = bytearray(r.bytes(0x80 * 8))
    extras = r.randrange(0, 4)
    for j in range(3):   # the partners' chain, by +0x68, after the coaster's own
        at = 0x80 * (5 + j)
        put(nodes, at + 0x64, 'I', r.choice([0, r.getrandbits(32)]))
        put(nodes, at + 0x68, 'I', node(5 + j + 1) if j + 1 < extras else 0)
    for i in range(count):
        kind = {'tested': TESTED, 'kept': TESTED | KEPT, 'off': r.choice([0, KEPT]), 'cut': TESTED | KEPT, 'flags change': TESTED}.get(op) if op != 'mixed' and op != 'single' else r.choice([0, KEPT, TESTED, TESTED, TESTED | KEPT])
        flags = r.getrandbits(32) & ~(TESTED | KEPT) | kind
        put(nodes, 0x80 * i + 8, 'I', flags)
        put(nodes, 0x80 * i + 0x64, 'I', node(i + 1) if i + 1 < count else 0)
        put(nodes, 0x80 * i + 0x68, 'I', r.choice([0, node(5), node(5 + r.randrange(0, 3))]))
    if op == 'cut':
        fill(log, CLEAR, 1, bytes(4), offset=0x64)
    elif op == 'flags change':
        fill(log, CLEAR, 1, r.bytes(4), offset=8)
    answers(log, ALLOC, TABLE)
    owner = bytearray(r.bytes(0xb0))
    put(owner, 0x54, 'I', node(0) if count else 0)
    put(owner, 0xa0, 'I', r.choice([0, 1, 3, 0x40, r.getrandbits(16)]))
    regions = {LOG: log, OWNER: owner, NODES: nodes, TABLE: r.bytes(0xa0), CW: bytes(4), 0x792238: r.bytes(4), 0x792230: r.bytes(4), 0x7922e8: r.bytes(4), 0x7922e4: r.bytes(4)}
    return Call(0x439ab0, [OWNER], regions, conv='c', ret='c', cw=r.choice([0x7f, 0x27f, 0x37f, 0x47f, 0xc7f]))


if __name__ == '__main__':
    B.main(case)
