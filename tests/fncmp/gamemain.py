# The game's WinMain (src/game/core/gamemain: 0x45a960) against the original: the strings and the heap real; every other callee and the message imports recorders (the messages hashed, the peek, get and frame answers dealt from tables); the flags, slots, online object and chat client varied. Bit 0 of the flags is always set by then, so the loop's test of it goes the one way.
# usage: gamemain.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, blob_bytes, fill, global_heap, heap, import_table, log_region, set_return, strings  # noqa: E402
import gamestring  # noqa: E402

CHECK, ARG, GETCWD, ROOTS, SUBFOLDERS, BALANCE, STARTUP, DISC, SCREEN, CREATE, GAMESTART, POLL, FRAME, DOWN, SHUT = range(1, 16)
SLOTS_DTOR, SLOTS_CTOR, SLOTS_INIT, ONLINE_DTOR, LOADFILE, LANGLOAD, FIXED, FONTS, ONLINE_CTOR, SLOTS_END, CHAT_DTOR, ERRSHOW, FREEFONTS, SELF = range(16, 30)
REGMSG, PEEK, TRANSLATE, DISPATCH, GETMSG = range(40, 45)
CD = {0x45f640: (CHECK, 0), 0x5f86b0: (ARG, 1), 0x5f5690: (ARG, 1), 0x67b9f0: (GETCWD, 2), 0x5c0c10: (ROOTS, 1), 0x5c0d00: (ROOTS, 1), 0x5c2ce0: (SUBFOLDERS, 2), 0x45b1f0: (BALANCE, 0), 0x45ba20: (BALANCE, 0), 0x449df0: (STARTUP, 0),
      0x45f7a0: (DISC, 0), 0x449eb0: (SCREEN, 0), 0x449ec0: (SCREEN, 0), 0x44e080: (CREATE, 0), 0x54dcf0: (GAMESTART, 0), 0x54e360: (FRAME, 0), 0x51af90: (DOWN, 0), 0x44a1d0: (DOWN, 0), 0x5503f0: (SHUT, 0), 0x44e560: (SHUT, 0)}
TH = {0x5b5bf0: (POLL, 0), 0x5c74b0: (SLOTS_DTOR, 0),0x5c7420: (SLOTS_CTOR, 0), 0x5c7590: (SLOTS_INIT, 0), 0x5b5920: (ONLINE_DTOR, 0), 0x424930: (LOADFILE, 1), 0x419710: (LANGLOAD, 0), 0x408470: (FIXED, 0), 0x41a5a0: (FONTS, 1), 0x5b50b0: (ONLINE_CTOR, 0),
      0x5c8650: (SLOTS_END, 0), 0x4e5950: (CHAT_DTOR, 0), 0x405750: (ERRSHOW, 0), 0x41a5f0: (FREEFONTS, 0), 0x5c5520: (SELF, 0)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()}
HOOKS.update({a: _recorder(k, n) for a, (k, n) in TH.items()})
B = Bench([(0x45a960, 0x45ad59), (0x5da3c0, 0x5da3c1), (0x424dc0, 0x424e10), (0x424e10, 0x424e6e), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES, hooks=HOOKS)
SLOTS, ONLINE, CHAT, OLD_SLOTS, PEEK_T, GET_T, FRAME_T = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
IMPORTS = {'RegisterWindowMessageA': (REGMSG, 0x6fd350, 1, 0, 0), 'PeekMessageA': (PEEK, 0x6fd354, 5, 0, 0), 'TranslateMessage': (TRANSLATE, 0x6fd358, 1, 0, 0), 'DispatchMessageA': (DISPATCH, 0x6fd35c, 1, 0, 0), 'GetMessageA': (GETMSG, 0x6fd360, 4, 0, 0)}


def table(values, rest):
    return struct.pack('<48I', *(values + [rest] * (48 - len(values))))


def case(r):
    log = log_region()
    imports = import_table(log, IMPORTS)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    run = r.random() < 0.9
    set_return(log, CHECK, r.choice([1, 1, 0x101]) if run else r.choice([0, 2, 0x100]))
    flags = r.getrandbits(32) & ~1
    flags = flags & ~2 if r.random() < 0.85 else flags | 2
    flags = flags & ~4 if r.random() < 0.6 else flags | 4
    for k in (GETCWD, SELF, REGMSG):
        set_return(log, k, r.getrandbits(32))
    strings(log, GETCWD, 0, 1)
    strings(log, ROOTS, 0, 1)
    strings(log, SUBFOLDERS, 0, 1)
    blob_bytes(log, LOADFILE, 0, 0, inner=(8, 0xffff))
    set_return(log, SLOTS_CTOR, SLOTS)
    set_return(log, ONLINE_CTOR, ONLINE)
    set_return(log, LANGLOAD, r.choice([0, 1, 1, 1, 1, 1, 0x100]))
    set_return(log, DISC, r.choice([0, 0x100, 1, 1, 1, 1, 1, 0xff]))
    set_return(log, CREATE, r.choice([0, 1, 1, 1, 1, 1, 0x100]))
    answers(log, PEEK, PEEK_T)
    answers(log, GETMSG, GET_T)
    answers(log, FRAME, FRAME_T)
    strings(log, PEEK, 0, 1)
    strings(log, GETMSG, 0, 1)
    msg = r.bytes(28)
    fill(log, PEEK, 0, msg, which=0)
    fill(log, GETMSG, 0, r.bytes(28), which=1)
    blob(log, TRANSLATE, 0, 0x7f)
    blob(log, DISPATCH, 0, 0x7f)
    slots = bytearray(r.bytes(0x80))
    struct.pack_into('<i', slots, 0x60, r.choice([-1, 0, 5]))
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 10 + [1, 1, 2])), gamestring.LOCALE: bytes(4), SLOTS: slots, ONLINE: r.bytes(0x10), CHAT: r.bytes(0x10), OLD_SLOTS: r.bytes(0x10),
               # (a slot's answers stop moving once the log holds 40 calls: at most two messages and one frame that goes on, so the loop ends before that)
               PEEK_T: table([r.choice([0, 1, 1]) for _ in range(r.randrange(0, 3))], 0),
               GET_T: table([r.choice([1, 1, 1, 1, 0, 0x100]) for _ in range(2)], 1),
               FRAME_T: table([r.choice([0, 0, 8, 0x10]) for _ in range(r.randrange(0, 2))], 2),
               0x7a1a14: struct.pack('<I', flags), 0x7a1968: r.bytes(4), 0x7a0eb0: r.bytes(4), 0x7a1a44: r.bytes(4), 0x7a1614: r.bytes(4), 0x7a0eac: r.bytes(4), 0x7a1c5c: r.bytes(4),
               0xf81ebc: struct.pack('<I', r.choice([0, OLD_SLOTS, SLOTS])), 0xf7d88c: struct.pack('<I', r.choice([0, ONLINE])), 0x7cddec: struct.pack('<I', r.choice([0, CHAT]))}
    # (the message's wParam compared only where the loop ran: else the original's slot is stack garbage)
    return Call(0x45a960, [r.getrandbits(32) for _ in range(4)], regions, conv='s', ret='a' if run and not flags & 2 else 'v')


if __name__ == '__main__':
    B.main(case)
