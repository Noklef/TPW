# The C++ library's time_put (src/game/core/timeput): do_put of char and wchar_t against the originals, over the strings' and _Strftime's ports: the stream
# buffer's overflow a thiscall recorder, mbrtowc (btowc's) a cdecl recorder, _getptd, _tzset and the locks recorders; the names and the struct tm made here.
# usage: timeput.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, blob, fill, heap, import_table, log_fold, log_region, recorder, set_return, strings  # noqa: E402

GETPTD, LOCK, UNLOCK, OVERFLOW, INC, DEC, TZSET, MBRTOWC = 2, 7, 8, 12, 14, 15, 29, 23


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x6ab160, 0x6ab36f), (0x6a4540, 0x6a4747), (0x693dc0, 0x693e15),
           (0x690800, 0x690941), (0x690950, 0x690eee), (0x690ef0, 0x690f1d), (0x690f20, 0x690fb3), (0x690fc0, 0x691025), (0x691030, 0x6912dc), (0x67df40, 0x67e010),
           (0x45b340, 0x45b9a0), (0x47b1c0, 0x47b27c), (0x47b280, 0x47b2fe), (0x6918f0, 0x691a1a), (0x691ba0, 0x691cca), (0x67bb90, 0x67bd82),
           (0x67acb0, 0x67acc6), (0x67acc0, 0x67acd8), (0x67ae00, 0x67aefe), (0x67d250, 0x67d340), (0x67c250, 0x67c288), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1),
           (0x687d40, 0x687d72), (0x688060, 0x688088), (0x688090, 0x6882f9), (0x688300, 0x68849c), (0x6884a0, 0x688608), (0x67dcd0, 0x67dcf0)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x687d80: _recorder(TZSET, 0, cdecl=True),
                 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x691de0: lock,0x691ea0: lambda e: None, 0x67ad90: _free, 0x67a7d0: _cdecl_log})
OUT, BUFFER, POINTER, COUNT, VTABLE, FACET, AREA, A_OVERFLOW, WHEN, PTD, NAMES, PCTYPE, TZ, STRINGS = (RAM, RAM + 0x20, RAM + 0x80, RAM + 0x90, RAM + 0xa0, RAM + 0xc0, RAM + 0x100,
                                                                                RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x400, RAM + 0x500, RAM + 0x700, RAM + 0xc00)
OPS = [n for n in ('put', 'wput') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
SPECS = b'aAbBcdHIjmMpSUwWxXyYzZ%q'
PICTURES = [b'd', b'dd', b'ddd', b'dddd', b'M', b'MMM', b'yy', b'yyyy', b'H', b'hh', b'mm', b'ss', b'tt', b"'x'", b' ', b'/', b':']


def case(r):
    name = r.deal(OPS)
    log = log_region()
    log_fold(log)
    regions = {LOG: log, HEAP: heap(0x1000), OUT: r.bytes(8),
               IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})}
    regions[0x6fe5ac] = struct.pack('<I', 0xffffffff) + bytes(0x10)
    regions[PCTYPE] = struct.pack('<256H', *[0x8000 if i >= 0x81 else 0x20 for i in range(256)])
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0xfb1440] = bytes(4)
    regions[0xfb3ca4] = struct.pack('<2I', 0, 0)
    regions[0xfb1510] = struct.pack('<I', r.choice([0, 0, 1]))
    regions[0xfb192c] = struct.pack('<2I', r.randrange(2), r.randrange(2))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    regions[0x781468] = struct.pack('<2I', TZ, TZ + 0x10)
    regions[TZ] = b'PST\0'.ljust(0x10, b'\0') + b'PDT\0'
    regions[PTD] = bytes(0x20)
    set_return(log, GETPTD, PTD)
    text = bytearray()
    pointers = []
    for i in range(43):   # (the facet's names: days, months, AM/PM, the date and time pictures)
        pointers.append(STRINGS + len(text))
        text += (b''.join(r.choice(PICTURES) for _ in range(r.randrange(4))) if i >= 40 else r.choice([b'', b'n', b'Sun', b'Sunday', b'name%d' % i])) + b'\0'
    regions[STRINGS] = bytes(text) + bytes(8)
    regions[NAMES] = struct.pack('<43I', *pointers)
    regions[0x781f8c] = struct.pack('<I', NAMES)
    regions[WHEN] = struct.pack('<9i', r.randrange(-2, 62), r.randrange(-2, 62), r.randrange(-2, 26), r.randrange(-1, 33), r.randrange(-1, 13), r.choice([-1, 0, 70, 99, 100, 199, 8099]),
                                r.randrange(-1, 8), r.randrange(-1, 367), r.choice([0, 1, -1]))
    # The stream: a put area with room for some characters (or none), then overflow.
    room = r.choice([0, 0, 1, 2, 5, 9, 12, 30, 0xffffffff])
    place = r.choice([AREA, AREA, AREA, 0])
    regions[POINTER] = struct.pack('<I', place)
    regions[COUNT] = struct.pack('<I', room if room < 0x100 else (room if place else 0))
    regions[AREA] = r.bytes(0x100)
    regions[VTABLE] = struct.pack('<2I', 0, recorder(OVERFLOW, 1))
    regions[BUFFER] = struct.pack('<I', VTABLE) + bytes(0x1c) + struct.pack('<I', POINTER) + bytes(0xc) + struct.pack('<I', COUNT)
    answers(log, OVERFLOW, A_OVERFLOW)
    regions[A_OVERFLOW] = struct.pack('<40I', *[r.choice([0, 0x41, 0x41, 0xffffffff, 0xffff, 0xffff0041]) for _ in range(40)])
    if name == 'wput':
        strings(log, MBRTOWC, 0, out=9)
        blob(log, MBRTOWC, 1, 1)
        fill(log, MBRTOWC, 0, r.bytes(2) + b'\0\0')
        set_return(log, MBRTOWC, r.choice([0, 1, 1, 1, 0xffffffff, 0xfffffffe]))
    regions[FACET] = struct.pack('<3I', r.getrandbits(32), r.getrandbits(32), r.choice([NAMES, NAMES, NAMES, 0]))
    format_ = r.choice(SPECS + b'\0\xe9')
    modifier = r.choice([0, 0, 0, ord('#'), ord('#'), ord('E'), r.randrange(256)])
    rand = r.getrandbits(32) & 0xffffff00
    return Call(0x6a4540 if name == 'wput' else 0x6ab160, [FACET, OUT, r.getrandbits(32), r.choice([BUFFER, BUFFER, BUFFER, 0]), r.getrandbits(32), WHEN, rand | format_,
                                                         r.getrandbits(32) & 0xffffff00 | modifier], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
