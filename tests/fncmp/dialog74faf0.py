# The dialogs built with the bar windows hidden (src/game/ui/dialog74fb20: Dialog74fb20_OpenFor 0x48ac40, Dialog74faf0_OpenFor 0x48ae70, Dialog74faf0_Handler 0x48a970, CameraMode_Leave 0x48aff0) against the original. Recorders as dialog74fb20.py's 10-21;
# Camera_ToggleWalkMode 22, 0x4e15b0 thiscall 23, KeyMaps_ResetAll 24, Focus_Restore 25, Window_CreateChild thiscall 26, Window_SetHandler thiscall 27, Setting_ReadMode thiscall 28, atexit 29, the message box 0x47f020 30 (its record's set bytes hashed), 0x516c10 thiscall 31;
# the dialog's slot 5. The text 0x1d7 widened as the original inlines Language_WideText: Records_DataIfTerminated and Wide_FromNarrow run on a text table with the record, or too short, the converter on or off.
# usage: dialog74faf0.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,OpenB,Handler,Leave)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog74fb20 import world  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return, strings, vtable  # noqa: E402

CALLS = {'Open': 0x48ac40, 'OpenB': 0x48ae70, 'Handler': 0x48a970, 'Leave': 0x48aff0}
B = Bench([(0x48a970, 0x48ac36), (0x48ac40, 0x48adb0), (0x48ae70, 0x48afe3), (0x48aff0, 0x48b017), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0)],
          hooks={0x4a2a90: _recorder(10, 0, cdecl=True), 0x4a29d0: _recorder(11, 0, cdecl=True), 0x65fd0e: _recorder(12, 2), 0x65d42e: _recorder(13, 2), 0x65f345: _recorder(14, 2),
                 0x65e72d: _recorder(15, 2), 0x65e59b: _recorder(16, 1), 0x65968e: _recorder(17, 0, cdecl=True), 0x658d9f: _recorder(18, 4, cdecl=True), 0x42a190: _recorder(19, 0, cdecl=True),
                 0x40cf60: _recorder(20, 0, cdecl=True), 0x65f66e: _recorder(21, 3), 0x42ae70: _recorder(22, 0, cdecl=True), 0x4e15b0: _recorder(23, 1), 0x40cfa0: _recorder(24, 0, cdecl=True), 0x4862a0: _recorder(25, 0, cdecl=True),
                 0x65dd7b: _recorder(26, 7), 0x65e526: _recorder(27, 1), 0x550ca0: _recorder(28, 0), 0x67a7d0: _recorder(29, 1, cdecl=True), 0x47f020: _recorder(30, 8, cdecl=True), 0x516c10: _recorder(31, 0)})
WINDOW, DIALOG, VT, CONV, TABLE, LIST, TEXT, CODES = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x800, RAM + 0xa00, RAM + 0xb00, RAM + 0xc00, RAM + 0x4000
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def flags(r):
    return struct.pack('<I', r.choice([0, 2, 4, 6, 0x404, 0x406, 0x400, r.getrandbits(32)]))


def texts(r, regions):
    lang = bytearray(r.bytes(0x600))
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0xa0, 0, 0xff, r.getrandbits(8)]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, r.randrange(0x800)]), CODES)
    put(lang, 0x54c, 'II', CONV, r.choice([0, 1, 1, r.getrandbits(32)]))
    put(lang, 0x564, 'I', TABLE)
    body = bytes(r.randrange(1, 0x80) for _ in range(r.randrange(0, 12))) + bytes(1)
    record = struct.pack('<I', r.choice([1, 1, 1, 1, 2]) | len(body) << 8) + body + bytes(0x40)
    table = put(bytearray(r.bytes(0x20)), 8, 'II', r.choice([0x1d8, 0x1d8, 0x1d8, 0x1d7, 0]), LIST - 4 * 0x1d7)
    regions.update({0x78b530: lang, CONV: conv, CODES: r.bytes(0x1000), TABLE: table, LIST: struct.pack('<I', TEXT), TEXT: record, 0x7ba058: bytes([0x55]) * 0x404})


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = world(r, log)
    regions[0x790ab0] = flags(r)
    if op == 'Leave':
        return Call(CALLS['Leave'], [], {LOG: log, 0x790ab0: flags(r)}, ret='v')
    regions[0x7cb2ac] = r.bytes(12)
    if op in ('Open', 'OpenB'):
        window = bytearray(regions[0x7c24c8])
        window[:4] = struct.pack('<I', r.choice([0, WINDOW]))
        regions[0x7c24c8] = window
        set_return(log, 10, r.choice([0, 0, 1, r.getrandbits(32)]))
        set_return(log, 26, WINDOW)
        set_return(log, 12, DIALOG)
        regions[0xfaa5d8] = r.bytes(4)
        regions[DIALOG] = put(bytearray(r.bytes(0x10)), 0, 'I', VT)
        regions[VT] = vtable({5: 1}, size=0x20)
        return Call(CALLS[op], [r.getrandbits(32)], regions, ret='v')
    regions[0x7cf83c] = r.bytes(4)
    regions[0x78d910] = bytes([r.getrandbits(8), r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    since = r.randrange(0, 1 << 31)
    regions[0x7c2508] = struct.pack('<I', since)
    set_return(log, 17, since + r.choice([0x7d0, 0x7d0, 0x7d1, 0x7d1, 5000, 5000, 0, -1, r.randrange(-1 << 31, 1 << 31)]))
    set_return(log, 21, r.getrandbits(32))
    regions[0xfb34c7] = bytes([r.choice([0, 1, 1, 0xfe, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2, 2, 2, 1, 0, 0x102]))
    blob_bytes(log, 30, 0, 0xff7f)
    texts(r, regions)
    message = r.choice([0x15, 0x10006, 0x10006, 0x10006, 0x1000b, 0x1000b, 0x1000b, 0x14, 0x16, 0x10005, 0x10007, 0x1000a, 0x1000c, r.getrandbits(32)])
    a = r.choice({0x10006: [1, 1, 1, 1, 1, 1, 0, 2, 0x1b, r.getrandbits(32)], 0x1000b: [0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0, 1, 0x1a, r.getrandbits(32)]}.get(message, [1, 0x1b, 0, r.getrandbits(32)]))
    b = r.getrandbits(32) & r.choice([0xff00ffff, 0xff00ffff, 0xff00ffff, 0xffffffff])
    return Call(CALLS['Handler'], [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
