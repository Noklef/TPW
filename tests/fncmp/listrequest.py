# The online lists' requests (src/game/online/listrequest) against the originals: session.py's reset_all() world and open_city()'s imports; a list with a vtable of
# recorders (busy, ask, take), an owner of 0-3 items, a native answer as reset_city() makes them; the callback a recorder of 3 cdecl arguments, the command queue and
# the login dialog recorders by hook and env.
# usage: listrequest.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Done,Start,Text,Member,Login,LoginMember,Has108)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, put, recorder, set_return, vtable  # noqa: E402
import session  # noqa: E402

DIALOG, POST = 16, 25
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x5b5000, 0x5be000), (0x5f5f10, 0x5f5faa), (0x67afa0, 0x67b0d0), (0x4bae30, 0x4bae62), (0x41a830, 0x41a834), (0x4c25e0, 0x4c25f9), (0x658d9f, 0x658e04), (0x659710, 0x659740),
           (0x65f697, 0x65f6e1), (0x48b390, 0x48b440)],
          hooks={0x4ba980: C(DIALOG, 1), 0x47f410: C(POST, 2)})
KINDS = [(0x5b9b00, 0x5bc7d0, 0x5bc710), (0x5b9d60, 0x5bcd10, 0x5bca70)]
MEMBERS = [(0x5ba030, 0x5bd070, 0x5bcfb0, 0xf8), (0x5ba2f0, 0x5bd4b0, 0x5bd3f0, 0xfc), (0x5ba560, 0x5bd8c0, 0x5bd800, 0x100)]
OPS = [k for k in os.environ.get('OPS', 'Done,Start,Text,Member,Login,LoginMember,Has108').split(',') if k]
MAIN, OWNER, NODES, ITEMS, VT2, VT3, NATIVE, ARRAY = (RAM + 0x10000 + 0x400 * k for k in range(8))
CALLBACK = recorder(20, 3, cdecl=True)
SIZES = {0x5b9b00: 0x30, 0x5b9d60: 0x66, 0x5ba030: 0x10, 0x5ba2f0: 0x26, 0x5ba560: 0x34}


def fields(r, data, at, native_at, count_at):
    """A list's fields at `at` in the bytes `data`: its vtable, flags and error, the callback, the owner and a native answer (a count of uses of 1, 2 or 3; 1: freed, with
    no array or one of 0-2 elements)."""
    put(data, at, 'I', VT2)
    put(data, at + 8, 'I', r.choice([0, 0, 1]))
    put(data, at + 0x14, 'I', r.choice([0, 1, 1, 1]))
    put(data, at + 0x18, 'I', r.choice([0, 0, 0, 0x4f]))
    put(data, at + 0x1c, 'I', r.choice([0, CALLBACK, CALLBACK]))
    put(data, at + 0x28, 'I', r.choice([0, 0, 1]))
    put(data, at + 0x2c, 'I', OWNER)
    put(data, at + 0x30, 'I', r.choice([0, native_at, native_at, native_at]))
    put(data, at + 0x34, 'I', VT2)
    return count_at


def world(r):
    regions = session.reset_all(r, 'ResetAll').regions
    log, table = regions[LOG], regions[IMPORT_TABLE]
    for k, (slot, n) in enumerate([(0x6fd4e4, 0), (0x6fd534, 4), (0x6fd53c, 4)]):
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(21 + k, n))
    struct.pack_into('<I', table, 0x6fd500 - IMPORT_TABLE, recorder(24, 0))
    set_return(log, 22, r.choice([1, 1, 1, 0]))
    set_return(log, 21, r.choice([session.OBJ, session.OBJ + 0x40]))
    set_return(log, 1, r.choice([0, 0, 0, 1]))   # (busy: vtable +4, and OutputDebugStringA's answer nobody reads)
    set_return(log, 2, r.getrandbits(32))
    s = regions[session.SESSION]
    put(s, 0x60, 'I', r.choice([0, 1, 1]))
    put(s, 0xd4, 'I', r.choice([0, 0, session.OBJ]))
    put(s, 0x58, 'I', r.choice([0, 0x10, r.getrandbits(32)]))
    put(s, 0x108, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions.update({0x7a1524: put(bytearray(r.bytes(4)), 0, 'I', r.choice([0, session.OBJ])), 0x7a1556: r.bytes(2), 0x7cc53c: bytes([r.choice([0, 1])]), 0x78b530: r.bytes(0x10)})
    regions[0xf7d88c] = struct.pack('<I', session.SESSION)
    n = r.choice([0, 0, 1, 2, 3])
    nodes = bytearray(0x10 * max(n, 1))
    for i in range(n):
        struct.pack_into('<III', nodes, 0x10 * i, ITEMS + 8 * i, NODES + 0x10 * (i + 1) if i + 1 < n else 0, NODES + 0x10 * (i - 1) if i else 0)
    regions[NODES] = nodes
    regions[ITEMS] = b''.join(struct.pack('<II', VT3, r.getrandbits(32)) for _ in range(4))
    regions[OWNER] = struct.pack('<IIII', r.getrandbits(32), NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, n)
    regions[VT2] = vtable({0: 1, 1: 0, 2: 2, 3: 1, 5: 1})
    regions[VT3] = vtable({0: 1})
    main = bytearray(r.bytes(0x48))
    fields(r, main, 0, NATIVE, 0)
    regions[MAIN] = main
    lists = regions[session.LISTS]
    for i in (6, 7, 8):
        fields(r, lists, 0x40 * i, session.REC + 0x10 * i, 0)
    uses = r.choice([1, 1, 2, 3])
    regions[NATIVE] = struct.pack('<IIII', r.getrandbits(32), r.choice([0, ARRAY + 4]), r.getrandbits(32), uses)
    regions[ARRAY] = struct.pack('<I', r.choice([0, 1, 2])) + r.bytes(0x100)
    return regions, main


def case(r):
    op = r.deal(OPS)
    regions, main = world(r)
    if op == 'Done':
        done = r.deal([k[0] for k in KINDS] + [m[0] for m in MEMBERS])
        return Call(done, [MAIN, r.choice([0, 0, 0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')
    if op == 'Start':
        start = r.deal([k[1] for k in KINDS])
        return Call(start, [MAIN] + [r.getrandbits(32)] * (start == 0x5bcd10) + [r.choice([0, CALLBACK]), r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Text':
        return Call(0x5bcbc0, [MAIN, r.getrandbits(32), r.choice([0, CALLBACK]), r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Member':
        return Call(r.deal([m[1] for m in MEMBERS]), [MAIN], regions, conv='t', ret='v')
    if op == 'Login':
        return Call(r.deal([k[2] for k in KINDS]), [r.choice([0, 1, r.getrandbits(32)]), MAIN], regions, ret='v')
    if op == 'LoginMember':
        return Call(r.deal([m[2] for m in MEMBERS]), [r.choice([0, 1, r.getrandbits(32)])], regions, ret='v')
    return Call(0x5bc8e0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
