# The authorization stamp (src/game/core/authstamp: 0x5d55a0) against the original: the phrase at 0x763f3c the game's, or the plain
# one at 0x774bd8 changed somewhere (or cut short); the key at 0x774b78 the game's or a phrase's bytes changed, its run before a byte
# that matches the phrase's 0..0x46 long (the first equal now and then); the output at 0xf827f8 random beforehand.
# usage: authstamp.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x5d55a0, 0x5d5627)])
PLAIN = b'he Quick Brown Fox Jumped over the Lazy dog ...'


def case(r):
    regions = {}
    plain = bytearray(PLAIN + bytes(0x60 - len(PLAIN)))
    if r.random() < 0.3:
        plain[r.randrange(len(PLAIN) + 1)] = r.choice([0, r.randrange(1, 256)])
    regions[0x774bd8] = bytes(plain)
    phrase = bytearray(b'T' + PLAIN + bytes(0x80 - 1 - len(PLAIN)))
    if r.random() < 0.7:
        for _ in range(r.choice([1, 1, 2, 5])):
            phrase[r.randrange(1, len(PLAIN) + 2)] = r.randrange(256)
    if r.random() < 0.5:
        phrase[1:] = bytes(r.randrange(1, 256) for _ in range(0x7f))
    phrase[0] = r.randrange(256)
    run = r.choice([0, 1, 2, r.randrange(0x47), r.randrange(0x47)])
    key = bytearray(0x48)
    for i in range(0x48):
        key[i] = (phrase[i] + r.randrange(1, 256)) & 0xff if i < run else (phrase[i] if i == run else r.randrange(256))
    regions[0x763f3c] = bytes(phrase)
    regions[0x774b78] = bytes(key)
    regions[0xf827f8] = r.bytes(0x80)
    return Call(0x5d55a0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
