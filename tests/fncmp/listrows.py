# A list's new row window (src/game/ui/listrows: 0x480d30 / 0x4a8380) against the originals. HilightList_NewRow / List7cc188_NewRow:
# window.py's tree and queues (Window_Ctor's surroundings), the heap failing now and then, font 7 (Language_Font, of group 0-2) a recorder
# whose slot 1 gives its height (edges too), the divisor word [0xfaa5c0] any but 0.
# The factories with children (List7cc338_NewRow 0x4b4c70, List7cc530_NewRow 0x4bacc0 and the others of CHILDREN below): the same, the
# fonts of those that keep their index in a dword (CACHES) that dword's (7 mostly, 0-12), the height cache any dword, 0 mostly; the
# children (Window_CreateChild) made with windowfactory.py's timer_env() surroundings and heap, the row's allocation failing now and then
# but for UNCHECKED's (the child of no window faults in both, sic; never the children's: CreateChild's failures fault in both,
# windowfactory.py); 0x4944d0's state [0x7ca154] 0-3 mostly.
# HilightList_InitRow / List7cc188_InitRow (0x480dd0 / 0x4a8420): see init_case() below. List7cc530_InitRow (0x4bad70): init_case()'s,
# the row's first child +0x5c a window of random bytes (its +0x98 none, an object of the +0x98 kind or the one found; +0x4c none or an
# object; its slot 7 a recorder), the GUI's registered windows of register() below with "i_boxtick" among them or not.
# The jump List7ca314_NewRowJump (0x49c870): List7ca314_NewRow's cases. The initialisers with a text child (0x49c880, 0x49f170, 0x4ae7d0,
# 0x4bcca0): see child_case() below.
# usage: listrows.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=HilightList_NewRow,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import rendererpart  # noqa: E402
import windowfactory  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, blob_bytes, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import RANGES, VTABLE, queues, tree  # noqa: E402

CALLS = {'HilightList_NewRow': 0x480d30, 'List7cc188_NewRow': 0x4a8380, 'HilightList_InitRow': 0x480dd0, 'List7cc188_InitRow': 0x4a8420,
         'List7cc338_NewRow': 0x4b4c70, 'List7cc530_NewRow': 0x4bacc0, 'List7cc530_InitRow': 0x4bad70, 'List7ca140_NewRow': 0x4938c0,
         'List7ca158_NewRow': 0x4944d0, 'List7ca314_NewRow': 0x49bbb0, 'List7cb24c_NewRow': 0x49f0b0, 'List7cb264_NewRow': 0x4a02b0,
         'List7cc200_NewRow': 0x4ad240, 'List7cc250_NewRow': 0x4ae700, 'List7cc654_NewRow': 0x4bcbc0, 'List7ca314_NewRowJump': 0x49c870,
         'List7ca314_InitRow': 0x49c880, 'List7cb24c_InitRow': 0x49f170, 'List7cc250_InitRow': 0x4ae7d0, 'List7cc654_InitRow': 0x4bcca0,
         'List7cc94c_NewRow': 0x4c02d0, 'List7cca90_NewRow': 0x4c1b30}
TEXT_ROWS = {'List7ca314_InitRow', 'List7cb24c_InitRow', 'List7cc250_InitRow', 'List7cc654_InitRow'}
# the factories that make children (with windowfactory.py's surroundings), those whose first allocation failing would fault in both
# (their child or handler of no window, sic), and the dwords their height is cached in with the dword their font index is in (or 7)
CHILDREN = {k for k in CALLS if k.endswith('NewRow')} - {'HilightList_NewRow', 'List7cc188_NewRow'}
UNCHECKED = {'List7cc530_NewRow', 'List7ca158_NewRow', 'List7cb24c_NewRow', 'List7cb264_NewRow', 'List7cc94c_NewRow', 'List7cca90_NewRow'}
CACHES = {'List7cc338_NewRow': (0x7cc35c, 0x756f38), 'List7ca140_NewRow': (0x7ca140, 0x7508b4), 'List7ca158_NewRow': (0x7ca158, 0x750d54),
          'List7ca314_NewRow': (0x7ca314, 0x7523dc), 'List7cb24c_NewRow': (0x7cb24c, None), 'List7cb264_NewRow': (0x7cb264, 0x752934),
          'List7cc200_NewRow': (0x7cc200, 0x755134), 'List7cc250_NewRow': (0x7cc250, 0x75583c), 'List7cc94c_NewRow': (0x7cc94c, None),
          'List7cca90_NewRow': (0x7cca90, None)}
# (the initialisers: the text part's construction (textparts.py's ranges), SetLayout (0x48fc40, 0x48f420), the pixels (trackedglobal.py's),
# the renderer's tag 0x30 list (0x48f5c0, Renderer_Add, Pool_AddSorted), Window_Notify40000)
PART_RANGES = [(0x48f150, 0x48f290), (0x5775d0, 0x577660), (0x579b40, 0x579c30), (0x588320, 0x588420), (0x588d70, 0x588e50), (0x58a4f0, 0x58a560),
               (0x58dc50, 0x58dd70), (0x590b90, 0x590c50), (0x590a70, 0x590a75), (0x588ca0, 0x588d6e), (0x6597b0, 0x6598ed), (0x659994, 0x6599bc),
               (0x659d00, 0x659d53), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x67afa0, 0x67b0d0), (0x48f6a0, 0x48f740),
               (0x5fa680, 0x5fa6b0), (0x6ad5d0, 0x6ad670), (0x6ade00, 0x6adf30), (0x6b0230, 0x6b0290), (0x48fc40, 0x48fdac), (0x48f420, 0x48f5b7),
               (0x48f3d0, 0x48f409), (0x588940, 0x5889a1), (0x56af20, 0x56af9c), (0x588e40, 0x588ee8), (0x48f5c0, 0x48f650), (0x576410, 0x576550),
               (0x576270, 0x576320), (0x65c5d5, 0x65c672), (0x480dd0, 0x480e62), (0x4a8420, 0x4a84b2)]
# (GuiWindows_FindByName and Window_SetObject98 with what they call; the new rows and their children: windowfactory.py's)
REG_RANGES = [(0x659bc9, 0x659c42), (0x6588b6, 0x6588ef), (0x65d070, 0x65d1c8), (0x65f5b7, 0x65f5fe), (0x668f5a, 0x668f9c), (0x65d547, 0x65d558)]
B = Bench(sorted(set(RANGES + PART_RANGES + REG_RANGES + windowfactory.FACTORY + [
    (0x4b4c70, 0x4b4d34), (0x4bacc0, 0x4bad69), (0x4bad70, 0x4bae2b), (0x4938c0, 0x4939c7), (0x4944d0, 0x494679), (0x49bbb0, 0x49bc74),
    (0x49f0b0, 0x49f16e), (0x4a02b0, 0x4a03c1), (0x4ad240, 0x4ad334), (0x4ae700, 0x4ae7c9), (0x4bcbc0, 0x4bcc9d), (0x490250, 0x490300),
    (0x4907b0, 0x490860), (0x49c870, 0x49c96b), (0x49f170, 0x49f27f), (0x4ae7d0, 0x4ae8d6), (0x4bcca0, 0x4bce08), (0x65c388, 0x65c55a),
    (0x65d56b, 0x65d58c), (0x65d773, 0x65d7cc), (0x6616b0, 0x661710), (0x6617f0, 0x661820), (0x4903e0, 0x4904d0), (0x490940, 0x490a00), (0x6acb70, 0x6accb0),
    (0x6acd60, 0x6ace70), (0x6b00e0, 0x6b0140), (0x4c02d0, 0x4c03dd), (0x4c1b30, 0x4c1c28)])))
ROW, OLD, OBJ98, VT98, VTOLD, OBJ4C, REND = RAM + 0x9000, RAM + 0x9200, RAM + 0x9300, RAM + 0x9380, RAM + 0x9400, RAM + 0x9480, RAM + 0x9500
PARENTS = RAM + 0x9600
NODES, FOUND, FVT, CUT, BOX, BOXVT = RAM + 0xa000, RAM + 0xa100, RAM + 0xa200, RAM + 0xa300, RAM + 0xa500, RAM + 0xa780
TEXT, TEXTVT = RAM + 0xa800, RAM + 0xa980


def hash_name(name, multiplier=0x2f):
    h = 0
    for c in name.encode():
        h = ((c - 256 if c > 127 else c) ^ h) * multiplier & 0xffffffff
    return h


def register(r, regions, name):
    """the GUI's registered windows [0xfaa610]: a chain of 0-3 nodes, one of them `name` (Gui_HashName 0x2f) or none, its window a
    recorder (slot 8 giving CUT, an object of the +0x98 kind, or 0 or OBJ98); VT98 releasing too (WindowImpl_Release's slot 5)"""
    log = regions[LOG]
    n = r.choice([0, 1, 2, 3])
    found = r.randrange(n) if n and r.random() < 0.85 else -1
    nodes = bytearray(r.bytes(0x18 * 3))
    for i in range(n):
        put(nodes, 0x18 * i, 'I', FOUND)
        put(nodes, 0x18 * i + 0x10, 'I', hash_name(name) if i == found else r.choice([hash_name('!frame'), r.getrandbits(32)]))
        put(nodes, 0x18 * i + 0x14, 'I', NODES + 0x18 * (i + 1) if i + 1 < n else 0)
    regions[NODES] = nodes
    regions[0xfaa610] = struct.pack('<I', NODES if n else 0)
    regions[VT98] = vtable({0: 1, 3: 2, 5: 1})   # (the old object +0x98 released too: WindowImpl_Release's slot 5)
    regions[FOUND] = struct.pack('<I', FVT) + r.bytes(0x1c)
    regions[FVT] = vtable({})
    set_return(log, 8, r.choice([CUT, CUT, CUT, 0, OBJ98]))
    regions[CUT] = put(bytearray(r.bytes(0x20)), 0, 'I', VT98)
    put(regions[CUT], 0x10, 'HH', r.choice([0, 1, 2]), r.choice([0, 1, 2]))


def init_case(r, op):
    """HilightList_InitRow / List7cc188_InitRow: rendererpart.py's surroundings of a part's construction, the heap failing the part now
    and then, font 7 (Language_Font, of group 0-2); a row of random bytes, its rectangle record (+0x2c) small, its impl +0x9c none (a new
    row) or an old one (a recorder vtable, the words +0x10 / +0x12 about 1: released or deleted), +0x98 none or a recorder (slot 3's
    event pointer not compared), +0x4c none, an object or font 7 itself, shown (+0x48 bit 0, up its two parents or fewer) or not; the
    GUI's size words about 2048 x 1536 (the part's pixels small); the renderer's pool out of nodes (the tag 0x30 list left alone)."""
    rendererpart.OPS = ['Ctor']
    regions = dict(rendererpart.case(r).regions)
    regions.pop(rendererpart.PART)
    log = log_region()
    regions[LOG] = log
    regions[HEAP] = heap(0x4000, r.choice([0] * 9 + [1]))
    lang = bytearray(r.bytes(0x400))
    group = r.choice([0, 1, 2])
    put(lang, 4, 'I', group)
    regions[0x78b530] = lang
    font = 0x78b530 + 0x1c + (7 + 13 * group) * 0x18
    row = bytearray(r.bytes(0x138))
    left, top = r.randrange(-5, 30), r.randrange(-5, 30)
    put(row, 0x30, '4h', left, top, left + r.randrange(0, 40), top + r.randrange(0, 40))
    put(row, 0x9c, 'I', OLD if r.random() < 0.15 else 0)
    put(row, 0x98, 'I', r.choice([0, 0, OBJ98]))
    put(row, 0x4c, 'I', r.choice([0, 0, 0, OBJ4C, font]))
    put(row, 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1]))
    put(row, 0x60, 'I', r.choice([0, PARENTS, PARENTS]))
    for k in range(2):   # (the parent and its parent: shown or not, the last without a parent)
        parent = put(bytearray(r.bytes(0x80)), 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1, 1]))
        regions[PARENTS + 0x80 * k] = put(parent, 0x60, 'I', r.choice([0, PARENTS + 0x80]) if k == 0 else 0)
    regions[ROW] = row
    regions[OLD] = put(put(bytearray(r.bytes(0x20)), 0, 'I', VTOLD), 0x10, 'HH', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    regions[VTOLD] = vtable({5: 1})
    regions[OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', VT98)
    regions[VT98] = vtable({0: 1, 3: 2})
    blob_bytes(log, 3, 1, 0)   # (its slot 3's event: a stack pointer, logged as a hash of none of its bytes; windownotify.py checks them)
    regions[OBJ4C] = r.bytes(0x20)
    regions[0xfaa5c0] = struct.pack('<hhh', r.choice([1536, 1536, 768, 0x600 + r.randrange(-3, 4)]), 0, r.choice([2048, 2048, 1024, 0x800 + r.randrange(-3, 4)]))
    regions[0x87a6d0] = struct.pack('<I', REND)
    regions[REND] = put(bytearray(r.bytes(0x40)), 4, 'I', 0)
    return Call(CALLS[op], [ROW], regions, ret='v')


def child_case(r, op):
    """the initialisers with a text child (TEXT_ROWS): init_case()'s row, the heap failing the first or the second part now and then; its
    first child +0x5c TEXT, random bytes as the row's (its rectangle +0x2c small, its impl +0x9c none or OLD, +0x98 none or OBJ98, +0x4c
    none, OBJ4C or a font, shown or not; its parent the row), its vtable a recorder (slot 7); the fonts' dwords 0-12, 7 mostly; the
    text parts' slot 9 (their text: 0x48f830, a known skip) a recorder of one argument in both, and the 0x6ff3b0 and 0x6ff410
    kinds' slot 3 (their layout, 0x4903e0 / 0x490940, unported: the driver has no game code to run) one of two (its event as init_case()'s); for List7cc654_InitRow the child's next
    two siblings (+0x54) windows as List7cc530_InitRow's box, the GUI's registered windows of register() with "i_boxtick" or not."""
    regions = dict(init_case(r, op).regions)
    regions[HEAP] = heap(0x4000, r.choice([0] * 7 + [1, 3, 4]))   # (the parts' allocations and the one the first part's construction
    # survives: its others, the second and the fifth, fault in both)
    text = bytearray(r.bytes(0x138))
    put(text, 0, 'I', TEXTVT)
    left, top = r.randrange(-5, 30), r.randrange(-5, 30)
    put(text, 0x30, '4h', left, top, left + r.randrange(0, 40), top + r.randrange(0, 40))
    put(text, 0x9c, 'I', OLD if r.random() < 0.15 else 0)
    put(text, 0x98, 'I', r.choice([0, 0, OBJ98]))
    put(text, 0x4c, 'I', r.choice([0, 0, OBJ4C]))
    put(text, 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1]))
    put(text, 0x60, 'I', ROW)
    regions[TEXT] = text
    regions[TEXTVT] = vtable({7: 1})
    put(regions[ROW], 0x5c, 'I', TEXT)
    for font in (0x7523dc, 0x75583c):
        regions[font] = struct.pack('<I', r.choice([7] * 4 + list(range(13))))
    regions[0x6ff344] = struct.pack('<I', recorder(9, 1))
    for vt in (0x6ff3b0, 0x6ff410):   # (the other part kinds' slot 3, 0x4903e0 / 0x490940: unported, so not native code)
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
    if op == 'List7cc654_InitRow':
        register(r, regions, 'i_boxtick')
        regions[BOXVT] = vtable({7: 1})
        for k, at in enumerate((BOX, BOX + 0x120)):
            box = bytearray(r.bytes(0x120))
            put(box, 0, 'I', BOXVT)
            put(box, 0x98, 'I', r.choice([0, 0, OBJ98, CUT]))
            put(box, 0x4c, 'I', r.choice([0, OBJ4C]))
            put(box, 0x60, 'I', ROW)
            regions[at] = box
        put(text, 0x54, 'I', BOX)
        put(regions[BOX], 0x54, 'I', BOX + 0x120)
    return Call(CALLS[op], [ROW], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op in TEXT_ROWS:
        return child_case(r, op)
    if op == 'List7ca314_NewRowJump':
        c = case_for(r, 'List7ca314_NewRow')
        return Call(CALLS[op], c.args, c.regions, ret='a')
    if op == 'List7cc530_InitRow':
        regions = dict(init_case(r, op).regions)
        register(r, regions, 'i_boxtick')
        box = bytearray(r.bytes(0x120))
        put(box, 0, 'I', BOXVT)
        put(box, 0x98, 'I', r.choice([0, 0, OBJ98, CUT]))
        put(box, 0x4c, 'I', r.choice([0, OBJ4C]))
        put(box, 0x60, 'I', ROW)   # (its parent the row: shown or not up the row's parents)
        regions[BOX] = box
        regions[BOXVT] = vtable({7: 1})
        put(regions[ROW], 0x5c, 'I', BOX)
        return Call(CALLS[op], [ROW], regions, ret='v')
    if op.endswith('InitRow'):
        return init_case(r, op)
    return case_for(r, op)


def case_for(r, op):
    """the factories' cases (above)"""
    log = log_region()
    regions = {LOG: log}
    wins = tree(r, regions)
    queues(r, regions)
    regions[HEAP] = heap(0x400, r.choice([0] * 5 + [1]))
    regions[0xfaa540] = struct.pack('<II', r.randrange(-10, 120) & 0xffffffff, r.randrange(-10, 40) & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                    r.choice([0] * 6 + [1]))
    regions[0xfaa5c0] = struct.pack('<H', r.choice([1, 2, 3, 0x600, 0x7fff, 0x8000, 0xffff, 0xfffd, r.randrange(1, 0x10000)]))
    lang = bytearray(r.bytes(0x400))
    group = r.choice([0, 1, 2])
    put(lang, 4, 'I', group)
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', VTABLE)
    regions[0x78b530] = lang
    regions[VTABLE] = vtable({})
    set_return(log, 1, r.choice([r.randrange(0, 40), 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
    if op in CHILDREN:
        regions[VTABLE] = vtable({5: 1})
        windowfactory.timer_env(r, regions, log)
        regions[HEAP] = heap(0x2000, r.choice([0] * 5 + ([] if op in UNCHECKED else [1])))
        regions[0xfaa5ec] = struct.pack('<I', r.choice([wins[0], 0, 0]))
        cache, font = CACHES.get(op, (None, None))
        for _, other in CACHES.values():   # (the other lists' font dwords not 7: the exe's 7 would hide reading the wrong one)
            if other:
                regions[other] = struct.pack('<I', r.choice([0, 1, 12]))
        if font:
            which = r.choice([7] * 6 + list(range(13)))
            put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', r.getrandbits(32))
            put(lang, 0x1c + (which + 13 * group) * 0x18, 'I', VTABLE)
            regions[font] = struct.pack('<I', which)
        if cache:
            regions[cache] = struct.pack('<I', r.choice([0, 0, 0, r.randrange(1, 40), r.getrandbits(32)]))
        if op == 'List7ca158_NewRow':
            regions[0x7ca154] = struct.pack('<I', r.choice([0, 1, 2, 3] * 3 + [4, r.getrandbits(32)]))
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
