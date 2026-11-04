# TqiVideo_DecodeFrame16 (0x671800), TqiVideo_DecodeFrame32 (0x6718a0; its decoders 0x67538b 13, 0x675767 14) and TqiVideo_DecodeFrameYuy2 (0x671940) (src/game/gfx/tqiframe) against the originals. Recorders (as TqiFrameEnv's): the macroblock decoders 0x67519d 10, 0x675579 11, 0x674faf 12 (cdecl); TgqVideo_SetBlockBase real. A frame of up to 3 x 3 macroblocks (sizes not always
# multiples of 16, sometimes 0), the player's flag +0x40 either way, pitches odd now and then.
# usage: tqiframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x671800, 0x67189a), (0x6718a0, 0x67193b), (0x671940, 0x6719d5), (0x66fb60, 0x66fb90)], hooks={0x67519d: _recorder(10, 3, cdecl=True), 0x675579: _recorder(11, 3, cdecl=True), 0x674faf: _recorder(12, 3, cdecl=True),
                                                                                                             0x67538b: _recorder(13, 3, cdecl=True), 0x675767: _recorder(14, 3, cdecl=True)})
PLAYER, SETTINGS = RAM, RAM + 0x1000
OUT = RAM + 0x10000


def case(r):
    player = bytearray(r.bytes(0x800))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x40, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    settings = bytearray(r.bytes(0x80))
    put(settings, 0x60, 'II', r.choice([0, 1, 16, 17, 32, 48]), r.choice([0, 1, 16, 31, 32, 48]))
    return Call(r.choice([0x671800, 0x6718a0, 0x671940]), [PLAYER, r.getrandbits(32), OUT, r.getrandbits(32), r.choice([0x20, 0x40, 0x41, 640, 1280, r.getrandbits(12)])], {LOG: log_region(), PLAYER: player, SETTINGS: settings}, conv='f', ret='v')


if __name__ == '__main__':
    B.main(case)
