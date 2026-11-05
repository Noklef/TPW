# The IME's window messages (src/game/core/imeinput: Ime_PickLayout 0x4796b0, Ime_OnMessage 0x4798e0, Ime_WindowProc 0x47a880) against the originals: everything of the IME's real, the imports and the old procedure recorders. The lists are 32 where a message brings them to 32, so no blank's unset dwords show (imecandlist.py sees those).
# usage: imemessages.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, heap, import_table, log_region, put, recorder, set_return  # noqa: E402
import imeattach  # noqa: E402

ATEXIT, HANDLER = 1, 2
LAYOUTS, PROPERTY, DEFPROC, WIDEN, PEEK, TRANSLATE, DISPATCH, LEAD, GETCTX, RELEASE, LIST, OPEN, FORM, NOTIFY = range(40, 54)
IMPORTS = {'GetKeyboardLayoutList': (LAYOUTS, 0x6fd310, 2, 0, 0), 'ImmGetProperty': (PROPERTY, 0x6fd08c, 2, 0, 0), 'DefWindowProcA': (DEFPROC, 0x6fd340, 4, 0, 0), 'MultiByteToWideChar': (WIDEN, 0x6fd174, 6, 1 << 2, 1 << 4), 'PeekMessageA': (PEEK, 0x6fd354, 5, 0, 1),
           'TranslateMessage': (TRANSLATE, 0x6fd358, 1, 0, 0), 'DispatchMessageA': (DISPATCH, 0x6fd35c, 1, 0, 0), 'IsDBCSLeadByte': (LEAD, 0x6fd154, 1, 0, 0), 'ImmGetContext': (GETCTX, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (RELEASE, 0x6fd080, 2, 0, 0),
           'ImmGetCandidateListA': (LIST, 0x6fd078, 4, 0, 0), 'ImmGetOpenStatus': (OPEN, 0x6fd06c, 1, 0, 0), 'ImmSetCompositionWindow': (FORM, 0x6fd074, 2, 0, 0), 'ImmNotifyIME': (NOTIFY, 0x6fd068, 4, 0, 0)}
B = Bench([(0x478720, 0x4787e0), (0x478860, 0x478b60), (0x478d20, 0x478d40), (0x478d40, 0x47914b), (0x479150, 0x4796a8), (0x4796b0, 0x4798d9), (0x4798e0, 0x479d3d), (0x479d80, 0x47a39f), (0x47a880, 0x47a950), (0x6b5570, 0x6b55be), (0x6b70d0, 0x6b70d8), (0x47ae40, 0x47dd50),
           (0x45b380, 0x45b9a0), (0x405000, 0x406000), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67a400, 0x690000), (0x6918f0, 0x691cd0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
IME, LISTS, PEEK_T, LAYOUT_T, PROP_T, LIST_DATA = imeattach.IME, RAM + 0x3000, RAM + 0x3800, RAM + 0x3900, RAM + 0x3a00, RAM + 0x3b00
MESSAGES = [0x50, 0x100, 0x101, 0x102, 0x10d, 0x10e, 0x10f, 0x281, 0x282, 0x282, 0x282, 0x282, 0x282, 0x282, 0x283, 0x284, 0x285, 0x286, 0x286, 0x287, 0x28f, 0x290, 0x291, 0x292, 0x120, 0xf]


def table(values, rest):
    return struct.pack('<48I', *(values + [rest] * (48 - len(values))))


def layouts(r, log, regions):
    """The layout list; returns a layout to look for."""
    n = r.choice([0, 1, 2, 3, 5])
    values = [r.randrange(1, 8) for _ in range(n)]
    regions[LAYOUT_T] = table([n, n], n)
    answers(log, LAYOUTS, LAYOUT_T)
    if n:
        fill(log, LAYOUTS, 1, struct.pack('<%dI' % n, *values), which=2)
    regions[PROP_T] = table([r.choice([0, 0x10000, 0x20000, 0x30000, 0x40000, 0xffff]) for _ in range(6)], 0x10000)
    answers(log, PROPERTY, PROP_T)
    return r.randrange(1, 8)


def case(r):
    log = log_region()
    imports = import_table(log, IMPORTS)
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 16 + [1, 2]))}
    layout = layouts(r, log, regions)
    set_return(log, DEFPROC, r.getrandbits(32))
    set_return(log, HANDLER, r.getrandbits(32))
    set_return(log, GETCTX, r.choice([0, 0x1000, 0x1000]))
    set_return(log, RELEASE, r.getrandbits(32))
    set_return(log, OPEN, r.choice([0, 0, 1]))
    set_return(log, LIST, 0)
    set_return(log, LEAD, r.choice([0, 1, 1]))
    set_return(log, WIDEN, r.choice([0, 1, 1, 1, 2]))
    fill(log, WIDEN, 4, struct.pack('<H', r.getrandbits(16)), which=0)
    fill(log, PEEK, 0, r.bytes(28), which=1)
    blob(log, TRANSLATE, 0, 0x7f)
    blob(log, DISPATCH, 0, 0x7f)
    blob(log, FORM, 1, 7)
    which = r.random()
    if which < 0.12:
        return Call(0x4796b0, [layout], regions, conv='c', ret='a')
    answers(log, PEEK, PEEK_T)
    regions[PEEK_T] = table([r.choice([0, 1, 1]) for _ in range(r.randrange(0, 4))], r.choice([0, 0, 0, 1]))
    ime = bytearray(r.bytes(0x170))
    imeattach.composition(r, ime, regions)
    message = r.choice(MESSAGES)
    wparam = r.randrange(0x100)
    lparam = r.getrandbits(32)
    if message == 0x50:
        lparam = layout
    if message == 0x10f:
        lparam = r.choice([0, 1, 0x400])
    if message == 0x286:
        wparam = r.choice([0, 0x41, 0x8140, 0x4100, r.getrandbits(32)])
    if message == 0x282:
        wparam = r.choice([0, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 7, 8, 8, 9, r.getrandbits(32)])
        if wparam in (3, 5):
            regions[LISTS] = bytes(0x24 * 32)
            put(ime, 0x3c, '3I', LISTS, LISTS + 0x480, LISTS + 0x480)
            lparam = r.choice([0, 1, 0x80000000, r.getrandbits(32)])
        else:
            imeattach.lists(r, ime, regions)
            lparam = r.choice([0, 1, 3, 5, r.getrandbits(32)])
    put(ime, 0x50, 'I', r.choice([0, recorder(HANDLER, 4, True)]))
    put(ime, 0x54, 'i', r.choice([-1, 0, 1, 2]))
    regions[IME] = ime
    if which < 0.3:
        keys = imeattach.map_regions(r, regions, r.choice([0, IME, IME, IME]))
        window = r.choice(keys) if keys and r.random() < 0.7 else r.randrange(0, 100)
        return Call(0x47a880, [window, message, wparam, lparam], regions, conv='s', ret='a', ignore=[(imeattach.TREE, 2)])
    return Call(0x4798e0, [IME, r.getrandbits(32), message, wparam, lparam], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
