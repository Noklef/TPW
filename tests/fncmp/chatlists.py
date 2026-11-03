# The chat object's list files (src/game/online/chatlists: 0x4ef010, 0x4ef3b0, 0x4ef880, 0x4efc10) against the originals: the stream calls, the file calls and the online folder's WString recorders (their stack `this` masked, mask_this()), the strings and the tree real.
# usage: chatlists.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, blob_array, blob_bytes, echo, fill, heap, import_table, log_region, mask_this, recorder, set_return, strings  # noqa: E402

FBCTOR, OPEN, SETSTATE, FBINIT, FBDTOR, IOSBASE, IOSDTOR, WSTRDTOR, SINGLETON, OFSDTOR, WSASSIGN, WSAPPEND, FORMAT, FCLOSE = range(14)
STREAMCTOR, CLEAR, SHARE, RELEASE, FOPEN, GETFACET, GETLINE, WIDEN, SETBUDDY, ASSIGN, FIND, TOGGLE, FBREAD, FACET = range(14, 28)
FORMATW, WRITE = FOPEN, GETFACET   # (the saves have no fopen or facet)
ONLINE, NOTIFY = 40, 41
STACK_THIS = [FBCTOR, OPEN, SETSTATE, FBINIT, FBDTOR, IOSBASE, IOSDTOR, WSTRDTOR, OFSDTOR, WSASSIGN, WSAPPEND, STREAMCTOR, CLEAR, SHARE, RELEASE, GETLINE, WIDEN, ASSIGN, FBREAD]
HOOKS = {0x4f3720: (FBCTOR, 1), 0x4f3920: (OPEN, 2), 0x4f0110: (SETSTATE, 2), 0x4f5990: (FBINIT, 2), 0x4f08c0: (FBDTOR, 0), 0x693830: (IOSBASE, 0), 0x4f0100: (IOSDTOR, 0), 0x5f6a60: (WSTRDTOR, 0), 0x5c7420: (SINGLETON, 0), 0x4f0540: (OFSDTOR, 0),
         0x4f1560: (STREAMCTOR, 4), 0x4f1be0: (STREAMCTOR, 3), 0x4f1620: (CLEAR, 2), 0x4ef7f0: (SHARE, 1), 0x4ef790: (RELEASE, 0), 0x4f1640: (WIDEN, 1), 0x4f1ca0: (GETLINE, 3), 0x4ee710: (SETBUDDY, 2), 0x4051d0: (ASSIGN, 2), 0x4f03c0: (FIND, 2),
         0x4eb450: (TOGGLE, 1), 0x4f5ad0: (FBREAD, 0)}
CDECL = {0x67a650: (FORMAT, 3), 0x67b620: (FCLOSE, 1), 0x693a70: (FOPEN, 2), 0x4f6a30: (GETFACET, 3), 0x67af00: (FORMATW, 3), 0x4f6be0: (WRITE, 2)}
hooks = {at: _recorder(k, n) for at, (k, n) in HOOKS.items()}
hooks.update({at: _recorder(k, n, cdecl=True) for at, (k, n) in CDECL.items()})
hooks.update({0x5c8980: _recorder(ONLINE, 1, com=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
B = Bench([(0x4ef010, 0x4ef384), (0x4ef3b0, 0x4ef789), (0x4ef830, 0x4ef840), (0x4ef880, 0x4efc02), (0x4efc10, 0x4f0005), (0x4f1bd0, 0x4f1be0), (0x4f23f0, 0x4f2437), (0x4f1340, 0x4f13f8), (0x4f44d0, 0x4f4577), (0x4f5430, 0x4f54d7), (0x45b590, 0x45b8c5),
           (0x405280, 0x405390), (0x479150, 0x4791b0), (0x4f0250, 0x4f0367), (0x4f39e0, 0x4f3b06), (0x4f6080, 0x4f6086), (0x4f6090, 0x4f618e), (0x4f6190, 0x4f6266), (0x47b750, 0x47bcf0), (0x479230, 0x479256), (0x67acd0, 0x67acee)], hooks=hooks)
OPS_TABLE = {'savebuddies': 0x4ef010, 'loadbuddies': 0x4ef3b0, 'saveignored': 0x4ef880, 'loadignored': 0x4efc10, 'wcslen': 0x67acd0}
SELF, NAME, TREE, NIL, FOLDER_VT, FOLDER_TEXT, STREAM, STREAM_VB, FACET_OBJ, FACET_VT, ANSWERS, FILE, NIL_E, KEY_TEXT = (RAM + 0x1000 + 0x400 * i for i in range(14))
OPS = [n for n in OPS_TABLE if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
LINES = ['one', '', 'a longer name', 'x' * 15]


def wide(text, size):
    return text.encode('utf-16-le').ljust(size, b'\0')


def tree(r, self_at, offset, self_bytes):
    """A right spine of nodes after a head, each node's +0x10 a text or null. The ignored list's nil is its own (0x7ced34), so advancing it as the buddies' tree (0x7ced2c) shows."""
    nil = NIL_E if offset == 0xea4 else NIL
    count = r.choice([0, 0, 1, 2, 4])
    data = bytearray(0x400)
    head = TREE
    nodes = [TREE + 0x40 * (i + 1) for i in range(count)]
    struct.pack_into('<3I', data, 0, nodes[0] if nodes else head, nodes[0] if nodes else nil, nodes[-1] if nodes else head)
    for i, node in enumerate(nodes):
        struct.pack_into('<3I', data, node - TREE, nil, nodes[i - 1] if i else head, nodes[i + 1] if i + 1 < count else nil)
        struct.pack_into('<I', data, node - TREE + 0x10, r.choice([0, r.getrandbits(32)]))
    struct.pack_into('<I', self_bytes, offset, head)
    return data


def case(r):
    name = r.deal(OPS)
    if name == 'wcslen':
        return Call(0x67acd0, [NAME], {NAME: wide(r.choice(LINES), 0x20)}, conv='c', ret='a')
    log = log_region()
    table = import_table(log, {})
    table.extend(bytes(0x100))   # (0x6fd484 is past import_table()'s 0x400 bytes)
    struct.pack_into('<I', table, 0x6fd484 - IMPORT_TABLE, recorder(NOTIFY, 2, True))
    strings(log, NOTIFY, 0, out=1)
    for k in STACK_THIS:
        mask_this(log, k)
    self_bytes = bytearray(r.bytes(0x1000))
    struct.pack_into('<I', self_bytes, 0, r.choice([r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, 0]))
    struct.pack_into('<I', self_bytes, 0xc, r.choice([0, r.getrandbits(32)]))
    tree_bytes = tree(r, SELF, 0xea4 if name.endswith('ignored') else 0xe94, self_bytes)
    file_now = r.choice([0, FILE] if name.startswith('save') or name == 'loadignored' else [0, 0, 0, FILE])
    fill(log, STREAMCTOR, 0, struct.pack('<3I', file_now, r.getrandbits(32), r.getrandbits(32)), offset=0x50)
    strings(log, STREAMCTOR, 0, out=1)
    fill(log, ONLINE, 0, struct.pack('<3I', FOLDER_VT, r.getrandbits(32), FOLDER_TEXT), which=1)
    strings(log, ONLINE, 0, out=1)
    fill(log, FORMAT, 0, b'path%d\0' % r.randrange(100), which=2)
    strings(log, FORMAT, 0, out=1)
    strings(log, OPEN, 1)
    set_return(log, OPEN, r.choice([FILE, FILE, 0]))
    set_return(log, FCLOSE, r.choice([0, 0, 1]))
    streams = bytearray(0x100)
    bad = r.choice([2, 4, 6])
    struct.pack_into('<I', streams, 0, STREAM_VB)
    struct.pack_into('<I', streams, 8 + 4, r.choice([0, 1, 0x18, 0xf1]))
    struct.pack_into('<I', streams, 0x80, STREAM_VB)
    struct.pack_into('<I', streams, 0x88 + 4, bad | r.choice([0, 1, 0x10]))
    n = r.choice([0, 1, 2, 3])
    singleton = r.choice([0, r.getrandbits(32) | 1])
    regions = {LOG: log, IMPORT_TABLE: table, SELF: self_bytes, NAME: b'name%d\0' % r.randrange(100), TREE: tree_bytes, NIL: bytes(0x30), NIL_E: bytes(0x30), 0x7ced2c: struct.pack('<I', NIL), 0x7ced34: struct.pack('<I', NIL_E), 0xf81ebc: struct.pack('<I', singleton),
               0x700630: struct.pack('<2I', 0, 0x58), 0x700640: struct.pack('<2I', 0, 0x5c), 0x75c6d0: b'suffixA\0', 0x75c6e8: b'suffixB\0', FOLDER_VT: struct.pack('<11I', *[recorder(WSAPPEND if i == 8 else WSASSIGN, 1) for i in range(11)]), FOLDER_TEXT: b'text\0',
               STREAM: streams, STREAM_VB: struct.pack('<2I', 0, 8), HEAP: heap(0x4000, fail=r.choice([0, 0, 1]) if not singleton else 0)}   # (a string's failed allocation only crashes both)
    if name.startswith('save'):
        fill(log, FORMATW, 0, wide('line%d' % r.randrange(100), 16), which=3)
        strings(log, FORMATW, 0, out=1)
        strings(log, WRITE, 0, out=1)
        blob(log, WRITE, 1, 3)
    else:
        strings(log, FOPEN, 1)
        set_return(log, FOPEN, r.choice([FILE, FILE, FILE, 0]))
        echo(log, SHARE, 0)
        strings(log, SHARE, 0, out=1)
        strings(log, GETFACET, 0, out=1)
        set_return(log, GETFACET, FACET_OBJ)
        regions[FACET_OBJ] = struct.pack('<I', FACET_VT)
        regions[FACET_VT] = struct.pack('<11I', *[recorder(FACET, 1) for _ in range(11)])
        set_return(log, FACET, r.choice([10, 0x2a]))
        fill(log, GETLINE, 0, b''.join(wide(r.choice(LINES), 0x20) for _ in range(3)), which=3, records=3)
        strings(log, GETLINE, 0, out=1)
        regions[ANSWERS] = struct.pack('<40I', *([STREAM] * n + [STREAM + 0x80] * (40 - n)))
        answers(log, GETLINE, ANSWERS)
        blob_bytes(log, SETBUDDY, 0, 0xfff0, inner=(4, 0x3ff))
        blob_array(log, ASSIGN, 0, 1, 2)
        strings(log, FIND, 0, out=0b11)
        head = struct.unpack_from('<I', self_bytes, 0xea4)[0]
        fill(log, FIND, 0, struct.pack('<2I', head, r.getrandbits(32) | 1), which=4, records=2)
        fill(log, FIND, 1, struct.pack('<I', KEY_TEXT + 2), which=5, offset=4)   # (the key is a copy of a wide string that the assign recorder leaves empty: a text of its own, so that its release shows)
        regions[KEY_TEXT] = bytes([0, r.choice([0, 3, 0xff])]) + b'k\0'
        strings(log, TOGGLE, 0, out=1)
    return Call(OPS_TABLE[name], [SELF, NAME], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
