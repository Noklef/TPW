# DctVideo_DecodeMacroblock (src/game/gfx/dctmacroblock, 0x676e40) against the original. Recorders (DctMacroblockEnv): DctVlc_DecodeBlock 10 (its answers from a table: 1 the DC's fill), DctIdct_Block 11, TqiVideo_BlockBytes32 12; the bit reader's Skip and ReadCode, FillDc and the RefBlock copies real over their
# buffers. The code table's values small enough that the reference stays inside its region; the last frame [0xfaac60] absent a quarter of the time; the top bits random, so each of the three modes; pitches odd now and then.
# usage: dctmacroblock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region  # noqa: E402

B = Bench([(0x676e40, 0x6770ab), (0x6770b0, 0x677105), (0x677110, 0x677134), (0x677140, 0x677197), (0x676a70, 0x676b47), (0x676b50, 0x676bbe)],
          hooks={0x677c00: _recorder(10, 0, cdecl=True), 0x67832e: _recorder(11, 2, cdecl=True), 0x677dad: _recorder(12, 4, cdecl=True)})
SOURCE, REF, TABLE, WORDS, OUT = RAM, RAM + 0x2000, RAM + 0x8000, RAM + 0x8100, RAM + 0x9000
READER, BUFFERS, CODES, DC = 0xfbd64c, 0xfaa660, 0xfbd44c, 0xfbc14c


def case(r):
    log = log_region()
    answers(log, 10, TABLE)
    codes = b''.join(struct.pack('<I', (r.randrange(-6, 7) & 0x3ff) << 22 | r.getrandbits(14) << 8 | r.randrange(1, 12)) for _ in range(64))
    regions = {LOG: log, SOURCE: r.bytes(0x8000), TABLE: b''.join(struct.pack('<I', r.choice([1, 1, 0, 2, r.getrandbits(32)])) for _ in range(40)), WORDS: r.bytes(0x40),
               READER: struct.pack('<3I', WORDS, r.getrandbits(32), r.randrange(33)), BUFFERS: r.bytes(0x600) + struct.pack('<I', r.choice([0, RAM, r.getrandbits(32) | 1, 1])), CODES: codes, DC: r.bytes(4)}
    return Call(0x676e40, [REF, OUT, r.choice([0x20, 0x40, 0x41, 0x80, 0x140, r.randrange(0x20, 0x140)])], regions, conv='f', ret='v')


if __name__ == '__main__':
    B.main(case)
