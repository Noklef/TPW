# A ride base's cells and frames on its grid (src/game/model/ridebase: 0x46a5a0) against the original, with Flags_AnyLow4 and
# Matrix_Identity; Frame_FitToGrid, RideBase_FillCells and GlobalMem_Alloc are recorders. Grids of 0-20 by 0-20 cells, sparse; the owner, its
# +0x6c and the model's bits dealt; 0-5 frames, each with bit 0x40 or not, one now and then the best the first fit gives; each fit writes
# hits (up to 12, or -1) through its out, the buffer through its cursor's frame layout and a best frame. Stack pointers are not logged.
# usage: ridebase.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, strings  # noqa: E402

FIT = _recorder(1, 13, cdecl=True)


def fit_hook(e):
    """The fit's 28 stack dwords, past a recorder's 15: logged as 13, the matrix by address (driver.cpp's ride_base_fit does the same)."""
    extra = 4 if e.tail else 0
    sp = e.r['ESP'] + extra
    args = [e.u32(sp + 4 * i) for i in range(28)]
    keep = e.r['ESP']
    e.r['ESP'] = keep - 0x100
    e.write(e.r['ESP'] + extra, struct.pack('<13I', args[0], args[1], args[2], sp + 12, *args[19:28]))
    FIT(e)
    e.r['ESP'] = keep


B = Bench([(0x46a5a0, 0x46aa10), (0x469930, 0x469940), (0x46f150, 0x46f178), (0x5da3c0, 0x5da3c1)],
          hooks={0x469a80: fit_hook, 0x46aa10: _recorder(2, 5, cdecl=True), 0x45b9d0: _recorder(3, 2, cdecl=True)})
OBJECT, HOLDER, MODEL, OWNER, NODE, GRID, TABLE, BLOCKS, FRAMES, CELLS = (RAM + k for k in (0, 0x40, 0x80, 0x100, 0x180, 0x1c0, 0x200, 0x400, 0x800, 0x1000))
FRAME, BLOCK = 0xa0, 0x40


def case(r):
    log = log_region()
    frames = r.deal([0, 1, 2, 3, 4, 5])
    area = bytearray(r.bytes(6 * FRAME))
    for k in range(6):
        put(area, k * FRAME, 'I', r.getrandbits(32) & ~0x40 | r.deal([0x40, 0x40, 0]))
    anyflag = any(area[k * FRAME] & 0x40 for k in range(frames))
    has_owner = r.deal(['full'] * 6 + ['empty', 'none'])
    if has_owner == 'none' and anyflag:
        has_owner = 'empty'

    model = bytearray(r.bytes(0x80))
    put(model, 0x30, 'I', r.getrandbits(32) & ~0x188 | r.deal([0, 0x80, 0, 0]) | r.deal([0, 0x100]) | r.deal([8, 8, 0]))
    put(model, 0x44, 'H', frames)
    put(model, 0x70, 'I', FRAMES)
    put(model, 0x78, 'I', NODE)

    holder = bytearray(r.bytes(0x40))
    put(holder, 4, 'I', MODEL)
    put(holder, 0x2c, 'I', r.deal([0, FRAMES, FRAMES + FRAME, r.getrandbits(32) | 1]))
    owner = bytearray(r.bytes(0x80))
    put(owner, 0x6c, 'I', 0 if has_owner == 'empty' else GRID)

    width, height = r.deal([0, 1, 2, 7, 13, 20, 20]), r.deal([0, 1, 2, 7, 13, 20, 20])
    if r.random() < 0.05:
        width = -1
    density = r.choice([0, 0.1, 0.3, 0.7, 1])
    cells = bytearray(22 * 0xa0)
    for k in range(0, len(cells), 8):
        put(cells, k, 'I', r.getrandbits(32) & ~0xf | (r.randrange(1, 16) if r.random() < density else 0))
        put(cells, k + 4, 'I', r.getrandbits(32))

    hits = [r.deal([-1, 0, 1, 3, 7, 12, 12]) for _ in range(4)]
    best = [r.deal([0, FRAMES, FRAMES + FRAME, FRAMES + 2 * FRAME]), 0, FRAMES + FRAME, 0]
    fill(log, 1, 6, struct.pack('<4i', *hits), records=4)
    fill(log, 1, 8, struct.pack('<4I', *best), which=1, records=4)
    fill(log, 1, 2, r.bytes(4 * 0x18), which=2, offset=0x60, records=4)
    strings(log, 1, 0, out=(1 << 2) | (1 << 6) | (1 << 8) | (1 << 9))
    blob(log, 1, 3, 0xffff)
    answers(log, 3, TABLE)
    table = [BLOCKS + BLOCK * k for k in range(8)] * 5
    if r.random() < 0.15:
        table[0] = 0

    regions = {LOG: log, OBJECT: struct.pack('<III', r.getrandbits(32), r.getrandbits(32), HOLDER), HOLDER: holder, MODEL: model, NODE: r.bytes(0x20), GRID: r.bytes(0x20),
               TABLE: struct.pack('<40I', *table), BLOCKS: r.bytes(8 * BLOCK), FRAMES: area, CELLS: cells, 0x7afe18: r.bytes(0x64c)}
    if has_owner != 'none':
        regions[OWNER] = owner
    with_cells = r.deal([1, 1, 1, 1, 0])
    return Call(0x46a5a0, [OBJECT, OWNER if has_owner != 'none' else 0, CELLS if with_cells else 0, width, height], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
