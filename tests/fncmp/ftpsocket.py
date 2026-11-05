# The FTP client's socket layer (src/game/online/ftp: 0x5c90a0, 0x5c8de0, 0x5c8e20, 0x5c8f50, 0x5d37c0, 0x5d3750, 0x5d36d0, 0x5c9090) against the original: Winsock is recorders in the import table, the original reaching them through its thunks at 0x657cc0, recv answering in turn and writing a record to the buffer.
# usage: ftpsocket.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, answers, blob, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402

CONNECT, HTONS, GETHOST, INETADDR, SOCKET, RECV, CLOSE, SOCKNAME, HOSTNAME, SEND, SHUTDOWN = range(11)
B = Bench([(0x5c8de0, 0x5c9001), (0x5c9090, 0x5c9102), (0x5d36d0, 0x5d3823), (0x657cc0, 0x657d2c), (0x67a400, 0x6902c0)])
OBJECT, OUT, TABLE, TEXT, HOST = RAM, RAM + 0x800, RAM + 0xa00, RAM + 0x1000, RAM + 0x800
SLOTS = {0x6fd404: (CONNECT, 3), 0x6fd408: (HTONS, 1), 0x6fd40c: (GETHOST, 1), 0x6fd410: (INETADDR, 1), 0x6fd414: (SOCKET, 3), 0x6fd418: (RECV, 4), 0x6fd41c: (CLOSE, 1),
         0x6fd420: (SOCKNAME, 3), 0x6fd3f8: (HOSTNAME, 2), 0x6fd428: (SEND, 4), 0x6fd444: (SHUTDOWN, 2)}
LINES = [b'220 Hello\r\n', b'220-more\r\n', b'99 no\r\n', b'600 no\r\n', b'599 z\r\n', b'100\r\n', b'xyz\r\n', b'\r\n', b'150-\r\n', b'226', b'\r', b'\n', b'  331 sp\r\n', b'+250 p\r\n', b'-250\r\n', b'250 ok']


def world(r):
    log = log_region()
    table = bytearray(0x600)
    for at, (k, n) in SLOTS.items():
        put(table, at - IMPORT_TABLE, 'I', recorder(k, n, com=True))
    return log, {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x100)}


def replies(r, n):
    return b''.join(r.choice(LINES) for _ in range(n))


def socket_object(r, regions, size=0x40):
    head = bytearray(r.bytes(size))
    put(head, 0, 'I', r.getrandbits(32))
    put(head, 4, 'I', r.choice([0xffffffff, 3, 0x1234, r.getrandbits(32)]))
    regions[OBJECT] = head


def send_case(r):
    log, regions = world(r)
    socket_object(r, regions)
    length = r.choice([-1, 0, 1, 0x10, 0x1fff, 0x2000, 0x2001, 0x4001, r.randrange(0x4800)])
    regions[TEXT] = bytes(r.randrange(1, 256) for _ in range(0x10)) * 0x480 + b'\0'
    strings(log, SEND, 2)
    table = bytearray(0x100)
    sent = [r.choice([1, 0x10, 0x1fff, 0x2000, 0x2000, 0x2000, 0]) for _ in range(6)] + [r.choice([0xffffffff, 0xfffffff0])]
    if r.random() < 0.3:
        sent[r.randrange(6)] = 0xffffffff
    struct.pack_into('<7I', table, 0, *sent)
    regions[TABLE] = table
    answers(log, SEND, TABLE)
    return Call(0x5c90a0, [OBJECT, TEXT if r.random() < 0.9 else 0, length], regions, conv='t', ret='a')


def send_line_case(r):
    log, regions = world(r)
    socket_object(r, regions)
    text = bytes(r.randrange(1, 256) for _ in range(r.choice([0, 1, 5, 0x40, 0x2004]))) + b'\0'
    regions[TEXT] = text
    strings(log, SEND, 2)
    set_return(log, SEND, r.choice([0xffffffff, 0x2000, 0x2000, 0x2000]))
    return Call(0x5c8de0, [OBJECT, TEXT], regions, conv='t', ret='a')


def read_reply_case(r):
    log, regions = world(r)
    socket_object(r, regions)
    obj = bytearray(0x600)
    obj[:0x40] = regions[OBJECT]
    text = replies(r, r.choice([0, 0, 1, 2, 3, 6]))
    obj[0x20:0x20 + len(text)] = text
    put(obj, 0x420, 'i', r.choice([len(text)] * 8 + [0, -1, -4, max(0, len(text) - 1), 0x3ff, 0x3f0]))
    regions[OBJECT] = obj
    data = replies(r, 8)[:0x70].ljust(0x70, b'\r')
    fill(log, RECV, 1, data, records=7)
    table = bytearray(0x100)
    struct.pack_into('<8I', table, 0, *[r.choice([1, 3, 8, 0x10, 0x10, 0x10]) for _ in range(7)], r.choice([0, 0xffffffff]))
    if r.random() < 0.2:
        put(table, 4 * r.randrange(4), 'I', r.choice([0, 0xffffffff]))
    regions[TABLE] = table
    answers(log, RECV, TABLE)
    regions[OUT] = r.bytes(0x100)
    wanted = r.getrandbits(32) & 0xffffff00 | r.choice([0, 1, 0x80])
    return Call(0x5c8e20, [OBJECT, OUT if r.random() < 0.8 else 0, r.choice([0, 1, 4, 0x10, 0x80]), wanted], regions, conv='t', ret='a')


def local_address_case(r):
    log, regions = world(r)
    strings(log, HOSTNAME, 0, out=1)
    fill(log, HOSTNAME, 0, r.choice([b'tpw\0', b'a.b.c\0\0\0', b'\0\0\0\0']))
    set_return(log, HOSTNAME, r.choice([0, 0, 0, 1, 0xffffffff]))
    strings(log, GETHOST, 1)
    set_return(log, GETHOST, r.choice([0, HOST, HOST, HOST]))
    host = bytearray(0x40)
    put(host, 0xc, 'I', HOST + 0x20)
    put(host, 0x20, 'I', HOST + 0x28)
    put(host, 0x28, 'I', r.choice([0, 0, 0x0100007f, r.getrandbits(32)]))
    regions[HOST] = host
    set_return(log, SOCKET, r.choice([0xffffffff, 7, 7, 7, r.getrandbits(32)]))
    set_return(log, HTONS, r.getrandbits(32))
    strings(log, INETADDR, 1)
    set_return(log, INETADDR, r.getrandbits(32))
    strings(log, CONNECT, 0, out=2)
    blob(log, CONNECT, 1, 3)
    set_return(log, CONNECT, r.choice([0, 0, 0, 0xffffffff, 1]))
    strings(log, SOCKNAME, 0, out=2)
    blob(log, SOCKNAME, 2, 1)
    fill(log, SOCKNAME, 1, r.bytes(0x10))
    return Call(0x5c8f50, [], regions, conv='c', ret='a')


def close_case(r, address, args):
    log, regions = world(r)
    socket_object(r, regions)
    strings(log, RECV, 0, out=2)
    fill(log, RECV, 1, r.bytes(0x10))
    table = bytearray(0x100)
    struct.pack_into('<4I', table, 0, r.choice([0, 0xffffffff, 1, 0x100]), r.choice([0, 0xffffffff, 1, 0x100]), r.choice([0, 0x100]), 0)
    regions[TABLE] = table
    answers(log, RECV, TABLE)
    return Call(address, [OBJECT] + args, regions, conv='t', ret='a' if args else 'v')


def case(r):
    op = r.deal(['send', 'line', 'reply', 'reply', 'reply', 'local', 'close', 'dtor', 'deleting', 'base'])
    if op == 'send':
        return send_case(r)
    if op == 'line':
        return send_line_case(r)
    if op == 'reply':
        return read_reply_case(r)
    if op == 'local':
        return local_address_case(r)
    if op == 'close':
        return close_case(r, 0x5d37c0, [])
    if op == 'dtor':
        return close_case(r, 0x5d3750, [])
    if op == 'base':
        return close_case(r, 0x5c9090, [])
    return close_case(r, 0x5d36d0, [r.getrandbits(32) & 0xfffffffe | r.getrandbits(1)])


if __name__ == '__main__':
    B.main(case)
