# The postcard sent (src/game/online/postcardsend: 0x4a8600) against the original. The unported calls, the strings' assigns, strtok, TokenList_PushChars, the post and the command queue recorders as driver.cpp's kPostcardSendEnv (strtok's answers in turn: tokens or none); the heap, sprintf and wcslen real.
# The mail's constructor answers its heap block (the first asked for); the heap refusing it faults in both, past the assigns. The session made or there; logged in or not; the host none, empty or a name; the wide strings short (the original's stack buffers hold no more).
# usage: postcardsend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, heap, import_table, log_region, put, set_return, strings  # noqa: E402
import thing_ctors  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

SESSION_CTOR, SAVE, MAIL_CTOR, ASSIGN, STRTOK, PUSH, ATTACH, MANAGER_CTOR, USE, ASSIGN_WIDE, CONNECT, COMMAND, COPY, POST = 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 21
B = Bench([(0x4a8600, 0x4a8acd), (0x5b6450, 0x5b6454), (0x5b6480, 0x5b6484), (0x5b6490, 0x5b6494), (0x5b64a0, 0x5b64a4), (0x41ce80, 0x41ce84), (0x41ce90, 0x41ce94), (0x4bae30, 0x4bae3f), (0x40f630, 0x40f650), (0x4a8bd0, 0x4a8bf7), (0x67a400, 0x6902c0)] + [x for x in thing_ctors.CENTRE_RANGES if x[0] < 0x67a400 or x[0] >= 0x6902c0],
          hooks={**thing_ctors.CENTRE_HOOKS, **{at: interlocked(step) for at, (_, step) in INTERLOCKED.items()}, 0x5b50b0: _recorder(SESSION_CTOR, 0), 0x5d4a30: _recorder(SAVE, 1), 0x5c93c0: _recorder(MAIL_CTOR, 0), 0x45b5e0: _recorder(ASSIGN, 2), 0x67c4e0: _recorder(STRTOK, 2, cdecl=True), 0x4a8ad0: _recorder(PUSH, 1), 0x5c99f0: _recorder(ATTACH, 1), 0x5c7420: _recorder(MANAGER_CTOR, 0),
                 0x5ca010: _recorder(USE, 1), 0x4051d0: _recorder(ASSIGN_WIDE, 2), 0x5c9f10: _recorder(CONNECT, 4), 0x47f410: _recorder(COMMAND, 2, cdecl=True), 0x40fb10: _recorder(POST, 1)})
SESSION, WIDE, TOKENS, TURNS, HOST, PANEL, MANAGER = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3800, RAM + 0x3a00, RAM + 0x4000, RAM + 0x5000


def text(r, n):
    return bytes(r.choice(b'abcdefgh.\\') for _ in range(r.randrange(n)))


def wide(s):
    return b''.join(bytes([c, 0]) for c in s) + b'\0\0'


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x3000, r.deal([0] * 12 + [1, 2]))}
    imports = bytearray(import_table(log, {'CopyFileA': (COPY, 0x6fd158, 3, 3, 0)}))
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions[IMPORT_TABLE] = imports
    regions[LOCALE] = bytes(4)   # (the CRT's wctomb: see window705950draw.py)
    regions[0xfb3ca4] = bytes(8)
    regions[0x7cb958] = text(r, 0x20) + b'\0'
    regions[0x7cbcb0] = bytes(0x100)
    regions[0x7cb408] = wide(r.deal([b'', b'body', b'a longer body of a card', text(r, 0x40)]))
    regions[0x7cb4d0] = wide(r.deal([b'', b'a@b', b'a@b.c;d@e.f', b'a@b, c@d\ne@f', text(r, 0x40)]))
    regions[0x7cbf38] = struct.pack('<i', r.deal([0, 1, 99999999, -1, r.getrandbits(31)]))
    regions[0x7cc18c] = struct.pack('<I', PANEL)
    regions[PANEL] = r.bytes(0x10)
    strings(log, SAVE, 1)
    strings(log, ASSIGN, 1)
    strings(log, STRTOK, 3)
    strings(log, PUSH, 1)
    strings(log, ATTACH, 1)
    strings(log, CONNECT, 1)
    set_return(log, MAIL_CTOR, HEAP + 0x10)
    tokens = bytearray(0x40)
    table = bytearray(0xc0)
    n = r.deal([0, 1, 1, 2, 3, 5])
    for i in range(n):
        put(tokens, 8 * i, '7s', text(r, 7))
        put(table, 4 * i, 'I', TOKENS + 8 * i)
    regions[TOKENS] = tokens
    regions[TURNS] = table
    answers(log, STRTOK, TURNS)

    session = bytearray(r.bytes(0x404))
    put(session, 0x60, 'I', r.deal([0, 1, 0x100]))
    put(session, 0x14, 'I', WIDE)
    put(session, 0x28, 'I', WIDE + 0x80)
    put(session, 0x44, 'I', WIDE + 0x100)
    name = bytearray(r.deal([b'', b'tim', b'noklef', b'a name of many bytes']))
    regions[WIDE] = bytearray(0x200)
    put(regions[WIDE], 0, '%ds' % 0x40, wide(text(r, 0x1e)))
    put(regions[WIDE], 0x80, '%ds' % 0x40, wide(text(r, 0x1e)))
    w = bytearray(wide(bytes(name)))
    if w[:2] and r.deal([0, 0, 1]):   # (high bytes the copy skips)
        w[1] = 0x41
    regions[WIDE][0x100:0x100 + len(w)] = w
    regions[SESSION] = session
    regions[0xf7d88c] = struct.pack('<I', r.deal([SESSION, SESSION, SESSION, 0]))
    set_return(log, SESSION_CTOR, SESSION)

    regions[MANAGER] = r.bytes(0x10)
    regions[0xf81ebc] = struct.pack('<I', r.deal([MANAGER, MANAGER, 0]))
    set_return(log, MANAGER_CTOR, MANAGER)
    regions[HOST] = r.deal([b'host.name\0', b'\0', b'a\0'])
    regions[0x7a14f4] = struct.pack('<I', r.deal([HOST, HOST, HOST, 0]))
    regions[0x7a1550] = struct.pack('<h', r.deal([25, 21, -1, 0x7fff, -0x8000]))
    regions[0x7cc53c] = bytes([r.deal([0, 1, 0xff])])
    set_return(log, CONNECT, r.deal([0, 1, 0x100]))
    thing_ctors.centre(r, regions)
    blob(log, POST, 0, 0x7)
    block = 0xb0   # (the centre's, after the mail's 0x94 bytes, the session's and the manager's when made)
    if regions[0xf7d88c] == bytes(4):
        block += 0x410
    if regions[0xf81ebc] == bytes(4) and not struct.unpack_from('<I', session, 0x60)[0]:
        block += 0x110
    return Call(0x4a8600, [], regions, ret='v', masks=thing_ctors.centre_masks(regions, block))


if __name__ == '__main__':
    B.main(case)
