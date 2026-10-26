# The game's folder paths of src/game/files/gamepaths against the originals: the fonts folder's path (0x468250) from the root folder's string (the first of
# the Strings5 object at 0xf816c0; only its data pointer at +8 is read): no text, an empty one (the byte before it read: "\" or not), roots with and
# without a trailing "\", up to 0x100 chars; the output buffer's bytes past the path compared too. TexFolders (0x4688b0) prints the two texture
# folder names into 0x7af650 / 0x7af720 by its flag (0, 1 or any), over random bytes. SaveFolder (0x5c7bd0) builds the root's wide form and the global
# wide "save" (0xf818d8, now and then another text) into out: MultiByteToWideChar a recorder (as in wideconv.py) answering a size and writing a wide text
# (NUL-terminated, now and then not within the size), GlobalAlloc / GlobalFree the heap stubs, the allocation failing now and then. UsersFolder
# (0x5c7c70) and OnlineFolder (0x5c8980) are the same with the global wide "\" (0xf82508) and "users" (0xf81ff8) / "online" (0xf820e8) appended,
# now and then other texts. SecondRoot (0x5aa650) takes a String by value (its five dwords on the stack: a drive letter mostly, any short text or
# none) and assigns it with ":" and the String at 0xf79fe8 appended to the String at 0xf816d4 (the heap's allocations failing now and then: the
# harness's countdown, a fault in both). FindDisc (0x5aa500): see find_disc().
# usage: gamepaths.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FontsFolder,TexFolders,SaveFolder,UsersFolder,OnlineFolder,SecondRoot,FindDisc)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fail_after, fill, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402

B = Bench([(0x468250, 0x4682b0), (0x5c5520, 0x5c5530), (0x67a400, 0x690000), (0x4688b0, 0x468902), (0x5c7bd0, 0x5c7c64), (0x424e10, 0x424e6e),
           (0x5f5660, 0x5f5690), (0x5f5fc0, 0x5f7f00), (0x5c7c70, 0x5c7cfd), (0x5c8980, 0x5c8a0d), (0x5aa650, 0x5aa720), (0x5c5530, 0x5c5560),
           (0x5aa500, 0x5aa64e), (0x5f5f00, 0x5f5fc0), (0x5f7f00, 0x5f9000), (0x6ace60, 0x6acea0), (0x485ac0, 0x485af4)])
PATHS = 0xf816c0
ROOT, OUT = RAM + 0x100, RAM + 0x1000
CHARS = b'abcXYZ09:\\/. _%'
SAVE, OUT_W = 0xf818d8, RAM + 0x2000
MB2WC = 40   # MultiByteToWideChar's recorder slot


def save_folder(r, address=0x5c7bd0):
    """0x5c7bd0 (stdcall; out): out not yet constructed. 0x5c7c70 / 0x5c8980 the same with the globals "\\" and "users" / "online" after it."""
    log = log_region()
    table = import_table(log, {'MultiByteToWideChar': (MB2WC, 0x6fd174, 6, 4, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    n = r.choice([0, 1, 3, 8, 20])
    root = bytes(r.choice(CHARS) for _ in range(n))
    wide = struct.pack('<%dH' % n, *root) + (bytes(2) if r.random() < 0.9 else b'a\0b\0\0\0')   # (the NUL 2 past the text)
    size = n + 1 if address != 0x5c7bd0 or r.random() < 0.8 else r.choice([0, 1, n, n + 4])
    set_return(log, MB2WC, size)
    fill(log, MB2WC, 4, wide[:0x70])
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, r.choice(range(1, 6)) if r.random() < (0.15 if address == 0x5c7bd0 else 0.03) else 0), ROOT: root + bytes(1) + r.bytes(8),
               OUT_W: r.bytes(0x20)}
    paths = bytearray(r.bytes(0x14))
    put(paths, 0, 'I', 0x703090)
    put(paths, 8, 'I', ROOT)
    regions[PATHS] = bytes(paths)
    save = 'save'.encode('utf-16-le') if r.random() < 0.85 else r.choice([b'', 'x'.encode('utf-16-le'), 'saves\\'.encode('utf-16-le')])
    gamestring.string(r, 1, SAVE, RAM + 0x3000, regions, save)
    if address in SUBFOLDERS:
        sep = '\\'.encode('utf-16-le') if r.random() < 0.85 else r.choice([b'', '/x'.encode('utf-16-le')])
        gamestring.string(r, 1, SEP, RAM + 0x3100, regions, sep)
        name, other = SUBFOLDERS[address]
        sub = other.encode('utf-16-le') if r.random() < 0.85 else r.choice([b'', 'abcdefghijklmnop'.encode('utf-16-le')])
        gamestring.string(r, 1, name, RAM + 0x3200, regions, sub)
    return Call(address, [OUT_W], regions, conv='s', ret='a')


SEP = 0xf82508
SUBFOLDERS = {0x5c7c70: (0xf81ff8, 'users'), 0x5c8980: (0xf820e8, 'online')}


def second_root(r):
    regions = {HEAP: heap(0x4000, r.choice(range(1, 5)) if r.random() < 0.05 else 0)}
    drive = r.choice([b'D', b'E', b'Z', b'c']) if r.random() < 0.7 else bytes(r.choice(CHARS) for _ in range(r.choice([0, 2, 9, 30])))
    gamestring.string(r, 0, RAM + 0x4000, RAM + 0x4100, regions, drive)
    arg = list(struct.unpack_from('<5I', regions.pop(RAM + 0x4000)))
    tail = b'\\TPW' if r.random() < 0.7 else bytes(r.choice(CHARS) for _ in range(r.choice([0, 1, 7, 40])))
    gamestring.string(r, 0, 0xf79fe8, RAM + 0x4200, regions, tail)
    gamestring.string(r, 0, PATHS + 0x14, RAM + 0x4300, regions)
    return Call(0x5aa650, arg, regions)


DISC, RECORDS, LABEL = RAM + 0x5000, RAM + 0x5100, RAM + 0x5400


def find_disc(r):
    """0x5aa500 (thiscall on the CD check: a StringList at +0, empty, the system-info object at +0x10): second_root()'s globals; the text a
    record (index 9 of the table at [0x78ba88]: there, out of range, or not terminated: no text) widened (0x78ba80 off: the static buffer
    0x7ba058 as it is); the caption the label String at 0xf7a7c8 (drives.py's labels) widened by MultiByteToWideChar (a recorder writing a
    wide text); GetLogicalDriveStringsA failing (its hash: a count past 0x100) for its first 0-3 calls, then drives.py's roots and answers;
    GetDriveTypeA one type, GetVolumeInformationA a label or a failure; MessageBoxW (logged) answering its hash (retry) for its first 0-3
    calls, then IDCANCEL (2). The heap fails now and then (its countdown; a fault in both)."""
    c = second_root(r)
    regions = c.regions
    log = log_region()
    table = import_table(log, {'MultiByteToWideChar': (MB2WC, 0x6fd174, 6, 4, 0), 'GetLogicalDriveStringsA': (41, 0x6fd0a0, 2, 0, 2),
                               'GetDriveTypeA': (42, 0x6fd09c, 1, 1, 0), 'GetVolumeInformationA': (43, 0x6fd0a4, 8, 1, 2), 'MessageBoxW': (44, 0x6fd378, 4, 0, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    labels = [b'THEMEPARK', b'ThemePark', b'TP', b'', b'THEMEPARKWORLD12345']
    label = r.choice(labels)
    n = len(label)
    set_return(log, MB2WC, n + 1)
    fill(log, MB2WC, 4, (struct.pack('<%dH' % n, *label) + bytes(2))[:0x70], 2)
    roots = r.choice([[b'C:\\', b'D:\\'], [b'A:\\', b'C:\\', b'D:\\', b'E:\\'], [b''], [b'C:\\'], [b'X', b'D:\\'], []])
    strings = b''.join(x + b'\0' for x in roots) + b'\0'
    fill(log, 41, 1, strings, 0)
    fail_after(log, 41, r.choice([0, 0, 1, 2, 3]), r.choice([len(strings) - 1, len(strings) - 1, 0]))
    set_return(log, 42, r.choice([5, 5, 3, 2, 4, 6, r.getrandbits(32)]))
    fill(log, 43, 1, r.choice(labels) + b'\0', 1)
    set_return(log, 43, r.choice([0, 1, 1, 1]))
    fail_after(log, 44, r.choice([0, 1, 2, 3]), 2)
    gamestring.string(r, 0, 0xf7a7c8, LABEL, regions, label)
    rec = bytearray(r.bytes(0x40))
    text = r.choice([b'Please insert the disc', b'', b'x', b'abc'])
    put(rec, 0, 'I', 1 | (len(text) + 1) << 8 if r.random() < 0.9 else r.getrandbits(32))
    rec[4:5 + len(text)] = text + b'\0'
    put(rec, 4 + (len(text) + 4 & ~3), 'I', 0 if r.random() < 0.9 else 1)
    regions.update({LOG: log, IMPORT_TABLE: table, HEAP: heap(0x8000, r.choice(range(1, 9)) if r.random() < 0.04 else 0),
                    DISC: struct.pack('<4I', 0x6fe680, 0, 0, 0) + r.bytes(0x20), 0xfb1440: bytes(4), 0xfa7610: r.bytes(4),
                    0x78ba88: struct.pack('<I', RECORDS), RECORDS: struct.pack('<4I', 0, 0, r.choice([10, 10, 10, 9]), RECORDS + 0x10)
                    + struct.pack('<10I', *[RECORDS + 0x40] * 10) + bytes(0x08) + rec, 0x78ba80: bytes(4),
                    0x7ba058: 'Disc?'.encode('utf-16-le') + bytes(2) + r.bytes(8)})
    return Call(0x5aa500, [DISC], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'FindDisc':
        return find_disc(r)
    if op == 'SecondRoot':
        return second_root(r)
    if op == 'SaveFolder':
        return save_folder(r)
    if op == 'UsersFolder':
        return save_folder(r, 0x5c7c70)
    if op == 'OnlineFolder':
        return save_folder(r, 0x5c8980)
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


OPS = [k for k in os.environ.get('OPS', 'FontsFolder,TexFolders,SaveFolder,UsersFolder,OnlineFolder,SecondRoot,FindDisc').split(',') if k]

if __name__ == '__main__':
    B.main(case)
