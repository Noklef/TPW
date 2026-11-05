# The guarded write file (src/game/core/guardfile: 0x5d4d10, 0x5d4fd0, 0x5d5350) against the originals. Recorders: the CRT's calls and the message pointer at 0x7748f4 (read live, with the mode 0x7748f0, the tags and the count 0xf827f4); strncmp and the guard
# heap's checks real or inlined.
# usage: guardfile.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,Close,Write)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_fold, log_region, put, recorder, set_return, strings  # noqa: E402

MALLOC, FREE, MESSAGE, STAT, FOPEN, FSEEK, FWRITE, FREAD, FFLUSH, FCLOSE, REMOVE, RENAME = range(1, 13)
B = Bench([(0x5d4d10, 0x5d4fcf), (0x5d4fd0, 0x5d534b), (0x5d5350, 0x5d5597), (0x5d4ba0, 0x5d4c11), (0x5d4c20, 0x5d4d03), (0x67c250, 0x67c290)],
          hooks={0x67dcd0: _recorder(MALLOC, 1, cdecl=True), 0x67ad90: _recorder(FREE, 1, cdecl=True), 0x67e900: _recorder(STAT, 2, cdecl=True), 0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67c100: _recorder(FSEEK, 3, cdecl=True), 0x67b6d0: _recorder(FWRITE, 4, cdecl=True),
                 0x67e1d0: _recorder(FREAD, 4, cdecl=True), 0x67d070: _recorder(FFLUSH, 1, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x67e530: _recorder(REMOVE, 1, cdecl=True), 0x67ecc0: _recorder(RENAME, 2, cdecl=True)})
HANDLE, TEXT, TABLE, BASE, DATA = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x4000
BLOCK = BASE + 8
NAMES = [b'a.dat', b'dir/b.c.dat', b'c:\\x\\d', b'noext', b'e.', b'f.g/h', b'a:b', b'.hidden', b'x.y:z', b'x.y\\z']


def tags(r):
    head, tail, freed = r.bytes(4), r.bytes(4), r.bytes(4)
    return head, tail, freed, {0x774934: head, 0x77492c: tail, 0x7749c0: freed, 0xf827f4: struct.pack('<I', r.choice([0, 1, 5, r.getrandbits(32)]))}


def base(r, n):
    log = log_region()
    log_fold(log)
    head, tail, freed, regions = tags(r)
    regions.update({LOG: log, 0x7748f4: struct.pack('<I', recorder(MESSAGE, n, cdecl=True))})
    return log, head, tail, freed, regions


def open_(r):
    log, head, tail, freed, regions = base(r, 2)
    name = r.choice(NAMES + [b'con:', b'con:x', b'CON:'])
    regions[TEXT] = name + b'\0'
    regions[0x7748f0] = struct.pack('<I', r.choice([0, 1, 2, 3, 4]))
    regions[BASE] = r.bytes(0x2430)
    status = bytearray(r.bytes(0x24))
    put(status, 6, 'H', r.choice([0x8180, 0x8100, 0x8000, 0x4180, 0x4100, 0x81ff, r.getrandbits(16)]))
    set_return(log, STAT, r.choice([0, 0, 0xffffffff]))
    fill(log, STAT, 1, bytes(status))
    strings(log, STAT, 1, out=0b10)
    strings(log, FOPEN, 0b11)
    if r.random() < 0.1:
        # No block: the messages' second arguments are no text, so no failed open beside it.
        set_return(log, MALLOC, r.choice([0, 0xffffffff]))
        set_return(log, FOPEN, HANDLE)
    else:
        set_return(log, MALLOC, BASE)
        regions[TABLE] = struct.pack('<2I', *r.choice([[HANDLE, 0], [HANDLE, 0], [0, HANDLE], [0, 0]])) + bytes(0x98)
        answers(log, FOPEN, TABLE)
        strings(log, MESSAGE, 0b10)
    return Call(0x5d4d10, [TEXT], regions, conv='s', ret='a')


def file_block(r, regions, head, tail):
    """A block: its fields any but the cursor, the end and the positions, which follow the buffer's use."""
    block = bytearray(r.bytes(0x2430))
    block[0:4] = head
    put(block, 4, 'I', 0x2424)
    block[0x2424 + 8:0x2424 + 12] = tail
    put(block, 8, 'I', HANDLE)
    return block


def close(r):
    log, head, tail, freed, regions = base(r, 1)
    block = file_block(r, regions, head, tail)
    mode = r.choice([0, 1, 2, 2, 3, 3, 4])
    put(block, 8 + 0x20, 'I', mode)
    put(block, 8 + 0x18, 'I', r.choice([0, 1, 1]))
    put(block, 8 + 0x1c, 'I', r.choice([0, 1, 1]))
    end = BLOCK + 0x424 + r.choice([0, 1, 0x100, 0x2000])
    put(block, 8 + 0x14, 'I', end)
    put(block, 8 + 0x10, 'I', end - r.choice([0, 1]))
    put(block, 8 + 0xc, 'I', r.getrandbits(16))
    name = r.choice(NAMES)
    block[8 + 0x24:8 + 0x24 + len(name) + 1] = name + b'\0'
    working = r.choice([b'a.tm1', b'dir/b.c.tm1'])
    block[8 + 0x224:8 + 0x224 + len(working) + 1] = working + b'\0'
    # The guards: good, or the head, the size, the tail or the freed mark.
    kind = r.choice(['good'] * 6 + ['head', 'freed', 'size', 'big', 'tail'])
    if kind == 'head':
        block[0] ^= 1
    elif kind == 'freed':
        block[0:4] = freed
    elif kind == 'size':
        put(block, 4, 'i', r.choice([0, -1]))
    elif kind == 'big':
        put(block, 4, 'I', 0x6400000)
    elif kind == 'tail':
        block[0x2424 + 8] ^= 1
    regions[BASE] = block
    n = struct.unpack_from('<i', block, 8 + 0x14)[0] - BLOCK - 0x424
    set_return(log, FWRITE, r.choice([n, n, n, n - 1, 0]))
    set_return(log, FCLOSE, r.choice([0, 0, 0, 0xffffffff]))
    set_return(log, RENAME, r.choice([0, 0, 0, 0xffffffff]))
    set_return(log, REMOVE, r.choice([0, 0xffffffff]))
    strings(log, REMOVE, 1)
    strings(log, RENAME, 0b11)
    return Call(0x5d4fd0, [r.choice([BLOCK] * 19 + [0])], regions, conv='s', ret='a')


def write(r):
    log, head, tail, freed, regions = base(r, 1)
    block = file_block(r, regions, head, tail)
    used = r.choice([0, 1, 0x100, 0x1fff, 0x2000])
    cursor = BLOCK + 0x424 + used
    end = cursor + r.choice([0, 0, 0, 1, 0x10]) if used < 0x1fff else cursor
    position = r.choice([0, 0x2000, 0x6000]) + used
    put(block, 8 + 4, 'I', position)
    put(block, 8 + 8, 'I', position + r.choice([0, 0, 1, 0x1000, 0x2001, 0x8000]))
    put(block, 8 + 0xc, 'I', max(position - used, 0))
    put(block, 8 + 0x10, 'II', cursor, end)
    put(block, 8 + 0x18, 'II', r.choice([1, 1, 1, 1, 0]), r.choice([0, 1]))
    regions[BASE] = block
    count = r.choice([0, 1, 0x10, 0x1fff, 0x2000, 0x2001, 0x2100, 0x4000, 0x6000])
    regions[DATA] = r.bytes(0x8000)
    # The writes' answers: the buffer's (if it flushes, with what the first part changes), then the straight one, whole or short. A whole answer is what keeps the rest within the buffer.
    fits = min(0x2000 - used, count)
    after = cursor + max(fits, 0)
    left = max(end, after) - BLOCK - 0x424
    whole = ((position + max(fits, 0)) + (count - fits)) // 0x2000 * 0x2000 - (position + max(fits, 0))
    straight = r.choice([whole, whole, whole, whole - 1] + ([0] if whole > 0 else []))
    flushes = fits > 0 or struct.unpack_from('<I', block, 8 + 0x1c)[0]
    regions[TABLE] = struct.pack('<40i', *(([r.choice([left, left, left - 1])] if flushes else []) + [straight] * 40)[:40])
    answers(log, FWRITE, TABLE)
    set_return(log, FREAD, r.choice([0, 0x10]))
    fill(log, FREAD, 0, r.bytes(0x40))
    return Call(0x5d5350, [r.choice([BLOCK] * 19 + [0]), DATA, count], regions, conv='s', ret='a')


OPS = {'Open': open_, 'Close': close, 'Write': write}


def case(r):
    names = os.environ.get('OPS', 'Open,Close,Write,Write').split(',')
    return OPS[r.choice(names)](r)


if __name__ == '__main__':
    B.main(case)
