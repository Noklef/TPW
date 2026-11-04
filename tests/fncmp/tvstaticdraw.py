# The screen of static drawn (src/game/gfx/tvstatic: 0x53e760) against the original: the screen's interface a COM recorder wrapped in 0x874fd8, the static's Surface at 0x874fd0 its own recorder;
# the drivers' Pixel a recorder (its calls past the log's room folded into a hash; the colour's index logged as its byte, the rest is the stack's), the seed random. Every case runs the
# whole 320 x 240 loop and no path shortens it. So the emulator's Pixel (display.py compares it) is a hook logging what the driver's recorder logs, a run is capped at CAP cases and --both is
# dropped: the path is one, what varies is the seed, the palette, the formats and the answers.
# usage: tvstaticdraw.py [CASES [SEED]] [--wild] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import surface  # noqa: E402
from fncmp import LOG, LOG_BYTES, LOG_ENTRIES, LOG_ENTRY, RAM, RAND_SEED, Bench, Call, _fold, blob, fill, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

SRC, SCREEN, COM_S, VT_D, VT_S, TABLE = RAM, RAM + 0x60, RAM + 0x70, RAM + 0x100, RAM + 0x200, RAM + 0x300
DST, SRC_HOLDER, STATE = 0x874fd8, 0x874fd0, 0xfa76b8
BLT, DESC, ISLOST, LOCK, RESTORE, UNLOCK = 5, 22, 24, 25, 27, 32
CAP = 10


def pixel(e):
    """Display_Pixel: slot 0 of the driver at the context's +0x18 logged (k 0, four arguments, the driver as this), the index as its byte."""
    sp = e.r['ESP'] + (4 if e.tail else 0)
    words = (0, 4, e.u32(STATE + 0x18), e.u32(sp), e.u32(sp + 4), e.u32(sp + 8), e.u32(sp + 12) & 0xff, 0, 0, 0, 0)
    count = e.u32(LOG)
    if count < LOG_ENTRIES:
        e.write(LOG + 4 + 4 * LOG_ENTRY * count, struct.pack('<%dI' % LOG_ENTRY, *words, 0))
        e.write(LOG, struct.pack('<I', count + 1))
    else:
        _fold(e, words)


B = Bench([(0x53e760, 0x53e911), (0x5f73a0, 0x5f76b7), (0x5f8f90, 0x5f9080), (0x5f92e0, 0x5f9630), (0x5f9ac0, 0x5f9cb0), (0x5f9d90, 0x5f9eb0),
           (0x61af20, 0x61af50), (0x61af60, 0x61b040), (0x61b090, 0x61b1f0), (0x5da3c0, 0x5da3c1)], hooks={0x5f97c0: pixel})


def case(r):
    log = log_region()
    log_fold(log)
    struct.pack_into('<I', log, LOG_BYTES, 1 << 3)
    for k in (DESC, DESC + 32):
        strings(log, k, 0, 1 << 1)
    strings(log, LOCK + 32, 0, 1 << 2)
    fill(log, DESC, 1, surface.desc(r), 0)
    fill(log, DESC + 32, 1, surface.desc(r), 1)
    fill(log, LOCK + 32, 2, surface.desc(r), 2)
    set_return(log, LOCK + 32, r.choice([0, 0, surface.LOST, 1]))
    set_return(log, UNLOCK + 32 & 63, r.choice([0, surface.LOST]))
    set_return(log, BLT, r.choice([0, 0, surface.LOST, 1]))
    for k in (ISLOST, ISLOST + 32):
        set_return(log, k, r.choice([0, surface.LOST]))
    for k in (RESTORE, RESTORE + 32):
        set_return(log, k, r.choice([0, 1]))
    hashed = r.choice([1, 3, 5])   # (Blt's rectangles and DDBLTFX on the stack: one hashed, the others 0)
    blob(log, BLT, hashed, 0xf if hashed < 5 else (1 << 25) - 1)
    strings(log, BLT, 0, sum(1 << i for i in (1, 3, 5) if i != hashed))
    state = bytearray(r.bytes(0x74))
    put(state, 8, '4i', 0, 0, 320, 240)
    put(state, 0x18, 'I', STATE + 0x40)
    for at in (0x38, 0x40, 0x44, 0x48):
        put(state, at, 'I', TABLE)
    regions = {LOG: log, RAND_SEED: r.bytes(4), DST: r.bytes(0x50), SRC_HOLDER: struct.pack('<I', SRC), SRC: surface.surface(r, COM_S), SCREEN: struct.pack('<I', VT_D), COM_S: struct.pack('<I', VT_S),
               VT_D: surface.vt(0), VT_S: surface.vt(32), TABLE: vtable({0: 4}, 16), STATE: state, 0xfa71f8: r.bytes(0x400), 0xfa76b0: struct.pack('<II', r.getrandbits(32), r.choice([0, 0x100])),
               0x7a1968: r.bytes(4), 0x7a1930: r.bytes(4)}
    return Call(0x53e760, [SCREEN], regions, steps=4_000_000)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        sys.argv[1] = str(min(int(sys.argv[1]), CAP))
    sys.argv = [a for a in sys.argv if a != '--both']
    B.main(case)
