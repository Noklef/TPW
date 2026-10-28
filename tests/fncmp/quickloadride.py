# A ride's quickload indexes loaded and its texture directories set (0x461a60, src/game/files/quickload) against the original: texdirs.py's
# vfs_case() (vfs.py's mounts, files.py's quickload tables at RAM + 0x40000 loaded or not, the texture directories) and vfs.py's memory
# source, every file it opens a qickload.txt text (names apart by blanks, now and then a long one or odd characters; its size answered as
# the text's mostly); the opens all answered 1, or the first one or two only (the ride located, then one index or none), or none (not
# located); the directories [0x79cb4c] (the first a path, now and then 0) and the second directory's name 0x7af650; the ride's name with
# no '%' (with no folder it is sprintf's format, sic) and its folder (not empty: an empty one's check reads the byte before it, stack
# garbage; now and then 0, which faults in both once located).
# usage: quickloadride.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import files  # noqa: E402
import texdirs  # noqa: E402
import vfs  # noqa: E402
from fncmp import Bench, Call, LOG, RAM, fail_after, set_return  # noqa: E402

RANGES = texdirs.merged(vfs.RANGES + [
    (0x41ce90, 0x41ceb0), (0x4273e0, 0x427460), (0x4274e0, 0x427630), (0x4618a0, 0x461930), (0x461a60, 0x461ee0), (0x46e640, 0x46e660),
    (0x46e670, 0x46ea90), (0x5c4d50, 0x5c4fb0), (0x5c5000, 0x5c5020), (0x5f60b0, 0x5f6160), (0x5f61a0, 0x5f6210), (0x5f7180, 0x5f71a0),
    (0x67a400, 0x690000), (0x5da3c0, 0x5da3c1)])
B = Bench(RANGES)
ARGS, DIRECTORIES, SECOND = RAM + 0x68000, RAM + 0x3d00, 0x7af650
SEPARATORS = [b' ', b'\r\n', b'\n', b'\t', b'  ', b'\r\n\r\n', b' \t']


def quick_text(r):
    words = []
    for _ in range(r.choice([0, 1, 2, 5, r.randrange(30)])):
        k = r.random()
        if k < 0.75:
            words.append(files.name(r).replace(b' ', b'_') or b'x')
        elif k < 0.85:
            words.append(bytes(r.choice(b'abc.\\_-#;!') for _ in range(r.randrange(1, 8))))
        else:
            words.append(b'r' * r.choice([0xff, 0x100, 0x101, 0x130]))
    text = b''.join(w + r.choice(SEPARATORS) for w in words)
    if r.random() < 0.3:
        text = r.choice(SEPARATORS) + text
    if r.random() < 0.3 and text:
        text = text.rstrip()
    return text


def case(r):
    regions, root = texdirs.vfs_case(r, [0] * 12 + [1])
    log = regions[LOG]
    data = quick_text(r)
    vfs.memory_source(r, log, regions, data)
    set_return(log, 8, r.choice([len(data)] * 6 + [0, len(data) // 2, len(data) + 0x40]))   # (a huge size: the heap stub zeroes past its end)
    k = r.random()
    if k < 0.15:
        set_return(log, 2, 0)   # (not located)
    elif k < 0.4:
        fail_after(log, 2, r.choice([1, 2]), 0)
    top = DIRECTORIES + 0x40
    path = r.choice([root, b'C:\\TP\\' + root, vfs.folder(r)])
    put_dirs = struct.pack('<I', 0 if r.random() < 0.05 else top)
    regions[DIRECTORIES] = put_dirs + r.bytes(0x3c) + path + b'\0'
    regions[0x79cb4c] = struct.pack('<I', DIRECTORIES)
    regions[0x79c9d0] = r.bytes(0x17c)
    regions[SECOND] = r.choice([b'second', b'Data\\B', b'x', b'', b'b\\']) + b'\0'
    name = files.name(r).replace(b'%', b'a')
    folder = r.choice([vfs.folder(r), root, b'rides\\' + name, b'a\\']) or b'rides'
    third = r.choice([b'tex', b'', b'c\\', None])
    text = b'\\' + name + b'\0' + folder + b'\0' + (third or b'') + b'\0'
    regions[ARGS] = text + r.bytes(8)
    args = [ARGS + 1, 0 if r.random() < 0.03 else ARGS + 2 + len(name), 0 if third is None else ARGS + 3 + len(name) + len(folder)]
    return Call(0x461a60, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
