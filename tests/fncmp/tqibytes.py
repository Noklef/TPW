# The TQI block of table bytes (src/game/gfx/tqivideo: 0x677dad and its row 0x677eb6) against the originals: random luma, chroma and table 0xfb7f30, out and its pitch of either sign. The row gets ECX and EDX 0, as 0x677dad leaves them.
# usage: tqibytes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x677dad, 0x6780c1)])
LUMA, CHROMA, OUT = RAM, RAM + 0x800, RAM + 0x10000
TABLE = 0xfb7f30
SIZE = 0x2000


def regions(r):
    return {TABLE: r.bytes(0x100), LUMA: r.bytes(0x400), CHROMA: r.bytes(0x200), OUT: r.bytes(SIZE)}


def case(r):
    if r.deal(['Block', 'Row']) == 'Row':
        return Call(0x677eb6, [LUMA, CHROMA, OUT, 0, 0], regions(r), conv='esiediebpecxedx')
    pitch = r.choice([0x20, 0x40, 0x100, -0x20, -0x100])
    out = OUT + (SIZE - 0x100 if pitch < 0 else 0)
    return Call(0x677dad, [LUMA, CHROMA, out, pitch & 0xffffffff], regions(r))


if __name__ == '__main__':
    B.main(case)
