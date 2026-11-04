# The C library's fatal messages and path splitting (src/game/core/crtmessage) against the originals: the Win32 calls COM recorders in their import slots, the locks cdecl recorders, the message table 0x781680 made here
# (the driver has not the exe's strings). The exit recorders return, so the code after them runs to its end.
# usage: crt_message.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, import_table, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, EXIT, HOOK = 7, 8, 9, 10
WRITE, STD, MODULE, EXITP, LOAD, FIND, BOX, ACTIVE, POPUP = 14, 15, 16, 17, 18, 19, 20, 21, 22
B = Bench([(0x67efc0, 0x67efea), (0x67eff0, 0x67f018), (0x68a170, 0x68a1ad), (0x68a1b0, 0x68a386), (0x68efb0, 0x68f03f), (0x67ae00, 0x67aefe), (0x68a390, 0x68a437), (0x67f020, 0x67f191)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
# The listing stops at the calls that never return in the game; the exe's bytes after them (83 c4 04 c3, c3) end the run here, where the exit recorders return.
B.emu.code.update({0x67efe6: ('ADD', 'ESP,0x4'), 0x67efe9: ('RET', ''), 0x67f016: ('RET', '')})
B.emu.order = sorted(set(B.emu.order) | {0x67efe6, 0x67efe9, 0x67f016})
B.emu.next = dict(zip(B.emu.order, B.emu.order[1:]))
TEXTS, BLOCK, PATH, OUT, ANSWERS = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x800, RAM + 0x1000
CODES = [2, 8, 9, 0x10, 0x12, 0x1e, 0xfc, 0xff, 0xfc, 0xff, 0, 1, 0x100]
OPS = [n for n in ('amsg', 'fast', 'banner', 'nmsg', 'nmsg', 'box', 'mbsnbcpy', 'mbsnbcpy', 'splitpath', 'splitpath') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
PATHS = [b'', b'\0:x', b'a', b'C:', b'C:\\', b'C:\\dir\\name.ext', b'C:/dir/sub\\name', b'name.ext', b'.ext', b'dir.d/name', b'dir.d\\', b'a:b:c', b'C:name.tar.gz', b'\x81\\dir\\n\x81.x', b'\\\x81', b'd\\' + b'n' * 0x120 + b'.' + b'e' * 0x120, b'd' * 0x120 + b'\\x']


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'write': (WRITE, 0x6fd0e0, 5, 0b10, 0b1000), 'std': (STD, 0x6fd1ac, 1, 0, 0), 'module': (MODULE, 0x6fd194, 3, 0, 0b10), 'exit': (EXITP, 0x6fd244, 1, 0, 0),
                                                         'load': (LOAD, 0x6fd0b4, 1, 0, 0), 'find': (FIND, 0x6fd0b0, 2, 0, 0)})}
    strings(log, BOX, 0b10)
    texts = bytearray(0x400)
    entries = []
    for i, code in enumerate([2, 8, 9, 0xa, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0xfc, 0xff]):
        text = b'message %d\r\n' % code if code < 0xfc else (b'\r\n' if code == 0xfc else b'runtime error ')
        texts[0x20 * i:0x20 * i + len(text)] = text
        entries += [code, TEXTS + 0x20 * i]
    texts[0x3e0:0x3e8] = b'beyond\0\0'
    regions[TEXTS] = texts
    beyond = r.random() < 0.1
    regions[0x781680] = struct.pack('<38I', *entries, 0x100 if beyond else 0xd2d0920, TEXTS + 0x3e0)
    regions[0xfb135c] = struct.pack('<I', r.choice([0, 0, 1, 1, 2]))
    regions[0x77ea84] = struct.pack('<I', r.choice([1, 1, 2]))
    regions[0x77ea80] = struct.pack('<I', recorder(EXIT, 1, cdecl=True))
    regions[0xfb1888] = struct.pack('<I', r.choice([0, recorder(HOOK, 0, cdecl=True)]))
    regions[BLOCK] = struct.pack('<20I', *[r.choice([0x1234, 0x1234, 0xffffffff]) for _ in range(20)])
    regions[0xfb3ba0] = struct.pack('<I', r.choice([BLOCK, BLOCK, 0]))
    set_return(log, STD, 0x5555)
    set_return(log, MODULE, r.choice([0x20, 0x20, 0]))
    fill(log, MODULE, 1, r.choice([b'C:\\game\\tp.exe\0', b'C:\\' + b'p' * 0x38 + b'\0', b'C:\\' + b'q' * 0x39 + b'\0', b'C:\\' + b'r' * 0x60 + b'\\tp.exe\0']))
    set_return(log, LOAD, r.choice([0x7000, 0x7000, 0x7000, 0]))
    regions[ANSWERS] = struct.pack('<3I', r.choice([recorder(BOX, 4, True), recorder(BOX, 4, True), recorder(BOX, 4, True), 0]), r.choice([recorder(ACTIVE, 0, True), 0]), r.choice([recorder(POPUP, 1, True), 0]))
    answers(log, FIND, ANSWERS)
    set_return(log, BOX, r.choice([1, 2, 3]))
    set_return(log, ACTIVE, r.choice([0, 0x900]))
    set_return(log, POPUP, 0xa00)
    loaded = r.random() < 0.3
    regions[0xfb1900] = struct.pack('<3I', recorder(BOX, 4, True) if loaded else 0, r.choice([recorder(ACTIVE, 0, True), 0]) if loaded else r.getrandbits(32), r.choice([recorder(POPUP, 1, True), 0]) if loaded else r.getrandbits(32))
    code = 0x100 if beyond and r.random() < 0.7 else r.choice(CODES)
    if name in ('amsg', 'fast'):
        return Call(0x67efc0 if name == 'amsg' else 0x67eff0, [code], regions, conv='c')
    if name == 'banner':
        return Call(0x68a170, [], regions, conv='c')
    if name == 'nmsg':
        return Call(0x68a1b0, [code], regions, conv='c')
    if name == 'box':
        regions[PATH] = b'a text\0'
        return Call(0x68efb0, [PATH, PATH + 2, r.getrandbits(32)], regions, conv='c', ret='a')
    lead = bytearray(0x101)
    if r.random() < 0.7:
        lead[0x81] = lead[0x82] = 4
    regions[0xfb1669] = bytes(lead)
    regions[0xfb1870] = struct.pack('<I', r.choice([0, 1, 1]))
    regions[OUT] = r.bytes(0x600)
    if name == 'mbsnbcpy':
        text = bytes(r.choice([0x41, 0x41, 0x81, 0x82, 0]) for _ in range(r.choice([0, 1, 2, 3, 5, 8, 0x21]))) + b'\0'
        regions[PATH] = text + r.bytes(0x10)
        return Call(0x68a390, [OUT + r.randrange(4), PATH + r.randrange(0, 2) * (len(text) > 1), r.choice([0, 1, 2, 3, 4, 7, 8, 0x10, 0x40])], regions, conv='c', ret='a')
    regions[PATH] = r.choice(PATHS) + b'\0'
    parts = [OUT + 0x180 * i if r.random() < 0.8 else 0 for i in range(4)]
    return Call(0x67f020, [PATH] + parts, regions, conv='c')


if __name__ == '__main__':
    B.main(case)
