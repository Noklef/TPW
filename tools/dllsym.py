"""dllsym.py: names the source line of offsets in the hook DLL, from its PDB (Windows' dbghelp.dll through ctypes; nothing to install).
usage: dllsym.py OFFSET... [--dll PATH]   (hex offsets from the DLL's base: tpw.log's exception line gives EIP and the profile line the
base, e.g. 6CB967EF - 6CAB0000 = e67ef; --dll defaults to build/dinput.dll, its .pdb beside it)"""
import ctypes
import os
import sys
from ctypes import wintypes as W

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
BASE = 0x10000000   # (any base: the offsets are looked up relative to it)


class Symbol(ctypes.Structure):
    _fields_ = [('SizeOfStruct', W.ULONG), ('TypeIndex', W.ULONG), ('Reserved', ctypes.c_ulonglong * 2), ('Index', W.ULONG),
                ('Size', W.ULONG), ('ModBase', ctypes.c_ulonglong), ('Flags', W.ULONG), ('Value', ctypes.c_ulonglong),
                ('Address', ctypes.c_ulonglong), ('Register', W.ULONG), ('Scope', W.ULONG), ('Tag', W.ULONG), ('NameLen', W.ULONG),
                ('MaxNameLen', W.ULONG), ('Name', ctypes.c_char * 512)]


class Line(ctypes.Structure):
    _fields_ = [('SizeOfStruct', W.DWORD), ('Key', ctypes.c_void_p), ('LineNumber', W.DWORD), ('FileName', ctypes.c_char_p),
                ('Address', ctypes.c_ulonglong)]


def main(argv):
    dll = os.path.join(ROOT, 'build', 'dinput.dll')
    if '--dll' in argv:
        i = argv.index('--dll')
        dll = argv[i + 1]
        del argv[i:i + 2]
    dbg = ctypes.WinDLL('dbghelp.dll')
    process = W.HANDLE(0x1234)
    dbg.SymSetOptions(0x2 | 0x10)   # SYMOPT_UNDNAME, SYMOPT_LOAD_LINES
    if not dbg.SymInitializeW(process, None, False):
        sys.exit('SymInitialize failed')
    dbg.SymLoadModuleExW.restype = ctypes.c_ulonglong
    dbg.SymLoadModuleExW.argtypes = [W.HANDLE, W.HANDLE, W.LPCWSTR, W.LPCWSTR, ctypes.c_ulonglong, W.DWORD, ctypes.c_void_p, W.DWORD]
    if not dbg.SymLoadModuleExW(process, None, dll, None, BASE, 0x400000, None, 0):
        sys.exit('cannot load %s (error %d)' % (dll, ctypes.GetLastError()))
    for a in argv:
        address = ctypes.c_ulonglong(BASE + int(a, 16))
        sym = Symbol(SizeOfStruct=ctypes.sizeof(Symbol) - 511, MaxNameLen=511)
        disp = ctypes.c_ulonglong()
        name = '%s+0x%x' % (sym.Name.decode(), disp.value) if dbg.SymFromAddr(process, address, ctypes.byref(disp), ctypes.byref(sym)) else '?'
        line, ldisp = Line(SizeOfStruct=ctypes.sizeof(Line)), W.DWORD()
        where = '%s:%d' % (line.FileName.decode(), line.LineNumber) if dbg.SymGetLineFromAddr64(process, address, ctypes.byref(ldisp), ctypes.byref(line)) else ''
        print('%s  %s  %s' % (a, name, where))


if __name__ == '__main__':
    main(sys.argv[1:])
