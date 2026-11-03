# Dialog7cb2b8_Build (src/game/ui/dialog7cb2b8build, 0x4a1d70) against the original. Setting_ReadMode is the port, crt_atexit fncmp._cdecl_log, GuiText_Measure 19 (its bounds hashed, box filled),
# BarMesh_Construct 20, Window_SetObject98 21, Window_SetHandler 22; the build and the windows as in dialogbuild.py (scripted()), [0x7cb2ac] the parent. The script: 0-2 plain children, the top bar 0x33 with its 0x37, 0x34, 0x36,
# the repair panel 0x2d (0x2e missing now and then), the money 0x2f (0x30, 0x31 each missing now and then, 0x32 always), the dialog 0x1d (0x21's chain, each level missing now and then, 0x24; 0x1e, 0x1f; 0x20; 0x25),
# each of 0x2d, 0x2f, 0x1d missing now and then, each list shuffled; the script failing now and then.
# usage: dialog7cb2b8build.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import listrows  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, _cdecl_log, _recorder, blob, fill, heap, recorder, set_return, strings, vtable  # noqa: E402

OP = 'Dialog7cb2b8_Build'
HANDLER = 0x65e526
MANAGER = 0x35100000
OURS = {0x4a0fb0, 0x4a11e0, 0x4a1190, 0x4a0db0, 0x4a0a20, 0x4a14f0, 0x4a0a90, 0x4a1cd0, 0x4a0840}
RECORD_HANDLER = _recorder(22, 1)


def set_handler(e):
    """the port's own handlers recorded; the window constructors' (their port calls the real one, not the env) as Window_SetHandler does: +0x114 unless none, AX 1"""
    handler = e.u32(e.r['ESP'])
    if handler in OURS:
        return RECORD_HANDLER(e)
    if handler:
        e.write(e.r['ECX'] + 0x114, struct.pack('<I', handler))
    e.r['EAX'] = (e.r['EAX'] & 0xffff0000) | 1
    e.r['ESP'] += 4


dialogbuild.DIALOGS[OP] = (0x7cb2b8, 0x752940, (0x10,), (0x7cb28c, 0x7cb2a0, 0x7cb290, 0x7cb2a4, 0x7cb2c4, 0x7cb2d0, 0x7cb2d4))
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [(0x4a1d70, 0x4a258f), (0x65d7bc, 0x65d840), (0x65cf17, 0x65cf50), (0x65d3a3, 0x65d41c), (0x41a850, 0x41a870), (0x4925a0, 0x4925cb), (0x576550, 0x5765d0)]),
          hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x4929f0: _recorder(19, 7, cdecl=True), 0x477870: _recorder(20, 1), 0x65d070: _recorder(21, 1), HANDLER: set_handler})


def maybe(r, chance=0.9):
    return r.random() < chance


def plain(r, w, ident, script=(5,), size=60):
    dialogbuild.child(r, w, ident, script, size, 1)


def slot(r, w):
    """the child 0x32, its rectangle now and then upside down (the centring's halves round to zero)"""
    plain(r, w, 0x32)
    if r.random() < 0.3:
        w[-2] = (w[-4] - r.randrange(1, 9)) & 0xffff


def shuffled(r, w, makers):
    r.shuffle(makers)
    for m in makers:
        m()
    w.append(5)


def script(r, mode, guard):
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, 0x1000 | r.getrandbits(16)]))
    bar = []   # (0x36 is needed unless the mode is 2, and the mode is read when the guard's bit is clear: always there then, or both fault on its absence)
    shuffled(r, bar, [lambda i=i: plain(r, bar, i) for i in (0x37, 0x34) + ((0x36,) if mode != 2 or not guard & 1 or maybe(r, 0.3) else ())])
    repair = []
    shuffled(r, repair, [lambda: plain(r, repair, 0x2e)] if maybe(r) else [])
    money = []
    shuffled(r, money, [lambda: slot(r, money)] + [lambda i=i: plain(r, money, i) for i in (0x30, 0x31) if maybe(r, 0.8)])
    end = []
    shuffled(r, end, [lambda: plain(r, end, 0x23)] if maybe(r) else [])
    mid = []
    shuffled(r, mid, [lambda: plain(r, mid, 0x22, end, 80)] + ([lambda: plain(r, mid, 0x24)] if maybe(r, 0.8) else []))
    graph = []
    shuffled(r, graph, [lambda: plain(r, graph, 0x1f)] if maybe(r) else [])
    dialog = []
    makers = [lambda: plain(r, dialog, 0x1e, graph, 80), lambda: plain(r, dialog, 0x20), lambda: plain(r, dialog, 0x25)]
    if maybe(r):
        makers.append(lambda: plain(r, dialog, 0x21, mid, 120))
    shuffled(r, dialog, makers)
    tops = [lambda: plain(r, w, 0x33, bar, 120)]
    if maybe(r):
        tops.append(lambda: plain(r, w, 0x2d, repair, 120))
    if maybe(r):
        tops.append(lambda: plain(r, w, 0x2f, money, 120))
    if maybe(r):
        tops.append(lambda: plain(r, w, 0x1d, dialog, 160))
    r.shuffle(tops)
    for m in tops:
        m()
    return w + [5] * 4


def case(r):
    regions = dialogbuild.scripted(r, OP)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[HEAP] = heap(0x100000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    for vt in dialogbuild.KIND_VTABLES:   # (the windows' slots 5 and 9)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    mode = r.choice([2] * 3 + [0, 1, 3, r.getrandbits(32)])
    guard = r.choice([1, 1, 1, 0, r.getrandbits(8)])
    w = script(r, mode, guard)
    regions[0x752940] = struct.pack('<%dH' % len(w), *w)
    regions[0x7cb2ac] = struct.pack('<I', window.WINDOW2)
    regions[0xfb34c1] = bytes([guard])
    regions[0xfb3b7c] = struct.pack('<I', mode)
    regions[listrows.REND][0x1c:0x20] = bytes(4)   # (the renderer's list of tag 0x30 empty: a part leaving it walks the list)
    regions[0xfaa5c0] = struct.pack('<h', r.choice([1536, 768, 0x600 + r.randrange(-3, 4)]))
    regions[0xfaa5c4] = struct.pack('<h', r.choice([2048, 1024, 0x800 + r.randrange(-3, 4)]))
    blob(log, 19, 1, 0b1111)
    strings(log, 19, 0, out=0b1000000)
    # The measured box. The money window's part takes that window's size for its pixels, its words unsigned: a negative height or width (0xffxx) runs only with
    # the other side about 0, or the pixels overrun the heap in both.
    shape = r.choice(['any'] * 5 + ['low', 'low', 'narrow'])
    if shape == 'low':      # (a negative height, odd or even: the centring's halves)
        width, tall = r.choice([0, 0, 0, 1]), -r.randrange(17, 60)
    elif shape == 'narrow':   # (a negative width, the height 0)
        width, tall = -r.randrange(1, 20), -0x10
        regions[0xfaa5c0] = struct.pack('<h', 1536)
    else:
        width, tall = r.randrange(0, 120), r.randrange(0, 40)
    fill(log, 19, 6, struct.pack('<4i', r.getrandbits(8), r.getrandbits(8), width, tall))
    set_return(log, 20, r.getrandbits(32))
    regions[0x6ff2d8 + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[0x6ff2d8 + 0x14] = struct.pack('<I', recorder(31, 1))
    return Call(0x4a1d70, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
