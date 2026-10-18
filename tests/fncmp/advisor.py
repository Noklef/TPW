# The advisor's response lookup (src/game/world/advisor: 0x5997b0) against the original: tables of up to eight entries ended by the id
# 0x270f anywhere (first included), ids repeated, the wanted id among them, past the end, the end id itself or missing.
# usage: advisor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, put  # noqa: E402

B = Bench([(0x5997b0, 0x5997f8)])


def case(r):
    table = bytearray(r.bytes(0x20 * 8))
    end = r.randrange(0, 8)
    ids = [r.choice([1, 2, 3, 100, 0x270e]) for _ in range(8)]
    for i in range(8):
        put(table, 0x20 * i, 'I', 0x270f if i == end else ids[i])
    wanted = r.choice(ids + [0x270f, 0, 4])
    return Call(0x5997b0, [wanted], {0x768fb8: table}, ret='a')


if __name__ == '__main__':
    B.main(case)
