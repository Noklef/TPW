# The texture directories (src/game/gfx/texdirs: 0x46e610-0x46ea80) against the originals:
# the setter's paths built (sprintf, run by the driver's stand-in) and compared with the old
# table's strings, the new block chained (global_heap()); the first path, the current pointer
# and the reset freeing the chain. Load (0x46ead0): a texture set's textures given directories,
# looked for through the quickload indexes or the mounts (vfs.py's); ForModel (0x46e4e0): the
# directories set for a model (see load() and for_model()).
# usage: texdirs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Reset,First,Current,SetCurrent,Load,ForModel)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put  # noqa: E402
import files  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(vfs.RANGES + [(0x46e4e0, 0x46e602), (0x46ead0, 0x46ed78), (0x46e610, 0x46e627), (0x46e630, 0x46e636), (0x46e640, 0x46e64a), (0x46e670, 0x46ea7b),
           (0x46ea80, 0x46eacb), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30), (0x67a400, 0x690000)]))
STRINGS, TABLE, CHAIN = RAM, RAM + 0x800, RAM + 0x900
BASES = [b'', b'C:\\TP\\', b'data\\', b'x', b'..\\levels\\']


def text(r):
    return bytes(r.choice(b'ab\\.') for _ in range(r.choice([0, 1, 2, r.randrange(30)])))


SET, ENTRIES, NAMES, DIRS, DIRTEXT, INFO, MODEL = RAM + 0x5000, RAM + 0x5100, RAM + 0x5400, RAM + 0x5800, RAM + 0x5900, RAM + 0x5600, RAM + 0x5700


def vfs_case(r, fail):
    """vfs.py's mounts and files.py's quickload tables (at RAM + 0x40000 on), the heap (failing as fail picks) and the texture
    directories: a table of 4 (the root + a folder mostly, a backslash after each, at DIRS), its count, the indices (-1 often) and the
    current pointer (an index of the quickload tables mostly). Returns the regions and the root."""
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice(fail)), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    quick, count = files.quickload(r, RAM + 0x40000)
    regions.update(quick)
    table, text = bytearray(), bytearray()
    for _ in range(4):
        d = r.choice([root + b'\\' + vfs.folder(r), vfs.name_under(r, root), vfs.folder(r), b''])
        d = d + b'\\' if d and r.random() < 0.9 else d
        text += r.bytes(1).replace(b'\0', b'a')   # (the byte before, read for an empty one)
        table += put(bytearray(4), 0, 'I', DIRTEXT + len(text))
        text += d + b'\0'
    regions[DIRS] = table
    regions[DIRTEXT] = text
    indices = bytearray(12)
    for k in range(3):
        put(indices, 4 * k, 'i', r.choice([-1, 0, 1, 2, 3] if k == 0 else [-1, -1, 0, 1, 2, 3]))
    current = r.randrange(min(count, 4)) if 0 < count < 0x100 else 0
    globals_ = put(bytearray(r.bytes(0x14)), 0, 'I', current if r.random() < 0.9 else r.getrandbits(3))
    put(globals_, 0xc, 'I', DIRS)
    put(globals_, 0x10, 'I', r.choice([4, 3, 0, r.getrandbits(16)]))
    regions[0x7b0620] = globals_
    regions[0x74d980] = indices
    return regions, root


def texture_set(r, regions, root):
    """A set (+0x36 textures, a word, 0x10 bytes each from [+0x54], +0xc a name: files.py's mostly, now and then under the root)."""
    s = bytearray(r.bytes(0x60))
    n = r.choice([0, 1, 2, 3, 5])
    put(s, 0x36, 'H', n)
    put(s, 0x54, 'I', ENTRIES)
    entries, names = bytearray(r.bytes(0x10 * max(n, 1))), bytearray()
    for i in range(n):
        put(entries, 0x10 * i + 0xc, 'I', NAMES + len(names))
        names += (files.name(r) if r.random() < 0.8 else vfs.name_under(r, root)) + b'\0'
    regions[SET] = s
    regions[ENTRIES] = entries
    regions[NAMES] = bytes(names) + b'\0'


def load(r):
    """0x46ead0 (cdecl; set or 0, probe, any): vfs_case() and texture_set(); the probe 0 mostly."""
    regions, root = vfs_case(r, [0] * 5 + [1, 1])   # (only the first block: a string's failing faults in both)
    texture_set(r, regions, root)
    args = [SET if r.random() < 0.97 else 0, r.choice([0, 0, 1, r.getrandbits(32)]), r.getrandbits(32)]
    return Call(0x46ead0, args, regions)


def for_model(r):
    """0x46e4e0 (cdecl; info, model): vfs_case() (the heap failing its first block only, as later ones would fault in both: the
    directories' block is written unchecked); the info's +8 the root or a folder (a backslash after it or not; empty now and then),
    +0xc a folder, [+0x10]+4 a name (files.py's); the model's [[+0x24]+4] texture_set()'s set or 0; Quickload_Locate's buffer
    0x79c9d0 and directories [0x79cb4c] (64 dwords) as vfs.py's locate()."""
    regions, root = vfs_case(r, [0, 0, 0, 1])
    texture_set(r, regions, root)
    texts = [r.choice([root, root + b'\\', vfs.folder(r), vfs.folder(r) + b'\\', b'']), r.choice([vfs.folder(r), b'', b'x\\']),
             files.name(r)]
    text, ptrs = bytearray(), []
    for t in texts:
        text += r.choice([b'\\', b'a'])
        ptrs.append(INFO + 0x40 + len(text))
        text += t + b'\0'
    info = bytearray(r.bytes(0x40))
    put(info, 8, 'II', ptrs[0], ptrs[1])
    put(info, 0x10, 'I', INFO + 0x30)
    put(info, 0x34, 'I', ptrs[2])
    regions[INFO] = info + text
    model = bytearray(r.bytes(0x30))
    put(model, 0x24, 'I', MODEL + 0x28)
    put(model, 0x2c, 'I', SET if r.random() < 0.9 else 0)
    regions[MODEL] = model
    regions[0x79c9d0] = r.bytes(0x17c)
    regions[0x79cb4c] = put(bytearray(4), 0, 'I', RAM + 0x3d00)
    block = put(bytearray(r.bytes(0x100)), 0, 'I', RAM + 0x3d80)   # the directory [0] a text (the base the directories are set under)
    base = r.choice([b'', b'C:\\TP\\', root + b'\\'])
    block[0x80:0x81 + len(base)] = base + b'\0'
    regions[RAM + 0x3d00] = block
    return Call(0x46e4e0, [INFO, MODEL], regions)


def case(r):
    op = r.choice(OPS)
    if op == 'Load':
        return load(r)
    if op == 'ForModel':
        return for_model(r)
    ram = bytearray(r.bytes(0xa00))
    count = r.choice([0, 1, 3, 4, r.getrandbits(32)])
    globals_ = bytearray(r.bytes(0x14))
    put(globals_, 8, 'I', r.getrandbits(32))  # the chain (only the reset follows it)
    put(globals_, 0xc, 'I', TABLE)
    put(globals_, 0x10, 'I', count)
    indices = bytearray(r.bytes(12))
    for k in range(3):
        put(indices, 4 * k, 'i', r.choice([-1, 0, 1, 2, 3, 3]))
    regions = {RAM: ram, 0x7b0620: globals_, 0x74d980: indices, HEAP: heap(0x2000), **global_heap()}
    if op == 'Set':
        at = STRINGS
        base = r.choice(BASES)
        names = [r.choice([None, text(r), text(r) + b'\\', b'']) for _ in range(3)]
        ptrs = []
        for s in [base] + names:
            if s is None:
                ptrs.append(0)
                continue
            if s == b'':
                put(ram, at - RAM, 'B', r.choice([0x5c, 0x41]))  # the byte before an empty name
                at += 1
            p = at
            put(ram, at - RAM, '%ds' % (len(s) + 1), s + b'\0')
            at += len(s) + 1
            ptrs.append(p)
        olds = []
        for k in range(4):  # the old table: often the very path a name builds
            name = names[r.randrange(3)]
            s = base + (name or b'') + (b'' if name and name.endswith(b'\\') else b'\\')
            s = r.choice([s, s, text(r), base + text(r)])
            put(ram, at - RAM, '%ds' % (len(s) + 1), s + b'\0')
            olds.append(at)
            at += len(s) + 1
        put(ram, TABLE - RAM, '4I', *olds)
        if r.random() < 0.1:  # no table yet (only read through an index)
            put(globals_, 0xc, 'I', 0)
            put(indices, 0, '3i', -1, -1, -1)
        return Call(0x46e670, ptrs, regions)
    if op == 'Reset':
        n = r.choice([0, 1, 2, 4])
        put(globals_, 8, 'I', CHAIN if n else 0)
        for i in range(n):
            put(ram, CHAIN - RAM + 0x10 * i, 'I', CHAIN + 0x10 * (i + 1) if i + 1 < n else 0)
        return Call(0x46ea80, [], regions)
    if op == 'SetCurrent':
        return Call(0x46e640, [r.getrandbits(32)], regions)
    return Call(0x46e610 if op == 'First' else 0x46e630, [], regions, ret='a')


OPS_ALL = ['Set', 'Set', 'Set', 'Reset', 'First', 'Current', 'SetCurrent', 'Load', 'Load', 'ForModel', 'ForModel']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
