# The save and load dialogs' handlers and list fillers (src/game/ui/savedialog: 0x49e880, 0x49e9b0, 0x49ea30, 0x49ec80, 0x49edf0) against the originals. A list at 0xf7b560 of up to 5 named items, its search (0x5accb0), the file time's calls, WString, Window_FindChild and
# the CRT real, the two time imports recorders; recorders (driver.cpp's kSaveDialogEnv): the list's first 0x5ac8f0 10, its save 0x5ac610 11 (the string hashed), load 0x5ac5d0 12, 0x409300 13, Window_SendMessage 14, Window_PostMessage 15, Focus_Restore 16, KeyMaps_EnableAllAndClearFlag 17,
# Dialog_BaseHandler 18, Window7059f0_NthItemData 19 (its out filled), Charset_FormatWide 20 (filling its buffer; its arguments hashed), the box 0x47f020 21, Window7059f0_AddItem 22 (the texts hashed), slot 9 of the windows 23.
# usage: savedialog.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=LoadHandler,Save,SaveHandler,FillLoad,FillSave)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, heap, import_table, log_region, put, recorder, set_return, strings, vtable  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
HOOKS = {0x5ac8f0: _recorder(10, 0), 0x5ac610: _recorder(11, 1), 0x5ac5d0: _recorder(12, 1), 0x409300: _recorder(13, 0), 0x658d9f: C(14, 4), 0x658ccc: C(15, 4), 0x4862a0: C(16, 0), 0x486b60: C(17, 0), 0x47ee90: C(18, 4), 0x664c71: _recorder(19, 2), 0x6acd60: C(20, 7),
         0x47f020: C(21, 8), 0x66403b: _recorder(22, 4)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x49e880, 0x49e9a7), (0x49e9b0, 0x49ea27), (0x49ea30, 0x49ec7f), (0x49ec80, 0x49eded), (0x49edf0, 0x49efa8), (0x424d10, 0x424d40), (0x5accb0, 0x5acd00), (0x5fc5a0, 0x5fc5d0), (0x5fc660, 0x5fc750), (0x5f6980, 0x5f6a90), (0x5f7800, 0x5f7840),
           (0x65d7bc, 0x65d840), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)], hooks=HOOKS)
WINDOW, ITEMS, KIDS = RAM + 0x100, RAM + 0x800, RAM + 0x2000
LIST = 0xf7b560
NAMES = ['one', 'Two', 'three', 'Four', 'file', 'name']
CHILD = 0x1234
CALLS = {'LoadHandler': 0x49e880, 'Save': 0x49e9b0, 'SaveHandler': 0x49ea30, 'FillLoad': 0x49ec80, 'FillSave': 0x49edf0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def wide(text):
    return text.encode('utf-16le') + b'\0\0'


def case(r):
    op = r.deal(OPS)
    log = log_region()
    imports = import_table(log, {'FileTimeToLocalFileTime': (40, 0x6fd15c, 2, 0, 1 << 1), 'FileTimeToSystemTime': (41, 0x6fd0bc, 2, 0, 1 << 1)})
    blob(log, 41, 0, 3)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    set_return(log, 40, 1)
    set_return(log, 41, 1)
    fill(log, 40, 1, r.bytes(8), which=0)
    fill(log, 41, 1, struct.pack('<8H', r.randrange(1990, 2030), r.randrange(1, 13), r.randrange(7), r.randrange(1, 29), r.randrange(24), r.randrange(60), r.randrange(60), r.randrange(1000)), which=1)
    regions = {LOG: log, HEAP: heap(), IMPORT_TABLE: imports, LOCALE: bytes(4), 0xfb3ca4: bytes(8), 0x78ba94: r.bytes(4), 0x786ba4: r.bytes(4), 0x7ba058: bytes(0x100), 0x7cb244: struct.pack('<2I', r.getrandbits(32), r.getrandbits(32))}
    first, _ = kids(r, regions, KIDS, (CHILD, 0x23bce2))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[VT] = put(vtable({}, size=0x10), 0x24, 'I', recorder(23, 1))
    # The list: nodes {item, next}, items with their name at +8 and their time at +0x14.
    n = r.randrange(0, 6) if op in ('LoadHandler', 'FillLoad', 'FillSave') else r.randrange(1, 6)
    names = r.sample(NAMES, n)
    block = bytearray(0x800)
    for k, name in enumerate(names):
        item, text, node = 0x20 * k, 0x100 + 0x20 * k, 0x300 + 8 * k
        put(block, item + 8, 'I', ITEMS + text)
        put(block, item + 0x14, '2I', r.getrandbits(32), r.getrandbits(32) & 0x7fffffff)
        block[text:text + 2 * len(name) + 2] = wide(name)
        put(block, node, '2I', ITEMS + item, ITEMS + node + 8 if k + 1 < n else 0)
    regions[ITEMS] = block
    regions[LIST] = struct.pack('<2I', r.getrandbits(32), ITEMS + 0x300 if n else 0)
    set_return(log, 10, ITEMS if n else 0)
    typed = r.deal(names + ['other', 'ONE'])
    regions[0x7ca720] = wide(typed).ljust(0x20, b'\0')
    set_return(log, 18, r.getrandbits(32))
    fill(log, 19, 1, struct.pack('<I', r.randrange(n + 1) if op == 'LoadHandler' else r.randrange(max(n, 1))), which=2)
    strings(log, 19, 0, 1 << 1)
    blob_bytes(log, 11, 0, 0xf0, (8, 0x7fff))
    blob_bytes(log, 21, 0, 0xf70, (0, 0x7fff))
    blob(log, 22, 0, 3)
    fill(log, 20, 1, wide('formatted'), which=3)
    if op == 'FillLoad' or op == 'FillSave':
        at = 6 if op == 'FillLoad' else 5
        blob(log, 20, at, 0x7fff)
        strings(log, 20, 0, (1 << 1 | 1 << 4 | 1 << 5 | 1 << 6) & ~(1 << at))
        return Call(CALLS[op], [WINDOW], regions, ret='v')
    blob(log, 20, 4, 0xf)
    strings(log, 20, 0, 1 << 1 | 1 << 5 | 1 << 6)
    if op == 'Save':
        return Call(CALLS[op], [r.getrandbits(32)], regions, ret='a')
    message = r.deal([5, 0x14, 0x100, 0x100, 0x100, 0x400, 0x400, 0x400, 0x802, 0x804, 6, 0x13, 0x15, 0xff, 0x101, 0x3ff, 0x401, 0x801, 0x803, 0x805, r.getrandbits(32)])
    a = r.choice([CHILD, CHILD, 0x23bce2, 0x777]) if message == 0x400 else r.choice([-1, -1, -2, -2, 0, 7, r.getrandbits(32)]) & 0xffffffff
    b = r.choice([-1, 0, 1, r.getrandbits(32)]) & 0xffffffff
    return Call(CALLS[op], [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
