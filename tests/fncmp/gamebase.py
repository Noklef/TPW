# The game's base folder as text (src/game/files/gamepaths: 0x40ef40) against the original: the static String at 0x788028 made now (guard bit clear: atexit a recorder, its dword compared) or given
# (any text, its other guard bits kept), the root String 0xf816e8 and the Strings 0x787f50 and 0x786b68 any texts, with and without their "\" (the global 0xf80be0, now and then another text).
# usage: gamebase.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region  # noqa: E402
import gamestring  # noqa: E402

ATEXIT = 2
B = Bench(gamestring.B_RANGES + [(0x40ef40, 0x40efe3), (0x619240, 0x619248), (0x5c5520, 0x5c5530)], hooks={0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
TEXTS = [b'', b'\\', b'c:\\tpw', b'c:\\tpw\\', b'data', b'Data\\', b'a\\b\\c', b'0123456789abcdef0123']


def case(r):
    regions = {LOG: log_region(), HEAP: heap(0x2000), gamestring.LOCALE: bytes(4)}
    for k, at in enumerate((0xf816e8, 0x787f50, 0x786b68)):
        gamestring.string(r, 0, at, RAM + 0x100 * k, regions, r.choice(TEXTS))
    gamestring.string(r, 0, 0xf80be0, RAM + 0x300, regions, b'\\' if r.random() < 0.9 else r.choice([b'', b'/', b'\\\\']))
    made = r.random() < 0.5
    regions[0x7887e4] = bytes([(r.getrandbits(8) | 1) if made else r.getrandbits(8) & ~1])
    if made:
        gamestring.string(r, 0, 0x788028, RAM + 0x400, regions, r.choice(TEXTS))
    else:
        regions[0x788028] = r.bytes(0x14)
    return Call(0x40ef40, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
