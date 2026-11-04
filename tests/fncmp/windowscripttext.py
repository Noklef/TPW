# The GUI library's window scripts in text (src/game/ui/windowscripttext: 0x66080d, 0x66149c, 0x6607ca; src/game/ui/guitoken: 0x66db20, 0x66ded6, 0x66df68) against the originals, on windowscript.py's window. The CRT's scan 0x67f620 a recorder (driver.cpp's kGuiTokenEnv): its text and
# format compared, its output pointers (the callers' locals) not, the numbers it would write a fill; the line's other tokens real (atoi, strncpy, strcmp, strchr: emulated on one side and the driver's stand-ins on the other). A line may end short: the readers then run on past its terminator.
# Not given: DRAWOBJ / CLIENTOBJ names of 12 bytes or more (the original's local then runs on into the stack), a second RECT on a window (0x665eb0 is not ported), lines under ADDTHUMBBUT's button (its handler 0x66bb9b), negative POLY or column counts (they run on until they wrap).
# usage: windowscripttext.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Command,Rect,Point,Line,Lines,Build)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, strings  # noqa: E402
from listrows import hash_name  # noqa: E402
from window import VTABLE, WINDOW2  # noqa: E402
from windowfactory import no_handlers  # noqa: E402
from windowscript import FLAGS, RANGES, REG, REGWIN, window_env  # noqa: E402

SCAN = 10
CALLS = {'Command': 0x66db20, 'Rect': 0x66ded6, 'Point': 0x66df68, 'Line': 0x66080d, 'Lines': 0x66149c, 'Build': 0x6607ca}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(RANGES + [(0x659b00, 0x659c42), (0x66080d, 0x661464), (0x66149c, 0x661525), (0x6607ca, 0x66080d), (0x66db20, 0x66de4c), (0x66ded6, 0x66dfe6), (0x67a400, 0x6902c0)], hooks={0x67f620: _recorder(SCAN, 6, cdecl=True)})
TEXT, OUT, LIST = RAM + 0xc000, RAM + 0xe000, RAM + 0xe800
COMMANDS = {0: b'CRE', 1: b'DRAWOBJ', 2: b'CLIENTOBJ', 3: b'CLIENTAREA', 4: b'HITREGION', 5: b'END', 6: b'ADDUPBUT', 7: b'ADDDNBUT', 8: b'ADDTHUMBBUT', 9: b'ADDSCROLL', 0xa: b'LISTRECT', 0xb: b'SETCOLUMNS', 0xc: b'ADDCOLUMNBUT', 0xd: b'ANG'}
KIND_NAMES = {1: b'WINDOW', 2: b'BUTTON', 3: b'SCROLL', 4: b'LISTBOX', 5: b'EDITBOX', 6: b'IMAGE', 7: b'LISTCONTROL', 8: b'CHECKBUTTONS', 9: b'BAR', 10: b'TEXTBOX', 11: b'KNOB'}
LEFT = {0: 1, 3: 1, 6: 1, 7: 1, 8: 1, 9: 1, 0xa: 1, 0xc: 1}   # (the commands that read a rectangle)


def coord(r):
    return r.choice([r.randrange(-40, 60)] * 6 + [0, 0x7fff, -0x8000, -1, 0x12345])


def scan(r, log, calls, numbers):
    """the scan's fills: `numbers` written by each of `calls` calls (the arguments past them are other locals: not filled); the pointers not compared"""
    strings(log, SCAN, 1 | 2, out=0b111100)
    for i in range(numbers):
        fill(log, SCAN, 2 + i, b''.join(struct.pack('<i', coord(r)) for _ in range(calls)), which=i, records=calls)


def numbers_text(r, n):
    return b','.join(b'%d' % r.randrange(-99, 99) for _ in range(n))


def word_text(r):
    return b'%d' % r.choice([0, 1, 2, 5, -1, 0x7fff, 0x10000, r.randrange(-99, 99)])


def rect_line(r, command):
    """a line that reads one rectangle (its scan filled by the caller)"""
    args = []
    if command == 0:
        kind = r.randrange(1, 12)
        args = [KIND_NAMES[kind], b'%d' % no_handlers(kind, r.choice(FLAGS)), b'title', b'%d' % r.choice([0, 1, 2, 3, 0x10, r.randrange(1 << 31)])]
    elif command == 0xc:
        args = [word_text(r), b'column']
    return b' '.join([COMMANDS[command]] + args + [numbers_text(r, 4)])


def one_line(r, log, command):
    """any line, with its scan's fills"""
    if command in LEFT:
        scan(r, log, 1, 4)
        text = rect_line(r, command)
        if command == 0 and r.random() < 0.2:   # a kind that is none
            text = b'CRE NOKIND 1 title 1 1,2,3,4'
    elif command in (1, 2):
        text = COMMANDS[command] + b' ' + r.choice([b'name', b'x', b'elevenbytes'])
    elif command == 4:
        shape = r.choice([b'RECT', b'CIRCLE', b'ELLIPSE', b'POLY', b'POLY', b'rect', b'LINE', b'ELLIPSES', b'A_NAME_OF_MANY_BYTES'])
        if shape == b'RECT':
            scan(r, log, 1, 4)
            text = b'HITREGION RECT ' + numbers_text(r, 4)
        elif shape == b'CIRCLE':
            scan(r, log, 1, 2)
            text = b'HITREGION CIRCLE ' + numbers_text(r, 2) + b' ' + word_text(r)
        elif shape == b'ELLIPSE':
            scan(r, log, 2, 2)
            text = b'HITREGION ELLIPSE ' + numbers_text(r, 2) + b' ' + numbers_text(r, 2)
        elif shape == b'POLY':
            n = r.choice([0, 1, 2, 3, 4, 6])
            scan(r, log, max(n, 1), 2)
            text = b' '.join([b'HITREGION POLY', b'%d' % n] + [numbers_text(r, 2) for _ in range(n)])
        else:
            text = b'HITREGION ' + shape + b' 1,2'
    elif command == 0xb:
        n = r.choice([0, 1, 2, 3, 5])
        scan(r, log, max(n, 1), 2)
        text = b' '.join([b'SETCOLUMNS', b'%d' % n] + [numbers_text(r, 2) for _ in range(n)])
    elif command == 0xd:
        strings(log, SCAN, 1 | 2, out=0b111100)
        for i in range(2):   # (past 16 bits too: the words kept)
            fill(log, SCAN, 2 + i, struct.pack('<i', r.deal([5, 0x12345, -0x12345, 0x7fff, -3])), which=i)
        text = b'ANG ' + numbers_text(r, 2)
    else:
        text = COMMANDS.get(command, b'NOPE') + r.choice([b'', b' 1 2'])
    return r.choice([b'', b'', b'', b'', b' ', b' \t']) + text


def block(r, text):
    return text + b'\0' + bytes(r.choice(b'ab 1') for _ in range(0x20)) + b'\0' * 0x10


def registry(r, regions):
    if r.random() < 0.6:
        regions[REG] = struct.pack('<I12sII', REGWIN, b'name', hash_name('name'), 0)
        regions[REGWIN] = struct.pack('<I', VTABLE)
        regions[0xfaa610] = struct.pack('<I', REG)
    else:
        regions[0xfaa610] = struct.pack('<I', 0)


def line_case(r):
    log = log_region()
    command = r.deal(list(COMMANDS) + [0, 3, 4, 4, 4, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xd, 0xd, 0xe])
    regions, _ = window_env(r, log, [11, 11, 3] if command == 0xd else [3, 3, 4, 7, 7, 11, 1], [0] * 30 + [1, 2, 3, 5])
    if r.deal([0, 0, 1]):   # the flag that keeps a scroll's up and down buttons out
        win = regions[WINDOW2]
        put(win, 0x48, 'I', struct.unpack_from('<I', win, 0x48)[0] | 0x40)
    registry(r, regions)
    regions[TEXT] = block(r, one_line(r, log, command))
    regions[OUT] = r.bytes(4)
    return Call(CALLS['Line'], [WINDOW2, TEXT, r.deal([OUT, OUT, 0])], regions, conv='t', ret='w')


def lines_case(r, op):
    log = log_region()
    regions, _ = window_env(r, log, [3, 3, 4, 7, 7, 11, 1], [0])
    registry(r, regions)
    texts = []
    rects = 0
    for _ in range(r.deal([0, 1, 2, 3, 5, 8])):
        command = r.choice([0, 0, 0, 3, 5, 6, 7, 8, 9, 0xa, 0xc, 1, 0xe])
        if command in LEFT:
            rects += 1
            texts.append(rect_line(r, command))
            if command == 8:
                texts.append(b'END')
        else:
            texts.append(COMMANDS.get(command, b'NOPE') + (b' name' if command == 1 else b''))
    scan(r, log, max(rects, 1), 4)
    pointers = []
    lines = bytearray()
    for text in texts + [b'END'] * 12:
        pointers.append(TEXT + len(lines))
        lines += block(r, text)
    regions[TEXT] = bytes(lines)
    regions[LIST] = struct.pack('<%dI' % len(pointers), *pointers)
    regions[OUT] = r.bytes(4)
    if op == 'Lines':
        return Call(CALLS[op], [WINDOW2, LIST, r.deal([OUT, OUT, 0]), r.deal([0, 0, 1])], regions, conv='t', ret='w')
    return Call(CALLS[op], [WINDOW2, LIST, r.deal([0, 0x66bb9b])], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Line':
        return line_case(r)
    if op in ('Lines', 'Build'):
        return lines_case(r, op)
    log = log_region()
    regions = {LOG: log, OUT: r.bytes(0x20)}
    if op == 'Command':
        name = r.deal(list(COMMANDS.values()) + [b'NOPE', b'end', b'EN', b'ENDS'])
        regions[TEXT] = block(r, r.choice([b'', b'', b' ', b' \t']) + name + r.choice([b'', b' 1 2', b' ']))
        return Call(CALLS[op], [TEXT, r.deal([OUT, OUT, 0])], regions, ret='a')
    regions[TEXT] = block(r, r.choice([b'1,2,3,4 5,6', b'1,2,3,4', b'', b' 7', b'a,b']))
    n = 4 if op == 'Rect' else 2
    outs = [OUT + 4 * i for i in r.sample(range(8), n)]
    if r.deal([0, 0, 0, 1]):
        outs[r.randrange(n)] = 0
    strings(log, SCAN, 1 | 2, out=0 if n == 4 else 0b110000)
    for i in range(n):
        fill(log, SCAN, 2 + i, struct.pack('<i', coord(r)), which=i)
    return Call(CALLS[op], [TEXT] + outs, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
