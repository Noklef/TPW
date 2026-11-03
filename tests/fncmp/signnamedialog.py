# The handlers of the dialogs that name a thing (src/game/ui/signnamedialog: SignNameDialog_Handler 0x48d640, SignNameDialog7ca214_Handler 0x4993e0) against the originals. Recorders (their own benches have them; driver.cpp's kSignNameDialogHandlerEnv):
# KeyMaps_EnableAllAndClearFlag 20, Focus_Restore 21, 0x4e2950 22, 0x4e2a50 23, 0x4682b0 24, RemoveFontResourceA 25 (stdcall, through the import slot), File_Delete 26 (its result on the stack), GlobalSave_Instance 27, GlobalSave_SetSignNameA 28, B 29,
# CheatSystem_Construct 30, atexit 31, CheatSystem_SetOn 10, Thing_NameBuffer7ba058 11, Window_PostMessage 12, Window_SendMessage 13, Window_SetFocus 14; the slots 1, 3 and 9 vtable recorders. Window_FindChild, Window_DwordA0, Window_CallDefault,
# wcslen and wcscmp run. The two names often the scrambled secret strings, so the cheats go on.
# usage: signnamedialog.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x48d640, 0x48d856), (0x4993e0, 0x499758), (0x65d7bc, 0x65d840), (0x65e705, 0x65e72d), (0x65f66e, 0x65f697), (0x5da3c0, 0x5da3c1), (0x67acd0, 0x67ad00), (0x67c290, 0x67c2e2)],
          hooks={0x486b60: _recorder(20, 0, cdecl=True), 0x4862a0: _recorder(21, 0, cdecl=True), 0x4e2950: _recorder(22, 1), 0x4e2a50: _recorder(23, 1), 0x4682b0: _recorder(24, 3, cdecl=True), 0x5f5d60: _recorder(26, 2, cdecl=True),
                 0x409410: _recorder(27, 0, cdecl=True), 0x5b0940: _recorder(28, 1), 0x5b0970: _recorder(29, 1), 0x4053b0: _recorder(30, 0), 0x67a7d0: _recorder(31, 1, cdecl=True), 0x4053c0: _recorder(10, 1),
                 0x486280: _recorder(11, 1, cdecl=True), 0x658ccc: _recorder(12, 4, cdecl=True), 0x658d9f: _recorder(13, 4, cdecl=True), 0x65e59b: _recorder(14, 1)})
WINDOW, OWNER, THING, VT, PARENT, INDEX, KIDS, FONTS, SECRET_A, SECRET_B, PATHS = (RAM + 0x400 * k for k in range(11))
EDIT_A, EDIT_B = 0x3943c, 0x3943d


def kid(r, ident, following):
    k = bytearray(r.bytes(0x100))
    put(k, 0, 'I', VT)
    put(k, 0x8c, 'I', ident)
    put(k, 0x54, 'I', following)
    put(k, 0x5c, 'I', 0)
    return k


def scrambled(units):
    out, i = list(units), 0
    while i < len(out):
        out[i] ^= i
        if not out[i]:
            return out[:i]
        i += 1
    return out


def text(units, size=0x20):
    data = bytearray(size)
    for i, u in enumerate(units[:size // 2 - 1]):
        put(data, 2 * i, 'H', u)
    return bytes(data)


def case(r):
    op = r.deal([0x48d640, 0x4993e0])
    log = log_region()
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 27, r.getrandbits(32))
    strings(log, 26, 0, out=1)
    message = r.choice([0x100] * 8 + [0x14, 0x802, 0x802, 0x803, 0x803, 0x804, 0x801, 0x805, 0x13, 0xff, 0x101, r.getrandbits(32)])
    a = r.choice([0xffffffff] * 4 + [0xfffffffe, 0xfffffffd, 0, EDIT_A, EDIT_A, EDIT_B, r.getrandbits(32)])
    if message in (0x802, 0x803):
        a = r.choice([EDIT_A, EDIT_A, EDIT_B, EDIT_A - 1, r.getrandbits(32)])
    window = bytearray(r.bytes(0x120))
    put(window, 0, 'I', VT)
    put(window, 0x5c, 'I', KIDS)
    put(window, 0x118, 'I', CDECL_LOG4)
    put(window, 0xa0, '2I', THING, OWNER)    # (the dwords A0: the thing, the owner)
    count = r.choice([0, 0, 1, 2, 3])
    fonts = bytearray()
    for i in range(count):
        font = bytearray(r.bytes(0x14))
        put(font, 0, 'I', VT)
        put(font, 8, 'I', PATHS + 0x10 * i)
        fonts += font
    # The names: random, or those whose scrambling gives the strings the game compares with, or one that a unit of 0 cuts short.
    plain_a = [r.randrange(1, 0x10000) for _ in range(r.randrange(14))]
    plain_b = [r.randrange(1, 0x10000) for _ in range(r.randrange(14))]
    if plain_a and r.random() < 0.2:
        k = r.randrange(len(plain_a))
        plain_a[k] = k
    secret_a, secret_b = scrambled(plain_a), scrambled(plain_b)
    if r.random() < 0.2:
        secret_a = [r.randrange(1, 0x10000)] + secret_a
    elif r.random() < 0.2:
        secret_b = secret_b[:-1] + [1]
    index = r.randrange(0x10)
    regions = {LOG: log, WINDOW: window, OWNER: put(bytearray(0x20), 0, 'I', VT), THING: r.bytes(0x20), VT: vtable({1: 0, 3: 1, 9: 1}), PARENT: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS + 0x200),
               KIDS: bytes(kid(r, EDIT_A, KIDS + 0x100) + kid(r, EDIT_B, 0) + kid(r, 0x4f3af, 0)), INDEX: struct.pack('<H', index), PATHS: r.bytes(0x40), SECRET_A: text(secret_a), SECRET_B: text(secret_b),
               0x7c2668: text(plain_a), 0x7c2688: text(plain_b), 0x751a34: struct.pack('<2I', SECRET_A, SECRET_B), 0x789c1c: struct.pack('<2I', FONTS, FONTS + 0x14 * count), 0x6fd058: struct.pack('<I', recorder(25, 1, True)),
               0x7ca210: struct.pack('<I', PARENT), 0x7ca214: r.bytes(4), 0x7cf83c: struct.pack('<I', INDEX - 0x1da732 & 0xffffffff), 0x7cfb90: struct.pack('<I', 1) + struct.pack('<I', THING) * (0x10 * 5),
               0xfb34c4: bytes([r.choice([1, 0, 0xfe, 3])])}
    if count:
        regions[FONTS] = bytes(fonts)
    return Call(op, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
