# The IME host's candidate window (src/game/core/ime: the raw handler 0x480d10 on the host 0x7b4e38, its handler 0x4809f0, the result's
# insertion 0x480fd0, and the IME's candidate notes 0x47a830 / 0x47a6d0) against the originals. The host at 0x7b4e38: its text window +4
# window705a20.py's text_fixture() (SetSelection's: its text 0-20 characters or none, its size +0x13c about it), its text +0x1a8 unfetched
# (the words 0, 1: ImeHost_OnWindow's) or one of 0-30 characters, the window's cursor / anchor inside the host's text (beyond it the
# original reads stack garbage); the IME +0x1c's result (the string +8's characters +0xc) 0-10 characters or none, its window +0x4c and
# index +0x54 random, its description string +0x28 none or a block of its own, shared or frozen, its +0x168 / +0x16c / +0x184 matching the
# host's +0x14 / +0x10 or not. The imports recorders: ImmGetContext (a context or none), ImmNotifyIME, GetKeyboardLayout,
# ImmGetDescriptionA (filling a NUL-ended description), MultiByteToWideChar (answering from a table: its first call the length, 0-40;
# filling the second's out buffer), ImmReleaseContext, OutputDebugStringA. The candidate window's default handler +0x118 CDECL_LOG4.
# The messages 0x14, 0x400, 0x401, 0x405 and others, a and b about the host's. The host's update 0x481100 (ui/imecandidates): see
# update_case().
# usage: imehost.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SelectCandidate,SetCandidatePage,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
_OPS = os.environ.pop('OPS', None)
import window705a20  # noqa: E402
import window7059f0  # noqa: E402
if _OPS is not None:
    os.environ['OPS'] = _OPS
from fncmp import (CDECL_LOG4, HEAP, IMPORT_TABLE, LOG, LOG_RETURNS, RAM, Bench, Call, Mask, answers, blob_bytes, fill, heap,  # noqa: E402
                   import_table, put, recorder, set_return)
from window705a20 import SEL, SEL_TEXT, WIN_RANGES, text_fixture  # noqa: E402
from window7059f0 import COLS3, HVT, LIST3, LINKS3  # noqa: E402
from window import MANAGER, VTABLE  # noqa: E402

CALLS = {'SelectCandidate': 0x47a830, 'SetCandidatePage': 0x47a6d0, 'InsertResult': 0x480fd0, 'OnMessage': 0x4809f0, 'WindowProc': 0x480d10,
         'Update': 0x481100}
B = Bench(sorted(set(WIN_RANGES + window7059f0.LIST_RANGES + [
    (0x47a830, 0x47a878), (0x47a6d0, 0x47a830), (0x480fd0, 0x4810f6), (0x4809f0, 0x480af9), (0x480d10, 0x480d2f), (0x6b557c, 0x6b55b2),
    (0x47a9a0, 0x47a9b0), (0x47a980, 0x47a99f), (0x66852b, 0x668564), (0x67bed0, 0x67bf10), (0x65f66e, 0x65f697), (0x405280, 0x405390),
    (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
    # (Update's: ranges.py 0x481100, with ime_cache.py's vector helpers)
    (0x47a960, 0x47a990), (0x47a9a0, 0x47abb0), (0x47ac10, 0x47af90), (0x47b300, 0x47b330), (0x47b5b0, 0x47b980), (0x47d620, 0x47d790),
    (0x47db20, 0x47db90), (0x480b00, 0x480d20), (0x480fd0, 0x481390), (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x65c672, 0x65c717),
    (0x65d55a, 0x65d57b), (0x65d773, 0x65d7cc), (0x65d7d0, 0x65d832), (0x65d9dd, 0x65da9d), (0x65f697, 0x65f6e1), (0x663085, 0x663564),
    (0x6636f6, 0x663728), (0x663ba2, 0x663c21), (0x663cb8, 0x663d66), (0x663dfa, 0x6644fa), (0x6649d5, 0x664b78), (0x66525c, 0x6654a4),
    (0x665739, 0x6658c9), (0x668564, 0x66858d), (0x67acd0, 0x67ad00), (0x47ae40, 0x47ae53), (0x47ae60, 0x47b060), (0x47b600, 0x47b750),
    (0x479230, 0x479256), (0x47d9d0, 0x47db00), (0x4791c0, 0x479221)])))
IMPORTS = {'ImmGetContext': (20, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (21, 0x6fd080, 2, 0, 0), 'ImmNotifyIME': (22, 0x6fd068, 4, 0, 0),
           'GetKeyboardLayout': (23, 0x6fd304, 1, 0, 0), 'ImmGetDescriptionA': (24, 0x6fd088, 3, 0, 1 << 1),
           'MultiByteToWideChar': (25, 0x6fd174, 6, 1 << 2, 1 << 4), 'OutputDebugStringA': (26, 0x6fd1e0, 1, 1 << 0, 0)}
HOST = 0x7b4e38
RESULT, NAME_BLOCK, WIDEN, CAND = RAM + 0x50000, RAM + 0x50100, RAM + 0x50200, RAM + 0x50400


def wide(r, n):
    return b''.join(struct.pack('<H', r.choice([r.randrange(1, 0x80), r.randrange(1, 0x10000)])) for _ in range(n))


def fixture(r):
    regions, log, n = text_fixture(r)
    regions[IMPORT_TABLE] = import_table(log, IMPORTS)
    w = regions[SEL]
    host = bytearray(r.bytes(0x3b0))
    put(host, 4, 'I', SEL)
    if r.random() < 0.5:   # unfetched: the window's text is taken (none: an empty one)
        put(host, 0x1a8, 'HH', 0, 1)
        length = n if struct.unpack_from('<I', w, 0x138)[0] else 0
    else:
        length = r.choice([0, 1, 4, 12, 30])
        host[0x1a8:0x1a8 + 2 * length + 2] = wide(r, length) + bytes(2)
    put(w, 0x13c, 'I', r.choice([0, 1, n, n, n + 1, n + 5, 0xff]))
    put(w, 0x14a, 'hh', r.randrange(0, length + 1), r.choice([-1, -1, r.randrange(0, length + 1)]))
    ime = 0x1c
    k = r.choice([0, 1, 3, 10])
    regions[RESULT] = wide(r, k) + bytes(2)
    put(host, ime + 0xc, 'I', r.choice([RESULT] * 5 + [0]))
    put(host, ime + 0x54, 'I', r.choice([0xffffffff, 0, 1, r.getrandbits(32)]))
    count = r.choice([0, 0x7f, 1, 0xff])
    block = bytearray(r.bytes(0x48))
    block[1] = count
    regions[NAME_BLOCK] = block
    put(host, ime + 0x28, 'IIII', r.getrandbits(32), r.choice([0, 0, NAME_BLOCK + 2]), r.randrange(0, 20), 31)
    page = r.choice([0, 1, 5, r.getrandbits(32)])
    put(host, 0x10, 'III', page, r.choice([0, 3, page]), r.choice([0, 9, r.getrandbits(32)]))
    put(host, ime + 0x168, 'II', r.choice([struct.unpack_from('<I', host, 0x14)[0], 0, 7]), r.choice([page, page + 1]))
    host[ime + 0x184] = r.choice([0, 1, 1])
    regions[HOST] = host
    cand = bytearray(r.bytes(0x120))
    put(cand, 0x118, 'I', CDECL_LOG4)
    regions[CAND] = cand
    set_return(log, 20, r.choice([0, 0x1234, 0x1234, r.getrandbits(32)]))
    set_return(log, 23, r.getrandbits(32))
    desc = bytes(r.randrange(0x20, 0x7f) for _ in range(r.choice([0, 3, 20]))) + bytes(1)
    fill(log, 24, 1, desc)
    regions[WIDEN] = struct.pack('<48I', r.choice([0, 1, 3, len(desc), 40]), *[r.choice([0, 1, len(desc)]) for _ in range(47)])
    answers(log, 25, WIDEN)
    fill(log, 25, 4, wide(r, r.choice([0, 2, len(desc) - 1 if len(desc) < 30 else 20])) + bytes(2), 1)
    return regions, host


ENTRIES, STRS, BLOCKS = RAM + 0x60000, RAM + 0x60100, RAM + 0x60400


def strings(r, ram, blocks, which, m):
    """m strings (16 bytes: a random allocator dword, the characters of a counted block of BLOCKS or none, the length, the room) as
    vector `which` of STRS (four, eight strings each), room past them or not; returns (first, last, end, lengths), each length the
    characters' count (0 for none: the text "" (0x6fd6c4) then)"""
    first, lengths = STRS + 0x80 * which, []
    for i in range(m):
        at = 0x40 * (4 * which + i)
        if r.random() < 0.15:
            put(ram, first - STRS + 0x10 * i, '4I', r.getrandbits(32), 0, 0, 0)
            lengths.append(0)
            continue
        n = r.choice([0, 1, 3, 10, 20])
        blocks[at + 1] = r.choice([0, 1, 2, 0xfe])
        blocks[at + 2:at + 4 + 2 * n] = wide(r, n) + bytes(2)
        put(ram, first - STRS + 0x10 * i, '4I', r.getrandbits(32), BLOCKS + at + 2, n, min(0x1d, n + r.choice([0, 0, 2])))
        lengths.append(n)
    return first, first + 0x10 * m, first + 0x10 * min(8, m + r.choice([0, 1, 3])), lengths


def update_masks(fail, ncols, dwords, lengths):
    """Update's items added: the heap offsets of their strings' allocator bytes (a stack byte the original never sets), column 0 a string
    column of these lengths, column 1 not; the array grown first when full (AddItem's Window7059f0_Grow), the heap failing its allocation
    fail (window7059f0.allocator_masks()'s allocator)"""
    nxt, offsets, count, stride = HEAP + 0x10 + fail, [], 0, 4 + ncols

    def alloc(size):
        nonlocal nxt
        countdown, p = nxt & 15, nxt & ~15
        if countdown == 1:
            nxt = p
            return 0
        nxt = p + ((size + 15) & ~15) + (countdown - 1 if countdown else 0)
        return p

    for n in lengths:
        if ncols and count * stride >= dwords:
            grow = (0x14 if dwords == 0 else 0xa) * stride
            if alloc((dwords + grow) * 4):
                dwords += grow
        count += 1
        if ncols:
            p = alloc(0x10)
            if p:
                offsets.append(p - HEAP)
                if n:
                    alloc(((n | 31) + 2) * 2)
    return [Mask(HEAP, offsets, "the new strings' allocator byte: a stack byte the original never sets")] if offsets else []


def update_case(r):
    """Update (0x481100): fixture()'s host and text window with window7059f0.py's list_env() as the list +8 (0-2 columns: the texts are a
    local pair, the string and the count; column 1 not a string column, column 0 one when the strings are cached already), its recorders
    apart from the text window's and the imports' (its objects' slot 3 as id 30, the highlight's as 31, the manager's slot 13 as 29); the
    candidate window +0 CAND (its handler +0x114 the logger) mostly, or none with no strings (its list would be opened: a new window whose
    raw handler 0x480d10 the driver cannot run); +0xc set mostly, +0xd 0 or 1. The IME: no current entry now and then, else of three
    entries one with 0-4 strings: cached already (the mark +0x184 set, +0x160 its first dword, the cache's strings its own, +0x180 its +0x20
    mostly) or not (no mark, or another first dword; the old cache's strings any); the position +0x16c about the count, +0x10 it or not."""
    regions, host = fixture(r)
    log = regions[LOG]
    for _ in range(32):
        window, lregions, idx = window7059f0.list_env(r)
        ncols = struct.unpack_from('<h', window, 0x164)[0]
        if ncols <= 2:
            break
    llog = lregions.pop(LOG)
    lregions.pop(HEAP)
    put(lregions[VTABLE], 0xc, 'I', recorder(30, 2))
    put(lregions[HVT], 0xc, 'I', recorder(31, 2))
    blob_bytes(log, 31, 1, 0, (0x14, 0xfff))
    if MANAGER + 0x40 in lregions:
        lregions[MANAGER + 0x40] = put(bytearray(lregions[MANAGER + 0x40]), 4 * 13, 'I', recorder(29, 1))
        log[LOG_RETURNS + 8 * 29:LOG_RETURNS + 8 * 30] = llog[LOG_RETURNS + 8 * 13:LOG_RETURNS + 8 * 14]
    regions.update(lregions)
    cand = r.choice([CAND] * 3 + [0])
    put(host, 0, 'I', cand)
    host[0xc] = r.choice([1] * 6 + [0, r.randrange(1, 0x100)])
    host[0xd] = r.choice([0, 1])
    put(host, 8, 'I', LIST3)
    c = regions[CAND]
    c[4] = r.choice([0] * 6 + [1])
    put(c, 0x114, 'I', r.choice([CDECL_LOG4] * 6 + [0]))
    # the IME
    ime = 0x1c
    ram, blocks, entries = bytearray(r.bytes(0x200)), bytearray(r.bytes(0x400)), bytearray(r.bytes(0x24 * 3))
    index = -1 if r.random() < 0.1 else r.randrange(3)
    cached = r.random() < 0.65
    m = r.choice([0, 1, 1, 2, 3, 4]) if cand else 0
    key = r.getrandbits(32)
    e = 0x24 * max(index, 0)
    put(entries, e, 'I', key)
    put(host, ime + 0x3c, 'I', ENTRIES)
    put(host, ime + 0x54, 'i', index)
    lengths = []
    if cached:
        first, last, end, lengths = strings(r, ram, blocks, 0, m)
        put(host, ime + 0x170, '4I', r.getrandbits(32), first, last, end)
        put(entries, e + 0x10, '4I', r.getrandbits(32), *strings(r, ram, blocks, 1, r.choice([0, 2]))[:3])
        put(host, ime + 0x160, 'I', key)
        host[ime + 0x184] = r.choice([1, 1, 1, 0x80])
        put(host, ime + 0x180, 'I', struct.unpack_from('<I', entries, e + 0x20)[0] if r.random() < 0.7 else r.getrandbits(32))
        position = r.choice([0, 1, m - 1, m, -1, r.getrandbits(32)]) & 0xffffffff
        put(host, ime + 0x16c, 'I', position)
    else:
        put(entries, e + 0x10, '4I', r.getrandbits(32), *strings(r, ram, blocks, 1, m)[:3])
        put(host, ime + 0x170, '4I', r.getrandbits(32), *strings(r, ram, blocks, 0, r.choice([0, 1, 3]))[:3])
        if r.random() < 0.5:
            host[ime + 0x184] = 0
        else:
            host[ime + 0x184] = 1
            put(host, ime + 0x160, 'I', key ^ (1 << r.randrange(32)))
        position = r.choice([0, 1, m - 1, m, -1, r.getrandbits(32)]) & 0xffffffff
        put(entries, e + 0xc, 'I', position)
    put(host, ime + 0x188, 'I', r.choice([0, r.getrandbits(32)]))
    put(host, 0x10, 'I', r.choice([position, position, 0, r.getrandbits(32)]))
    regions[ENTRIES], regions[STRS], regions[BLOCKS] = entries, ram, blocks
    # the list: column 0 a string column only when the strings are cached already (the items' strings' allocations then come first)
    bottoms = [r.choice([0, 0, 1, -1]) if cached else r.choice([1, -1]), r.choice([1, 1, -1]), 0, 0]
    regions[COLS3] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in bottoms)
    count = struct.unpack_from('<i', window, 0x150)[0]
    links = struct.unpack('<%di' % (len(regions[LINKS3 - 0x20]) // 4), regions[LINKS3 - 0x20])[8:]
    item, reach = struct.unpack_from('<i', window, 0x160)[0], 0
    while item >= 0 and reach < 64:   # (Clear's walk: past the items it reaches it reads cells as links; window7059f0.py's list_case)
        reach += 1
        if links[item + 2] >= 0 or links[item] >= 0:
            item = links[item + 2] if links[item + 2] >= 0 else links[item]
        elif links[item + 3] < 0:
            item = -2
        else:
            item = links[item + 3]
            for _ in range(64):
                if not (links[item] < 0 and links[item + 3] >= 0):
                    break
                item = links[item + 3]
            item = links[item]
    if r.random() < 0.9:
        put(window, 0x150, 'i', min(count, reach))
    if struct.unpack_from('<I', window, 0x144)[0] and not struct.unpack_from('<i', window, 0x158)[0]:   # (the dwords the array's: with
        put(window, 0x158, 'i', len(links))   # 0, Clear's ResetItems resets no item and AddItem's RootEnd walks the old links, which can cycle)
    if not struct.unpack_from('<i', window, 0x150)[0]:   # (no items: Clear returns at once, so the links an empty list's, as ResetItems leaves
        put(window, 0x160, 'i', -1)                      # them: else AddItem's RootEnd walks random ones, which can cycle)
        arr = bytearray(regions[LINKS3 - 0x20])
        stride = ncols + 4
        n = struct.unpack_from('<i', window, 0x158)[0] // stride
        for i in range(max(n, 1)):
            put(arr, 0x20 + 4 * stride * i, 'i', stride * (i + 1) if i < n - 1 else -1)
            put(arr, 0x20 + 4 * (stride * i + 2), 'ii', -1, -2)
        regions[LINKS3 - 0x20] = arr
    fail = r.choice([0] * 12 + [1, 2, 3])
    regions[HEAP] = heap(0x4000, fail)
    masks = []
    same = struct.unpack_from('<I', entries, e + 0x20) == struct.unpack_from('<I', host, ime + 0x180)
    if host[0xc] and cand and cached and index >= 0 and same and bottoms[0] == 0:   # (the items added)
        masks = update_masks(fail, ncols, struct.unpack_from('<i', window, 0x158)[0], lengths)
    return Call(CALLS['Update'], [HOST], regions, conv='t', ret='v', masks=masks)


def case(r):
    op = r.deal(OPS)
    if op == 'Update':
        return update_case(r)
    regions, host = fixture(r)
    if op == 'SelectCandidate':
        return Call(CALLS[op], [HOST + 0x1c, r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'SetCandidatePage':
        first = r.choice([0, 1, 5, r.getrandbits(32)])
        return Call(CALLS[op], [HOST + 0x1c, first, r.choice([first, first + 9, r.getrandbits(32)]), r.getrandbits(32)], regions, conv='t',
                    ret='v')
    if op == 'InsertResult':
        return Call(CALLS[op], [HOST], regions, conv='t', ret='v')
    message = r.choice([0x14, 0x400, 0x401, 0x401, 0x405, 0x405, 0x13, 0x402, 0x404, r.getrandbits(32)])
    a = r.choice([struct.unpack_from('<I', host, 0x14)[0], 0, 2, r.getrandbits(32)])
    b = r.choice([struct.unpack_from('<I', host, 0x10)[0], 0, 4, r.getrandbits(32)])
    if op == 'OnMessage':
        return Call(CALLS[op], [HOST, CAND, message, a, b], regions, conv='t', ret='a')
    return Call(CALLS[op], [CAND, message, a, b], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
