# The mailing lists opened (src/game/ui/dialog7cc654open: Dialog7cc654_Open 0x4bc710) against the original. Window_FindChild among decoys, UserSlots_Instance, the heap and String_AnsiToWide (MultiByteToWideChar a recorder) real; the rest recorders as driver.cpp's kDialog7cc654OpenEnv. 0x5cc600 ends the loop by a counted fail_after, the row's fields left as they are.
# The formatter's three arguments on the stack: one in three cases its third hashed with the two past it, its first with the one past it, its second alone, so their order and contents show.
# usage: dialog7cc654open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fail_after, fill, heap, import_table, log_fold, log_region, put, set_return, strings  # noqa: E402

CREATE, TEXT_PART, RENDERER_PART, SET_IMPL, FONT, SET4C, NOTIFY_IMPL, NOTIFY3000, NOTIFYC00, SET_HANDLER, SURFACE, BUILD_ROWS, COLUMN, FIND_NAME, HIGHLIGHT, WIDE_TEXT, BUTTON, HOVER, FIRST, NEXT, FORMAT, ADD, REFRESH, FOCUS, PUSH, MB2WC = range(1, 27)
SLOTS_CTOR = 27
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4bc710, 0x4bcb70), (0x65d7bc, 0x65d840)],
          hooks={0x47ed80: C(CREATE, 3), 0x48f6a0: _recorder(TEXT_PART, 1), 0x48f150: _recorder(RENDERER_PART, 1), 0x65d1c8: _recorder(SET_IMPL, 1), 0x485a70: C(FONT, 1), 0x65c388: _recorder(SET4C, 1), 0x65f5b7: _recorder(NOTIFY_IMPL, 1), 0x65c4b9: _recorder(NOTIFY3000, 2), 0x65c428: _recorder(NOTIFYC00, 2),
                 0x65e526: _recorder(SET_HANDLER, 1), 0x5fa6f0: _recorder(SURFACE, 1), 0x6639cb: _recorder(BUILD_ROWS, 2), 0x6636b2: _recorder(COLUMN, 2), 0x659bc9: C(FIND_NAME, 1), 0x662eee: _recorder(HIGHLIGHT, 1), 0x485b00: C(WIDE_TEXT, 1), 0x486660: C(BUTTON, 7), 0x485d20: C(HOVER, 3), 0x5cc5e0: _recorder(FIRST, 0),
                 0x5cc600: _recorder(NEXT, 0), 0x6acd60: C(FORMAT, 7), 0x66403b: _recorder(ADD, 4), 0x663324: _recorder(REFRESH, 0), 0x65e59b: _recorder(FOCUS, 1), 0x5d6110: _recorder(PUSH, 2), 0x5c7420: _recorder(SLOTS_CTOR, 0)})
SLOTS, WINDOW, TEXT, HOLDER, CENTRE, PLACE, KIDS, LIST_KIDS = (RAM + 0x1000 * k for k in range(1, 9))


def wide(text):
    return text.encode('utf-16le') + b'\0\0'


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x1000, r.deal([0, 0, 0, 0, 1, 2])), 0x78ba94: r.bytes(4), CENTRE: r.bytes(4), PLACE: wide('text')}
    regions[IMPORT_TABLE] = import_table(log, {'MultiByteToWideChar': (MB2WC, 0x6fd174, 6, 4, 0)})
    slots = bytearray(r.bytes(0x110))
    put(slots, 0xc4, 'I', r.deal([1, 1, 1, 1, 1, 0, 0x100, r.getrandbits(32)]))
    put(slots, 0x64 + 0x24, 'I', r.deal([TEXT, TEXT, TEXT, 0]))
    regions[SLOTS] = slots
    regions[TEXT] = r.deal([b'\0', b'lists\0', b'a longer name of a list\0'])
    regions[0xf81ebc] = struct.pack('<I', r.deal([SLOTS, SLOTS, SLOTS, SLOTS, SLOTS, 0]))
    set_return(log, SLOTS_CTOR, SLOTS)
    regions[0xf85614] = struct.pack('<I', HOLDER)
    regions[HOLDER] = put(bytearray(r.bytes(0x20)), 0x14, 'I', CENTRE)

    first, top = kids(r, regions, KIDS, (0xee2, 0xee4, 0xee8, 0xee9))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    set_return(log, CREATE, WINDOW)
    regions[0x7cc654] = r.bytes(4)
    list_first, _ = kids(r, regions, LIST_KIDS, (0x10, 0x11, 0x12, 0x13))
    put(regions[top[0xee8]], 0x5c, 'I', list_first)

    set_return(log, TEXT_PART, r.getrandbits(32))
    set_return(log, RENDERER_PART, r.getrandbits(32))
    set_return(log, FONT, r.getrandbits(32))
    set_return(log, FIND_NAME, r.getrandbits(32))
    set_return(log, WIDE_TEXT, PLACE)
    strings(log, SURFACE, 0, 1 << 0)
    log_fold(log)   # (the calls past the log's 40 are compared too, and an answers() table would stop turning there: the loop would never end, in both)
    fail_after(log, NEXT, r.deal([0, 1, 2, 3, 6]), r.deal([0, 0x100, 0xffffff00]), counted=True)
    which = r.deal([(4, 0x3ff), (5, 0x1f), (6, 0x7fff)])
    fill(log, FORMAT, 1, wide('a row'))
    strings(log, FORMAT, 0, 1 << 1 | (1 << 4 | 1 << 5 | 1 << 6) & ~(1 << which[0]))
    blob(log, FORMAT, *which)
    blob_bytes(log, ADD, 0, 0xff0f, (4, 0xfff))   # (the row's text, its 12 bytes: the buffer's rest is stack garbage)
    return Call(0x4bc710, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
