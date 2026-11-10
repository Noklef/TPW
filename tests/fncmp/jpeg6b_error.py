# libjpeg 6b's jerror.c methods (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the module through its wrapper unit
# third_party/wrap/jpeg-6b/jerror.c) against the exe's copies: emit_message (0x631340), format_message (0x631380), reset_error_mgr
# (0x631440). cinfo->err (+0, at ERR) is a jpeg_error_mgr of 0x84 bytes whose output_message (+8) is a recorder. format_message's
# sprintf is the exe's C runtime's (0x67a650), which the library's calls through jpeg6b.cpp's jpeg6b_exe_sprintf: the driver's
# Jpeg6b_SetCrtEnv gives it two recorders, one for each of format_message's calls (the string: buffer, text, msg_parm.s; the ints:
# buffer, text and the eight), and the emulator's hook on 0x67a650 tells the calls apart the way format_message does (the text's
# first % followed by an s), so each side logs the call it made with the arguments it passed. The texts are the exe's own table
# (0x7039d0, 0x7b codes) or a small one of formats in STR, with an add-on table or none.
# error_exit (0x6312d0): output_message a recorder, jpeg_destroy (the library's, jcomapi.c's 0x63e040 in the exe) with cinfo->mem at
# MEM (self_destruct +0x28 a recorder) or none, then the exe's exit (0x67f920), which ends the case where it is called (fncmp's
# _exit_case, the driver's exit_case through Jpeg6b_SetCrtEnv: the regions are compared as they are then). output_message (0x631300):
# format_message (+0xc) a recorder logging the buffer (on the stack) as 0 and writing a string of up to 0x6f bytes into it, then the
# exe's fprintf (0x67dad0) a recorder of the exe's stderr (0x77e840), its "%s\n" (0x764ff0, .data: varied here) and the buffer, the
# last two as strings. jpeg_std_error stays the exe's (docs/porting-libraries.md).
# usage: jpeg6b_error.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=emit,format,reset,exit,output)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _exit_case, _recorder, fill, log_region, recorder, strings  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'emit_message': (0x631340, 0x631380), 'format_message': (0x631380, 0x631440), 'reset_error_mgr': (0x631440, 0x631460),
         'error_exit': (0x6312d0, 0x6312f3), 'output_message': (0x631300, 0x631336), 'jpeg_destroy': (0x63e040, 0x63e063)}
K_OUTPUT, K_FORMAT, K_DESTRUCT = 1, 2, 3
K_SPRINTF_S, K_SPRINTF_I, K_EXIT, K_FPRINTF = 27, 28, 29, 30  # (driver.cpp's kJpeg6bCrtEnv)
STDERR, NEWLINE = 0x77e840, 0x764ff0  # (the exe's stderr, its "%s\n")
EXE_TABLE, EXE_LAST = 0x7039d0, 0x7b
_sprintf_s, _sprintf_i = _recorder(K_SPRINTF_S, 3, cdecl=True), _recorder(K_SPRINTF_I, 10, cdecl=True)


def _sprintf(e):
    """The exe's sprintf (0x67a650): format_message's string call or its ints call, by the text as format_message tells them apart."""
    p = e.u32(e.r['ESP'] + (4 if e.tail else 0) + 4)
    isstring = False
    while True:
        ch = e.read(p, 1)[0]
        p += 1
        if ch == 0:
            break
        if ch == 0x25:
            isstring = e.read(p, 1)[0] == 0x73
            break
    (_sprintf_s if isstring else _sprintf_i)(e)


B = Bench(sorted(FUNCS.values()), hooks={0x67a650: _sprintf, 0x67f920: _exit_case(K_EXIT), 0x67dad0: _recorder(K_FPRINTF, 3, cdecl=True)})

CINFO, ERR, TABLE, ADDON, STR, BUF, MEM = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800, RAM + 0x900
TEXTS = [b'plain', b'', b'%d and %d', b'%s', b'file %s here', b'100%', b'%%s', b'%x%s', b'a%sb%d', b'%u %u %u %u %u %u %u %u', b'%c']


def common(r):
    log = log_region()
    cinfo = bytearray(r.bytes(0x10))
    struct.pack_into('<I', cinfo, 0, ERR)
    err = bytearray(r.bytes(0x84))
    struct.pack_into('<I', err, 8, recorder(K_OUTPUT, 1, cdecl=True))
    return {CINFO: cinfo, ERR: err, LOG: log}


def format_(r):
    regs = common(r)
    err = regs[ERR]
    strs, at = bytearray(), []
    for t in TEXTS:
        at.append(STR + len(strs))
        strs += t + b'\0'
    regs[STR] = bytes(strs)
    if r.random() < 0.3:
        table, last = EXE_TABLE, r.choice([EXE_LAST, EXE_LAST, 0, 5])
    else:
        n = r.randrange(1, 9)
        regs[TABLE] = struct.pack('<%dI' % n, *[r.choice(at) if i == 0 or r.random() < 0.9 else 0 for i in range(n)])  # (message 0 is read for any code)
        table, last = TABLE, r.choice([n - 1, n - 1, 0, r.randrange(n)])
    addon = r.random() < 0.5
    if addon:
        n = r.randrange(1, 6)
        regs[ADDON] = struct.pack('<%dI' % n, *[r.choice(at) if r.random() < 0.9 else 0 for _ in range(n)])
        first = r.choice([1000, 1000, last + 1, last, 1, r.randrange(-5, 2000)])
        hi = r.choice([first + n - 1, first + n - 1, first, first - 1])
    else:
        first, hi = r.choice([(0, 0), (1000, 1004)])
    code = r.choice([0, 1, last, last + 1, -1, first, hi, first + 1, hi + 1, first - 1, r.randrange(-3, last + 2),
                     r.randrange(first - 2, hi + 3), r.getrandbits(32) - (1 << 31)])
    struct.pack_into('<i', err, 0x14, code)
    struct.pack_into('<Ii', err, 0x70, table, last)
    struct.pack_into('<Iii', err, 0x78, ADDON if addon else 0, first, hi)
    regs[BUF] = b'\0' * 0xc8
    return Call(0x631380, [CINFO, BUF], regs)


def emit(r):
    regs = common(r)
    struct.pack_into('<ii', regs[ERR], 0x68, r.choice([0, 1, 2, 3, 3, 4, -1, r.getrandbits(32) - (1 << 31)]),
                     r.choice([0, 0, 1, 2, r.getrandbits(32) - (1 << 31)]))
    return Call(0x631340, [CINFO, r.choice([-1, -1, -2, 0, 1, 2, 3, 4, 5, r.getrandbits(32) - (1 << 31)])], regs)


def reset(r):
    return Call(0x631440, [CINFO], common(r))


def exit_(r):
    regs = common(r)
    regs[CINFO] = bytearray(r.bytes(0x18))
    struct.pack_into('<II', regs[CINFO], 0, ERR, MEM if r.random() < 0.8 else 0)
    regs[MEM] = r.bytes(0x28) + struct.pack('<I', recorder(K_DESTRUCT, 1, cdecl=True)) + r.bytes(0x24)
    return Call(0x6312d0, [CINFO], regs, ret='v')


def output(r):
    regs = common(r)
    struct.pack_into('<I', regs[ERR], 0xc, recorder(K_FORMAT, 2, cdecl=True))
    strings(regs[LOG], K_FORMAT, 0, out=0b10)
    strings(regs[LOG], K_FPRINTF, 0b110)
    text = bytes(r.choice([0x41, 0x7a, 0x25, 0x20, r.randrange(1, 256)]) for _ in range(r.choice([0, 1, 5, 0x40, 0x6f])))
    fill(regs[LOG], K_FORMAT, 1, text + b'\0')
    regs[NEWLINE] = r.choice([b'%s\n\0', b'%s\n\0', b'x%sy\0', bytes(r.randrange(1, 256) for _ in range(3)) + b'\0'])
    return Call(0x631300, [CINFO], regs, ret='v')


OPS = [f for f in (emit, format_, reset, exit_, output) if not os.environ.get('OPS') or f.__name__.rstrip('_') in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
