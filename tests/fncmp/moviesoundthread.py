# The movie's sound device and thread (src/game/gfx/moviesoundthread: 0x6724a0, 0x672520, 0x672850, 0x672910) against the originals. The stream and the device are over a vtable of COM
# recorders: Lock fills its four outs (logged as 0) with two spans in the case's regions, its answers from a table (a few failures before a 0, each with a Restore); the imports recorders,
# the thread's flag from a table (at most three turns). The distance, the primary's bytes and DirectSoundCreate are recorders as driver.cpp's kMovieSoundThreadEnv (DirectSoundCreate logged
# without ECX, a stdcall's, and it fills the place).
# usage: moviesoundthread.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, import_table, log_region, mask_this, put, recorder, set_return, strings  # noqa: E402

END, DISTANCE, BYTES, CREATE, EXCHANGE, INCREMENT, DECREMENT, SLEEP, LOCK, UNLOCK, RESTORE, CAPS, LEVEL, RELEASE = range(10, 24)
B = Bench([(0x6724a0, 0x672520), (0x672520, 0x672730), (0x672850, 0x672910), (0x672910, 0x672950)],
          hooks={0x67e180: _recorder(END, 0, cdecl=True), 0x672730: _recorder(DISTANCE, 0), 0x672950: _recorder(BYTES, 0), 0x6d76e0: _recorder(CREATE, 3)})
PLAYER, SETTINGS, STREAM, DEVICE, PRIMARY, VTABLE, DATA, SPAN1, SPAN2, TURNS, LOCKS, PLACE = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3010, RAM + 0x3020, RAM + 0x4000, RAM + 0x5000, RAM + 0x6000, RAM + 0x7000, RAM + 0x8000, RAM + 0x9000, RAM + 0xa000


def base(r, log):
    table = bytearray(0x60)
    for offset, stub in ((8, recorder(RELEASE, 1, True)), (0x10, recorder(CAPS, 2, True)), (0x18, recorder(LEVEL, 3, True)), (0x2c, recorder(LOCK, 8, True)), (0x4c, recorder(UNLOCK, 5, True)), (0x50, recorder(RESTORE, 1, True))):
        put(table, offset, 'I', stub)
    player = bytearray(r.bytes(0x500))
    put(player, 0, 'I', SETTINGS)
    settings = bytearray(r.bytes(0x100))
    put(settings, 0x5c, 'I', STREAM)
    objects = struct.pack('<I', VTABLE) + r.bytes(0xc)
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, VTABLE: table, STREAM: objects * 3}
    return player, settings, regions


def write_fixture(r, log, player, regions):
    index = r.deal([0, 1, 23, 24, 25, 47, -1, -25])
    put(player, 0x374, 'i', index)
    total = r.deal([0, 1, 3, 4, 7, 0x10, 0x40, 0x41, 0x80])
    first = r.deal([total, total, 0, total // 2, total // 2 + 1 if total else 0])
    second = total - first
    place = 0x68 + (index - 24 * int(index / 24)) * 0x10
    put(player, place, 'I', DATA)
    put(player, place + 4, 'I', total)
    put(player, place + 8, 'I', r.getrandbits(32))
    put(player, 0x36c, 'i', r.deal([0x8000, 0x1000, 0x7fffffff, 3]))
    put(player, 0x370, 'i', r.deal([0, 5, 0x7ff0, 0x7fffffff]))
    put(player, 0x388, 'I', r.deal([0, 1, 1, 0x80000000]))
    put(player, 0x3cc, 'I', r.deal([0, 0, 0, 1]))
    regions[DATA] = r.bytes(0xa0)
    regions[SPAN1] = b'\xcd' * 0xa0
    regions[SPAN2] = b'\xcd' * 0xa0
    fill(log, LOCK, 3, struct.pack('<I', SPAN1), which=0)
    fill(log, LOCK, 4, struct.pack('<I', first), which=1)
    fill(log, LOCK, 5, struct.pack('<I', SPAN2), which=2)
    fill(log, LOCK, 6, struct.pack('<I', second), which=3)
    strings(log, LOCK, 0, 0b1111000)
    before = [r.deal([1, 0x88780096, 0x80000000]) for _ in range(r.deal([0, 0, 0, 1, 2, 3]))]
    regions[LOCKS] = struct.pack('<40I', *(before + [0] * (40 - len(before))))
    answers(log, LOCK, LOCKS)
    return total


def write_case(r, log, player, settings, regions):
    total = write_fixture(r, log, player, regions)
    return Call(0x672520, [PLAYER, r.deal([0, 1, 0x10, 0x40, total])], regions, conv='f', ret='v')


def main_case(r, log, player, settings, regions):
    total = write_fixture(r, log, player, regions)
    put(player, 0x368, 'I', r.deal([0, 1, 1]))
    set_return(log, DISTANCE, r.deal([total, total - 1, total + 1, 0, 0x80000000, 0xffffffff]))
    turns = [r.deal([1, 1, 2, 0]) for _ in range(3)]
    if struct.unpack_from('<I', player, 0x388)[0] and not struct.unpack_from('<I', player, 0x3cc)[0]:
        turns[2] = 0   # (a second block's copy would read the first's cleared place, in both)
    regions[TURNS] = struct.pack('<40I', *(turns + [0] * 37))
    answers(log, EXCHANGE, TURNS)
    return Call(0x6724a0, [PLAYER], regions, ret='v')


def device_fixture(r, log, regions):
    set_return(log, CREATE, r.deal([0, 0, 0, 0x88780078]))
    fill(log, CREATE, 1, struct.pack('<I', DEVICE))
    mask_this(log, CREATE)
    set_return(log, LEVEL, r.deal([0, 0, 0, 0x88780032]))


def create_case(r, log, player, settings, regions):
    device_fixture(r, log, regions)
    regions[PLACE] = r.bytes(4)
    return Call(0x672910, [PLACE, r.getrandbits(32)], regions, conv='f', ret='a')


def open_case(r, log, player, settings, regions):
    device_fixture(r, log, regions)
    flags = r.getrandbits(32) & ~0xc00000
    put(settings, 4, 'I', flags | r.deal([0, 0x800000, 0x800000, 0x800000]) | r.deal([0, 0x400000]))
    put(settings, 0x54, 'I', r.deal([0, 0, DEVICE]))
    put(settings, 0x58, 'I', r.deal([0, PRIMARY, PRIMARY]))
    blob(log, CAPS, 1, ((1 << 24) - 1) & ~2)
    fill(log, CAPS, 1, r.bytes(4), which=1, offset=4)
    set_return(log, CAPS, r.getrandbits(32))
    set_return(log, BYTES, r.getrandbits(32))
    return Call(0x672850, [PLAYER], regions, conv='t', ret='a')


def case(r):
    log = log_region()
    player, settings, regions = base(r, log)
    regions[IMPORT_TABLE] = import_table(log, {'InterlockedExchange': (EXCHANGE, 0x6fd200, 2, 0, 0), 'InterlockedIncrement': (INCREMENT, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (DECREMENT, 0x6fd12c, 1, 0, 0),
                                               'Sleep': (SLEEP, 0x6fd1d4, 1, 0, 0)})
    make = r.deal([write_case, write_case, main_case, main_case, create_case, open_case, open_case])
    return make(r, log, player, settings, regions)


if __name__ == '__main__':
    B.main(case)
