# The text window of [0x7cc654] fitted to its text (src/game/ui/dialog7cc654: Dialog7cc654_FitText, 0x4bc100) against the original.
# guitext.py's world (the measure: the buffer's text, font 6's entry, the fonts' line heights) with the screen's scale [0xfaa5c4] / [0xfaa5c0]
# usual or any (not 0); the window [0x7cc654] with 0-3 decoy children around the text window (child 0xee2), it with 0-3 decoys around the
# bar (0xee3). The text window's saved rectangle (+0x2c) about a line high (or any), its slot 9 a recorder; the bar's Window_SetShown
# surroundings (window.py's shown_env, cut down: shown or not, the text window and the window its parents, no children or objects, its
# handler +0x114 a logger or none, the hover's root [0xfaa5d8] none, the manager [0x77c48c]'s slot 13 a recorder), its range (+0x138-+0x144)
# small, its slot 7 a recorder answering 0 in AX or not.
# usage: dialog7cc654.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fit)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import guitext  # noqa: E402
import window  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench(dialog7cc5f4.merged(guitext.RANGES + window.RANGES + [(0x4bc100, 0x4bc26d), (0x65d4f6, 0x65d521), (0x66b47d, 0x66b508)]))
OPS = [k for k in os.environ.get('OPS', 'Fit').split(',') if k]
DIALOG, TEXTWIN, BAR, DECOYS, VT, MANAGER = RAM + 0x10000, RAM + 0x10200, RAM + 0x10400, RAM + 0x10600, RAM + 0x10e00, RAM + 0x10f00
K_SLOT7, K_SLOT9 = 24, 25   # (the recorders the bar's slot 7 and the text window's slot 9 log as: clear of the fonts' and drivers')


def children(r, regions, target, first, at):
    """0-3 decoys (ids other than the target's), the target among them; returns the head"""
    n = r.choice([0, 0, 1, 2, 3])
    pos = r.randrange(n + 1)
    order = [None] * pos + [target] + [None] * (n - pos)
    nodes = []
    for i, w in enumerate(order):
        if w is None:
            w = at + window.NODE * i
            regions[w] = window.node(r, r.choice([0, 1, 0xee4, 0xee1, 0x10ee2, 0x10ee3, r.getrandbits(32)]), 0, r.choice([0, 0x7fff0000]))
        nodes.append(w)
    for w, nxt in zip(nodes, nodes[1:] + [0]):
        put(regions[w], 0x54, 'I', nxt)
    return nodes[0]


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x400)}
    lang, table, codes, lead = guitext.world(r, log, regions)
    entry_font = struct.unpack_from('<I', table, 24 * (6 + 13 * lang) + 0x14)[0]
    n = guitext.buffer_text(r, regions, lead)
    regions[0x7cb4d0] = r.bytes(0x10)   # (the wide text: narrowed with no table, so the buffer's bytes are what is measured)
    gw = r.choice([2048, 640, 800, 1024, r.randrange(1, 0x8000), r.randrange(-0x8000, 0)])
    gh = r.choice([1536, 480, 600, 768, r.randrange(1, 0x8000), r.randrange(-0x8000, -1)])
    regions[0xfaa5c0] = struct.pack('<hhhh', gh, r.getrandbits(15), gw, r.getrandbits(15))
    regions[0xfaa540] = r.bytes(8)
    regions[0xfaa5d8] = struct.pack('<5I', 0, r.getrandbits(32), r.choice([0, BAR]), r.getrandbits(32), r.getrandbits(32))
    # the windows: the dialog, the text window, the bar
    text = bytearray(r.bytes(0x120))
    put(text, 0, 'I', VT)
    left, top = r.randrange(-0x40, 0x300), r.randrange(-0x40, 0x300)
    height = r.choice([r.randrange(-4, 0x30)] * 4 + [r.randrange(-0x8000, 0x8000)])
    width = r.choice([r.randrange(0, 0x200)] * 4 + [r.randrange(-0x8000, 0x8000)])
    put(text, 0x2c, 'I4h', 0x705920, left, top, (left + width + 0x8000) % 0x10000 - 0x8000, (top + height + 0x8000) % 0x10000 - 0x8000)
    put(text, 0x60, 'I', r.choice([0, DIALOG]))
    put(text, 0x8c, 'I', 0xee2)
    bar = bytearray(r.bytes(0x150))
    put(bar, 0, 'I', VT + 0x80)
    put(bar, 0x48, 'I', struct.unpack_from('<I', bar, 0x48)[0] & ~1 | r.choice([0, 1]))
    put(bar, 0x5c, 'I', 0)
    put(bar, 0x60, 'I', TEXTWIN)
    put(bar, 0x8c, 'I', 0xee3)
    put(bar, 0x98, 'II', 0, 0)
    put(bar, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
    small = [0, 0, 1, 2, 5, 0x10, -1, r.randrange(-0x40, 0x40), r.getrandbits(32) - (1 << 31)]
    put(bar, 0x138, '4i', *[r.choice(small) for _ in range(4)])
    regions.update({TEXTWIN: text, BAR: bar})
    put(regions[TEXTWIN], 0x5c, 'I', children(r, regions, BAR, DECOYS, DECOYS))
    dialog = bytearray(r.bytes(0x120))
    put(dialog, 0x5c, 'I', children(r, regions, TEXTWIN, DECOYS + 0x400, DECOYS + 0x400))
    put(dialog, 0x60, 'I', 0)
    regions[DIALOG] = dialog
    regions[0x7cc654] = struct.pack('<I', DIALOG)
    vt = vtable({})
    put(vt, 9 * 4, 'I', recorder(K_SLOT9, 1))
    vt2 = vtable({})
    put(vt2, 7 * 4, 'I', recorder(K_SLOT7, 1))
    regions[VT] = vt + vt2   # (32 slots each: the bar's at VT + 0x80)
    if r.random() < 0.7:
        set_return(log, K_SLOT7, r.choice([0, 0x10000, 1, 0x8000, r.getrandbits(32)]))
    regions[0x77c48c] = struct.pack('<I', MANAGER)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40) + bytes(0x3c) + vtable({13: 1})
    if r.random() < 0.5:
        set_return(log, 13, r.choice([0, 1, -1, r.getrandbits(32)]))
    return Call(0x4bc100, [], regions, ret='v', masks=guitext.heap_masks(entry_font and codes and n == 1))


if __name__ == '__main__':
    B.main(case)
