# libjpeg 6b's jdmarker.c, the marker reader (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b) against the exe's copies.
# cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder (slot 0), emit_message +4 another (slot 1, (cinfo, level)), msg_code +0x14,
# msg_parm +0x18, trace_level +0x68 (0..2: get_dqt's TRACEMS8 rows at 2); mem +4 at MEM: alloc_small ZALLOC over the case heap (its
# (cinfo, pool, size) is ZALLOC's (opaque, items, size): JPOOL_PERMANENT, pool 0, asks for 0 bytes, so jpeg_alloc_huff_table /
# _quant_table's tables all land on the heap's first block, the same in both); src +0x18 at SRC: next_input_byte, bytes_in_buffer,
# fill_input_buffer +0xc a cdecl recorder (slot 2) answering TRUE or FALSE, skip_input_data +0x10 (slot 3, (cinfo, num_bytes)),
# resync_to_restart +0x14 (slot 4, (cinfo, desired)); the tables' pointers quant_tbl_ptrs[4] +0x90, dc_ / ac_huff_tbl_ptrs[4]
# +0xa0 / +0xb0 at QTBL / HTBL or null; comp_info +0xc4 at COMP; unread_marker +0x17c; marker +0x194 at MARKER (my_marker_reader: saw_SOI
# +0xc, saw_SOF +0xd, next_restart_num +0x10, discarded_bytes +0x14, process_COM +0x18 a cdecl recorder (slot 6), process_APPn[16] +0x1c
# another (slot 5), each answering TRUE or FALSE).
# The source is the game's (see docs/porting.md), so the recorder's "refill" moves nothing: the code reloads next_input_byte and
# bytes_in_buffer as they were at its last INPUT_SYNC, the same in both. The bytes at BUF are a marker's segment (from its length on, the
# marker in unread_marker) or a stream of segments for the scanning functions, then EOI markers. A case whose bytes_in_buffer stops
# short of the segment's end has fill_input_buffer answer FALSE (the function suspends): a TRUE there would reread from the last sync,
# where next_marker can loop for ever on FF fill bytes and get_dht / get_dqt / get_sos can read a count past their arrays. TRUE comes
# with the whole segment in the buffer (and, for get_dri, skip_variable and first_marker, a reread from the start, which is bounded).
# The marker streams for read_markers are well formed (lengths that match, no 0xFF inside COM / APPn / DNL payloads, every Huffman
# table 256 values) so the code's reading of them is the one modelled here; the single-segment cases bend them (bad lengths, counts,
# indexes, ids).
# Mask (get_dht only): get_dht reads count values into a local huffval[256] and copies all 256 into the table (MEMCOPY), so the table's
# huffval past the largest count read so far in that call is stack garbage, different in the two runs. dht_writes() models the call's
# reads (from the segment's length on, with the suspension) and masks those bytes of each table it copies into, its last copy's. Nothing
# reads them: get_dht's listing (0x63cb60..0x63cee0) writes the table with REP MOVSD from the local and never reads a table, and
# jpeg_alloc_huff_table (0x63e090) writes sent_table +0x111 only; the emulator fails a case if the exe's code reads one.
# usage: jpeg6b_marker.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=reset,jinit,soi,sof,sos,dht,dqt,dri,skip,appn,app0,
# app14,next,first,read,restart,resync)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, Mask, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); the table allocators are jcomapi.c's
FUNCS = {'jpeg_resync_to_restart': (0x63b8f0, 0x63b9f0), 'next_marker': (0x63b9f0, 0x63bb10), 'jinit_marker_reader': (0x63bb10, 0x63bb80),
         'get_interesting_appn': (0x63bb80, 0x63bcc0), 'examine_app0': (0x63bcc0, 0x63bf50), 'examine_app14': (0x63bf50, 0x63c030),
         'skip_variable': (0x63c030, 0x63c0e0), 'read_markers': (0x63c0e0, 0x63c470), 'get_soi': (0x63c470, 0x63c530),
         'get_sof': (0x63c530, 0x63c850), 'get_sos': (0x63c850, 0x63cb60), 'get_dht': (0x63cb60, 0x63cee0), 'get_dqt': (0x63cee0, 0x63d170),
         'get_dri': (0x63d170, 0x63d260), 'first_marker': (0x63d260, 0x63d300), 'read_restart_marker': (0x63d300, 0x63d3a0),
         'reset_marker_reader': (0x63d3a0, 0x63d3d0), 'jpeg_alloc_quant_table': (0x63e070, 0x63e090),
         'jpeg_alloc_huff_table': (0x63e090, 0x63e0b0)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, SRC, MARKER, COMP = RAM, RAM + 0x200, RAM + 0x290, RAM + 0x2c0, RAM + 0x300, RAM + 0x400
HTBL, QTBL, BUF, DATA = RAM + 0x800, RAM + 0x1100, RAM + 0x1400, RAM + 0x2000
K_ERROR, K_EMIT, K_FILL, K_SKIP, K_RESYNC, K_APPN, K_COM = 0, 1, 2, 3, 4, 5, 6
QUANT_PTRS, DC_PTRS, AC_PTRS, COMP_INFO, UNREAD_MARKER, MARKER_ = 0x90, 0xa0, 0xb0, 0xc4, 0x17c, 0x194
HTBL_STRIDE, QTBL_STRIDE, COMP_SIZE = 0x114, 0x84, 0x54
M_SOF0, M_DHT, M_SOI, M_EOI, M_SOS, M_DQT, M_DRI, M_APP0, M_COM, M_RST0 = 0xc0, 0xc4, 0xd8, 0xd9, 0xda, 0xdb, 0xdd, 0xe0, 0xfe, 0xd0
TAIL = b'\xff\xd9' * 8  # (after the bytes in the buffer: a reread past them meets EOI)


def common(r, marker=None, tables=False):
    """tables: the table pointers at QTBL / HTBL or null (get_dht, get_dqt, read_markers), else never read."""
    log = log_region()
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, 0x18, SRC)
    struct.pack_into('<I', cinfo, MARKER_, MARKER)
    if tables:
        struct.pack_into('<4I', cinfo, QUANT_PTRS, *[0 if r.random() < 0.3 else QTBL + QTBL_STRIDE * i for i in range(4)])
        struct.pack_into('<8I', cinfo, DC_PTRS, *[0 if r.random() < 0.3 else HTBL + HTBL_STRIDE * i for i in range(8)])
    struct.pack_into('<i', cinfo, UNREAD_MARKER, 0 if marker is None else marker)
    err = bytearray(struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x7c))
    struct.pack_into('<i', err, 0x68, r.choice([0, 1, 2, 2]))
    m = bytearray(r.bytes(0xa8))
    m[0xc], m[0xd] = r.choice([0, 1]), r.choice([0, 1])
    struct.pack_into('<ii', m, 0x10, r.randrange(8), r.choice([0, 0, r.randrange(1, 100)]))
    struct.pack_into('<I', m, 0x18, recorder(K_COM, 1, cdecl=True))
    struct.pack_into('<16I', m, 0x1c, *[recorder(K_APPN, 1, cdecl=True)] * 16)
    set_return(log, K_APPN, r.choice([1, 1, 1, 0]))
    set_return(log, K_COM, r.choice([1, 1, 0]))
    set_return(log, K_RESYNC, r.choice([1, 1, 0]))
    regions = {CINFO: cinfo, ERR: err, MEM: struct.pack('<I', ZALLOC) + r.bytes(0x1c), MARKER: m, LOG: log, HEAP: heap(0x400)}
    if tables:
        regions.update({HTBL: r.bytes(HTBL_STRIDE * 8), QTBL: r.bytes(QTBL_STRIDE * 4)})
    return regions


def source(r, regions, data, need=None, reread=False):
    """data at BUF, bytes_in_buffer the whole of it or short of need (then a refill fails; reread: or succeeds, rereading)."""
    need = len(data) if need is None else need
    n = len(data)
    fill_ok = True
    if r.random() < 0.3:
        n = r.randrange(0, need) if need else 0
        fill_ok = reread and n > 0 and r.random() < 0.5
    set_return(regions[LOG], K_FILL, int(fill_ok))
    regions[BUF] = bytes(data) + TAIL
    regions[SRC] = struct.pack('<II', BUF, n) + r.bytes(4) + struct.pack('<III', recorder(K_FILL, 1, cdecl=True),
                                                                          recorder(K_SKIP, 2, cdecl=True),
                                                                          recorder(K_RESYNC, 2, cdecl=True)) + r.bytes(4)
    return n, fill_ok


def seg(marker, payload):
    return bytes([0xff, marker]) + struct.pack('>H', len(payload) + 2) + bytes(payload)


def comps(r, regions, n, ids=None):
    """comp_info: n entries (and one past them, which get_sos writes when no id matches), ids distinct."""
    ids = ids or r.sample(range(256), n)
    c = bytearray(r.bytes(COMP_SIZE * (n + 1)))
    for i in range(n):
        c[i * COMP_SIZE] = 0
        struct.pack_into('<i', c, i * COMP_SIZE, ids[i])
    regions[COMP] = c
    struct.pack_into('<I', regions[CINFO], COMP_INFO, COMP)
    struct.pack_into('<i', regions[CINFO], 0x24, n)
    return ids


# --- payloads (from the length on) ---

def sof_payload(r, ncomp, good=True):
    p = bytearray([8 if good or r.random() < 0.7 else r.choice([12, 0])])
    p += struct.pack('>HH', r.choice([1, 8, 600, r.randrange(1, 0x10000)]) if good or r.random() < 0.8 else 0,
                     r.choice([1, 16, 800, r.randrange(1, 0x10000)]) if good or r.random() < 0.8 else 0)
    p.append(ncomp)
    for i in range(ncomp):
        p += bytes([r.randrange(256), r.choice([0x11, 0x22, 0x21, r.randrange(256)]), r.randrange(4) if good else r.randrange(256)])
    length = len(p) + 2 if good or r.random() < 0.7 else r.randrange(0, 40)
    return struct.pack('>H', length) + p


def sos_payload(r, ids, n=None, good=True):
    n = r.randrange(1, min(4, len(ids)) + 1) if n is None else n
    p = bytearray([n])
    for i in range(n):
        p += bytes([ids[i % len(ids)] if ids and (good or r.random() < 0.8) else r.randrange(256), r.choice([0x00, 0x11, r.randrange(256)])])
    p += bytes([r.choice([0, 1, r.randrange(64)]), r.choice([63, 0, r.randrange(64)]), r.choice([0, 0x10, 0x01, r.randrange(256)])])
    length = len(p) + 2 if good or r.random() < 0.7 else r.randrange(0, 20)
    return struct.pack('>H', length) + p


def huff_table(r, full=False):
    """index, bits[16], values: a count of 256 when full, else 0..256 (most small)."""
    if full:
        total = 256
    else:
        total = r.choice([0, 1, 12, 162, 256, r.randrange(0, 257), r.randrange(0, 40)])
    bits = [0] * 16
    for _ in range(total):
        bits[r.randrange(16)] += 1
    if any(b > 255 for b in bits):
        bits = [16] * 16
    return bytes(bits), bytes(r.randrange(256) for _ in range(sum(bits)))


def dht_payload(r, full=False, good=True, tables=None):
    p = bytearray()
    for _ in range(tables or r.choice([1, 1, 2, 4])):
        index = r.choice([0, 1, 2, 3, 0x10, 0x11, 0x12, 0x13]) if good or r.random() < 0.8 else r.randrange(4, 8)
        bits, vals = huff_table(r, full)
        p += bytes([index]) + bits + vals
    length = len(p) + 2
    if not good and r.random() < 0.4:
        length = r.randrange(0, length)  # (short: the loop ends early, a count past the length, or JERR_BAD_LENGTH)
    return struct.pack('>H', length) + p


def dqt_payload(r, good=True):
    p = bytearray()
    for _ in range(r.choice([1, 1, 2, 4])):
        n = r.randrange(4) if good or r.random() < 0.7 else r.randrange(4, 12)
        prec = r.choice([0, 0, 1]) if good or r.random() < 0.8 else r.randrange(2, 16)
        p.append(prec << 4 | n)
        for _ in range(64):
            p += struct.pack('>H', r.randrange(1, 0x10000)) if prec else bytes([r.randrange(1, 256)])
    length = len(p) + 2
    if not good and r.random() < 0.4:
        length = r.randrange(0, length)
    return struct.pack('>H', length) + p


def app_data(r, marker):
    k = r.random()
    if marker == M_APP0 and k < 0.4:
        d = bytearray(b'JFIF\x00' + bytes([r.choice([1, 1, 2]), r.randrange(3), r.randrange(3)]) + r.bytes(4) +
                      bytes([r.choice([0, 0, 2, r.randrange(256)]), r.choice([0, 0, 3, r.randrange(256)])]))
        d += r.bytes(r.choice([0, 0, 18, r.randrange(0, 30)]))
    elif marker == M_APP0 and k < 0.7:
        d = bytearray(b'JFXX\x00' + bytes([r.choice([0x10, 0x11, 0x13, r.randrange(256)])]) + r.bytes(r.randrange(0, 12)))
    elif marker == M_APP0 + 14 and k < 0.6:
        d = bytearray(b'Adobe' + r.bytes(7) + r.bytes(r.choice([0, 0, r.randrange(0, 10)])))
    else:
        d = bytearray(r.bytes(r.choice([0, 3, 14, r.randrange(0, 30)])))
    if r.random() < 0.15 and len(d) > 2:
        d = d[:r.randrange(len(d))]  # (cut inside the signature or the header)
    return d


def no_ff(data):
    return bytes(b if b != 0xff else 0xfe for b in data)


# --- the operations ---

def reset(r):
    return Call(0x63d3a0, [CINFO], common(r))


def jinit(r):
    return Call(0x63bb10, [CINFO], common(r))


def soi(r):
    return Call(0x63c470, [CINFO], common(r, M_SOI), ret='b')


def sof(r):
    regions = common(r, r.choice([0xc0, 0xc1, 0xc2, 0xc9, 0xca]))
    ncomp = r.choice([1, 3, 3, 4, r.randrange(0, 11)])
    good = r.random() < 0.7
    comps(r, regions, max(ncomp, 1))
    if r.random() < 0.5:
        struct.pack_into('<I', regions[CINFO], COMP_INFO, 0)  # (allocated: JPOOL_IMAGE, on the heap)
        del regions[COMP]
    source(r, regions, sof_payload(r, ncomp, good))
    w = r.randrange(256) << 8 if r.wild else 0
    return Call(0x63c530, [CINFO, r.choice([0, 1]) | w, r.choice([0, 1]) | w], regions, ret='b')


def sos(r):
    regions = common(r, M_SOS)
    ids = comps(r, regions, r.choice([1, 3, 4, r.randrange(1, 6)]))
    good = r.random() < 0.7
    n = None if good or r.random() < 0.5 else r.choice([0, 5])
    source(r, regions, sos_payload(r, ids, n, good))
    return Call(0x63c850, [CINFO], regions, ret='b')


def dht_writes(regions, data, n, fill_ok):
    """get_dht's table copies: {table address: the huffval bytes past what the call's reads set} for the last copy into each."""
    cinfo = regions[CINFO]
    ptrs = list(struct.unpack_from('<8I', cinfo, DC_PTRS))
    pos, left = 0, n

    def byte():
        nonlocal pos, left
        if left == 0:
            if not fill_ok:
                raise EOFError
            pos, left = 0, n  # (a refill rereads from the call's start: get_dht syncs only at its end)
        left -= 1
        pos += 1
        return data[pos - 1] if pos - 1 < len(data) else 0

    writes, known = {}, 0
    try:
        length = byte() << 8
        length += byte()
        length -= 2
        while length > 16:
            index = byte()
            count = sum(byte() for _ in range(16))
            length -= 17
            if count > 256:
                return None  # (past huffval: a case the bench does not make)
            for _ in range(count):
                byte()
            length -= count
            known = max(known, count)
            slot = (4 + index - 0x10) if index & 0x10 else index
            if not 0 <= slot < 8:
                return None
            if ptrs[slot] == 0:
                ptrs[slot] = HEAP + 0x10
            writes[ptrs[slot]] = known
    except EOFError:
        pass
    return writes


def dht(r):
    for _ in range(20):
        regions = common(r, M_DHT, True)
        payload = dht_payload(r, False, r.random() < 0.7)
        n, fill_ok = source(r, regions, payload)
        writes = dht_writes(regions, payload, n, fill_ok)
        if writes is not None:
            break
    masks = []
    for table, known in writes.items():
        region, base = (HEAP, table - HEAP) if table >= HEAP and table < HEAP + 0x1000 else (HTBL, table - HTBL)
        if known < 256:
            masks.append(Mask(region, range(base + 0x11 + known, base + 0x111),
                              'get_dht copies all of its local huffval[256]; past the counts read it is stack garbage (header)'))
    return Call(0x63cb60, [CINFO], regions, ret='b', masks=masks)


def dqt(r):
    regions = common(r, M_DQT, True)
    source(r, regions, dqt_payload(r, r.random() < 0.7))
    return Call(0x63cee0, [CINFO], regions, ret='b')


def dri(r):
    regions = common(r, M_DRI)
    payload = struct.pack('>HH', 4 if r.random() < 0.8 else r.randrange(0, 8), r.randrange(0x10000))
    source(r, regions, payload, reread=True)
    return Call(0x63d170, [CINFO], regions, ret='b')


def skip(r):
    regions = common(r, r.choice([0xcc, 0xdc, M_COM, 0xe5]))
    source(r, regions, struct.pack('>H', r.choice([0, 1, 2, 3, 20, r.randrange(0x10000)])), reread=True)
    return Call(0x63c030, [CINFO], regions, ret='b')


def appn(r):
    marker = r.choice([M_APP0, M_APP0, M_APP0 + 14, M_APP0 + 14, M_APP0 + r.randrange(16)])
    regions = common(r, marker)
    d = app_data(r, marker)
    length = len(d) + 2 if r.random() < 0.8 else r.choice([0, 1, 2, r.randrange(0, 40)])
    source(r, regions, struct.pack('>H', length) + d, reread=True)
    return Call(0x63bb80, [CINFO], regions, ret='b')


def app0(r):
    regions = common(r)
    d = app_data(r, M_APP0)
    regions[DATA] = bytes(d) + r.bytes(0x20 - len(d)) if len(d) < 0x20 else bytes(d[:0x20])
    datalen = min(len(d), 14) if r.random() < 0.8 else r.randrange(0, 15)
    return Call(0x63bcc0, [CINFO, DATA, datalen, r.choice([0, 0, 18, r.randrange(-5, 1000)])], regions)


def app14(r):
    regions = common(r)
    d = app_data(r, M_APP0 + 14)
    regions[DATA] = bytes(d) + r.bytes(0x20 - len(d)) if len(d) < 0x20 else bytes(d[:0x20])
    datalen = min(len(d), 14) if r.random() < 0.8 else r.randrange(0, 15)
    return Call(0x63bf50, [CINFO, DATA, datalen, r.choice([0, 0, r.randrange(-5, 1000)])], regions)


def junk(r, n, markers=True):
    out = bytearray()
    while len(out) < n:
        k = r.random()
        if k < 0.7 or (k >= 0.85 and not markers):
            out.append(r.randrange(0, 0xff))
        elif k < 0.85:
            out += b'\xff\x00'
        else:
            out += b'\xff' * r.randrange(1, 4) + bytes([r.randrange(1, 0xff)])
    return bytes(out[:n])


def marker_bytes(r):
    """Junk (some), FF fill (some), then a marker."""
    return junk(r, r.choice([0, 0, 1, 5, r.randrange(0, 12)])) + b'\xff' * r.choice([1, 1, 2, 3]) + bytes([r.choice(
        [M_SOI, M_SOS, M_RST0 + r.randrange(8), 0xc0, 0x01, r.randrange(1, 0xff)])])


def next_(r):
    regions = common(r)
    source(r, regions, marker_bytes(r))
    return Call(0x63b9f0, [CINFO], regions, ret='b')


def first(r):
    regions = common(r)
    data = b'\xff\xd8' if r.random() < 0.7 else bytes([r.choice([0xff, r.randrange(256)]), r.choice([0xd8, r.randrange(256)])])
    source(r, regions, data, reread=True)
    return Call(0x63d260, [CINFO], regions, ret='b')


def stream(r):
    """A well-formed marker stream (see the header), ending in SOS or EOI; and the components' ids."""
    out = bytearray()
    ids = r.sample(range(256), 4)
    ncomp = r.choice([1, 3, 3, 4])
    if r.random() < 0.9:
        out += b'\xff\xd8'
    for _ in range(r.choice([1, 2, 4, 6, r.randrange(0, 10)])):
        k = r.randrange(12)
        if k == 0:
            out += b'\xff' + bytes([r.choice([0xc0, 0xc1, 0xc2, 0xc9, 0xca])]) + sof_frame(r, ids, ncomp)
        elif k == 1:
            out += seg(M_DHT, dht_payload(r, True, tables=1)[2:])
        elif k == 2:
            out += seg(M_DQT, dqt_payload(r)[2:])
        elif k == 3:
            out += seg(M_DRI, struct.pack('>H', r.randrange(0x10000)))
        elif k in (4, 5):
            m = M_APP0 + r.choice([0, 14, r.randrange(16)])
            p = no_ff(app_data(r, m))
            out += b'\xff' + bytes([m]) + struct.pack('>H', len(p) + 2) + p
        elif k == 6:
            out += b'\xff\xfe' + no_ff(struct.pack('>H', 2 + 5) + r.bytes(5))
        elif k == 7:
            out += b'\xff' + bytes([r.choice([0xcc, 0xdc])]) + no_ff(struct.pack('>H', r.choice([2, 4, 6])) + r.bytes(4))
        elif k == 8:
            out += b'\xff' + bytes([r.choice([M_RST0 + r.randrange(8), 0x01])])
        elif k == 9:
            out += junk(r, r.randrange(1, 6), markers=False) + b'\xff' * r.randrange(0, 3)
        elif k == 10:
            out += b'\xff' + bytes([r.choice([0xc3, 0xc5, 0xcb, 0x02, 0xf0])])
        else:
            out += b'\xff\xd8'
    if r.random() < 0.7:
        out += b'\xff\xda' + sos_payload(r, ids[:ncomp])
    else:
        out += b'\xff\xd9'
    return bytes(out), ids, ncomp


def sof_frame(r, ids, ncomp):
    p = bytearray([8]) + struct.pack('>HH', r.choice([8, 600, r.randrange(1, 0x10000)]), r.choice([16, 800, r.randrange(1, 0x10000)]))
    p.append(ncomp)
    for i in range(ncomp):
        p += bytes([ids[i], r.choice([0x11, 0x22, 0x21]), r.randrange(4)])
    return no_ff(struct.pack('>H', len(p) + 2)) + bytes(p)


def read(r):
    regions = common(r, tables=True)
    data, ids, ncomp = stream(r)
    comps(r, regions, 4, ids)  # (a SOS before any SOF looks its ids up there; get_sof's allocation is the sof op's)
    struct.pack_into('<i', regions[CINFO], 0x24, r.choice([ncomp, 4]))
    regions[MARKER][0xc] = r.choice([0, 0, 1])
    regions[MARKER][0xd] = r.choice([0, 0, 1])
    if r.random() < 0.2:
        struct.pack_into('<i', regions[CINFO], UNREAD_MARKER, r.choice([M_SOI, 0xd3]))
    source(r, regions, data)
    return Call(0x63c0e0, [CINFO], regions, ret='a')


def restart(r):
    regions = common(r)
    nrn = regions[MARKER][0x10] & 7
    struct.pack_into('<i', regions[CINFO], UNREAD_MARKER, r.choice([0, 0, M_RST0 + nrn, M_RST0 + r.randrange(8), 0xd9]))
    source(r, regions, marker_bytes(r) if r.random() < 0.5 else junk(r, 3) + bytes([0xff, M_RST0 + nrn]))
    return Call(0x63d300, [CINFO], regions, ret='b')


def resync(r):
    regions = common(r)
    desired = r.randrange(8)
    struct.pack_into('<i', regions[CINFO], UNREAD_MARKER, r.choice([0xd9, 0x01, 0xc4, M_RST0 + r.randrange(8),
                                                                    M_RST0 + (desired + r.choice([1, 2, -1, -2])) % 8]))
    data = b''.join(marker_bytes(r) for _ in range(r.choice([1, 2, 3])))
    source(r, regions, data)
    return Call(0x63b8f0, [CINFO, desired if not r.wild or r.random() < 0.7 else r.randrange(-20, 20)], regions, ret='b')


next_.__name__ = 'next'
OPS = [f for f in (reset, jinit, soi, sof, sos, dht, dqt, dri, skip, appn, app0, app14, next_, first, read, restart, resync)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
