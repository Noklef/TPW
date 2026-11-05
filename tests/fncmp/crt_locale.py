# The C library's setlocale and the search behind it (src/game/core/crtlocale) against the originals: the Win32 calls COM recorders in their import slots, the locks, _nh_malloc and the categories' init routines cdecl recorders, free the heap's hash.
# Bench limits: the callback that EnumSystemLocalesA calls does not run, so a named language or country always ends with no locale from setlocale; each GetLcidFrom* is called alone with a state already set. [0xfb3ca8] is 0, as no recorder can clear it for setlocale's wait. A first search on a system that is not NT is left out: it calls the wrapper at 0x68e550.
# usage: crt_locale.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, NHMALLOC, INIT = 7, 8, 28, 9
LCID, VERSION, VALIDCP, VALIDLOCALE, ENUM, INFO = 14, 15, 16, 17, 18, 19
B = Bench([(0x68e2a0, 0x68e33e), (0x68e480, 0x68e502), (0x68e450, 0x68e471), (0x68d800, 0x68d833), (0x68d840, 0x68d8a6), (0x68e680, 0x68e6be), (0x683ac0, 0x683d88), (0x683d90, 0x683ee5), (0x683ef0, 0x68400b), (0x684010, 0x684202), (0x684210, 0x684254), (0x684260, 0x684367), (0x684370, 0x6843da), (0x68db70, 0x68dbae), (0x67c250, 0x67c288), (0x68dbb0, 0x68dd41),
           (0x68dd50, 0x68ddad), (0x68ddb0, 0x68de3a), (0x68e120, 0x68e17e), (0x68e260, 0x68e29e), (0x68e340, 0x68e35b), (0x68e360, 0x68e443), (0x68e510, 0x68e549), (0x68e6c0, 0x68e6e1),
           (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67ae00, 0x67aefe), (0x689180, 0x6891af), (0x686a00, 0x686a19)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67ad90: _free})
A_MALLOC, A_INIT, A_INFO, A_LCID, PCTYPE = (RAM + 0x100 * i for i in range(5))
NAMES_AT, STRINGS, TEXT, TEXT2, OUT, ID, CP, POINTER, TABLE, KEYS = (RAM + 0x800 + 0x200 * i for i in range(10))
BLOCKS = [HEAP + 0x100 + 0x180 * i for i in range(40)]
C = 0x780dc8
STATE_ON_STACK = [(0xfb18ec, 8)]   # (the search's language and country: names in _expandlocale's frame)
CATEGORIES = [b'LC_ALL', b'LC_COLLATE', b'LC_CTYPE', b'LC_MONETARY', b'LC_NUMERIC', b'LC_TIME']
CACHE_NAME, CACHE_TEXT = b'French_France.1252', b'fra'
WHOLE = [b'', b'C', b'.1252', b'.ACP', b'.OCP', b'.0', b'.', b'..', b'English', b'English_USA', b'English_USA.1252', b'_USA', b'_USA.1252', b'English.1252', b'x,y', b'x_y,z', b'a_b.c,d', b'a.b', b'a_b_c', b'a_b.c.d', b'_', b',', b'a__', b'a_b.', b'a.',
         b'x' * 0x40, b'x' * 0x3f + b'_y', b'x_' + b'y' * 0x40, b'x_' + b'y' * 0x3f, b'a_b.' + b'9' * 6, CACHE_NAME, CACHE_TEXT]
PAIRS = [b'LC_CTYPE=C;LC_TIME=.1252', b'LC_COLLATE=;', b'LC_BAD=C', b'LC_=C', b'LC_;', b'LC_TIME', b'LC_TIME=', b'LC_TIME=C;', b'LC_TIME=C;;', b'LC_TIME=C;=x', b'LC_TIME=C;LC_BAD=x;LC_CTYPE=C', b'LC_TIME=;x=y', b'LC_TIM=C;LC_TIME=C', b'LC_ALL=C',
         b'LC_TIMEX=C', b'LC_COLLATE=C;LC_CTYPE=C;LC_MONETARY=C;LC_NUMERIC=C;LC_TIME=C', b'LC_TIME=English', b'LC_TIME=C;x', b'LC_NUMERIC=.1252;LC_MONETARY=' + CACHE_TEXT, b'LC_C', b'LC_MONETARY=' + CACHE_NAME + b';;', b'LC_TIME=;;LC_CTYPE=']
COMPARE = [b'', b'a', b'ab', b'abc', b'abd', b'abcd', b'B', b'\x80', b'\xff', b'a\xff']
SETS = [b'', b'_.,', b';', b'=;', b'a', b'\x80\xff', b'abc']
PLAIN = [b'', b'a', b'abc', b'ABC', b'ab1', b'@', b'[', b'`', b'{', b'Z', b'z', b'A', b'\x80', b'abc\xc1', b'ab cd']
KEY_LIST = [b'america', b'canadian', b'english', b'french', b'german', b'usa']
QUERIES = KEY_LIST + [b'ENGLISH', b'aaa', b'zzz', b'french2', b'', b'b', b'f', b'german ']
CODE_PAGES = [None, b'', b'ACP', b'OCP', b'1252', b'abc', b'acp', b'ACP2', b'932x', b'0', b'-1', b'  7']
OPS = [n for n in ('countryenum', 'countryenum', 'testlanguage', 'testlanguage', 'testcountry', 'fixgrouping', 'fixgrouping', 'freemon', 'freemon', 'hex', 'strncmp', 'strcspn', 'strcats', 'strtolc', 'strtolc', 'lctostr', 'primary', 'isnt', 'translate', 'translate', 'langcountry', 'language', 'country', 'default', 'codepage', 'qualified', 'qualified', 'qualified', 'expand', 'expand', 'expand',
                       'getall', 'getall', 'setcat', 'setcat', 'setcat', 'setlocale', 'setlocale', 'setlocale', 'setlocale', 'setlocale') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def names(r):
    """A names block: language +0, country +0x40, code page +0x80."""
    data = bytearray(0x100)
    language, country, page = r.choice([b'', b'', b'english', b'eng', b'ab', b'german']), r.choice([b'', b'', b'usa', b'us', b'canadian']), r.choice([b'', b'', b'ACP', b'OCP', b'1252', b'abc'])
    data[0:len(language)], data[0x40:0x40 + len(country)], data[0x80:0x80 + len(page)] = language, country, page
    return data


def case(r):
    name = r.deal(OPS)
    if name == 'testcountry':   # (the table read where it lies)
        languages = [r.choice([0xc0c, 0xc1a, 0x1007, 0x436, r.getrandbits(16)]) for _ in range(11)]
        return Call(0x68e450, [r.choice(languages + [0x409, 0x10c0c, r.getrandbits(32)])], {0x781a20: struct.pack('<11H', *languages)}, conv='c', ret='a')
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x4000),
               IMPORT_TABLE: import_table(log, {'lcid': (LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (VALIDLOCALE, 0x6fd284, 2, 0, 0), 'enum': (ENUM, 0x6fd1f8, 2, 0, 0), 'info': (INFO, 0x6fd28c, 4, 0, 0b100)})}
    answers(log, NHMALLOC, A_MALLOC)
    answers(log, INIT, A_INIT)
    answers(log, INFO, A_INFO)
    answers(log, LCID, A_LCID)
    regions[A_MALLOC] = struct.pack('<40I', *[BLOCKS[i] if r.random() < 0.93 else 0 for i in range(40)])
    regions[A_INIT] = struct.pack('<40I', *[0 if r.random() < 0.85 else 1 for _ in range(40)])
    regions[A_INFO] = struct.pack('<40I', *[0 if r.random() < 0.12 else 1 for _ in range(40)])
    regions[A_LCID] = struct.pack('<40I', *([r.choice([0x10000, 0x10000, 0x20000]) if name == 'setlocale' else r.choice([0x409, 0x411, 0x10000])] * 40))
    set_return(log, VALIDCP, 0 if r.random() < 0.1 else 1)
    set_return(log, VALIDLOCALE, 0 if r.random() < 0.1 else 1)
    # What GetLocaleInfoA gives: a code page's number or a name, in turn.
    fill(log, INFO, 2, r.choice([b'1252\0\0\0\0', b'932\0\0\0\0\0', b'0\0\0\0\0\0\0\0']) + b'Abc\0\0\0\0\0' + b'Xyz\0\0\0\0\0', records=3)
    fill(log, VERSION, 0, struct.pack('<5I', 0x94, 4, 0, 1381, r.choice([2, 2, 1, 0, 3])), which=1)
    set_return(log, VERSION, 0 if r.random() < 0.15 else 1)
    # The rest of the C library's state, in the "C" locale for _stricmp and atol.
    regions[0x77e7dc] = struct.pack('<i', 1)
    table = bytearray(0x200)
    for c in b'0123456789':
        table[c * 2] = 4
    table[0x20 * 2] = 8
    regions[PCTYPE] = table
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0xfb1438] = struct.pack('<6I', 0x11, 0x22, 0, 0x44, 0x55, 0x66)
    regions[0xfb1450] = struct.pack('<I', 0x4e4)
    regions[0xfb18b8] = r.bytes(0x24)
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 1]), 0)
    regions[0xfb142c] = struct.pack('<3H', 0x40c, 0x40c, 0x4e4) + b'\0\0' + struct.pack('<I', 0x4e4)
    regions[0xfb18dc] = struct.pack('<8I', 0x1234, r.choice([0, 1]), 7, r.choice([0, 1]), TEXT, TEXT2, r.choice([0, 0x100, 0x200, 0x300, 1, 2, 4, 7, 0x104, 0x301, 0x302, 0x304, 0x107, 0x207, 0x700]), 0x5678)
    regions[0xfb18fc] = struct.pack('<I', recorder(INFO, 4, True))
    # The strings the code compares with and adds, from the image's data.
    regions[0x708be0] = b';\0\0\0=;\0\0=\0\0\0_.,\0'
    regions[0x74c77c] = b'_\0\0\0'
    regions[0x747800] = b'.\0\0\0'
    regions[0x7096ec] = b'OCP\0ACP\0'
    if name == 'countryenum':   # (the search's country: TEXT2, in the block at 0xfb18dc)
        fill(log, INFO, 2, r.choice([b'USA\0\0\0\0\0', b'usa\0\0\0\0\0', b'FRA\0\0\0\0\0', b'\0\0\0\0\0\0\0\0']))
        regions[TEXT2] = r.choice([b'usa\0', b'USA\0', b'FRA\0', b'ger\0', b'\0'])
        regions[KEYS] = r.choice([b'0409\0', b'0c0c\0', b'411\0', b'\0', b'xyz\0', b'00010409\0'])
        regions[0x781a20] = struct.pack('<11H', *[r.choice([0x409, 0xc0c, 0x411, 0x807, 0x1000 + i]) for i in range(11)])
        return Call(0x68e2a0, [KEYS], regions, conv='s', ret='a')
    if name == 'testlanguage':   # (the search's language: TEXT, in the block at 0xfb18dc)
        fill(log, INFO, 2, r.choice([b'0409\0\0\0\0', b'0c0c\0\0\0\0', b'409\0\0\0\0\0', b'\0\0\0\0\0\0\0\0']))
        regions[TEXT] = r.choice([b'english\0', b'eng\0', b'eng-us\0', b'\0'])
        return Call(0x68e480, [r.choice([0x409, 0xc0c, 0x809, 0x10409, r.getrandbits(32)]), r.choice([0, 1, 1, 0x100])], regions, conv='c', ret='a')
    # The categories: the strings theirs or "C"; the cache of one.
    strings_ = bytearray(0x200)
    at = 0
    pointers = []
    for text in CATEGORIES:
        strings_[at:at + len(text)] = text
        pointers.append(STRINGS + at)
        at += 0x20
    regions[STRINGS] = strings_
    uniform = r.random() < 0.5
    held = regions[HEAP]
    locales = []
    for i in range(6):
        text = b'C' if uniform else r.choice([b'C', b'C', CACHE_NAME, b'German'])
        if i == 0:
            locales.append(r.choice([0, 0, HEAP + 0x3d00]))
        elif text == b'C' and r.random() < 0.7:
            locales.append(C)
        else:
            block = HEAP + 0x20 + 0x18 * i
            held[block - HEAP:block - HEAP + len(text) + 1] = text + b'\0'
            locales.append(block)
    table_ = bytearray(0x48)
    init = recorder(INIT, 0, cdecl=True)
    for i in range(6):
        struct.pack_into('<3I', table_, i * 12, pointers[i], locales[i], init)
    regions[0x780dc8] = b'C\0\0\0\0\0\0\0' + bytes(table_)
    cache = bytearray(0x110)
    cache[0:len(CACHE_TEXT)] = CACHE_TEXT
    cache[0x88:0x88 + len(CACHE_NAME)] = CACHE_NAME
    regions[0x780e18] = cache
    # The name tables, as this bench's own: the search's order of keys.
    keys = bytearray(0x200)
    entries = bytearray(0x30)
    for i, key in enumerate(KEY_LIST):
        keys[i * 0x10:i * 0x10 + len(key)] = key
        struct.pack_into('<I4s', entries, i * 8, KEYS + i * 0x10, b'%c%c%c\0' % (0x41 + i, 0x41 + i, 0x41 + i))
    regions[KEYS] = keys
    regions[TABLE] = entries
    for at_, n in ((0x781760, 0x41), (0x781968, 0x17)):
        big = bytearray(n * 8)
        for i in range(n):
            key = KEY_LIST[i * len(KEY_LIST) // n]
            struct.pack_into('<I4s', big, i * 8, KEYS + KEY_LIST.index(key) * 0x10, b'%c%c%c\0' % (0x41 + i % 26, 0x61 + i % 26, 0x41 + i % 26))
        regions[at_] = big
    regions[NAMES_AT] = names(r)
    regions[OUT] = bytearray(r.bytes(4) + bytes(0x1fc))
    regions[ID] = r.bytes(8)
    regions[CP] = r.bytes(4)
    regions[TEXT] = bytes(0x200)
    regions[TEXT2] = bytes(0x200)

    def given(which, data):
        regions[which] = (data + b'\0' + bytes(0x200))[:0x200]
        return which

    if name == 'fixgrouping':
        regions[TEXT] = bytes(r.choice(b'0123456789;;;xA\x80:/') for _ in range(r.randrange(12))) + bytes(4)
        return Call(0x68d800, [TEXT], regions, conv='c')
    if name == 'freemon':
        regions[TEXT2] = struct.pack('<10I', *[r.getrandbits(32) if i < 3 else r.choice([0, HEAP + 0x100 * i]) for i in range(10)])
        if r.random() < 0.3:
            regions[TEXT2] = regions[TEXT2][:0xc] + struct.pack('<I', 0xfb1940) + regions[TEXT2][0x10:]
        return Call(0x68d840, [r.choice([TEXT2, TEXT2, TEXT2, 0])], regions, conv='c')
    if name == 'hex':
        regions[TEXT] = bytes(r.choice(b'0123456789abcdefABCDEFgG/:@`\xe9-') for _ in range(r.randrange(10))) + bytes(4)
        return Call(0x68e680, [TEXT], regions, conv='c', ret='a')
    if name == 'strncmp':
        return Call(0x67c250, [given(TEXT, r.choice(COMPARE)), given(TEXT2, r.choice(COMPARE)), r.choice([0, 1, 2, 3, 4, 5, 0x7fffffff])], regions, conv='c', ret='a')
    if name == 'strcspn':
        return Call(0x68db70, [given(TEXT, r.choice(COMPARE + WHOLE)), given(TEXT2, r.choice(SETS))], regions, conv='c', ret='a')
    if name == 'strcats':
        regions[OUT] = bytearray(r.choice([b'', b'ab']) + bytes(0x1f0))
        return Call(0x684210, [OUT, r.choice([-1, 0, 1, 2, 3]), given(TEXT, r.choice(COMPARE)), given(TEXT2, r.choice(COMPARE)), TEXT], regions, conv='c')
    if name == 'strtolc':
        return Call(0x684260, [OUT, given(TEXT, r.choice(WHOLE))], regions, conv='c', ret='a')
    if name == 'lctostr':
        return Call(0x684370, [OUT, NAMES_AT], regions, conv='c')
    if name == 'primary':
        return Call(0x68e6c0, [given(TEXT, r.choice(PLAIN))], regions, conv='c', ret='a')
    if name == 'isnt':
        return Call(0x68e510, [], regions, conv='c', ret='a')
    if name == 'translate':
        regions[POINTER] = struct.pack('<I', given(TEXT, r.choice(QUERIES)))
        return Call(0x68dd50, [TABLE, r.choice([-1, 0, 1, 2, 3, 4, 5, 5, 5, 5]), POINTER], regions, conv='c', ret='a')
    if name in ('langcountry', 'language', 'country'):
        given(TEXT, r.choice(PLAIN + [b'eng', b'english']))
        given(TEXT2, r.choice(PLAIN + [b'usa', b'canadian']))
        return Call({'langcountry': 0x68ddb0, 'language': 0x68e120, 'country': 0x68e260}[name], [], regions, conv='c')
    if name == 'default':
        return Call(0x68e340, [], regions, conv='c')
    if name == 'codepage':
        page = r.choice(CODE_PAGES)
        return Call(0x68e360, [given(TEXT, page) if page is not None else 0], regions, conv='c', ret='a')
    if name == 'qualified':
        if r.random() < 0.3:
            regions[0xfb18fc] = struct.pack('<I', 0)
            fill(log, VERSION, 0, struct.pack('<5I', 0x94, 4, 0, 1381, 2), which=1)   # (not NT: the wrapper at 0x68e550, not ported, which the driver cannot call)
            set_return(log, VERSION, 1)
            if r.random() < 0.8:
                regions[NAMES_AT][0x80:0x84] = r.choice([b'1252', b'12\0\0', b'0\0\0\0'])
        return Call(0x68dbb0, [NAMES_AT, r.choice([ID, ID, 0]), r.choice([OUT, OUT, OUT, 0, NAMES_AT])], regions, conv='c', ret='a')
    if name == 'expand':
        return Call(0x684010, [given(TEXT, r.choice(WHOLE)) if r.random() < 0.97 else 0, OUT, r.choice([ID, ID, 0]), r.choice([CP, CP, 0]), r.getrandbits(32)], regions, conv='c', ret='a', ignore=STATE_ON_STACK)
    if name == 'getall':
        return Call(0x683ef0, [], regions, conv='c', ret='a')
    if name == 'setcat':
        return Call(0x683d90, [r.choice([1, 2, 2, 3, 4, 5]), given(TEXT, r.choice(WHOLE)) if r.random() < 0.97 else 0], regions, conv='c', ret='a', ignore=STATE_ON_STACK)
    category = r.choice([-1, 6, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0])
    locale = r.choice(WHOLE) if category or r.random() < 0.4 else r.choice(PAIRS)
    return Call(0x683ac0, [category, given(TEXT, locale) if r.random() < 0.93 else 0], regions, conv='c', ret='a', ignore=STATE_ON_STACK)


if __name__ == '__main__':
    B.main(case)
