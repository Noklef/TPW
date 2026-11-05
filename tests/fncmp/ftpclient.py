# The FTP client (src/game/online/ftp: 0x5d1980 receive, 0x5d1c70 data setup, 0x5d2350 data connect, 0x5d1f10 login, 0x5d2180 quit, 0x5d2400 retrieve, 0x5d22a0 names, 0x5d1ac0 connect, 0x5d2850 thread, 0x5d1760 constructor, 0x5d1880 and 0x5d1860 destructors, 0x5d29d0, 0x5d2a20, 0x5d2a60 Winsock's error box) against the original:
# the server's whole conversation waits in the reply buffer, the data channel is recv's answers; Winsock is recorders in the import table, the C files, the thread and the events are recorders by the env.
# A ready flag of 0 is not run: the Sleep loop waits on the other thread, without end in one.
# usage: ftpclient.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Receive,Setup,Data,Login,Quit,Retrieve,Names,Connect,Thread,Ctor,Dtor,Deleting,Box,Start,Clean)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob, fill, fill_this, heap, log_fold, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

CALLBACK, HTONS, GETHOST, INETADDR, SOCKET, RECV, CLOSE, SOCKNAME, HOSTNAME, SEND, SHUTDOWN, BIND, LISTEN, ACCEPT, NTOHS, WSASTART, WSACLEAN, WSAERR, MSGBOX, SLEEP = range(20)
FOPEN, FWRITE, FCLOSE, THREAD, EVENTSET, EVENTWAIT, EVENTCTOR, SEMDTOR, BASECTOR, CONNECT = range(20, 30)
B = Bench([(0x5c8db0, 0x5c8dd5), (0x5c9070, 0x5c908e), (0x5c8de0, 0x5c9001), (0x5c9090, 0x5c9102), (0x5d3750, 0x5d3823), (0x5d1760, 0x5d2714), (0x5d2720, 0x5d2842), (0x5d2850, 0x5d296c), (0x5d29d0, 0x5d2a7e),
           (0x657cc0, 0x657d2c), (0x45b340, 0x45b9a0), (0x67a400, 0x6902c0)],
          hooks={0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67b6d0: _recorder(FWRITE, 4, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x5c8b70: _recorder(THREAD, 2, cdecl=True),
                 0x5c8c20: _recorder(EVENTSET, 0), 0x5c8bf0: _recorder(EVENTWAIT, 1), 0x5c8bd0: _recorder(EVENTCTOR, 0), 0x6307e0: _recorder(SEMDTOR, 0), 0x5c8d80: _recorder(BASECTOR, 0)})
CB, OBJECT, TABLE, VT, HOST, BOUND, ARGS, CODES, TEXTS = RAM, RAM + 0x100, RAM + 0x3000, RAM + 0x3200, RAM + 0x3400, RAM + 0x3500, RAM + 0x3600, RAM + 0x3800, RAM + 0x3900
SLOTS = {0x6fd404: (CONNECT, 3), 0x6fd408: (HTONS, 1), 0x6fd40c: (GETHOST, 1), 0x6fd410: (INETADDR, 1), 0x6fd414: (SOCKET, 3), 0x6fd418: (RECV, 4), 0x6fd41c: (CLOSE, 1), 0x6fd420: (SOCKNAME, 3),
         0x6fd3f8: (HOSTNAME, 2), 0x6fd428: (SEND, 4), 0x6fd42c: (WSASTART, 2), 0x6fd430: (WSACLEAN, 0), 0x6fd434: (ACCEPT, 3), 0x6fd438: (NTOHS, 1), 0x6fd43c: (LISTEN, 2), 0x6fd440: (BIND, 3),
         0x6fd444: (SHUTDOWN, 2), 0x6fd424: (WSAERR, 0), 0x6fd37c: (MSGBOX, 4), 0x6fd1d4: (SLEEP, 1)}
OTHER = [200, 220, 221, 226, 227, 230, 250, 125, 150, 331, 332, 500, 99]
LISTING = [b'-rw-r--r--  1 tpw tpw %d Jan 1 park\r\n' % n for n in (0, 16, 48, 96)] + [b'short\n', b'no end of line here, no end of line here!']


def code(r, *good):
    value = r.choice(good) if r.random() < 0.85 else r.choice(OTHER)
    return b'%d x\r\n' % value


def pasv(r):
    if r.random() < 0.04:
        return r.choice([b'227 no bracket\r\n', b'227 (1,2\r\n'])
    numbers = [r.choice([0, 1, 127, 255, 256, r.randrange(0x10000), -1]) for _ in range(6)]
    return b'227 Entering (%d,%d,%d,%d,%d,%d)\r\n' % tuple(numbers)


def setup(r):
    return [pasv(r)] if r.random() < 0.6 else [code(r, 150, 200, 500), code(r, 200, 200, 200, 500)]


def conversation(r, regions, lines):
    obj = regions[OBJECT]
    text = b''.join(lines)[:0x3f0]
    obj[0x20:0x20 + len(text)] = text
    obj[0x20 + len(text)] = 0
    put(obj, 0x420, 'i', len(text))


def world(r):
    log = log_region()
    log_fold(log)
    table = bytearray(0x600)
    for at, (k, n) in SLOTS.items():
        put(table, at - IMPORT_TABLE, 'I', recorder(k, n, com=True))
    obj = bytearray(r.bytes(0x2780))
    put(obj, 4, 'I', r.choice([4, 4, 4, 4, 4, 0xffffffff]))
    put(obj, 0x420, 'i', 0)
    put(obj, 0x2424, 'I', r.choice([0xffffffff, 0xffffffff, 5]))
    put(obj, 0x2428, 'I', r.choice([0xffffffff, 0xffffffff, 6]))
    obj[0x243c:0x243c + 8] = r.choice([b'1.2.3.4\0', b'ftp.bf\0\0', b'255.255.255.255\0'[:8]])
    obj[0x249c:0x249c + 6] = b'guest\0'
    obj[0x24fc:0x24fc + 6] = r.choice([b'pw%d\0\0' % r.randrange(9), b'\0\0\0\0\0\0'])
    obj[0x255c:0x255c + 6] = b'l.dat\0'
    obj[0x265c:0x265c + 6] = b'r.dat\0'
    put(obj, 0x275c, 'I', CB)
    # The std::string of the local address: none (often), a pointer with a length of none, or a null one with a length.
    bound = r.choice(['none', 'none', 'string', 'null'])
    put(obj, 0x2764, 'I', 0 if bound in ('none', 'null') else BOUND + 1)
    put(obj, 0x2768, 'I', 0 if bound == 'none' else 7)
    put(obj, 0x276c, 'I', 0 if bound == 'none' else 0x1f)
    put(obj, 0x2778, 'I', 9)
    obj[0x277c] = r.choice([1, 2])
    obj[0x277d] = r.choice([0, 1])
    obj[0x277e] = 0
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x1000), OBJECT: obj, VT: vtable({0: 2}), BOUND: b'\x01' + b'9.8.7.6\0' + bytes(0x18)}
    cb = bytearray(8)
    put(cb, 0, 'I', VT)
    regions[CB] = cb
    regions[0x773ce0] = b'255.255.255.255\0'
    regions[0x6fe5b0] = r.choice([b'\0', b'5.5.5.5\0'])
    # The data channel: recv writes its records, as answered; 0 or an error at the end.
    fill(log, RECV, 1, b''.join(r.choice(LISTING)[:0x38].ljust(0x38, b'\0') for _ in range(2)), records=2)
    table = bytearray(0x100)
    got = [r.choice([1, 8, 0x10, 0x30, 0x38]) for _ in range(r.choice([0, 1, 3, 5]))] + [r.choice([0, 0xffffffff])]
    struct.pack_into('<%dI' % len(got), table, 0, *got)
    regions[TABLE] = table
    answers(log, RECV, TABLE)
    # Winsock's answers.
    set_return(log, SOCKET, r.choice([7, 7, 7, 7, 0xffffffff]))
    set_return(log, CONNECT, r.choice([0, 0, 0, 0, 0xffffffff]))
    set_return(log, BIND, r.choice([0, 0, 0, 0, 0xffffffff]))
    set_return(log, LISTEN, r.choice([0, 0, 0, 0, 0xffffffff]))
    set_return(log, ACCEPT, r.choice([8, 8, 8, 0xffffffff]))
    set_return(log, HTONS, r.getrandbits(32))
    set_return(log, NTOHS, r.getrandbits(32))
    set_return(log, INETADDR, r.choice([0xffffffff, 0x04030201, r.getrandbits(32)]))
    set_return(log, GETHOST, r.choice([0, HOST]))
    set_return(log, HOSTNAME, r.choice([0, 1]))
    host = bytearray(0x40)
    put(host, 0xc, 'I', HOST + 0x20)
    put(host, 0x20, 'I', HOST + 0x28)
    put(host, 0x28, 'I', r.choice([0, r.getrandbits(32)]))
    regions[HOST] = host
    fill(log, SOCKNAME, 1, r.bytes(0x10), which=1)
    set_return(log, FOPEN, r.choice([0, 0x40000, 0x40000, 0x40000]))
    set_return(log, THREAD, r.choice([1, 0xffffffff]))
    # What goes by name or on the stack is logged by its contents.
    for k, mask in ((SEND, 2), (INETADDR, 1), (GETHOST, 1), (FOPEN, 3)):
        strings(log, k, mask)
    strings(log, HOSTNAME, 0, out=1)
    fill(log, HOSTNAME, 0, b'tpw\0', which=3)
    strings(log, ACCEPT, 0, out=2)
    blob(log, ACCEPT, 2, 1)
    strings(log, SOCKNAME, 0, out=6)
    blob(log, SOCKNAME, 2, 1)
    blob(log, CONNECT, 1, 3)
    blob(log, BIND, 1, 0xf)
    strings(log, WSASTART, 0, out=2)
    for k in (HTONS, NTOHS):   # (their pushed register's high word is what came before)
        put(log, LOG_BYTES + 4 * k, 'I', 1 << 16)
    return log, regions


def on_stack_recv(log):
    strings(log, RECV, 0, out=2)


def receive_case(r):
    log, regions = world(r)
    return Call(0x5d1980, [OBJECT, r.getrandbits(32) & 0xffffff00 | r.choice([0, 1, 1, 2]), r.choice([0xffffffff, 0, 0x10, 0x30, 0x60, 0x38])], regions, conv='t', ret='a')


def setup_case(r):
    log, regions = world(r)
    conversation(r, regions, setup(r))
    return Call(0x5d1c70, [OBJECT], regions, conv='t', ret='a')


def data_case(r):
    log, regions = world(r)
    return Call(0x5d2350, [OBJECT], regions, conv='t', ret='a')


def login_lines(r):
    return [code(r, 220), code(r, 230, 331, 332), code(r, 230), code(r, 200)]


def login_case(r):
    log, regions = world(r)
    on_stack_recv(log)
    conversation(r, regions, login_lines(r))
    return Call(0x5d1f10, [OBJECT], regions, conv='t', ret='a')


def threaded(regions):
    regions[OBJECT][0x277e] = 1


def quit_case(r):
    log, regions = world(r)
    on_stack_recv(log)
    conversation(r, regions, [code(r, 221)])
    if r.random() < 0.2:
        threaded(regions)
    return Call(0x5d2180, [OBJECT], regions, conv='t', ret='a')


def retrieve_lines(r):
    second = code(r, 226, 250)   # (226 sets the data channel up again, 250 goes on to the file)
    return setup(r) + [code(r, 125, 150), second] + (setup(r) if second.startswith(b'226') else []) + [code(r, 150, 125), code(r, 226, 250)]


def retrieve_case(r):
    log, regions = world(r)
    conversation(r, regions, retrieve_lines(r))
    return Call(0x5d2400, [OBJECT], regions, conv='t', ret='a')


def names_case(r):
    log, regions = world(r)
    conversation(r, regions, retrieve_lines(r))
    args = bytearray(0x200)
    args[0:7] = b'local1\0'
    args[0x100:0x107] = b'remote\0'
    regions[ARGS] = args
    if r.random() < 0.2:
        threaded(regions)
    return Call(0x5d22a0, [OBJECT, ARGS, ARGS + 0x100], regions, conv='t', ret='a')


def connect_case(r):
    log, regions = world(r)
    on_stack_recv(log)
    conversation(r, regions, login_lines(r))
    args = bytearray(0x200)
    for at, text in ((0, r.choice([b'1.2.3.4', b'ftp.bf'])), (0x40, r.choice([b'guest', b'gu\rest', b'gu\nest', b'\r', b''])), (0x80, r.choice([b'pw', b'p\nw', b'p\rw', b'\n', b''])),
                     (0xc0, r.choice([b'', b'6.6.6.6', b'0123456789012345678901234567890123456789']))):
        args[at:at + len(text)] = text
    regions[ARGS] = args
    # The string before: none, or sharing its buffer at a count, with room or without.
    obj = regions[OBJECT]
    bound = regions[BOUND]
    bound = bytearray(bound)
    bound[0] = r.choice([0, 0, 1, 2, 0xff])
    regions[BOUND] = bound
    if r.random() < 0.7:
        put(obj, 0x2764, 'I', BOUND + 1)
        put(obj, 0x2768, 'I', 7)
        put(obj, 0x276c, 'I', r.choice([0x10, 0x1f, 0x20, 7, 6]))
    if r.random() < 0.3:
        threaded(regions)
    return Call(0x5d1ac0, [OBJECT, ARGS, ARGS + 0x40, ARGS + 0x80, 0 if r.random() < 0.2 else ARGS + 0xc0], regions, conv='t', ret='a')


def thread_case(r):
    log, regions = world(r)
    on_stack_recv(log)
    command = r.choice([0, 1, 2, 3, 4, 0xffffffff])
    conversation(r, regions, {1: login_lines, 2: lambda r: [code(r, 221)], 3: retrieve_lines}.get(command, lambda r: [])(r))
    # The one command, then the callback sets 0 (the thread's end).
    put(regions[OBJECT], 0x2778, 'I', command)
    fill_this(log, CALLBACK, bytes(4), offset=OBJECT + 0x2778 - CB, which=2)
    return Call(0x5d2850, [OBJECT], regions, conv='c', ret='v')


def ctor_case(r):
    log, regions = world(r)
    set_return(log, WSASTART, r.choice([0, 1]))
    return Call(0x5d1760, [OBJECT, CB, r.getrandbits(32)], regions, conv='t', ret='a', ignore=[(OBJECT + 0x2760, 1)])


def dtor_setup(r):
    log, regions = world(r)
    on_stack_recv(log)
    # The string's buffer: its count 0 or 0xff frees it, else one less.
    bound = bytearray(regions[BOUND])
    bound[0] = r.choice([0, 1, 2, 0xff])
    regions[BOUND] = bound
    if r.random() < 0.8:
        put(regions[OBJECT], 0x2764, 'I', BOUND + 1)
    return log, regions


def dtor_case(r):
    log, regions = dtor_setup(r)
    return Call(0x5d1880, [OBJECT], regions, conv='t', ret='v')


def deleting_case(r):
    log, regions = dtor_setup(r)
    return Call(0x5d1860, [OBJECT, r.getrandbits(32)], regions, conv='t', ret='a')


def base_dtors(r):
    log, regions = dtor_setup(r)
    address, args = r.choice([(0x5c9070, [r.getrandbits(32)]), (0x5c8db0, [r.getrandbits(32)]), (0x5c8dd0, [])])
    return Call(address, [OBJECT] + args, regions, conv='t', ret='a' if args else 'v')


def box_world(r):
    log, regions = world(r)
    set_return(log, WSAERR, r.choice([10061, 11001, 5, 0, 0xffff8000, 0x12345]))
    codes = bytearray(0x40)
    struct.pack_into('<5h', codes, 0, *r.sample([10061, 11001, 5, 10060, -0x8000, 7], 4), 0)
    regions[CODES] = codes
    texts = bytearray(0x20)
    struct.pack_into('<6I', texts, 0, *[r.getrandbits(32) for _ in range(6)])
    regions[TEXTS] = texts
    regions[0xf827ec] = struct.pack('<2I', CODES, TEXTS)
    return log, regions


def box_case(r):
    log, regions = box_world(r)
    return Call(0x5d29d0, [OBJECT], regions, conv='t', ret='v')


def start_case(r):
    log, regions = box_world(r)
    set_return(log, WSASTART, r.choice([0, 0, 1, 0xffffffff]))
    return Call(0x5d2a20, [OBJECT], regions, conv='t', ret='a')


def clean_case(r):
    log, regions = box_world(r)
    set_return(log, WSACLEAN, r.choice([0, 0, 1, 0xffffffff]))
    return Call(0x5d2a60, [OBJECT], regions, conv='t', ret='a')


CASES = {'Receive': receive_case, 'Setup': setup_case, 'Data': data_case, 'Login': login_case, 'Quit': quit_case, 'Retrieve': retrieve_case, 'Names': names_case, 'Connect': connect_case,
         'Thread': thread_case, 'Ctor': ctor_case, 'Dtor': dtor_case, 'Deleting': deleting_case, 'BaseDtors': base_dtors, 'Box': box_case, 'Start': start_case, 'Clean': clean_case}
OPS = os.environ.get('OPS', '').split(',') if os.environ.get('OPS') else list(CASES)


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
