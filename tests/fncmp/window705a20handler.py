# The edit window 0x705a20's handler and mouse (src/game/ui/window705a20handler: 0x66857d, 0x6680b7, 0x668269, 0x6681ac) against the original. Recorders as driver.cpp's kWindow705a20HandlerEnv; wcslen and the byte getter 0x668800 real;
# the list's slot 4 on a recorder of its own answering the table's places in turn, +0x4c's slot 1 the height. The word arguments are logged as words (the original pushes registers with other high halves).
# usage: window705a20handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, log_region, put, recorder, set_return, vtable  # noqa: E402

PLACE, HEIGHT, CARET, FOCUS, STOP, START, INSERT, ALL, TOGGLE, KEY, PARENT, TYPE, POST, DEFAULT, SELECT = range(14, 29)
B = Bench([(0x66857d, 0x668783), (0x6680b7, 0x6681ac), (0x668269, 0x668368), (0x6681ac, 0x66825c), (0x66825c, 0x668269), (0x668800, 0x668820), (0x67acd0, 0x67acf0), (0x680050, 0x68005b)],
          hooks={0x66836a: _recorder(CARET, 2), 0x65e59b: _recorder(FOCUS, 1), 0x662097: _recorder(STOP, 0), 0x6620ac: _recorder(START, 0), 0x666875: _recorder(INSERT, 1), 0x6677ae: _recorder(ALL, 0),
                 0x6667eb: _recorder(TOGGLE, 0), 0x667a72: _recorder(KEY, 2), 0x667fee: _recorder(PARENT, 2), 0x667833: _recorder(TYPE, 1), 0x65dcaf: _recorder(POST, 3),
                 0x65f6d1: _recorder(DEFAULT, 4, cdecl=True), 0x6668e9: _recorder(SELECT, 3)})
WINDOW, LIST, FONT, TEXT, TABLE, PLACES, SLOTS = RAM + 0x1000, RAM + 0x1200, RAM + 0x1300, RAM + 0x1400, RAM + 0x1500, RAM + 0x1600, RAM + 0x1a00
TIMER = RAM + 0x1b00
MESSAGES = [0x13, 0x1a, 0x1f, 0x20, 0x10, 0x10003, 0x10003, 0x10003, 0x10004, 0x10005, 0x10005, 0x10005, 0x10006, 0x10007, 0x10008, 0x10009, 0x1000a, 0x1000b, 0x1000c, 0xc]


def words(log, k, mask):
    struct.pack_into('<I', log, LOG_BYTES + 4 * k, mask << 16)


def case(r):
    log = log_region()
    for k, mask in ((CARET, 3), (KEY, 1), (PARENT, 1), (TYPE, 1), (SELECT, 5), (INSERT, 1)):
        words(log, k, mask)
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, HEIGHT, r.deal([0, 1, 2, 16, 0xffff, r.getrandbits(32)]))
    length = r.deal([0, 1, 2, 3, 5])
    window = bytearray(r.bytes(0x170))
    window[0x161] = r.deal([0, 0, 0, 0, 1])
    window[0x162] = r.deal([0, 0, 0, 0, 1])
    put(window, 0x11c, 'I', r.deal([0, 1, 1, 1, 3, 2]))
    put(window, 0x138, 'I', r.deal([TEXT, TEXT, TEXT, TEXT, 0]))
    put(window, 0x9c, 'I', r.deal([LIST, LIST, LIST, LIST, 0]))
    put(window, 0x4c, 'I', r.deal([FONT, FONT, FONT, FONT, 0]))
    put(window, 0x164, 'I', r.deal([TIMER, 0]))
    window[0x168:0x16c] = bytes(4)
    put(window, 0x30, 'h', r.deal([0, 5, -5, r.randrange(-300, 300)]))
    put(window, 0x32, 'h', r.deal([0, 5, -5, r.randrange(-300, 300)]))
    op = r.deal(['Handler', 'Handler', 'Nearest', 'Down', 'Move'])
    places = bytearray(0x400)
    table = bytearray(0xc0)
    first = 0
    if op != 'Nearest':  # (the first answer,the check of slot 4 (1, 0), is 0 now and then)
        put(table, 0, 'I', r.deal([PLACES, PLACES, PLACES, 0]))
        first = 1
    same = r.deal([0, 1, 2, 3])   # (0: places apart in both; 1: the same x, ties in y; 2: the same y; 3: lines near each other, x far: y's weight against x, and half a line's height)
    for i in range(6):
        if same == 3:
            put(places, 0x10 * i + 4, '2h', r.choice([0, 6000, 12000, 20000]), r.choice([0, 1, 2, 8, 16, -8]))
        else:
            put(places, 0x10 * i + 4, '2h', 5 if same == 1 else r.choice([0, 3, 10, 10, -10, 0x7fff, -0x8000]), 7 if same == 2 else r.choice([0, 3, 10, 10, -10, 0x7fff, -0x8000]))
        put(table, 4 * (i + first), 'I', PLACES + 0x10 * i)
    regions = {LOG: log, WINDOW: window, LIST: struct.pack('<I', SLOTS), FONT: struct.pack('<I', SLOTS + 0x40), SLOTS: put(bytearray(0x80), 0x10, 'I', recorder(PLACE, 2)), TIMER: r.bytes(0x20),
               TEXT: bytes(b for _ in range(length) for b in struct.pack('<H', 0x41)) + b'\0\0', TABLE: table, PLACES: places}
    put(regions[SLOTS], 0x44, 'I', recorder(HEIGHT, 0))
    answers(log, PLACE, TABLE)
    point = r.deal([0, 0x50005, 0xfffbfffb, r.getrandbits(32), r.randrange(-200, 200) << 16 & 0xffffffff | r.randrange(0, 200)])
    if op == 'Nearest':
        return Call(0x6681ac, [r.deal([LIST, LIST, LIST, 0]), r.deal([0, 3, 10, -10, r.getrandbits(32)]), r.deal([0, 3, 10, -10, r.getrandbits(32)]), r.deal([-1, 0, 1, 2, 3, 5])], regions, ret='a')
    button = r.deal([0, 0, 0, 1, r.getrandbits(32)])
    if op == 'Down':
        return Call(0x6680b7, [WINDOW, button, point], regions, conv='t')
    if op == 'Move':
        return Call(0x668269, [WINDOW, point], regions, conv='t')
    message = r.deal(MESSAGES)
    return Call(0x66857d, [WINDOW, message, button if message != 0x10003 else point, point], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
