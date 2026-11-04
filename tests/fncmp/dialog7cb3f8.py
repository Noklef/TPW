# The dialog [0x7cb3f8] of the four user slots (src/game/ui/dialog7cb3f8: 0x4a5fc0, 0x4a6000, 0x4a61b0, 0x4a61d0, 0x4a6290, 0x4a6580) against the originals. Every call a recorder (driver.cpp's kDialog7cb3f8Env); the children found are small windows whose slot 9 is a recorder too. The Open's calls run past the log's 40 entries: log_fold().
# usage: dialog7cb3f8.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, blob_bytes, log_fold, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Handler': 0x4a5fc0, 'Slot': 0x4a6000, 'ClearConfirmed': 0x4a61b0, 'Done': 0x4a61d0, 'DoneConfirmed': 0x4a6290, 'Open': 0x4a6580}
# The slots are only 32, so the Open's and the handlers' calls share numbers (no case makes both's).
FIND, ALLOC, COUNT, BUILD, TEXT, FONT, PART, RECORDS = 14, 15, 28, 12, 25, 18, 16, 27
DWORD_A0, SLOTS, IF_USED, FORMAT, BOX, DEFAULT = 11, 12, 13, 14, 15, 10
B = Bench([(0x4a5fc0, 0x4a5ffa), (0x4a6000, 0x4a61b0), (0x4a61b0, 0x4a61cc), (0x4a61d0, 0x4a6290), (0x4a6290, 0x4a62ab), (0x4a6580, 0x4a6a4a)],
          hooks={0x40cfa0: _recorder(10, 0, cdecl=True), 0x65d9dd: _recorder(11, 1), 0x65fd0e: _recorder(12, 2), 0x65e59b: _recorder(13, 1), 0x65d7d0: _recorder(14, 1), 0x67acc0: _recorder(15, 1, cdecl=True), 0x491920: _recorder(16, 1),
                 0x65d1c8: _recorder(17, 1), 0x485a70: _recorder(18, 1, cdecl=True), 0x65c388: _recorder(19, 1), 0x65c428: _recorder(20, 2), 0x492d20: _recorder(21, 1, cdecl=True), 0x65c5d5: _recorder(22, 4), 0x65e6d5: _recorder(23, 2), 0x65e526: _recorder(24, 1),
                 0x485b00: _recorder(25, 1, cdecl=True), 0x4a62b0: _recorder(26, 0, cdecl=True), 0x5c7420: _recorder(27, 0), 0x5c7bb0: _recorder(28, 0), 0x5d6110: _recorder(29, 2), 0x5e14a0: _recorder(30, 0),
                 0x65f66e: _recorder(10, 3), 0x65e705: _recorder(11, 1), 0x48bcd0: _recorder(12, 0, cdecl=True), 0x5c7d00: _recorder(13, 1), 0x6acd60: _recorder(14, 5, cdecl=True), 0x47f020: _recorder(15, 8, cdecl=True), 0x4a6e40: _recorder(16, 1, cdecl=True),
                 0x5c83b0: _recorder(17, 1), 0x65961c: _recorder(18, 0, cdecl=True), 0x659607: _recorder(19, 0, cdecl=True), 0x5c7e90: _recorder(20, 1), 0x5d5db0: _recorder(21, 0, cdecl=True), 0x4a6a50: _recorder(23, 1, cdecl=True)})
WINDOW, CHILDREN, VTABLE, TURNS, TURNS2, INSTANCE, RECORD, HOLDER, HOLDER2 = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x4100, RAM + 0x5000, RAM + 0x6000, RAM + 0x7000, RAM + 0x7100


def globals_regions(r):
    return {0x7cb3f4: r.bytes(12), 0x7540c0: r.bytes(0x14), 0x78d90e: bytes([r.deal([0, 1])]), 0x78ba94: struct.pack('<I', r.getrandbits(32)), 0x7ba058: bytes(8)}


def handler_case(r, op):
    log = log_region()
    regions = {LOG: log, WINDOW: r.bytes(0x10), **globals_regions(r)}
    regions[INSTANCE] = bytearray(r.bytes(0x80))
    slot = r.deal([0, 1, 2, 3])
    put(regions[INSTANCE], slot * 24 + 4, 'I', r.deal([0, 1, 0x80000000]))
    regions[RECORD] = r.bytes(0x10)
    set_return(log, DWORD_A0, slot)
    set_return(log, SLOTS, INSTANCE)
    set_return(log, IF_USED, RECORD)
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, TEXT, RECORD + 8)
    blob(log, FORMAT, 4, 0xf)
    blob_bytes(log, BOX, 0, 0xff7f)
    args = [WINDOW, r.deal({'Handler': [5, 0x14, 0x14, 0x13, 0x15, 4], 'Slot': [0x100, 0x100, 0x10001, 0x10002, 0x10006, 0x10006, 0x10003, 0x10000, 0xff, 0x10007, 0x10005], 'Done': [0x10006, 0x10006, 0x10001, 0x10002, 0x10000, 0x10003, 0x10005, 0x10007]}[op]), r.getrandbits(32), r.getrandbits(32)]
    return Call(CALLS[op], args, regions, ret='a')


def open_case(r):
    log = log_region()
    log_fold(log)
    regions = {LOG: log, **globals_regions(r), VTABLE: vtable({9: 1})}
    made = r.deal([WINDOW, WINDOW, WINDOW, 0])
    set_return(log, BUILD, made)
    table = bytearray(0xc0)
    children = bytearray(0x200)
    for i in range(9):
        put(table, 4 * i, 'I', CHILDREN + 0x10 * i)
        put(children, 0x10 * i, 'I', VTABLE)
    regions[CHILDREN] = children
    regions[TURNS] = table
    answers(log, FIND, TURNS)
    allocs = bytearray(0xc0)
    put(allocs, 0, '6I', *[r.deal([RECORD, RECORD, RECORD, 0]) for _ in range(6)])
    regions[TURNS2] = allocs
    answers(log, ALLOC, TURNS2)
    regions[RECORD] = bytes(8)
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, FONT, r.getrandbits(32))
    set_return(log, PART, r.deal([HOLDER, HOLDER + 4]))
    set_return(log, RECORDS, r.deal([HOLDER, HOLDER + 4]))
    set_return(log, COUNT, r.deal([0, 1, 2, 0x100]))
    regions[HOLDER] = put(bytearray(r.bytes(0x20)), 4, 'I', r.getrandbits(32))
    regions[HOLDER2] = r.bytes(0x20)
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[0xfaa5d8] = struct.pack('<I', r.getrandbits(32))
    regions[0xf81ebc] = struct.pack('<I', r.deal([0, 0, HOLDER2]))
    regions[0xf85614] = struct.pack('<I', HOLDER)
    return Call(CALLS['Open'], [], regions, ret='v')


def case(r):
    op = r.deal(list(CALLS))
    if op == 'Open':
        return open_case(r)
    if op in ('Handler', 'Slot', 'Done'):
        return handler_case(r, op)
    log = log_region()
    regions = {LOG: log, INSTANCE: r.bytes(0x10)}
    set_return(log, SLOTS, INSTANCE)
    set_return(log, DEFAULT, r.getrandbits(32))
    return Call(CALLS[op], [r.getrandbits(32)] if op == 'ClearConfirmed' else [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
