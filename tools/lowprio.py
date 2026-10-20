"""Lowers this process to below-normal priority on Windows, so benches, gates and the processes they start (the native driver,
builds; a child inherits a below-normal class) give way to the user's programs. They still use idle cores fully. A bench's case
count in the gate is fitted to a wall-clock budget, so a machine kept fully busy for long can still starve one into fewer cases.
Env TPW_PRIORITY=normal leaves the priority alone."""
import os
import sys


def below_normal():
    if sys.platform != 'win32' or os.environ.get('TPW_PRIORITY') == 'normal':
        return
    import ctypes
    k = ctypes.windll.kernel32
    k.GetCurrentProcess.restype = ctypes.c_void_p
    k.SetPriorityClass.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    k.SetPriorityClass(k.GetCurrentProcess(), 0x4000)  # BELOW_NORMAL_PRIORITY_CLASS
