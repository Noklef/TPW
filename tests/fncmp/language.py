# The language object's font set and id (src/game/core/language: 0x41a5a0, 0x41a820-0x41a850),
# its font lookup and Hebrew alignment (0x485a70, 0x4925a0, on the object at 0x78b530) against
# the originals: widths about the 512 / 640 / 800 steps and negative, ids Hebrew or not, font
# indexes about 13 and font sets beyond 3, alignments 0-3 and wild. Its texts widened (0x485ac0, 0x485b00, 0x485f00): the converter
# [0x78ba7c] (lead byte, table size and words as methods.py's Widen) on or off ([0x78ba80]), texts of 0-12 bytes (0x485ac0's about 0x200 too) or none, record
# tables [0x78ba94] / [0x78baa4] of 0-4 records (some not text records or unterminated) and indexes about them, the object a recorder; 0x496570 (src/game/ui/dialogs) with its label among other windows.
# Its fonts and objects released (0x41a5f0): see free(); constructed (0x419570) over any bytes and destroyed (0x4196a0) over free()'s
# world with the text style's font (+0x500) none or such an object.
# usage: language.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x41a5a0, 0x41a5df), (0x41a820, 0x41a85f), (0x485a70, 0x485a94), (0x4925a0, 0x4925cb), (0x485ac0, 0x485af4), (0x485b00, 0x485b40),
           (0x485f00, 0x485f54), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0),
           (0x496570, 0x496609), (0x65d7bc, 0x65d832), (0x41a5f0, 0x41a81d), (0x6ad200, 0x6ad229), (0x419570, 0x419675),
           (0x419680, 0x419688), (0x4196a0, 0x419707), (0x6ad130, 0x6ad156), (0x5f8240, 0x5f8250), (0x657d30, 0x657da0),
           (0x47f940, 0x47f95b), (0x47f980, 0x47f98b), (0x6ad5d0, 0x6ad612), (0x5fa680, 0x5fa697), (0x6ad640, 0x6ad65d), (0x6ad160, 0x6ad1af),
           (0x5f82a0, 0x5f82ab), (0x5f82e0, 0x5f82f8), (0x67a400, 0x690000)])
LANG = RAM + 0x100


def case(r):
    mem = put(r.bytes(0x400), 0x108, 'I', r.choice([0x40d, 0x409, 0x40d ^ (1 << r.randrange(32)), r.getrandbits(32)]))
    op = r.randrange(14)
    if op == 11:
        return free(r)
    if op == 12:
        return Call(0x419570, [LANG], {RAM: r.bytes(0x800)}, conv='t', ret='a')
    if op == 13:
        c = free(r)
        c.address = 0x4196a0
        lang = bytearray(c.regions[RAM])
        put(lang, 0x500, 'I', r.choice([0, RAM + 0x2000 + 0x10 * 0x7f]))
        put(lang, 0x14, 'I', 0)   # (the folder handle's object: none)
        for k in range(0x34):   # (the entries' blocks +0xc: none, or a block of the heap)
            put(lang, 0x1c + 0x18 * k + 0xc, 'I', 0)
        if r.random() < 0.5:
            c.regions[HEAP] = heap(0x400)
            put(lang, 0x1c + 0x18 * r.randrange(0x34) + 0xc, 'I', HEAP + 0x40)
        c.regions[RAM] = lang
        return c
    if op >= 7:
        return texts(r, op)   # (10: 0x496570)
    if op >= 5:
        put(mem, 4, 'I', r.choice([0, 1, 2, 3, r.randrange(100), r.getrandbits(32)]))
        put(mem, 8, 'I', r.choice([0x40d, 0x40d, 0x409, 0x40d ^ (1 << r.randrange(32))]))
        if op == 5:
            return Call(0x485a70, [r.choice([0, 1, 12, 13, 14, r.randrange(13), r.getrandbits(32)])], {0x78b530: mem}, ret='a')
        put(mem, 0x200, 'I', r.choice([0, 1, 2, 3, 2, 0, r.getrandbits(32)]))
        return Call(0x4925a0, [RAM + 0x200], {0x78b530: mem[:0x10], RAM: mem})
    if op == 0:
        w = r.choice([0x200, 0x280, 0x320]) + r.randrange(-2, 3)
        w = r.choice([w, r.getrandbits(32), -w & 0xffffffff])
        return Call(0x41a5a0, [LANG, w], {RAM: mem}, conv='t')
    if op == 3:
        return Call(0x41a840, [LANG, r.getrandbits(32)], {RAM: mem}, conv='t')
    return Call({1: 0x41a820, 2: 0x41a830, 4: 0x41a850}[op], [LANG], {RAM: mem}, conv='t', ret='a')


def free(r):
    """0x41a5f0: the fonts (+0x30 + set * 0x138 + entry * 0x18) and objects (+0x54c to +0x5a4) none or objects whose slot 0 is a
    recorder (this logged); the resource folder at +0xc: its index (+0xc) none or such an object, its handle (+0x10) one."""
    lang = bytearray(r.bytes(0x600))
    objs = bytearray(r.bytes(0x800))
    vt = RAM + 0x3000
    for k in range(0x80):
        put(objs, 0x10 * k, 'I', vt)
    full = r.random()
    slots = [0x30 + e * 0x18 + s * 0x138 for e in range(13) for s in range(4)] + list(range(0x54c, 0x5a8, 4)) + [0xc]
    for i, at in enumerate(slots):
        put(lang, at, 'I', RAM + 0x2000 + 0x10 * i if r.random() < full else 0)
    put(lang, 0x10, 'I', vt)
    return Call(0x41a5f0, [RAM], {RAM: lang, RAM + 0x2000: objs, vt: vtable({}), LOG: log_region()}, conv='t')


def text(r):
    return bytes(r.choice([r.randrange(1, 0x80), 0x80, 0x81, 0x82, 0xa0, 0xff, r.getrandbits(8)]) or 1 for _ in range(r.choice([0, 1, 2, 3, 5, 8, 12])))


def texts(r, op):
    lang = bytearray(r.bytes(0x600))
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0xa0, 0, 0xff, r.getrandbits(8)]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, 0x7ff, r.randrange(0x800)]),
        RAM + 0x4000)
    put(lang, 0x54c, 'II', RAM + 0x800, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    log = log_region()
    regions = {0x78b530: lang, RAM + 0x800: conv, RAM + 0x4000: r.bytes(0x1000), 0x7ba058: bytes([0x55]) * 0x404, LOG: log}
    if op == 7:
        long = bytes(r.randrange(1, 0x80 if r.random() < 0.7 else 256) for _ in range(r.choice([0x1ff, 0x200, 0x201, r.randrange(0x20, 0x210)])))   # (about the 0x200 limit)
        regions[RAM + 0x1800] = (text(r) if r.random() < 0.8 else long) + bytes(0x40)
        return Call(0x485ac0, [r.choice([RAM + 0x1800] * 5 + [0])], regions, ret='a')
    count = r.randrange(0, 5)
    table = bytearray(r.bytes(0x20))
    put(table, 8, 'II', count, RAM + 0xb00)
    regions[RAM + 0xa00] = table
    put(lang, 0x574 if op == 9 else 0x564, 'I', RAM + 0xa00)
    pool = bytearray()
    for i in range(5):
        body = text(r) + bytes([0]) * r.choice([1, 1, 2])
        head = r.choice([1] * 6 + [2, 0x101]) | (len(body) << 8 if r.random() < 0.9 else r.randrange(0x40) << 8)
        rec = bytearray(struct.pack('<I', head) + body + bytes(0x40 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    regions[RAM + 0xc00] = bytes(pool)
    regions[RAM + 0xb00] = struct.pack('<5I', *[RAM + 0xc00 + 0x40 * i for i in range(5)])
    index = r.choice([0, 1, 2, 3, 4, count, count - 1, 0xffffffff]) & 0xffffffff
    if op == 8:
        return Call(0x485b00, [index], regions, ret='a')
    if op == 10:   # 0x496570: the label (child 0x32a of child 0x329 of [0x7ca168], each among 0-2 others) a recorder; the texts' indexes 0x6b-0x6f
        put(table, 8, 'I', r.choice([0x70, 0x70, 0x6e, 0x6c, 5]))
        put(table, 0xc, 'I', RAM + 0xb00 - 4 * 0x6b)   # (indexes 0x6b-0x6f: the five records)
        regions[RAM + 0xa00] = table
        kids = []
        for level, ident in enumerate((0x329, 0x32a)):
            ids = r.sample([ident, 0x328, 0x32b, 7], r.choice([1, 2, 3]))
            if ident not in ids:
                ids[0] = ident
            kids.append(ids)
        windows = RAM + 0x5000
        nodes = {}
        for level, ids in enumerate(kids):
            for i, ident in enumerate(ids):
                nodes[(level, i)] = (windows + 0x100 * (4 * level + i), ident)
        regions[0x7ca168] = struct.pack('<I', windows + 0x800)
        top = bytearray(r.bytes(0x90))
        put(top, 0x5c, 'I', nodes[(0, 0)][0])
        regions[windows + 0x800] = top
        for (level, i), (address, ident) in nodes.items():
            buf = bytearray(r.bytes(0x90))
            put(buf, 0, 'I', RAM + 0x1100)
            put(buf, 0x54, 'I', nodes[(level, i + 1)][0] if (level, i + 1) in nodes else 0)
            put(buf, 0x5c, 'I', nodes[(1, 0)][0] if level == 0 and ident == 0x329 else 0)
            put(buf, 0x8c, 'I', ident)
            regions[address] = buf
        regions[RAM + 0x1100] = vtable({9: 1})
        if r.random() < 0.5:
            set_return(log, 9, r.getrandbits(32))
        return Call(0x496570, [r.choice([0, 1, 2, 3, 4, 4, 5, 0xffffffff, r.getrandbits(32)])], regions, ret='a')
    regions[RAM + 0x1000] = struct.pack('<I', RAM + 0x1100)
    regions[RAM + 0x1100] = vtable({9: 1})
    if r.random() < 0.5:
        set_return(log, 9, r.getrandbits(32))
    return Call(0x485f00, [RAM + 0x1000, index], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
