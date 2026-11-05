# A typed line parsed and run (src/game/online/chatcommand: ChatAvatar_ParseCommand 0x4ea490) against the original: chatcommand.py's Run world (its vote's
# always there: a keyword can ask for one), the scanner's keywords with their texts from the formatter (k 12, one of up to 14 keys of 1-3 letters, called
# in turn), the rest real. The line: empty, plain words (no token: kind 0x17 and its text), or a '/' that names no keyword (kind 0x18). Bench limit: a keyword's
# handler is called at its address in the exe, which the driver does not map, so no line here finds one (wscanner.py runs them on their own). The
# Chat's method 0x6fd488 is k 30 here, not 12: k 12 is the keywords' formatter. The heap's blocks are not compared (the tokens' allocator bytes are
# stack garbage and all of them freed by the end): its sizes and freed pointers are.
# usage: chatparse.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, put, strings  # noqa: E402
import chatcommand as cc  # noqa: E402
import wkeywords  # noqa: E402
import wscanner  # noqa: E402

cc.METHODS[0x6fd488] = (30, 2)
cc.TEXTS = tuple(30 if k == 12 else k for k in cc.TEXTS)
CHAT_FORMAT, KEYWORD_FORMAT = _recorder(25, 5, cdecl=True), _recorder(12, 4, cdecl=True)


def formatter(e):
    """The driver has one recorder for each caller: told apart by the destination (the avatar's +0x260; a keyword's is on the stack)."""
    sp = e.r['ESP'] + (4 if e.tail else 0)
    return (CHAT_FORMAT if e.u32(sp + 4) == cc.AVATAR + 0x260 else KEYWORD_FORMAT)(e)


B = Bench(cc.RANGES + wscanner.RANGES + wkeywords.RANGES + [(0x4ea490, 0x4ea740), (0x4f04d0, 0x4f0540), (0x4f7480, 0x4f749e)],
          hooks={**cc.HOOKS, 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x6acd60: formatter})
NIL, LINE = RAM + 0x7000, RAM + 0x7100
LETTERS = [0x41, 0x42, 0x61, 0x62, 0x78, 0x79, 0x5a, 0x7a]


def case(r):
    c = cc.run_case(r)
    regions = c.regions
    del regions[cc.TOKVEC], regions[cc.TOKENS]
    log = regions[LOG]
    if cc.OBJ not in regions:
        cc.vote(r, regions, log)
    length = r.choice([1, 1, 2, 3])
    records = r.randrange(1, 0x70 // (2 * length + 2) + 1)
    keys = [[r.choice(LETTERS) for _ in range(length)] for _ in range(records)]
    strings(log, 12, 0, out=1 << 1)
    fill(log, 12, 1, b''.join(struct.pack('<%dH' % (length + 1), *k, 0) for k in keys), which=1, records=records)
    nil = r.choice([0, NIL])
    regions[NIL] = put(put(bytearray(r.bytes(0x14)), 0, '3I', 0, r.getrandbits(32), 0), 0x10, 'I', 1)
    regions[0xf7c96c] = struct.pack('<II', r.getrandbits(32), nil)
    regions[0x78a374] = r.choice([b'', b'', b'a\0b\0']) + bytes(2)
    regions[HEAP] = heap(0x8000)
    words = [cc.name(r, r.choice([1, 2, 3])) for _ in range(r.choice([0, 1, 1, 2, 3]))]
    body = []
    for w in words:
        body += [0x20] * r.choice([1, 1, 2]) + list(w)
    kind = r.choice(['empty', 'plain', 'plain', 'plain', 'none', 'none', 'none'])
    if kind == 'empty':
        line = []
    elif kind == 'plain':
        line = [0x61] + body
    else:
        line = [0x20] * r.choice([0, 0, 1, 2]) + [0x2f, 0x7e] + body
    text = bytearray(r.bytes(2 * len(line) + 0x20))
    text[1] = r.choice([0, 0, 1, 2, 0xfe, 0xff])
    struct.pack_into('<%dH' % (len(line) + 1), text, 2, *line, 0)
    regions[LINE] = text
    ptr = LINE + 2 if line or r.random() < 0.5 else 0
    return Call(0x4ea490, [cc.AVATAR, r.getrandbits(32), ptr, len(line), max(len(line), r.choice([0, 31, 0x7d]))], regions, conv='t', ret='v',
                ignore=[(HEAP + 0x10, 0x8000 - 0x10)])


if __name__ == '__main__':
    B.main(case)
