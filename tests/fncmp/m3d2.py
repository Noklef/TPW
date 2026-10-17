# The M3D2 loader's error box (src/game/model/m3d2: 0x46f100) against the original: MessageBoxA is a COM-style import recorder (fncmp
# import_table()) logging the text and caption as strings and the style, answering OK, Cancel or anything else.
# usage: m3d2.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x46f100, 0x46f11f)])
K_BOX = 43
IMPORTS = {'MessageBoxA': (K_BOX, 0x6fd37c, 4, 0b0110, 0)}   # (hWnd, text, caption, type): text and caption logged as strings
TEXT = RAM


def case(r):
    log = log_region()
    set_return(log, K_BOX, r.choice([1, 2, 1, 2, 0, 3, 7, 0x102, r.getrandbits(32)]))
    text = r.choice([b'Bad chunk', b'', b'Missing texture %s', bytes(r.getrandbits(7) + 1 for _ in range(r.randrange(40)))])
    regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), TEXT: text + b'\0' + r.bytes(0x10)}
    return Call(0x46f100, [TEXT], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
