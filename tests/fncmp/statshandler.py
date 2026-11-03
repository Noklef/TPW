# The park statistics graphs' window handlers (src/game/gfx/statsgraph: 0x49a2e0 StatsGraph_Handler, 0x4a4b90 StatsGraph7cb318_Handler) against the originals: gauge.py's in_window()
# world with the gauge at the graph's address and statsgraph.py's lines (Max / Max2: the flags, the window kind); the saved rectangle +0x2c small, the default handler +0x118
# fncmp's logger. Messages: 0x15 (the set-up), 0x100 with the ids of the lines' boxes or others either side (b 1 or not; for the first -2: the dialog [0x7ca2e8] sent 4, its handler
# the logger), 0x101 with the three radio ids or others (b 1 or not), others. The lines' draws 0x49aa30 and 0x4a4750 cdecl recorder 22, 0x49ab80 (its buttons in dialogs' bench) cdecl
# recorder 23 (driver.cpp's kStatsGraphEnv).
# usage: statshandler.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=First,Second)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gauge  # noqa: E402
import statsgraph  # noqa: E402
from fncmp import CDECL_LOG4, RAM, Bench, Call, _recorder, put  # noqa: E402

B = Bench(gauge.merged(gauge.RANGES + [(0x49a2e0, 0x49a7a8), (0x4a4b90, 0x4a51d0), (0x4a51d0, 0x4a52b0), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00),
                                       (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x65d4f6, 0x65d520), (0x65f66e, 0x65f6e1), (0x658d9f, 0x658e04), (0x659710, 0x659721)]),
          hooks={0x67af00: gauge.crt.swprintf_hook, 0x49aa30: _recorder(22, 1, cdecl=True), 0x4a4750: _recorder(22, 1, cdecl=True), 0x49ab80: _recorder(23, 1, cdecl=True)})
DIALOG = RAM + 0x4800
# handler, the gauge, the lines of statsgraph.case, the messages
GRAPHS = {'First': (0x49a2e0, 0x7ca250, 'Max', [(0x15, 0)] * 4 + [(0x100, 0x12277 + i) for i in range(8)] * 2 + [(0x100, 0x12276), (0x100, 0x1227f), (0x100, 0xfffffffe)] * 2 +
                    [(0x101, 0x12274 + i) for i in range(3)] * 3 + [(0x101, 0x12273), (0x101, 0x12277), (0x14, 0x12277), (0x16, 0), (0x102, 0x12274)]),
          'Second': (0x4a4b90, 0x7cb318, 'Max2', [(0x15, 0)] * 4 + [(0x100, 0x4a68bc + i) for i in range(5)] * 2 + [(0x100, 0x4a68bb), (0x100, 0x4a68c1)] +
                     [(0x101, 0x4a68c2 + i) for i in range(3)] * 3 + [(0x101, 0x4a68c1), (0x101, 0x4a68c5), (0x14, 0x4a68bc), (0x16, 0), (0x102, 0x4a68c2)])}
OPS = [k for k in os.environ.get('OPS', ','.join(GRAPHS)).split(',') if k]
DECK = [(op, m) for op in OPS for m in GRAPHS[op][3]]


def case(r):
    op, (message, a) = r.deal(DECK)
    address, at, lines, _deck = GRAPHS[op]
    b = r.choice([1, 1, 1, 0, 2, r.getrandbits(32)])
    kind = r.choice([0, 1, 2] * 3 + [3, r.getrandbits(32)])
    kind_at, radio = {'First': (0x7ca2ec, 0x12274), 'Second': (0x7cb39c, 0x4a68c2)}[op]
    columns = None   # (what the handler leaves the gauge's columns: gauge.py's world, its label masks, is for them)
    if message == 0x15:
        columns = {1: 0x24, 2: 0x90}.get(kind, 0xc) if op == 'Second' else 0xc
    elif message == 0x101 and b == 1 and a - radio in (0, 1, 2):
        columns = (0x90, 0xc, 0x24)[a - radio]
    c = gauge.in_window(r)
    while columns is not None and struct.unpack_from('<i', c.regions[gauge.GRID], 0x28)[0] != columns:
        c = gauge.in_window(r)
    regions = dict(c.regions)
    regions[at] = regions.pop(gauge.GRID)
    regions.update(statsgraph.case(r, lines).regions)
    regions[kind_at] = struct.pack('<I', kind)
    window = bytearray(regions[gauge.WINDOW]) + bytearray(r.bytes(0x80))
    for field, top in ((4, 0x280), (6, 0x1e0), (8, 0x280), (0xa, 0x1e0)):  # (the saved rectangle's words)
        put(window, 0x2c + field, 'H', r.randrange(top))
    put(window, 0x118, 'I', CDECL_LOG4)
    regions[gauge.WINDOW] = window
    dialog = bytearray(r.bytes(0x120))
    dialog[4] = r.choice([0, 0, 0, 1])
    put(dialog, 0x114, 'I', CDECL_LOG4)
    regions.update({DIALOG: dialog, 0x7ca2e8: struct.pack('<I', r.choice([DIALOG] * 5 + [0])), 0xfaa5fc: struct.pack('<I', 0)})
    return Call(address, [gauge.WINDOW, message, a, b], regions, ret='a', masks=c.masks, ignore=[(gauge.guitext.STATE + 0x30, 4)])


if __name__ == '__main__':
    B.main(case)
