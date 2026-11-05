# Bench: SpriteQueue_DrawWithPanels 0x58a660: SpriteQueue_Draw, then the panels drawn last to first until one fails.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, set_return  # noqa: E402

DRAW, PANEL = 6, 7
B = Bench([(0x58a660, 0x58a6ae)], hooks={0x58a770: _recorder(DRAW, 0), 0x58a010: _recorder(PANEL, 0)})
SELF, ANSWERS = RAM, RAM + 0x4000


def case(r):
    log = log_region()
    set_return(log, DRAW, r.deal([0, 1, 1, 1, 5]))
    answers(log, PANEL, ANSWERS)
    count = r.randrange(7)
    regions = {LOG: log, SELF + 0x2000: r.bytes(0x20), SELF + 0x2408: struct.pack('<I', count), ANSWERS: struct.pack('<7I', *[r.choice([1, 1, 1, 1, 3, 0]) for _ in range(7)])}
    return Call(0x58a660, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
