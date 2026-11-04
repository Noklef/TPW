# The movie's chunks read (src/game/gfx/moviechunks: 0x66e410, 0x66e840, 0x66e920, 0x66e9a0, 0x66ed60) against the originals. The mmio calls and InterlockedIncrement import recorders (the read's bytes through its recorder, its answers in turn);
# the decoders recorders as driver.cpp's kMovieChunksEnv (the data, the position and the out, on the stack, logged by contents: one of them a case). MovieTags_ReadNumber the original. A chunk read through the dispatcher has the sound kind 10: the
# buffer's rest (not filled) is not looked at then. The kind's other values (0x66ecbc reads a stack dword nothing sets) are not dealt.
# usage: moviechunks.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, import_table, log_region, put, set_return, strings  # noqa: E402

MONO, STEREO, IMA, READ, SEEK, INCREMENT = 10, 11, 12, 13, 14, 15
B = Bench([(0x66e410, 0x66e838), (0x66e840, 0x66e91e), (0x66e920, 0x66e993), (0x66e9a0, 0x66ed52), (0x66ed60, 0x66ee62), (0x66ee80, 0x66eef0), (0x67b110, 0x67b13f)],
          hooks={0x672090: _recorder(MONO, 6, cdecl=True), 0x672210: _recorder(STEREO, 8, cdecl=True), 0x672cb3: _recorder(IMA, 5, cdecl=True)})
PLAYER, SETTINGS, TURNS, FRAMES, SOUND = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x100000, RAM + 0x400000
FRAME_END, SOUND_END = FRAMES + 0x124f80, SOUND + 0x493e0
TAGS = [b'SEAD', b'SNDC', b'CDNS', b'SCDl', b'1SNd', b'SEND', b'1SNe', b'SCEl', b'eDAM', b'MADe', b'kDAM', b'MADk', b'mDAM', b'MADm', b'lCCS', b'SCCl', b'lLCS', b'SCLl', b'lHCS', b'SCHl', b'1SNh', b'lDCS',
        b'fVGT', b'TGVf', b'pQGT', b'MUVf', b'pIQT', b'UV2f', b'kVGT', b'TGVk', b'XXXX', b'SCDm', b'\0\0\0\0']
SCHEDULE = 0x7f000000


def player_regions(r):
    player = bytearray(r.bytes(0x500))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x1c, 'I', FRAMES)
    put(player, 0x20, 'I', SOUND)
    put(player, 0x24, 'I', r.deal([FRAMES, FRAMES + 0x40, FRAME_END - 0x40, FRAME_END - 0x20, FRAME_END - 0x21]))
    put(player, 0x28, 'I', r.deal([SOUND, SOUND + 0x40, SOUND_END - 0x400, SOUND_END - 0x20, SOUND_END - 0x21]))
    for offset in (0x378, 0x380):
        put(player, offset, 'i', r.deal([0, 1, 23, 24, 25, 47, 1000, -1, -2]))
    put(player, 0x390, 'I', r.deal([0, 0, 1, r.getrandbits(32)]))
    put(player, 0x3b4, 'I', r.deal([0, 1, 2]))
    settings = bytearray(r.bytes(0x100))
    put(settings, 4, 'I', r.getrandbits(32) & ~0x8000 | r.deal([0, 0x8000]))
    put(settings, 0x7c, 'I', r.deal([0, 1, 2, 3]))
    put(settings, 0x84, 'I', r.deal([0, 1, 1, 1]))
    put(settings, 0x88, 'I', r.deal([0, 1]))
    put(settings, 0xa4, 'I', r.getrandbits(32))
    return player, settings


def read_answers(log, *first):
    table = bytearray(0xc0)
    put(table, 0, '%dI' % len(first), *first)
    answers(log, READ, TURNS)
    return table


def dispatch(r, log, player):
    put(player, 0x64, 'I', 10)
    tag = r.deal(TAGS)
    length = r.deal([8, 9, 0x10, 0x18, 0x28, 0x100, 0, 0x7fffffff]) & 0xffffffff
    fill(log, READ, 1, tag + struct.pack('<I', length) + r.bytes(4) + b'\xfd\x8a' + r.bytes(2), records=2)   # (the second read's bytes an empty sound header)
    strings(log, IMA, 0, 1)
    table = read_answers(log, r.deal([8, 8, 8, 8, 8, 8, 8, 8, 0, 0xffffffff, 7, 9]), r.deal([8, 8, 8, 8, 0, 0xffffffff, 0x20]))
    return Call(0x66e410, [PLAYER], {TURNS: table}, conv='t', ret='a')


def sound(r, log, player):
    size = r.deal([0, 0xc, 0x10, 0x1e, 0x1f, 0x2a, 0x48, 0x4e, 0x100, 0x1000, 0x12345])
    kind = r.deal([0, 0, 7, 7, 7, 10])
    channels = r.deal([1, 2])
    put(player, 0x64, 'I', kind)
    put(player, 0x58, 'I', channels)
    words = [r.deal([0, 1, -1, 0x7fff, -0x8000, r.getrandbits(16) - 0x8000]) for _ in range(4)]
    if kind == 0:
        data = struct.pack('<I', r.deal([0, 1, 2, 7, 0xb, 0xffffffff, 0x80000001])) + r.bytes(0x68)
    else:
        data = r.bytes(4) + struct.pack('<4h', *words) + r.bytes(0x60)
    fill(log, READ, 1, data)
    for k, position, data_arg, out in ((MONO, 1, 4, 5), (STEREO, 1, 6, 7), (IMA, None, 0, None)):
        pointers = [a for a in (position, data_arg, out) if a is not None]
        arg = r.deal(pointers)
        blob(log, k, arg, 0xff if arg == data_arg else 1)
        strings(log, k, 0, sum(1 << a for a in pointers if a != arg))
    table = read_answers(log, r.deal([0x48, 0x48, 0x48, 0x48, 0x48, 0, 0xffffffff]))
    set_return(log, SEEK, r.getrandbits(32))
    return Call(0x66e9a0, [PLAYER, size], {TURNS: table}, conv='f', ret='a')


def schedule(r, log):
    # (MovieTags_ReadNumber: a count byte, that many big-endian bytes.) Four bytes not read, the tags' list up to 0xfd, then the fields up to 0x8a.
    parts = bytearray(r.bytes(4))
    for _ in range(r.deal([0, 0, 1, 2])):
        parts += bytes([r.deal([0, 3, 0x80, 0xfc]), 1, r.getrandbits(8)])
    parts.append(0xfd)
    for _ in range(r.deal([0, 1, 2, 4, 5])):
        n = r.deal([1, 1, 2, 4])
        parts += bytes([r.deal([0x81, 0x82, 0x83, 0x84, 0x85, 0x80, 0x89, 0x8b, 0]), n]) + r.bytes(n)
    parts.append(0x8a)
    fill(log, READ, 1, bytes(parts))
    return read_answers(log, r.deal([0x48, 0x48, 0x48, 0x48, 0, 0xffffffff]))


def case(r):
    log = log_region()
    player, settings = player_regions(r)
    imports = import_table(log, {'mmioRead': (READ, 0x6fd3e0, 3, 0, 0b10), 'mmioSeek': (SEEK, 0x6fd3d8, 3, 0, 0), 'InterlockedIncrement': (INCREMENT, 0x6fd0c0, 1, 0, 0)})
    op = r.deal(['Dispatch', 'Dispatch', 'Dispatch', 'Video', 'Start', 'Sound', 'Sound', 'Schedule'])
    if op == 'Dispatch':
        call = dispatch(r, log, player)
    elif op == 'Video':
        fill(log, READ, 1, r.bytes(0x10))
        table = read_answers(log, r.deal([0x10, 0x10, 0x10, 0x10, 0, 0xffffffff]))
        strings(log, READ, 0, 0)
        call = Call(0x66e840, [PLAYER, r.deal([0x10, 0x20, 0x21, 0x40, 0x80000000]), r.deal([1, 2, 7])], {TURNS: table}, conv='f', ret='a')
    elif op == 'Start':
        fill(log, READ, 1, r.bytes(0x10))
        table = read_answers(log, r.deal([0x10, 0x10, 0x10, 0, 0xffffffff]))
        call = Call(0x66e920, [PLAYER, r.deal([0x10, 0x100, 0x1000])], {TURNS: table}, conv='f', ret='a')
    elif op == 'Sound':
        call = sound(r, log, player)
    else:
        call = Call(0x66ed60, [PLAYER, r.deal([0x10, 0x100, 0x1000])], {TURNS: schedule(r, log)}, conv='f', ret='a')
    call.regions.update({LOG: log, PLAYER: player, SETTINGS: settings, IMPORT_TABLE: imports, FRAMES: r.bytes(0x80), FRAME_END - 0x40: r.bytes(0x80), SOUND: bytes(0x400), SOUND_END - 0x400: bytes(0x800)})
    return call


if __name__ == '__main__':
    B.main(case)
