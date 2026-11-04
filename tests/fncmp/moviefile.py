# The movie's file opened, its headers counted, closed (src/game/gfx/moviefile: 0x66e080, 0x66e250, 0x66ef50) against the originals. The mmio calls are import recorders, calloc a
# recorder (0 or the rings), the next chunk (0x66e410, which has its own bench, moviechunks.py) a stand-in as driver.cpp's movie_file_next: 21 sound blocks read need more calls
# than a log's room lets a fill's records count. The stand-in's answer is one of four in the player's +0x4e0; it counts its calls in +0x4f0, frames in +0x384, blocks in +0x388
# and +0x390 (a skip answering 2: not +0x388), adds +0x4f4 to the sound's bytes +0x3a0 and +0x4f8 to the frames +0x38c. The rings are whole regions, so a few cases have them
# (2 ms per KB): a tenth of Open's, with 0x20 bytes past each end. The opens' divide: the bytes are never too few for the blocks, as the original would fault.
# usage: moviefile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_region, put, set_return  # noqa: E402

OPEN, BUFFER, READ, SEEK, CLOSE, CALLOC = range(10, 16)
PLAYER, SETTINGS, NAME, TURNS, ROOMS, FRAMES, SOUND = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000, RAM + 0x100000, RAM + 0x400000
FRAME_BYTES, SOUND_BYTES = 0x124f80, 0x493e0
HEADERS = [b'SEAD', b'SCHl', b'1SNh']
OTHERS = [b'SNDC', b'lHCS', b'daes', b'XXXX', b'\0\0\0\0', b'SEAE', b'SCHm', b'1SNi', b'TEAD', b'1Snh']


def _next(e):  # thiscall (): as driver.cpp's movie_file_next
    player = e.r['ECX']
    calls = e.u32(player + 0x4f0)
    answer = e.u32(player + 0x4e0 + 4 * (calls & 3))
    e.write(player + 0x4f0, struct.pack('<I', calls + 1 & 0xffffffff))
    for offset, more in ((0x384, 1), (0x388, 0 if answer == 2 else 1), (0x390, 1), (0x3a0, e.u32(player + 0x4f4)), (0x38c, e.u32(player + 0x4f8))):
        e.write(player + offset, struct.pack('<I', e.u32(player + offset) + more & 0xffffffff))
    e.r['EAX'] = answer


B = Bench([(0x66e080, 0x66e250), (0x66e250, 0x66e410), (0x66ef50, 0x66efd0)], hooks={0x66e410: _next, 0x67fcb0: _recorder(CALLOC, 2, cdecl=True), 0x67ad90: _free})


def base(r):
    player = bytearray(r.bytes(0x500))
    put(player, 0, 'I', SETTINGS)
    settings = bytearray(r.bytes(0x100))
    put(settings, 4, 'I', r.getrandbits(32) & ~0x84000 | r.deal([0, 0x80000]) | r.deal([0, 0x4000]))
    put(settings, 0xa4, 'I', r.deal([r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, 0]))
    return player, settings


def headers(r, log):
    """The file's first chunks: up to five of the headers, each stepped over, then another chunk or a failed read."""
    count = r.deal([0, 1, 1, 2, 3, 5])
    records = [r.deal(HEADERS) + struct.pack('<I', r.deal([8, 0x10, 0x48, 0, 7, r.getrandbits(32)])) for _ in range(count)]
    records.append(r.deal(OTHERS) + r.bytes(4))
    fill(log, READ, 1, b''.join(records), records=len(records))
    # (A failed read ends it: the last, or one sooner, which leaves the rest of the chunks unread.)
    turns = [8] * count + [r.deal([8, 8, 8, 0, 7, 9, 0xffffffff])]
    if count and r.random() < 0.2:
        turns[r.randrange(count)] = r.deal([0, 7, 0xffffffff])
    answers(log, READ, TURNS)
    return struct.pack('<%dI' % len(turns), *turns).ljust(0xa0, b'\0')


def count_case(r, log, player, settings):
    return Call(0x66e250, [PLAYER], {TURNS: headers(r, log)}, conv='t', ret='a')


def close_case(r, log, player, settings):
    put(player, 0x1c, 'I', r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    put(player, 0x20, 'I', r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    put(settings, 0x84, 'I', r.deal([0, 1, r.getrandbits(32)]))
    return Call(0x66ef50, [PLAYER], {HEAP: heap(0x40)}, conv='t', ret='a')


def open_case(r, log, player, settings):
    set_return(log, OPEN, r.deal([r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, 0]))
    set_return(log, BUFFER, r.deal([0, 0, 0, 0, 0, 0, 0, 1, 0x100, 0xffffff00]))
    put(settings, 0x7c, 'i', r.deal([-1, 0, 1, 1, 2, 3, 5, 6, 100]))
    regions = {TURNS: headers(r, log), NAME: r.bytes(8) + b'\0'}
    rooms = 'both' if r.random() < 0.1 else r.deal(['none', 'none', 'frames', 'frames', 'frames'])
    if rooms == 'both':
        room = [FRAMES, SOUND]
        regions[FRAMES] = b'\xcd' * (FRAME_BYTES + 0x20)
        regions[SOUND] = b'\xcd' * (SOUND_BYTES + 0x20)
    else:
        room = [r.getrandbits(32) | 1, 0] if rooms == 'frames' else [0, r.getrandbits(32)]
    regions[ROOMS] = struct.pack('<2I', *room).ljust(0xa0, b'\0')
    answers(log, CALLOC, ROOMS)
    put(player, 0x4e0, '4i', *r.deal([(1, 1, 1, 1), (1, 2, 1, 2), (2, 2, 2, 1), (1, 0, -1, -2), (1, 1, 4, -1), (-1, 1, 1, 1), (0, -2, 1, 4)]))
    put(player, 0x4f0, 'I', r.deal([0, 1, 2, 3]))
    put(player, 0x4f4, 'I', r.deal([0x10, 0x100, 0x1000, 0x4000, 0x9000, 0x9001]))
    put(player, 0x4f8, 'I', r.deal([0, 0, 1]))
    return Call(0x66e080, [NAME, PLAYER], regions, conv='f', ret='a')


def case(r):
    log = log_region()
    player, settings = base(r)
    imports = import_table(log, {'mmioOpen': (OPEN, 0x6fd3e4, 3, 1, 0), 'mmioSetBuffer': (BUFFER, 0x6fd3e8, 4, 0, 0), 'mmioRead': (READ, 0x6fd3e0, 3, 0, 0b10), 'mmioSeek': (SEEK, 0x6fd3d8, 3, 0, 0),
                                 'mmioClose': (CLOSE, 0x6fd3f0, 2, 0, 0)})
    make = r.deal([count_case, count_case, close_case, open_case, open_case, open_case])
    call = make(r, log, player, settings)
    call.regions.update({LOG: log, PLAYER: player, SETTINGS: settings, IMPORT_TABLE: imports})
    return call


if __name__ == '__main__':
    B.main(case)
