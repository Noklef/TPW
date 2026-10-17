# The system-info object's drive queries (src/game/files/drives) against the originals: its constructor (the CPU probe stood in for: fncmp._mark, the
# driver's stub_mark), the logical drives listed by type into a StringList, a volume found by label and the two helpers. The imports are recorders
# (COM-style, fncmp import_table()): GetLogicalDriveStringsA fills its stack buffer with up to four roots (or an empty first one, or a root without a
# colon) and answers their length or a failure; GetDriveTypeA one type for a case, logged by the root's text; GetVolumeInformationA a label (at most
# 0x14 bytes with its NUL, the size the search asks for) or a failure. The list has 0-3 nodes; the heap now and then fails an allocation (the game's
# null node dereferenced too).
# usage: drives.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ListDrives,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, _mark, fill, heap, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x5f8790, 0x5f8a50), (0x5f5f00, 0x5f9000), (0x67a400, 0x690000)], hooks={0x55e470: _mark})
THIS, RESULT, LIST, NODES, TEXTS, LABEL, BUF = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
LOCALE = 0xfb1440   # the CRT's LC_CTYPE handle: 0, the C locale (_stricmp's)
STRING_VTABLE = 0x703090
# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'GetLogicalDriveStringsA': (40, 0x6fd0a0, 2, 0, 2), 'GetDriveTypeA': (41, 0x6fd09c, 1, 1, 0), 'GetVolumeInformationA': (42, 0x6fd0a4, 8, 1, 2)}
K_STRINGS, K_TYPE, K_VOLUME = 40, 41, 42
ROOTS = [b'C:\\', b'D:\\', b'A:\\', b'E:\\', b'X', b'\\\\srv\\']
LABELS = [b'THEMEPARK', b'ThemePark', b'themepark', b'TP', b'', b'THEMEPARKWORLD12345']   # (the longest fills 0x14 bytes with its NUL)
# name: (address, stack arguments: R the result, L the list, t the drive types, S a label, P a path, B a label buffer, s its size; returns)
OPS = {
    'Ctor': (0x5f8790, '', 'a'), 'ListDrives': (0x5f87b0, 'RLt', 'a'), 'FindVolume': (0x5f8940, 'LS', 'b'), 'DriveLetter': (0x5f89d0, 'P', 'b'),
    'VolumeLabel': (0x5f89f0, 'RPBs', 'a'),
}


def string_list(r, regions):
    """A StringList (vtable, head, tail, count) at LIST of 0-3 nodes (a String: vtable, length, data, capacity, grow; next, prev) holding roots."""
    nodes = [NODES + 0x20 * i for i in range(r.choice([0, 1, 2, 3]))]
    texts = bytearray(r.bytes(0x100))
    data = bytearray(r.bytes(0x80))
    for i, n in enumerate(nodes):
        root = r.choice(ROOTS)
        texts[0x20 * i:0x20 * i + len(root) + 1] = root + b'\0'
        struct.pack_into('<5I2I', data, 0x20 * i, STRING_VTABLE, len(root), TEXTS + 0x20 * i, len(root) + 8, 0, nodes[i + 1] if i + 1 < len(nodes) else 0,
                         nodes[i - 1] if i else 0)
    regions[NODES], regions[TEXTS] = data, texts
    regions[LIST] = struct.pack('<4I', r.getrandbits(32), nodes[0] if nodes else 0, nodes[-1] if nodes else 0, len(nodes)) + r.bytes(0x10)


def case(r):
    op = r.choice(PICK)
    address, args, ret = OPS[op]
    log = log_region()
    roots = r.choice([[b'C:\\', b'D:\\'], [b'A:\\', b'C:\\', b'D:\\', b'E:\\'], [b''], [b'C:\\'], [b'X', b'D:\\'], []])
    strings = b''.join(x + b'\0' for x in roots) + b'\0'
    fill(log, K_STRINGS, 1, strings, 0)
    set_return(log, K_STRINGS, r.choice([len(strings) - 1, len(strings) - 1, len(strings) - 1, 0, 0xff, 0x100, 0x1000, len(strings)]))
    set_return(log, K_TYPE, r.choice([0, 1, 2, 3, 4, 5, 6, r.getrandbits(32)]))
    fill(log, K_VOLUME, 1, r.choice(LABELS) + b'\0', 1)
    set_return(log, K_VOLUME, r.choice([0, 1, 1, 1]))
    regions = {THIS: r.bytes(0x40), RESULT: r.bytes(0x10), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), LOCALE: bytes(4),
               HEAP: heap(0x1000, fail=r.choice([1, 2]) if r.random() < 0.05 else 0)}
    stack = []
    for x in args:
        if x == 'R':
            stack.append(RESULT)
        elif x == 'L':
            string_list(r, regions)
            stack.append(LIST)
        elif x == 't':
            stack.append(r.choice([0, 1, 2, 4, 8, 0xf, 0xa, r.getrandbits(32)]))
        elif x == 'S':
            regions[LABEL] = r.choice(LABELS) + b'\0' + r.bytes(0x20)
            stack.append(LABEL)
        elif x == 'P':
            regions[LABEL] = r.choice(ROOTS + [b'', b':', b'C:', b'z:x']) + b'\0' + r.bytes(0x20)
            stack.append(LABEL)
        elif x == 'B':
            regions[BUF] = r.bytes(0x40)
            stack.append(BUF)
        elif x == 's':
            stack.append(r.choice([0, 1, 0x14, 0x40, r.getrandbits(32)]))
    return Call(address, [THIS] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
