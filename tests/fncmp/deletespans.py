# The coaster editor's two span deletions (src/game/coaster/edittools: CoasterEdit_DeleteSpanBetween 0x447b60) against the original: CoasterEdit_DeleteSpan a
# recorder (k 28, no arguments; so the first piece stored at 0x79c640 is not seen), the real Track_PieceAt on a list of pieces.
# usage: deletespans.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x447b60, 0x447bac), (0x43e520, 0x43e53a)], hooks={0x446740: _recorder(28, 0, cdecl=True)})
GAME, PIECES = RAM + 0x100, RAM + 0x400


def case(r):
    count = r.randrange(1, 9)
    pieces = bytearray(r.bytes(0x80 * count))
    for i in range(count):
        struct.pack_into('<I', pieces, 0x80 * i + 0x64, PIECES + 0x80 * (i + 1) if i + 1 < count else 0)
    game = bytearray(r.bytes(0x80))
    struct.pack_into('<I', game, 0x3c + 0x18, PIECES)
    number = lambda: r.randrange(count) | r.getrandbits(16) << 16  # noqa: E731
    return Call(0x447b60, [number(), number()], {LOG: log_region(), 0x790fec: struct.pack('<I', GAME), 0x79c640: r.bytes(4), GAME: game, PIECES: pieces}, ret='v')


if __name__ == '__main__':
    B.main(case)
