# A part that shows the text its owner's record names (src/game/ui/textrecordpart: TextRecordPart_Refresh, 0x492070) against the original. Recorders (driver.cpp's kTextRecordPartEnv): Window_SendMessage 10, Records_DataIfTerminated 11, Language_Widen 12, Window_NotifyC00 13, Window_NotifyImpl100 14, Fn_669f80 15, Window_SetShown 16; the part's slot 1 a vtable recorder. wcschr and wcslen run.
# usage: textrecordpart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x492070, 0x492130), (0x67c2f0, 0x67c31e), (0x67acd0, 0x67acf0)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x6ace60: _recorder(11, 1), 0x485ac0: _recorder(12, 1, cdecl=True), 0x65c428: _recorder(13, 2), 0x65f5b7: _recorder(14, 1), 0x669f80: _recorder(15, 0), 0x65d9dd: _recorder(16, 1)})
PART, VT, ANSWER, TEXT, WIDE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def text(r):
    units = [r.randrange(0x20, 0x7f) for _ in range(r.choice([0, 0, 1, 5, 12]))]
    if units and r.random() < 0.4:
        units[r.randrange(len(units))] = 0xa
    return struct.pack('<%dH' % (len(units) + 1), *units, 0)


def case(r):
    log = log_region()
    part = bytearray(r.bytes(0x20))
    put(part, 0, 'I', VT)
    put(part, 4, 'H', r.choice([0, 1, 1, 1, r.getrandbits(16)]))
    put(part, 0xc, 'H', r.choice([0, 1, 1, 1, r.getrandbits(16)]))
    set_return(log, 10, r.choice([0, 0xffffffff, ANSWER, ANSWER, ANSWER, ANSWER, ANSWER]))
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 12, WIDE)
    answer = struct.pack('<2I', r.choice([0xffffffff, 0, 5, r.getrandbits(32)]), r.choice([0, 0, TEXT]))
    regions = {LOG: log, PART: part, VT: vtable({1: 0}), ANSWER: answer, TEXT: text(r), WIDE: text(r), 0x78a374: text(r), 0x78ba98: r.bytes(4)}
    return Call(0x492070, [PART], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
