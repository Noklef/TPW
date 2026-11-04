# The swear filter (src/game/core/swearfilter: 0x5d2d40, 0x5d3230) against the originals, towlower and iswspace the C locale's on both sides. Find: a text of a few letters in two cases and spaces, the word a piece of it
# (other case, one changed) or any. Apply: words cut from that text and exceptions at or about them, each with its ' ' or '*' or none (no word starting with a space past that: its loop goes round the address space),
# a null or empty word or text now and then, the flag any.
# usage: swearfilter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,Apply)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x5d2d40, 0x5d2dd5), (0x5d3230, 0x5d3523), (0x67e5d0, 0x67e666), (0x67a730, 0x67a740), (0x67f330, 0x67f3b0), (0x67c8d0, 0x67c9f0)])
OPS = [k for k in os.environ.get('OPS', 'Find,Apply').split(',') if k]
SELF, HEADS, NODES, WORDS, TEXT = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x400, RAM + 0x800
STRING = RAM + 0x20
SPACES = [0x20, 0x20, 0x20, 9, 0xa, 0xd]
LETTERS = [0x61, 0x41, 0x62, 0x42, 0x63, 0x5a, 0x7a, 0x31, 0x5f]


def wide(chars):
    return struct.pack('<%dH' % len(chars), *chars)


def sentence(r):
    out = []
    for _ in range(r.choice([0, 1, 2, 3, 5])):
        out += [r.choice(LETTERS[:r.choice([4, 9])]) for _ in range(r.choice([1, 2, 3, 5]))]
        out += [r.choice(SPACES) for _ in range(r.choice([0, 1, 1, 1, 2]))]
    if r.random() < 0.3:
        out = [r.choice(SPACES)] + out
    return out


def piece(r, text):
    """A piece of the text, the case flipped now and then, or one character changed; or any letters."""
    if text and r.random() < 0.85:
        at = r.randrange(len(text))
        out = text[at:at + r.choice([1, 2, 3, 4, 8])]
    else:
        out = [r.choice(LETTERS) for _ in range(r.choice([0, 1, 2, 3]))]
    out = [c ^ 0x20 if (0x41 <= c <= 0x5a or 0x61 <= c <= 0x7a) and r.random() < 0.3 else c for c in out]
    if out and r.random() < 0.1:
        out[r.randrange(len(out))] = r.choice(LETTERS)
    return out


def keyword(r, text):
    out = piece(r, text)
    while out and out[0] in SPACES:   # (see the usage)
        out = out[1:]
    return out


def word_list(r, head, nodes, blob, text, firsts):
    """(head's, nodes' bytes): a circular list of 0-4 nodes at `nodes`, the words appended to blob."""
    n = r.choice([0, 1, 1, 2, 2, 4])
    region = bytearray(r.bytes(0x18 * (n + 1)))
    put(region, 0, '2I', nodes + 0x18 if n else nodes, nodes + 0x18 * n)
    for k in range(1, n + 1):
        at = nodes + 0x18 * k
        chars = r.choice(firsts) + keyword(r, text)
        ptr = WORDS + len(blob) if r.random() > 0.05 else 0
        put(region, 0x18 * k, '2I', at + 0x18 if k < n else nodes, at - 0x18)
        put(region, 0x18 * k + 8, '4I', r.getrandbits(32), ptr, len(chars), len(chars))
        blob += wide(chars) + bytes(2)
    return nodes, region


def find(r):
    text = sentence(r)
    return Call(0x5d2d40, [TEXT, WORDS], {TEXT: wide(text) + bytes(2), WORDS: wide(keyword(r, text)) + bytes(2)}, ret='a')


def apply(r):
    text = sentence(r)
    blob = bytearray()
    head4, nodes4 = word_list(r, 0, NODES, blob, text, [[], [], [0x20], [0x2a]])
    head10, nodes10 = word_list(r, 0, NODES + 0x100, blob, text, [[]])
    self_ = bytearray(r.bytes(0x20))
    put(self_, 4, 'I', head4)
    put(self_, 0x10, 'I', head10)
    ptr = TEXT if r.random() > 0.05 else 0
    string = struct.pack('<4I', r.getrandbits(32), ptr, len(text), len(text))
    regions = {SELF: self_, STRING: string, NODES: nodes4, NODES + 0x100: nodes10, WORDS: bytes(blob) or bytes(2), TEXT: wide(text) + bytes(2), 0xfb1440: bytes(4)}
    return Call(0x5d3230, [SELF, STRING, r.choice([1, 1, 1, 0, 0x100, r.getrandbits(32)])], regions, conv='t', ret='b')


def case(r):
    return find(r) if r.deal(OPS) == 'Find' else apply(r)


if __name__ == '__main__':
    B.main(case)
