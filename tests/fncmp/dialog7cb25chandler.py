# The handler of the loans dialog [0x7cb25c] (src/game/ui/dialog7cb25c: Dialog7cb25c_Handler, 0x49f4e0) against the original. Recorders (their own benches have them; driver.cpp's kDialog7cb25cHandlerEnv): Window_SendMessage 10, Game_CancelAdvisor 11, Window_FindChild 12,
# Window7059f0_NthItemData 13 (its out the stack's, filled), the table's getter 14, Money_LoanOffer 15 (its out the stack's, filled), Source_Field0c 16, Charset_FormatWide 17 (its buffer the stack's; the number's five dwords hashed), the message box 0x47f020 18 (its record's set bytes hashed;
# the text the stack's, left out), Dialog7ca210_Open 19, Dialog7ca2e8_Open 20, Dialog7cc2ec_Open 21, Dialog_KeyHandler 22. A row's data a loan taken (-1 - i) or offered (i + 1), the money over, at or under what the loan still costs.
# usage: dialog7cb25chandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x49f4e0, 0x49f852)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x486b40: _recorder(11, 1, cdecl=True), 0x65d7d0: _recorder(12, 1), 0x664c71: _recorder(13, 2), 0x5195d0: _recorder(14, 0), 0x4d06b0: _recorder(15, 2), 0x6ad810: _recorder(16, 0),
                 0x6acd60: _recorder(17, 5, cdecl=True), 0x47f020: _recorder(18, 8, cdecl=True), 0x498d80: _recorder(19, 0, cdecl=True), 0x49ac60: _recorder(20, 0, cdecl=True), 0x4b2750: _recorder(21, 0, cdecl=True), 0x488ba0: _recorder(22, 4, cdecl=True)})
WINDOW, LIST = RAM, RAM + 0x100


def case(r):
    log = log_region()
    set_return(log, 12, r.choice([LIST, LIST, LIST, LIST, 0]))
    set_return(log, 14, r.getrandbits(32))
    set_return(log, 22, r.getrandbits(32))
    strings(log, 13, 0, out=0b10)
    fill(log, 13, 1, struct.pack('<i', r.choice([-1 - r.randrange(8), -1 - r.randrange(8), r.randrange(1, 9), r.randrange(1, 9), 0, r.randrange(-1 << 31, 1 << 31)])))
    strings(log, 15, 0, out=0b1)
    offer = bytearray(r.bytes(0x20))
    term, paid, rate = r.randrange(0, 100), r.randrange(0, 100), r.randrange(0, 1000)
    struct.pack_into('<I', offer, 0xc, term)
    struct.pack_into('<I', offer, 0x10, rate)
    struct.pack_into('<I', offer, 0x18, paid)
    fill(log, 15, 0, bytes(offer), which=1)
    set_return(log, 16, ((term - paid) * rate + r.choice([1, 1, 0, -1, r.randrange(-100, 100)])) & 0xffffffff)
    strings(log, 17, 0, out=0b10)
    blob_bytes(log, 17, 4, 0xfffff)
    blob_bytes(log, 18, 0, 0xff70)
    message = r.choice([0x400] * 8 + [0x100] * 6 + [5, 0x14, 0x14, 0x13, 0x15, 0xff, 0x3ff, r.getrandbits(32)])
    a = r.getrandbits(32)
    if message == 0x100:
        a = r.choice([0xfffffffe, 0xffffffff, 0xfffffffd, 0, 0x12c4b9, 0x12c4ba, 0x12c4ba, 0x12c4bb, 0x12c4bb, 0x12c4bc, 0x12c4bc, 0x12c4bd, a])
    b = r.choice([r.randrange(8), r.randrange(8), r.randrange(8), 0xffffffff, r.getrandbits(32)])
    regions = {LOG: log, 0x7cb25c: r.bytes(4), 0x7cf83c: r.bytes(4), 0x78ba94: r.bytes(4)}
    return Call(0x49f4e0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
