# The movie's frame shown (src/game/gfx/moviesurface: MovieSurface_Present, 0x66f600) against the original: the IDirectDraw and the surfaces over vtables of COM recorders, the stack structures logged by their contents (one rectangle of each Blt:
# a case), the Flip's busy answers in turn, the lock's pitch and buffer through its recorder, the frame's dwords compared in the buffer. MovieSurface_UpdateOverlay and DdErrors_Name the originals; OutputDebugStringA a recorder.
# usage: moviesurfacepresent.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, answers, blob, fill, import_table, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench([(0x66f600, 0x66fa6d), (0x66fa70, 0x66fb57), (0x672e30, 0x672e54)])
DRAW, OFFER, PRIMARY, BACK, PALETTE_OBJECT, VTABLES, PLAYER, SETTINGS, PALETTE, FRAME, BUFFER, TURNS = (RAM, RAM + 0x10, RAM + 0x20, RAM + 0x30, RAM + 0x40, RAM + 0x100, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000, RAM + 0x7000)
BLANK, LOST, RESTORE, FLIP, LOCK, UNLOCK, OVERLAY, PBLT, PLOST, PRESTORE, PFLIP, BBLT, BLOST, BRESTORE, ENTRIES, DEBUG = range(1, 17)
LOST_ERROR, BUSY, DRAWING = 0x887601c2, 0x887601ae, 0x8876021c


def vtable(*slots):
    table = bytearray(0x88)
    for offset, stub in slots:
        struct.pack_into('<I', table, offset, stub)
    return table


def case(r):
    log = log_region()
    flags = r.getrandbits(32) & ~(0x800 | 0x400 | 0x1000 | 0x8000000 | 0x30)
    flags |= r.deal([0, 0, 0, 0, 0x800]) | r.deal([0, 0, 0, 0x400]) | r.deal([0, 0x1000]) | r.deal([0, 0x8000000]) | r.deal([0, 0, 0x10, 0x20, 0x30])
    settings = bytearray(r.bytes(0x7b8))
    put(settings, 4, 'I', flags)
    put(settings, 0x10, '5I', DRAW, PRIMARY, BACK, OFFER, 0)
    put(settings, 0x24, '2I', PALETTE_OBJECT, PALETTE)
    put(settings, 0x48, 'I', r.deal([8, 8, 16, 0]))
    put(settings, 0x60, '2I', r.deal([0, 1, 2, 3, 4, 5]), r.deal([0, 1, 2, 3, 4, 16, 17, 32, 64, 80, 0x80000000, 0x80000010]))
    player = bytearray(r.bytes(0x650))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x4c, '4I', r.deal([1, 1, 1, 2, 2, 2, 4, 4, 0, 3]), r.deal([0, 0, 1]), r.deal([0, 1]), 0)
    put(player, 0x648, 'I', FRAME)
    if r.deal([1, 1, 1, 1, 0]) == 0:
        put(settings, 0x1c, 'I', 0)
    table = bytearray(0xc0)
    turns = [r.deal([0, BUSY, DRAWING, BUSY]) for _ in range(3)] + [0 if r.deal([1, 1, 0]) else 1]
    put(table, 0, '4I', *turns)
    answers(log, FLIP, TURNS)
    for k in (BLANK, UNLOCK, PBLT, BBLT, ENTRIES, OVERLAY):
        set_return(log, k, r.deal([0, 0, 0, 1, 0x80004005]))
    for k in (LOST, PLOST, BLOST):
        set_return(log, k, r.deal([0, 0, LOST_ERROR, 1, 0x887601c3]))
    for k in (RESTORE, PRESTORE, BRESTORE):
        set_return(log, k, r.deal([0, 0, 1]))
    set_return(log, LOCK, r.deal([0, 0, 0, 1]))
    set_return(log, PFLIP, r.deal([0, 0, 0x887601c2, 0x88760168, 1]))
    arg, other = r.deal([(1, 3), (3, 1)])
    for k in (PBLT, BBLT):
        blob(log, k, arg, 0xf)
        strings(log, k, 0, 1 << other)
    strings(log, OVERLAY, 0, 1 << 1 | 1 << 3 | 1 << 5)
    blob(log, LOCK, 2, (1 << 27) - 1)
    fill(log, LOCK, 2, struct.pack('<i4II', r.deal([0, 40, 64, 160, 161, 320, -160, -161]), 0, 0, 0, 0, BUFFER + 0x800), offset=0x10)
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, PALETTE: r.bytes(0x400), FRAME: r.bytes(0x400), BUFFER: r.bytes(0x1000), TURNS: table,
               DRAW: struct.pack('<I', VTABLES), OFFER: struct.pack('<I', VTABLES + 0x100), PRIMARY: struct.pack('<I', VTABLES + 0x200), BACK: struct.pack('<I', VTABLES + 0x300), PALETTE_OBJECT: struct.pack('<I', VTABLES + 0x400),
               VTABLES: vtable((0x58, recorder(BLANK, 3, True))),
               VTABLES + 0x100: vtable((0x60, recorder(LOST, 1, True)), (0x6c, recorder(RESTORE, 1, True)), (0x2c, recorder(FLIP, 3, True)), (0x64, recorder(LOCK, 5, True)), (0x80, recorder(UNLOCK, 2, True)), (0x84, recorder(OVERLAY, 6, True))),
               VTABLES + 0x200: vtable((0x14, recorder(PBLT, 6, True)), (0x60, recorder(PLOST, 1, True)), (0x6c, recorder(PRESTORE, 1, True)), (0x2c, recorder(PFLIP, 3, True))),
               VTABLES + 0x300: vtable((0x14, recorder(BBLT, 6, True)), (0x60, recorder(BLOST, 1, True)), (0x6c, recorder(BRESTORE, 1, True))),
               VTABLES + 0x400: vtable((0x18, recorder(ENTRIES, 5, True)))}
    regions[IMPORT_TABLE] = import_table(log, {'OutputDebugStringA': (DEBUG, 0x6fd1e0, 1, 1, 0)})
    return Call(0x66f600, [PLAYER], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
