# The movie's reader kept ahead, its rewind, the volume, the stop and the sound's thread (src/game/gfx/moviecontrol) against the originals. MovieChunks_Next and the CRT's 0x67e050 recorders as driver.cpp's kMovieControlEnv (the chunks' answers in turn);
# the imports recorders (InterlockedExchange's answers in turn; for the stop, its recorder writes the state the thread would leave, so the loop's Sleep is not reached there).
# usage: moviecontrol.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, import_table, log_region, put, recorder, set_return  # noqa: E402

NEXT, BEGIN, EXCHANGE, SLEEP, SEEK, PRIORITY, VOLUME = range(10, 17)
CALLS = {'Ahead': 0x66f380, 'Rewind': 0x66f480, 'Volume': 0x66f520, 'Stop': 0x66f580, 'Start': 0x6727c0, 'End': 0x672800}
B = Bench([(0x66f380, 0x66f46b), (0x66f480, 0x66f4c8), (0x66f520, 0x66f539), (0x66f580, 0x66f5f3), (0x6727c0, 0x672800), (0x672800, 0x672846)],
          hooks={0x66e410: _recorder(NEXT, 0), 0x67e050: _recorder(BEGIN, 3, cdecl=True)})
PLAYER, SETTINGS, TURNS, SOUND, SLOTS = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200


def case(r):
    log = log_region()
    player = bytearray(r.bytes(0x500))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x384, '2i', r.deal([0, 0x15, 0x15, 0x16, -1, 0x17]), r.deal([0, 0x15, 0x15, 0x16, -1, 0x17]))
    put(player, 0x3ac, 'i', r.deal([0, 0, 2, -1]))
    put(player, 0x3c4, 'I', r.deal([0, r.getrandbits(32)]))
    settings = bytearray(r.bytes(0x100))
    put(settings, 0, 'I', r.deal([PLAYER, PLAYER, PLAYER, 0]))
    put(settings, 4, 'I', r.getrandbits(32) & ~0x8000 | r.deal([0, 0x8000]))
    put(settings, 0x5c, 'I', r.deal([SOUND, SOUND, 0]))
    put(settings, 0x84, 'I', r.deal([0, 1, 1]))
    put(settings, 0x88, 'I', r.deal([0, 1]))
    put(settings, 0x9c, 'I', r.deal([1, 2, 5, 0, 6, 7]))
    turn = bytearray(0xc0)
    put(turn, 0, '6i', *[r.deal([-2, -1, 0, 1, 1, 2, 2, 2, 3, 0x100]) for _ in range(6)])
    answers(log, NEXT, TURNS)
    set_return(log, BEGIN, r.deal([0, r.getrandbits(32)]))
    op = r.deal(list(CALLS))
    imports = import_table(log, {'InterlockedExchange': (EXCHANGE, 0x6fd200, 2, 0, 0), 'Sleep': (SLEEP, 0x6fd1d4, 1, 0, 0), 'mmioSeek': (SEEK, 0x6fd3d8, 3, 0, 0), 'SetThreadPriority': (PRIORITY, 0x6fd1fc, 2, 0, 0)})
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, TURNS: turn, SOUND: struct.pack('<I', SLOTS), SLOTS: put(bytearray(0x40), 0x3c, 'I', recorder(VOLUME, 2, True)), IMPORT_TABLE: imports}
    if op == 'Stop':
        player[0x3e0:0x3e4] = struct.pack('<I', r.deal([0, 1, 0x100]))
        fill(log, EXCHANGE, 0, struct.pack('<I', 7 if player[0x3e0:0x3e4] != bytes(4) else 6))
    elif op == 'End':
        replies = bytearray(0xc0)
        put(replies, 0, '4I', r.deal([0, 1]), r.deal([0, 1]), r.deal([0, 1]), 1)
        regions[SLOTS + 0x100] = replies
        answers(log, EXCHANGE, SLOTS + 0x100)
    return Call(CALLS[op], {'Ahead': [PLAYER], 'Rewind': [PLAYER], 'Volume': [SETTINGS, r.getrandbits(32)], 'Stop': [SETTINGS], 'Start': [PLAYER], 'End': [PLAYER]}[op], regions, conv='t' if op in ('Ahead', 'Rewind', 'Start', 'End') else 'c',
                ret='v' if op in ('Ahead', 'Rewind') else 'a')


if __name__ == '__main__':
    B.main(case)
