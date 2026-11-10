# The C library's environment, its comparison and wide conversion through Win32 and _tzset (src/game/core/crtenv) against the originals: the Win32 calls COM recorders in their import slots, the locks, _nh_malloc, realloc,
# _amsg_exit and _getptd cdecl recorders, free the heap's hash. Allocations answer from a table of blocks in the heap region; the environment and its strings are made here.
# usage: crt_env.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_region, set_return  # noqa: E402

GETPTD, REALLOC, LOCK, UNLOCK, AMSG, NHMALLOC = 2, 6, 7, 8, 26, 28
CMPA, CMPW, TOWIDE, TONARROW, CPINFO, SETENV, ERROR, INCR, DECR, TZI = 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
GETW, GETA, FREEW, FREEA = 24, 25, 27, 29   # (GetEnvironmentStringsW / A, FreeEnvironmentStringsW / A)
B = Bench([(0x689b00, 0x689c56), (0x67fbf0, 0x67fc18), (0x688f10, 0x688f45), (0x689760, 0x68984e), (0x68f060, 0x68f325), (0x68b530, 0x68b56b), (0x691540, 0x691602), (0x68b5f0, 0x68b640), (0x68f540, 0x68f5b4), (0x68f5c0, 0x68f62f), (0x68f330, 0x68f534), (0x68b570, 0x68b5ea), (0x67fc20, 0x67fcae),
           (0x68ef70, 0x68efa2), (0x68ed80, 0x68ef6a), (0x68ed00, 0x68ed71), (0x687d80, 0x688058),
           (0x689180, 0x6891af), (0x67dcd0, 0x67dcf0), (0x67bfb0, 0x67c07c), (0x67acd0, 0x67acee), (0x67ae00, 0x67aefe), (0x67a480, 0x67a519), (0x686a00, 0x686a19)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True),
                 0x681740: _recorder(REALLOC, 2, cdecl=True), 0x67efc0: _recorder(AMSG, 1, cdecl=True), 0x67ad90: _free})
PTD, A_CMP, A_WIDE, A_NARROW, A_MALLOC, A_REALLOC, PCTYPE = (RAM + 0x100 * i for i in range(7))
ENV, WIDE_ENV, STRINGS, OUT, ZONE, STANDARD, DAYLIGHT, ENV2 = RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xc00, RAM + 0xd00, RAM + 0xd80, RAM + 0xe00, RAM + 0xf00
BLOCKS = [HEAP + 0x100 + 0x80 * i for i in range(40)]   # (each once: a block written over another's would give wild pointers, which the driver's process may hold as readable)
TEXTS = [b'PATH=c:\\x', b'TZ=EST5EDT', b'tz', b'A=1', b'AB=2', b'ab=', b'=C:=x', b'NOEQ', b'']
ZONES = [b'', b'EST5EDT', b'PST8', b'CET-1CEST', b'XYZ+5:30', b'XYZ-5:30:15QQQ', b'ABC12:', b'ABC:1', b'AB', b'ABC+', b'ABC1:2:3:4', b'ABC 7', b'ABC-', b'SAME']
OPTIONS = [b'A=1', b'TZ=X', b'A=', b'TZ=', b'NEW=v', b'NEW=', b'=x', b'noeq', b'']
OPS = [n for n in ('getenvlock', 'compare', 'compare', 'coll', 'mbschr', 'strdup', 'findenv', 'copyenv', 'setenv', 'setenv', 'wtomb', 'getenv', 'wcsnlen', 'setenvp', 'setenvp', 'wcsncnt', 'wcslk', 'wcslk', 'wcslk', 'wcs', 'tzset', 'tzset', 'envstrings', 'envstrings')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]

GETENV = {'getenv': 0x67fc20, 'getenvlock': 0x67fbf0}


def wide(text):
    return text.decode('latin-1').encode('utf-16-le') + b'\0\0'


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: bytearray(0x40), HEAP: heap(0x1600),
               IMPORT_TABLE: import_table(log, {'cmpa': (CMPA, 0x6fd270, 6, 0, 0), 'cmpw': (CMPW, 0x6fd274, 6, 0, 0), 'towide': (TOWIDE, 0x6fd174, 6, 0, 0), 'tonarrow': (TONARROW, 0x6fd1dc, 8, 0, 0b10010000), 'cpinfo': (CPINFO, 0x6fd180, 2, 0, 0b10),
                                                'setenv': (SETENV, 0x6fd278, 2, 0, 0), 'error': (ERROR, 0x6fd138, 0, 0, 0), 'incr': (INCR, 0x6fd0c0, 1, 0, 0), 'decr': (DECR, 0x6fd12c, 1, 0, 0), 'tzi': (TZI, 0x6fd21c, 1, 0, 0),
                                                'getw': (GETW, 0x6fd184, 0, 0, 0), 'geta': (GETA, 0x6fd188, 0, 0, 0), 'freew': (FREEW, 0x6fd18c, 1, 0, 0), 'freea': (FREEA, 0x6fd190, 1, 0, 0)})}
    set_return(log, GETPTD, PTD)
    answers(log, CMPA, A_CMP)
    answers(log, CMPW, A_CMP)
    answers(log, TOWIDE, A_WIDE)
    answers(log, TONARROW, A_NARROW)
    answers(log, NHMALLOC, A_MALLOC)
    answers(log, REALLOC, A_REALLOC)
    equal = r.random() < 0.4
    regions[A_CMP] = struct.pack('<40I', *[2 if equal else r.choice([1, 2, 2, 3, 0]) for _ in range(40)])
    regions[A_WIDE] = struct.pack('<40I', *[r.choice([3, 3, 3, 3, 1, 0]) for _ in range(40)])
    regions[A_NARROW] = struct.pack('<40I', *[r.choice([1, 2, 2, 3, 3, 3, 0]) if r.random() < 0.9 else 0 for _ in range(40)])
    regions[A_MALLOC] = struct.pack('<40I', *[BLOCKS[i] if r.random() < 0.9 else 0 for i in range(40)])
    regions[A_REALLOC] = struct.pack('<40I', *[r.choice([ENV2, ENV2, ENV2, 0]) for _ in range(40)])
    regions[0xfb1910] = struct.pack('<I', r.choice([0, 1, 1, 1, 2, 2, 3]))
    regions[0xfb1450] = struct.pack('<I', r.choice([0x4e4, 932]))
    regions[0xfb1870] = struct.pack('<2I', r.choice([0, 932]), r.choice([0x409, 0x411]))
    regions[0xfb1440] = struct.pack('<I', r.choice([0, 0x409, 0x409]))
    regions[0x77e7dc] = struct.pack('<i', r.choice([1, 2, 2, 3]))
    regions[0xfb3ca4] = struct.pack('<I', r.choice([0, 0, 1]))
    lead = bytearray(0x101)
    if r.random() < 0.7:
        lead[0x81] = lead[0x82] = 4
    regions[0xfb1669] = bytes(lead)
    fill(log, CPINFO, 1, struct.pack('<I2B12B', r.choice([1, 2, 2]), 0x3f, 0, *r.choice([[0x41, 0x4f] + [0] * 10, [0x81, 0x9f, 0xe0, 0xfc] + [0] * 8, [0x41] + [0] * 11, [0] * 12, [0x50, 0x60, 0x41, 0x41] + [0] * 8, [0x50, 0, 0x41, 0x42] + [0] * 8])))
    set_return(log, CPINFO, r.choice([1, 1, 1, 0]))
    fill(log, TONARROW, 4, bytes(r.choice([0x41, 0x42, 0x81, 0]) for _ in range(4)), which=0)
    fill(log, TONARROW, 7, struct.pack('<I', 1 if r.random() < 0.15 else 0), which=1)
    set_return(log, ERROR, r.choice([0x7a, 0x7a, 0x7a, 5]))
    # The environment.
    strings = bytearray(0x200)
    pointers = []
    at = 0
    for i in range(r.choice([0, 1, 3, 6])):
        text = r.choice(TEXTS)
        strings[at:at + len(text)] = text
        pointers.append(STRINGS + at)
        at += 0x20
    regions[STRINGS] = strings
    regions[ENV] = struct.pack('<16I', *(pointers + [0] * 16)[:16])
    regions[ENV2] = struct.pack('<16I', *(pointers + [0] + [0xabababab] * 16)[:16])   # (past the end what realloc leaves: garbage)
    regions[WIDE_ENV] = struct.pack('<8I', *([STRINGS + 0x100] * r.choice([0, 1, 2]) + [0] * 8)[:8])
    strings[0x100:0x108] = wide(b'A=b')
    env = r.choice([ENV, ENV, ENV, ENV, 0]) if name not in ('findenv',) else ENV
    regions[0xfb1380] = struct.pack('<3I', env, r.choice([env, 0, 0, 0]), r.choice([WIDE_ENV, WIDE_ENV, 0]))
    text = bytes(r.choice([0x41, 0x41, 0x3d, 0x81, 0x82, 0x42]) for _ in range(r.choice([0, 1, 2, 3, 5, 8]))) + b'\0'
    regions[OUT] = text + r.choice([b'A=B', b'=', b'']) + r.bytes(0x20)   # (past the end what a scan that does not stop would find)
    units = wide(bytes(r.choice([0x41, 0x42, 0x7f]) for _ in range(r.choice([0, 1, 2, 3, 5]))))
    if r.random() < 0.2 and len(units) > 2:
        units = b'\x00\x30' + units
    regions[ZONE] = (units + bytes(0x40))[:0x40]
    regions[STANDARD] = r.bytes(0x70)
    if name == 'compare':
        lengths = [0, 0, 1, 1, 2, 3, -1, 8, -2]
        if r.random() < 0.35:   # (one single byte against nothing: only then GetCPInfo's answer decides)
            regions[0xfb1910] = struct.pack('<I', 1)
            regions[OUT] = bytes([0x41, 0, 0x41, 0]) + r.bytes(0x20)
            size, ranges = r.choice([(1, [0x41, 0x4f]), (2, [0x41, 0x4f]), (2, [0x50, 0, 0x41, 0x42]), (2, [0x50, 0x60, 0x41, 0x42]), (2, [0x50, 0x60])])
            fill(log, CPINFO, 1, struct.pack('<I2B12B', size, 0x3f, 0, *(ranges + [0] * 12)[:12]))
            set_return(log, CPINFO, 1)
            a, b = r.choice([(1, 0), (0, 1)])
            return Call(0x68f060, [0x409, 1, OUT, a, OUT + 2, b, r.choice([0, 0x4e4])], regions, conv='c', ret='a')
        return Call(0x68f060, [0x409, 1, OUT, r.choice(lengths), OUT + 2, r.choice(lengths), r.choice([0, 0x4e4])], regions, conv='c', ret='a')
    if name == 'coll':
        return Call(0x68b530, [OUT, OUT + 2, r.choice([0, 1, 2, 5])], regions, conv='c', ret='a')
    if name == 'mbschr':
        return Call(0x691540, [OUT, r.choice([0x41, 0x3d, 0x42, 0, 0x8141, 0x8142, 0x813d, 0x81, 0x10041])], regions, conv='c', ret='a')
    if name == 'strdup':
        return Call(0x68b5f0, [r.choice([OUT, OUT, OUT, 0])], regions, conv='c', ret='a')
    if name == 'findenv':
        return Call(0x68f540, [STRINGS + 0x20, r.choice([0, 1, 2])], regions, conv='c', ret='a')
    if name == 'copyenv':
        return Call(0x68f5c0, [env], regions, conv='c', ret='a') if not env or r.random() < 0.9 else Call(0x68f5c0, [0], regions, conv='c', ret='a')
    if name == 'setenv':
        option = r.choice(OPTIONS) + b'\0'
        regions[OUT] = option + r.bytes(0x20)
        return Call(0x68f330, [r.choice([OUT, OUT, OUT, OUT, 0]), r.choice([0, 1])], regions, conv='c', ret='a')
    if name == 'wtomb':
        regions[0xfb1380] = struct.pack('<3I', env, 0, WIDE_ENV)
        return Call(0x68b570, [], regions, conv='c', ret='a')
    if name in GETENV:
        if r.random() < 0.3:   # (the wide environment's second entry fails, with the first already in the narrow one)
            regions[0xfb1380] = struct.pack('<3I', 0, 0, WIDE_ENV)
            regions[WIDE_ENV] = struct.pack('<8I', STRINGS + 0x100, STRINGS + 0x100, 0, 0, 0, 0, 0, 0)
            regions[A_NARROW] = struct.pack('<40I', 4, 4, 0, *[0] * 37)
            regions[A_MALLOC] = struct.pack('<40I', *BLOCKS)
            regions[A_REALLOC] = struct.pack('<40I', *[ENV2] * 40)
            regions[A_CMP] = struct.pack('<40I', *[2] * 40)
            regions[0xfb1910] = struct.pack('<I', 2)
            fill(log, TONARROW, 4, b'A=1\0', which=0)
            strings[0x20:0x22] = b'A\0'
            return Call(GETENV[name], [STRINGS + 0x20], regions, conv='c', ret='a')
        if r.random() < 0.3:   # (a shorter entry, with its "=" past the end)
            regions[ENV] = struct.pack('<4I', STRINGS + 0x40, 0, 0, 0)
            regions[0xfb1380] = struct.pack('<3I', ENV, 0, 0)
            regions[A_CMP] = struct.pack('<40I', *[2] * 40)
            strings[0x20:0x24] = b'xyz\0'
            strings[0x40:0x46] = b'AB\0=z\0'
            return Call(GETENV[name], [STRINGS + 0x20], regions, conv='c', ret='a')
        return Call(GETENV[name], [r.choice([STRINGS + 0x20, STRINGS + 0x20, STRINGS + 0x40, 0])], regions, conv='c', ret='a')
    if name == 'wcsnlen':
        return Call(0x688f10, [ZONE, r.choice([0, 1, 2, 3, 0x20])], regions, conv='c', ret='a')
    if name == 'envstrings':   # (__crtGetEnvironmentStringsA: the wide block or the narrow one, by the mode at 0xfb1660 (0: chosen now))
        block = r.choice([b'\0', b'A=1\0\0', b'A=1\0=C:=x\0BB=22\0\0', b'\0\0'])
        regions[HEAP][0x1500:0x1500 + len(block)] = block
        wblock = block.decode('latin-1').encode('utf-16-le')
        regions[HEAP][0x1560:0x1560 + len(wblock)] = wblock
        set_return(log, GETW, r.choice([HEAP + 0x1560, HEAP + 0x1560, 0]))
        set_return(log, GETA, r.choice([HEAP + 0x1500, HEAP + 0x1500, 0]))
        regions[0xfb1660] = struct.pack('<I', r.choice([0, 0, 1, 2, 3, 0xffffffff]))
        return Call(0x689b00, [], regions, conv='c', ret='a')
    if name == 'setenvp':   # (a failed malloc goes on through a null pointer, in both)
        block = r.choice([b'\0', b'A=1\0\0', b'=C:=x\0\0', b'A=1\0=C:=x\0BB=22\0\0', b'=x\0=y\0\0', b'long name=a longer text than the first\0B=\0\0'])
        regions[HEAP][0x1500:0x1500 + len(block)] = block
        regions[0xfb1354] = struct.pack('<I', HEAP + 0x1500)
        regions[0xfb1380] = r.bytes(4)
        if r.random() < 0.8:
            regions[A_MALLOC] = struct.pack('<40I', *BLOCKS)
        return Call(0x689760, [], regions, conv='c')
    if name == 'wcsncnt':
        return Call(0x68ef70, [ZONE, r.choice([0, 1, 2, 3, 0x20])], regions, conv='c', ret='a')
    if name in ('wcslk', 'wcs'):
        return Call(0x68ed80 if name == 'wcslk' else 0x68ed00, [r.choice([STANDARD, STANDARD, STANDARD, 0]), ZONE, r.choice([0, 1, 2, 3, 5, 0x20])], regions, conv='c', ret='a')
    # _tzset
    regions[0x708da0] = b'TZ\0\0'
    regions[0x781468] = struct.pack('<2I', STANDARD, DAYLIGHT)
    regions[DAYLIGHT] = r.bytes(0x70)
    regions[0x781470] = r.bytes(4)
    regions[0x781480] = r.bytes(4)
    regions[0x7813d8] = r.bytes(12)
    regions[0x77e7dc] = struct.pack('<i', 1)
    table = bytearray(0x200)
    for c in b'0123456789':
        table[c * 2] = 4
    table[0x20 * 2] = 8
    regions[PCTYPE] = table
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0xfb1458] = r.bytes(4)
    info = bytearray(0xac)   # (TIME_ZONE_INFORMATION: the bias, the standard name, its date's month +0x46, its bias +0x54, the daylight name +0x58, its date's month +0x9a, its bias +0xa8)
    struct.pack_into('<I', info, 0, r.choice([0, 60, -600 & 0xffffffff, 300]))
    info[4:12] = wide(b'Std')
    struct.pack_into('<H', info, 0x46, r.choice([0, 0, 1]))
    struct.pack_into('<I', info, 0x54, r.choice([0, 60]))
    info[0x58:0x60] = wide(b'Dly')
    struct.pack_into('<H', info, 0x9a, r.choice([0, 1, 1]))
    struct.pack_into('<I', info, 0xa8, r.choice([0, -60 & 0xffffffff]))
    regions[0xfb1460] = info
    set_return(log, TZI, r.choice([0, 1, 2, 0xffffffff]))
    zone = r.choice(ZONES)
    regions[ENV] = struct.pack('<4I', STRINGS, 0, 0, 0)
    regions[STRINGS] = bytearray(b'TZ=' + zone + b'\0' + bytes(0x1f0 - len(zone)) + b'')[:0x200]
    regions[0xfb1380] = struct.pack('<3I', r.choice([ENV, ENV, ENV, 0]), 0, 0)
    regions[0xfb1910] = struct.pack('<I', 2)
    regions[0xfb150c] = struct.pack('<I', r.choice([0, HEAP + 0x40, HEAP + 0x60]))
    regions[HEAP][0x40:0x45] = b'SAME\0'
    regions[HEAP][0x60:0x62] = b'x\0'
    if r.random() < 0.7:
        regions[A_CMP] = struct.pack('<40I', *[2] * 40)
    return Call(0x687d80, [], regions, conv='c')


if __name__ == '__main__':
    B.main(case)
