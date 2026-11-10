# The C++ library's time_get (src/game/core/timeget) against the originals: _Getloctxt of char and wchar_t, and do_get_weekday / do_get_monthname of both, over the
# strings' ports: the stream buffer's underflow and uflow thiscall recorders, the locks do nothing.
# usage: timeget.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, heap, log_fold, log_region, recorder  # noqa: E402

LOCK, UNLOCK, UNDERFLOW, UFLOW, THROW = 7, 8, 10, 11, 14


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x6ab3b0, 0x6ab6d3), (0x6a4a30, 0x6a4d73), (0x6aaa90, 0x6aaaf0), (0x6aaaf0, 0x6aab50), (0x6a3de0, 0x6a3e40), (0x6a3e40, 0x6a3ea0),
           (0x698600, 0x698640), (0x698bf0, 0x698c44), (0x4f1eb0, 0x4f1ed7), (0x69f870, 0x69f8aa), (0x69fea0, 0x69fef7), (0x5d0050, 0x5d0079), (0x67acb0, 0x67acc6), (0x67acc0, 0x67acd8),
           (0x45b340, 0x45b9a0), (0x47b1c0, 0x47b27c), (0x47b280, 0x47b2fe), (0x67d520, 0x67d7f0), (0x680430, 0x6804c5), (0x67f1a0, 0x67f222), (0x67f570, 0x67f615), (0x67dcd0, 0x67dcf0),
           (0x67ae00, 0x67aefe), (0x67d250, 0x67d340), (0x67c250, 0x67c288), (0x67d7f0, 0x67d82a), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1),
           (0x6918f0, 0x691a1a), (0x691ba0, 0x691cca), (0x67bb90, 0x67bd82)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x691de0: lock, 0x691ea0: lambda e: None, 0x67ad90: _free, 0x67a7d0: _cdecl_log,
                 0x67d3a0: _recorder(THROW, 2, com=True)})
OUT, BUFFER, NEXT, LEFT, VTABLE, AREA, A_UNDER, A_UFLOW, FACET, NAMES, FIRST, LAST, STATE, TM = (RAM + i for i in (0, 0x20, 0x80, 0x90, 0xa0, 0x1000, 0x200, 0x300, 0x400, 0x440, 0x800, 0x810, 0x820, 0x830))
OTHER = RAM + 0x600
OPS = [n for n in ('loctxt', 'loctxt', 'wloctxt', 'wloctxt', 'weekday', 'month', 'wweekday', 'wmonth') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
DAYS = b':Sun:Sunday:Mon:Monday:Tue:Tuesday:Wed:Wednesday:Thu:Thursday:Fri:Friday:Sat:Saturday'
MONTHS = b':Jan:January:Feb:February:Mar:March:Apr:April:May:May:Jun:June:Jul:July:Aug:August:Sep:September:Oct:October:Nov:November:Dec:December'
LISTS = [DAYS, MONTHS, b'', b':', b'::', b':a:ab:abc', b':abc:ab:a', b'xaxbx', b':S\xe9p:\xffx', b':' + b'a' * 0x90 + b':b', b'a', b':ab:ab']
WORDS = [b'', b'Sun', b'Sunday', b'Sund', b'Mon', b'Monday', b'Tuesday', b'Satur', b'Saturday!', b'Mar', b'March', b'May', b'Ma', b'Sep', b'September', b'x', b'a', b'ab', b'abc', b'abcd',
         b'S\xe9p', b'\xffx', b'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', b':', b'b']


def case(r):
    name = r.deal(OPS)
    wide = name.startswith('w')
    unit = 2 if wide else 1
    log = log_region()
    log_fold(log)
    regions = {LOG: log, HEAP: heap(0x1000), OUT: r.bytes(8)}
    regions[0x6fe5ac] = struct.pack('<I', 0xffffffff) + bytes(0x10)
    names = r.choice(LISTS if 'loctxt' in name else [DAYS, DAYS, DAYS] + LISTS if 'weekday' in name else [MONTHS, MONTHS, MONTHS] + LISTS)
    text = r.choice(WORDS) + r.choice([b'', b'', b' ', b'x', b'day'])
    if wide:
        names = b''.join(struct.pack('<H', c if r.random() < 0.97 else c | 0x100) for c in names)
        text = b''.join(struct.pack('<H', c if r.random() < 0.97 else c | 0x100) for c in text)
    regions[NAMES] = names + bytes(0x10)
    # The stream: its text waiting in the buffer, or nothing there and every character from underflow / uflow.
    buffered = r.random() < 0.8
    count = len(text) // unit
    if not buffered:
        text = text[:10 * unit]   # (two logged calls a character, and the answers stop moving when the log's 40 entries are full)
        count = len(text) // unit
    regions[AREA] = text + r.bytes(0x200 - len(text))
    regions[NEXT] = struct.pack('<I', AREA if buffered else r.choice([0, AREA]))
    regions[LEFT] = struct.pack('<I', count if buffered else 0)
    eof = 0xffffffff if not wide else r.choice([0xffffffff, 0xffff])
    units = list(struct.unpack('<%d%s' % (count, 'H' if wide else 'B'), text))
    regions[A_UNDER] = struct.pack('<40I', *((units if not buffered else [r.choice([eof, eof, 0x61])]) + [eof] * 40)[:40])
    regions[A_UFLOW] = struct.pack('<40I', *((units if not buffered else [r.choice([eof, 0x61])]) + [eof] * 40)[:40])
    answers(log, UNDERFLOW, A_UNDER)
    answers(log, UFLOW, A_UFLOW)
    regions[VTABLE] = struct.pack('<6I', 0, 0, 0, 0, recorder(UNDERFLOW, 0), recorder(UFLOW, 0))
    regions[BUFFER] = struct.pack('<I', VTABLE) + bytes(0x18) + struct.pack('<I', NEXT) + bytes(0xc) + struct.pack('<I', LEFT) + bytes(0x10)
    first = [BUFFER if r.random() < 0.95 else 0, r.getrandbits(32) & 0xffffff00 | r.choice([0, 0, 0, 0, 1])]
    last = [0 if r.random() < 0.93 else BUFFER, r.getrandbits(32) & 0xffffff00 | r.choice([0, 1])]
    if 'loctxt' in name:
        regions[FIRST] = struct.pack('<2I', *first)
        regions[LAST] = struct.pack('<2I', *last)
        return Call(0x6a4a30 if wide else 0x6ab3b0, [FIRST, LAST, r.choice([0, 0, 0, 1, 2]), NAMES], regions, conv='c', ret='a')
    other = r.choice(LISTS)   # (the list the member does not read)
    regions[OTHER] = (b''.join(struct.pack('<H', c) for c in other) if wide else other) + bytes(0x10)
    regions[FACET] = struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), *((NAMES, OTHER) if 'weekday' in name else (OTHER, NAMES)))
    regions[STATE] = struct.pack('<I', r.choice([0, 0, 0, 1, 2, 4, r.getrandbits(32)]))
    regions[TM] = r.bytes(0x24)
    address = {'weekday': 0x6aaa90, 'month': 0x6aaaf0, 'wweekday': 0x6a3de0, 'wmonth': 0x6a3e40}[name]
    return Call(address, [FACET, OUT] + first + last + [r.getrandbits(32), STATE, TM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
