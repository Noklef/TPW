# Three quads handed on (src/game/render/batchquad: 0x57ed50, 0x589f90, 0x589fd0) against the originals: Batch_DrawQuad (k 15) and Batch_DrawQuadInPlace (k 17; the spec logged as a hash, but for its points pointer and the stack dword +0x14) and SpriteFrame_FillItem
# (k 16; the spec by a fill, its answer 0 or not) recorders.
# usage: screenquad.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x57ed50, 0x57ee22), (0x589f90, 0x589fc3), (0x589fd0, 0x58a003)], hooks={0x565f90: _recorder(15, 2), 0x589b50: _recorder(16, 1), 0x566220: _recorder(17, 2)})
THIS = RAM
MASK = 0x7ffffff & ~(1 << 1) & ~(1 << 5)


def case(r):
    log = log_region()
    set_return(log, 15, r.getrandbits(32))
    set_return(log, 17, r.getrandbits(32))
    regions = {LOG: log, THIS: r.bytes(0x10), 0x879ab0: r.bytes(4)}
    blob(log, 15, 0, MASK)
    blob(log, 17, 0, MASK)
    kind = r.randrange(3)
    if kind == 0:
        return Call(0x57ed50, [THIS], regions, conv='t', ret='a')
    set_return(log, 16, r.choice([0, 1, 1, r.getrandbits(32)]))
    strings(log, 16, 0, out=1)
    fill(log, 16, 0, r.bytes(0x6c))
    return Call(0x589f90 if kind == 1 else 0x589fd0, [THIS], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
