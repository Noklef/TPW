# The sound system's reverb presets load (src/game/core/sndreverb, 0x6c5d20) against the original: SndReverb.map opened in the system's folder
# through the file library (fileopen.py's ranges, import recorders and answers, biased to a folder and file that open) and read through ReadFile
# as a sequenced read (seq_read: each read hands out the next bytes of the case's file). The file: the version (2000 mostly), a count of 0-5 and
# each record's byte (0 now and then: an empty slot) and 13 dwords; now and then no bytes or only a wrong version. Records already loaded (+0x68
# set) now and then. (The file is never cut inside a record: a short read there leaves the original's uninitialised locals in the record.)
# usage: sndreverb.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import fileopen  # noqa: E402  (the file library's ranges, imports and globals)
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, echo, heap, import_table, log_region, seq_read, set_return, vtable  # noqa: E402

B = Bench([(0x6c5d20, 0x6c5ff9)] + fileopen.RANGES)
SYSTEM, FOLDERTEXT, VT_OBJ, FILE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000
STRING_VTABLE, VERSION = 0x703090, 2000
FOLDERS = [b'C:\\tp\\data\\', b'data', b'data\\snd\\', b'', b'D:\\x']
ANSWERS = {'CreateFileA': [0xffffffff, 0x77, 0x77, 0x77], 'GetFileAttributesA': [0xffffffff, 0x10, 0x10, 0x20, 0x80], 'ReadFile': [1, 1, 1, 0],
           'IsDBCSLeadByte': [0, 0, 1], 'mmioOpenA': [0, 0x66, 0x66], 'FindFirstFileA': [0xffffffff, 0x99], 'InterlockedDecrement': [1, 1, 2],
           'InterlockedIncrement': [1, 2], 'CreateSemaphoreA': [0, 0x44, 0x44], 'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0],
           'GetFileSize': [0, 0x58, 0x1000]}


def reverb_file(r):
    """SndReverb.map's bytes: the version, the count, then each record's byte and (when it is not 0) its 13 dwords; or a cut header."""
    pick = r.random()
    if pick < 0.05:
        return b''
    if pick < 0.12:
        return struct.pack('<I', r.choice([VERSION + 1, 0, r.getrandbits(32) | 0x10000]))
    count = r.choice([0, 1, 2, 3, 5])
    out = bytearray(struct.pack('<2I', VERSION, count))
    for _ in range(count):
        present = r.choice([0, 1, 1, 0x80])
        out.append(present)
        if present:
            out += r.bytes(0x34)
    return bytes(out + r.bytes(r.choice([0, 0, 3])))


def case(r):
    log = log_region()
    for imp, k in fileopen.K.items():
        if r.random() < 0.9:
            set_return(log, k, r.choice(ANSWERS.get(imp, [0, 1, r.getrandbits(32)])))
    if r.random() < 0.8:
        echo(log, fileopen.K['SetFilePointer'], 1)
    data = reverb_file(r)
    seq_read(log, fileopen.K['ReadFile'], FILE, len(data), buffer=1, count=2, done=3, returns=False)
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    folder = r.choice(FOLDERS)
    system = bytearray(r.bytes(0x80))
    struct.pack_into('<5I', system, 0x30, STRING_VTABLE, len(folder), FOLDERTEXT, (len(folder) & ~7) + 8, 8)
    struct.pack_into('<I', system, 0x68, 0 if r.random() < 0.9 else r.getrandbits(32) | 1)
    if struct.unpack_from('<I', system, 0x6c)[0] == VERSION:
        struct.pack_into('<I', system, 0x6c, 0)
    regions = {LOG: log, SYSTEM: system, FOLDERTEXT: folder + b'\0' + r.bytes(8), FILE: data + bytes(4), IMPORT_TABLE: import_table(log, fileopen.IMPORTS),
               fileopen.GLOBALS: globals_, fileopen.PATHS: r.bytes(0x210), fileopen.LOCALE: bytes(4), VT_OBJ: vtable(fileopen.OBJECT_SLOTS),
               HEAP: heap(0x10000)}
    return Call(0x6c5d20, [SYSTEM], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
