# The dialogs' fallback message handler (src/game/ui/dialogbase: 0x47ee90) against the original: focus.py's FocusRestore set-up (a tree of
# windows, the message queues, the focus globals), one of the tree's windows called with its parent (+0x60) another of them or none and its
# default handler (+0x118) fncmp's logger (its result now and then set); message 0x14 mostly, else others. Create (0x47ed80):
# windowscript.py's Build set-up (windowfactory.py's CreateChild set-up: window.py's tree, the heap, the timers, the focus; the registry
# [0xfaa610]), the GUI root [0xfaa5d8] WINDOW2 or the tree's (now and then none: a fault in both), the kinds (slot 3, the tree's too) not 7, a script
# generated for the new kind 1 frame, the handler 0 or fncmp's logger, shaded or not; the registry's hash now and then "lolight"'s
# (its window's slot 8 answering 0: an implementation would be told about the frame through a block on the stack); guidepth.py's globals.
# Window7cb2cc_Open / _OpenOnGui (src/game/ui/focus: 0x4a2790, 0x48a6e0): Create's set-up with the script at 0x752e00, the window
# [0x7cb2cc] none (mostly) or set, [0xfaa638] random.
# usage: dialogbase.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=BaseHandler,Create,Open,OpenOnGui)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import struct  # noqa: E402

import focus  # noqa: E402
import window  # noqa: E402
import windowscript  # noqa: E402
from fncmp import CDECL_LOG4, LOG, Bench, Call, put, set_return  # noqa: E402
from windowfactory import FACTORY  # noqa: E402

CREATE = [(0x47ed80, 0x47eea0), (0x6588b6, 0x6588ff), (0x6589f9, 0x658abb), (0x658ccc, 0x658daf), (0x659409, 0x65959a), (0x659bc9, 0x659c52),
          (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aa90), (0x65ab64, 0x65ace0), (0x65bc90, 0x65c035),
          (0x65c979, 0x65c9cd), (0x65d1c8, 0x65d333), (0x65d42e, 0x65d4b0), (0x65d773, 0x65d7cc), (0x65dd7b, 0x65e559), (0x65e59b, 0x65e60a),
          (0x65f16b, 0x65f256), (0x65fd0e, 0x6607da), (0x6616b0, 0x661710), (0x6617f0, 0x661820), (0x662562, 0x66275e), (0x665fcd, 0x6661cc),
          (0x6688b3, 0x6689cc), (0x66a1e8, 0x66a265), (0x66a4c0, 0x66a5a8), (0x66ac00, 0x66ae67), (0x66bff7, 0x66c01b), (0x66c7e0, 0x66c83e),
          (0x66c970, 0x66cab7), (0x66d0c0, 0x66d1b6), (0x66d750, 0x66d82e), (0x66d9e0, 0x66da31), (0x67fff0, 0x680060), (0x65fbf0, 0x66076e),
          (0x659730, 0x659760), (0x659e83, 0x65a1d3), (0x65a3e9, 0x65a4a3), (0x65d547, 0x65d56a), (0x65d56b, 0x65d59d), (0x665bfb, 0x665c45),
          (0x66c01a, 0x66c7dc), (0x659c42, 0x659cc0), (0x659d43, 0x659da0), (0x659dac, 0x659df5), (0x65bb10, 0x65bb4f), (0x65cfff, 0x65d333),
          (0x65d7d0, 0x65d832), (0x661830, 0x661880), (0x663554, 0x6636c2), (0x6651ef, 0x66522e), (0x66c054, 0x66c0a6), (0x66c1a6, 0x66c203),
          (0x66c4a1, 0x66c578), (0x66cad3, 0x66cb79), (0x66c760, 0x66c7dc), (0x66c568, 0x66c5a4), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0)]     + [(a, a + 0x10) for a in (0x65c2c8, 0x65ad33, 0x6628e4, 0x6662a7, 0x668aad, 0x66ae73, 0x66c84a, 0x66cac3, 0x66d1e8, 0x66d875, 0x66da3d,
                               0x66a5b4, 0x66a271)]
B = Bench(sorted(set(window.RANGES + [(0x47ee90, 0x47eed0), (0x65d55a, 0x65d57b), (0x4a2790, 0x4a27b3), (0x48a6e0, 0x48a6f4),
                                      (0x662411, 0x662420)] + FACTORY + CREATE)))
OPENS = {'Open': 0x4a2790, 'OpenOnGui': 0x48a6e0}


def name_hash(text):
    h = 0
    for ch in text.encode():
        h = ((ch - 256 if ch & 0x80 else ch) & 0xffffffff ^ h) * 0x2f & 0xffffffff
    return h


LOLIGHT = name_hash('lolight')


def create_case(r):
    c = windowscript.run_case(r, 'Build')
    regions, log = c.regions, c.regions[LOG]
    set_return(log, 3, r.choice([4, 4, 1, 2, 11]))
    for k in [k for k in regions if window.TREE <= k < window.RECTS]:   # (the tree's kinds too: Gui_TopDepth walks the root's children)
        regions[k] = put(bytearray(regions[k]), 0, 'I', window.VTABLE)
    root = r.choice([windowscript.WINDOW2] * 6 + [window.TREE] * 5 + [0])
    regions[0xfaa5d8] = struct.pack('<I', root) + regions[0xfaa5d8][4:]
    h = r.getrandbits(32)
    if windowscript.REG in regions:
        reg = bytearray(regions[windowscript.REG])
        if r.random() < 0.5:
            put(reg, 0x10, 'I', LOLIGHT)
        regions[windowscript.REG] = reg
        h = struct.unpack_from('<I', reg, 0x10)[0]
    g = windowscript.Gen(r, 1, h)
    g.window(1, 0)
    words = g.w + [5] * 8
    regions[windowscript.WORDS] = struct.pack('<%dH' % len(words), *words)
    regions[0xfaa570] = r.bytes(4)
    regions[0xfaa594] = struct.pack('<i', r.choice([0, -1, 5, 0x7fffffff, -0x80000000, r.getrandbits(31)]))
    regions[0xfaa5c8] = r.bytes(4)
    return Call(0x47ed80, [windowscript.WORDS, r.choice([0, CDECL_LOG4]), r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, ret='a')


def open_case(r, op):
    c = create_case(r)
    regions = c.regions
    regions[0x752e00] = regions.pop(windowscript.WORDS)
    regions[0x7cb2cc] = struct.pack('<I', r.choice([0] * 8 + [r.getrandbits(32) | 1]))
    regions[0xfaa638] = r.bytes(4)
    return Call(OPENS[op], [], regions, ret='a')


def case(r, ops=None):
    """one of ops (OPS when None: port.py's mutants call case(r)); BaseHandler without dealing for none ([])"""
    ops = OPS if ops is None else ops
    op = r.deal(ops) if ops else None
    if op == 'Create':
        return create_case(r)
    if op in OPENS:
        return open_case(r, op)
    c = focus.case(r, ['FocusRestore'])
    wins = sorted(k for k in c.regions if window.TREE <= k < window.RECTS)
    w = r.choice(wins)
    put(c.regions[w], 0x60, 'I', r.choice(wins + [0]))
    put(c.regions[w], 0x118, 'I', CDECL_LOG4)
    if r.random() < 0.5:
        set_return(c.regions[LOG], 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
    message = r.choice([0x14] * 6 + [0, 4, 0x15, 0x10014, r.getrandbits(32)])
    c.address, c.args, c.ret = 0x47ee90, [w, message, r.getrandbits(32), r.getrandbits(32)], 'a'
    return c


OPS = [k for k in os.environ.get('OPS', 'BaseHandler,Create,Open,OpenOnGui').split(',') if k]

if __name__ == '__main__':
    B.main(case)
