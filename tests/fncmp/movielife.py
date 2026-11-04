# A movie's life (src/game/gfx/movielife: 0x66dff0, 0x66eef0, 0x66efd0, 0x66f050, 0x66f090, 0x66f4e0, 0x670710) against the originals, with the ports below them real where their
# fixture is small (0x66ef50, 0x672990, 0x6729e0, 0x66fa70). The callees with their own benches and the unported are recorders as driver.cpp's kMovieLifeEnv, the imports recorders.
# Two pass a register a recorder cannot log, so they are stand-ins as driver.cpp's: 0x670c20 hashes its entry's place in the player's +0x668; 0x66e080 keeps its name in the player's
# +0x66c and answers the name's first dword. The state's answers: a table (the log's room freezes its count, so at most two turns' worth, then 5; the sound's wait fills the log, so a case
# that waits has one turn, and the frozen answer is the 5). The wait's Sleeps go into the log's hash.
# usage: movielife.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_fold, log_region, put, recorder, set_return, strings  # noqa: E402

CALLOC, BEGIN, END, CALLBACK = 10, 11, 12, 13   # (cdecl)
HEADER, SOUND_START, BUFFER_START, THREAD, AHEAD, STREAM, PRESENT, SWAP, STOP, DROP = range(14, 24)
EXCHANGE, INCREMENT, DECREMENT, SLEEP, TIME, CURRENT, PRIORITY, CLOSE = range(24, 32)
RELEASE, STATUS, PLAY, OVERLAY, RESTORE = range(32, 37)
PLAYER, SETTINGS, NAME, TURNS, ROOM, VTABLE, OBJECTS = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000, RAM + 0x6000, RAM + 0x7000
ERRORS = [0x80070057, 0x88760168, 0x80004001, 0x80004005, 0x88760082, 0x887601c2]


def _show(e):  # fastcall (player, entry): as driver.cpp's movie_life_show
    player = e.r['ECX']
    e.write(player + 0x668, struct.pack('<I', (e.u32(player + 0x668) * 33 ^ (e.r['EDX'] - player)) & 0xffffffff))


def _open(e):  # fastcall (file, player): as driver.cpp's movie_life_open
    e.write(e.r['EDX'] + 0x66c, struct.pack('<I', e.r['ECX']))
    e.r['EAX'] = e.u32(e.r['ECX'])


C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x66dff0, 0x66e080), (0x66eef0, 0x66ef50), (0x66ef50, 0x66efd0), (0x66efd0, 0x66f050), (0x66f050, 0x66f090), (0x66f090, 0x66f380), (0x66f4d0, 0x66f520), (0x670710, 0x670860),
           (0x66fa70, 0x66fb57), (0x672990, 0x6729d6), (0x6729e0, 0x6729f9)],
          hooks={0x67fcb0: C(CALLOC, 2), 0x66e080: _open, 0x670860: _recorder(HEADER, 0), 0x672850: _recorder(SOUND_START, 0), 0x6700d0: _recorder(BUFFER_START, 0), 0x67e050: C(BEGIN, 3), 0x67e180: C(END, 0),
                 0x6727c0: _recorder(THREAD, 0), 0x66f380: _recorder(AHEAD, 0), 0x672a00: _recorder(STREAM, 0), 0x670c20: _show, 0x66f600: _recorder(PRESENT, 0), 0x66fb90: _recorder(SWAP, 0),
                 0x672800: _recorder(STOP, 0), 0x672bf0: _recorder(DROP, 0), 0x67ad90: _free})


def objects(r, regions):
    table = bytearray(0x90)
    for offset, stub in ((8, recorder(RELEASE, 1, True)), (0x24, recorder(STATUS, 2, True)), (0x30, recorder(PLAY, 4, True)), (0x6c, recorder(RESTORE, 1, True)), (0x84, recorder(OVERLAY, 6, True))):
        put(table, offset, 'I', stub)
    regions[VTABLE] = table
    regions[OBJECTS] = b''.join(struct.pack('<I', VTABLE) + r.bytes(0xc) for _ in range(8))


def obj(k):
    return OBJECTS + 0x10 * k


def pointer(r):
    return r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1])


def base(r, log, regions):
    objects(r, regions)
    player = bytearray(r.bytes(0x670))
    put(player, 0, 'I', SETTINGS)
    settings = bytearray(r.bytes(0x100))
    put(settings, 0, 'I', PLAYER)
    flags = r.getrandbits(32) & ~0x4c30800
    for bit in (0x800, 0x10000, 0x20000, 0x40000, 0x800000, 0x4000000):
        flags |= r.deal([0, bit])
    put(settings, 4, 'I', flags)
    for offset, k in ((0x10, 0), (0x1c, 1), (0x20, 2), (0x24, 3), (0x54, 4), (0x58, 5), (0x5c, 6)):
        put(settings, offset, 'I', r.deal([obj(k), obj(k), obj(k), 0]))
    put(settings, 0x10, 'I', obj(0))
    put(settings, 0x14, 'I', obj(7))
    put(settings, 0x84, 'I', r.deal([0, 1]))
    put(settings, 0xb0, 'I', r.deal([0, recorder(CALLBACK, 0, cdecl=True), recorder(CALLBACK, 0, cdecl=True)]))
    set_return(log, CALLBACK, r.deal([0, 0, 0, 1]))
    set_return(log, THREAD, r.deal([0, 0, 0, 0, 0, 0, 0, 1]))
    set_return(log, OVERLAY, r.deal([0, 0, 0, r.deal(ERRORS)]))
    strings(log, OVERLAY, 0, 0b101010)
    strings(log, STATUS, 0, 2)
    fill(log, STATUS, 1, struct.pack('<I', r.deal([0, 1, 4, 5, 2, 0xfffffffe])))
    log_fold(log)
    put(player, 0x3c, 'I', r.deal([0, 1, 1, 1]))
    put(player, 0x4c, 'I', r.deal([0, 1, 2, 2]))
    put(player, 0x37c, 'i', r.deal([0, 1, 23, 24, 25, 47, -1, -25]))
    for i in range(0x18):
        put(player, 0x1e8 + i * 0x10, 'I', r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
        put(player, 0x1f4 + i * 0x10, 'I', r.deal([0, 1, 2, 3, 4, 5, 6, 7]))
    put(player, 0x368, 'I', r.deal([0, 1]))
    put(player, 0x384, 'I', r.deal([0, 0, 1]))
    put(player, 0x388, 'I', r.deal([0, 0, 1]))
    put(player, 0x3e0, 'I', r.deal([0, 1]))
    put(player, 0x3e4, 'I', r.deal([0, 1]))
    return player, settings


def turns(r, player, settings):
    """The state's old values. (Past them 5, which ends the loop.) A case whose sound must wait: one turn."""
    waits = r.random() < 0.015   # (each its 302 recorder calls)
    put(settings, 0x98, 'i', r.deal([0, 1, 2, 3, 4, -1]))
    put(player, 0x3a4, 'i', 5)
    put(player, 0x394, 'i', r.deal([0, 0, 3, 9]))
    put(settings, 0x94, 'i', r.deal([1, 0x100, -4]) if waits else r.deal([-5, -8, -0x100]))
    values = [r.getrandbits(32)]
    if waits:
        values += [3]
    else:
        for _ in range(2):
            value = r.deal([3, 3, 3, 3, 4, 5, 0, 1, 7])
            values.append(value)
            if value == 4:   # (the wait's first exchange, then what ends it)
                values += [r.getrandbits(32), r.deal([3, 3, 0, 4, 5])]
    return struct.pack('<40I', *(values + [5] * (40 - len(values))))


def run_regions(r, log, player, settings):
    answers(log, EXCHANGE, TURNS)
    return {TURNS: turns(r, player, settings)}


def run_case(r, log, regions, player, settings):
    regions.update(run_regions(r, log, player, settings))
    return Call(0x66f090, [SETTINGS], regions, conv='t', ret='v')


def thread_case(r, log, regions, player, settings):
    regions.update(run_regions(r, log, player, settings))
    return Call(0x66f050, [SETTINGS], regions, ret='v')


def start_case(r, log, regions, player, settings):
    set_return(log, BEGIN, r.deal([0xffffffff, r.getrandbits(32), r.getrandbits(32)]))
    if struct.unpack_from('<I', settings, 4)[0] & 0x10000:
        answers(log, EXCHANGE, TURNS)
        before = [r.getrandbits(32) & ~3 for _ in range(r.deal([0, 1, 2, 3]))]
        regions[TURNS] = struct.pack('<40I', *(before + [3] * (40 - len(before))))
    else:
        regions.update(run_regions(r, log, player, settings))
    return Call(0x66efd0, [SETTINGS], regions, ret='a')


def destroy_case(r, log, regions, player, settings):
    for offset in (0x658, 0x65c, 0x660, 0x648, 0x64c, 0x654, 0x650, 0x1c, 0x20):
        put(player, offset, 'I', pointer(r))
    return Call(0x66eef0, [SETTINGS], regions, ret='a')


def surfaces_case(r, log, regions, player, settings):
    for offset in (0x658, 0x65c, 0x660, 0x648, 0x64c, 0x654, 0x650):
        put(player, offset, 'I', pointer(r))
    if struct.unpack_from('<I', player, 0x4c)[0] == 2:
        put(settings, 0x1c, 'I', obj(1))
    return Call(0x670710, [PLAYER], regions, conv='t', ret='v')


def made(r, log, regions, player, settings, address):
    regions[NAME] = struct.pack('<I', r.deal([0, 0, 0, 0, 1, 2])) + r.bytes(8)
    regions[ROOM] = b'\xcd' * 0x680
    set_return(log, CALLOC, r.deal([ROOM, ROOM, ROOM, ROOM, ROOM, 0]))
    answers(log, EXCHANGE, TURNS)
    regions[TURNS] = turns(r, player, settings)
    if struct.unpack_from('<I', settings, 4)[0] & 0x10000:   # (with a thread 0x66efd0 waits for a 3)
        regions[TURNS] = struct.pack('<40I', *([r.getrandbits(32), r.getrandbits(32) & ~3] + [3] * 38))
    return Call(address, [NAME, r.getrandbits(32), SETTINGS], regions, ret='a')


def create_case(r, log, regions, player, settings):
    return made(r, log, regions, player, settings, 0x66dff0)


def open_start_case(r, log, regions, player, settings):
    # (The player is the cleared one, so its loop runs on nothing, and its first dword the settings: the case's own PLAYER is not used.)
    return made(r, log, regions, player, settings, 0x66f4e0)


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x40)}
    player, settings = base(r, log, regions)
    imports = import_table(log, {'InterlockedExchange': (EXCHANGE, 0x6fd200, 2, 0, 0), 'InterlockedIncrement': (INCREMENT, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (DECREMENT, 0x6fd12c, 1, 0, 0),
                                 'Sleep': (SLEEP, 0x6fd1d4, 1, 0, 0), 'timeGetTime': (TIME, 0x6fd1d0, 0, 0, 0), 'GetCurrentThread': (CURRENT, 0x6fd254, 0, 0, 0), 'SetThreadPriority': (PRIORITY, 0x6fd1fc, 2, 0, 0),
                                 'mmioClose': (CLOSE, 0x6fd3f0, 2, 0, 0)})
    make = r.deal([run_case, run_case, run_case, run_case, thread_case, start_case, destroy_case, surfaces_case, create_case, open_start_case])
    call = make(r, log, regions, player, settings)
    call.regions.update({PLAYER: player, SETTINGS: settings, IMPORT_TABLE: imports})
    return call


if __name__ == '__main__':
    B.main(case)
