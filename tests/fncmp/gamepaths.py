# The game's folder paths of src/game/files/gamepaths against the originals: the fonts folder's path (0x468250) from the root folder's string (the first of
# the Strings5 object at 0xf816c0; only its data pointer at +8 is read): no text, an empty one (the byte before it read: "\" or not), roots with and
# without a trailing "\", up to 0x100 chars; the output buffer's bytes past the path compared too. TexFolders (0x4688b0) prints the two texture
# folder names into 0x7af650 / 0x7af720 by its flag (0, 1 or any), over random bytes. SaveFolder (0x5c7bd0) builds the root's wide form and the global
# wide "save" (0xf818d8, now and then another text) into out: MultiByteToWideChar a recorder (as in wideconv.py) answering a size and writing a wide text
# (NUL-terminated, now and then not within the size), GlobalAlloc / GlobalFree the heap stubs, the allocation failing now and then.
# usage: gamepaths.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FontsFolder,TexFolders,SaveFolder)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fill, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402

B = Bench([(0x468250, 0x4682b0), (0x5c5520, 0x5c5530), (0x67a400, 0x690000), (0x4688b0, 0x468902), (0x5c7bd0, 0x5c7c64), (0x424e10, 0x424e6e),
           (0x5f5660, 0x5f5690), (0x5f5fc0, 0x5f7f00)])
PATHS = 0xf816c0
ROOT, OUT = RAM + 0x100, RAM + 0x1000
CHARS = b'abcXYZ09:\\/. _%'
SAVE, OUT_W = 0xf818d8, RAM + 0x2000
MB2WC = 40   # MultiByteToWideChar's recorder slot


def save_folder(r):
    """0x5c7bd0 (stdcall; out): out not yet constructed."""
    log = log_region()
    table = import_table(log, {'MultiByteToWideChar': (MB2WC, 0x6fd174, 6, 4, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    n = r.choice([0, 1, 3, 8, 20])
    root = bytes(r.choice(CHARS) for _ in range(n))
    wide = struct.pack('<%dH' % n, *root) + (bytes(2) if r.random() < 0.9 else b'a\0b\0')
    size = n + 1 if r.random() < 0.8 else r.choice([0, 1, n, n + 4])
    set_return(log, MB2WC, size)
    fill(log, MB2WC, 4, wide[:0x70])
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, r.choice(range(1, 6)) if r.random() < 0.15 else 0), ROOT: root + bytes(1) + r.bytes(8),
               OUT_W: r.bytes(0x20)}
    paths = bytearray(r.bytes(0x14))
    put(paths, 0, 'I', 0x703090)
    put(paths, 8, 'I', ROOT)
    regions[PATHS] = bytes(paths)
    save = 'save'.encode('utf-16-le') if r.random() < 0.85 else r.choice([b'', 'x'.encode('utf-16-le'), 'saves\\'.encode('utf-16-le')])
    gamestring.string(r, 1, SAVE, RAM + 0x3000, regions, save)
    return Call(0x5c7bd0, [OUT_W], regions, conv='s', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'SaveFolder':
        return save_folder(r)
    if op == 'TexFolders':
        regions = {0x7af650: r.bytes(0xd0), 0x7af720: r.bytes(0x40)}
        return Call(0x4688b0, [r.choice([0, 1, 1, 2, 0x100, 0x80000000, r.getrandbits(32)])], regions, ret='a')
    regions = {}
    paths = bytearray(r.bytes(0x14))
    if r.random() < 0.1:
        put(paths, 8, 'I', 0)
    else:
        n = r.choice([0, 1, 2, 3, 8, 20, 60, 0x100])
        text = bytes(r.choice(CHARS) for _ in range(n))
        if n and r.random() < 0.4:
            text = text[:-1] + b'\\'
        before = bytes([r.choice([0x5c, 0x41, 0])])
        regions[ROOT - 1] = before + text + b'\0' + r.bytes(4)
        put(paths, 8, 'I', ROOT)
    regions[PATHS] = bytes(paths)
    regions[OUT] = r.bytes(0x140)
    return Call(0x468250, [OUT], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'FontsFolder,TexFolders,SaveFolder').split(',') if k]

if __name__ == '__main__':
    B.main(case)
