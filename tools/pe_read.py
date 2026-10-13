"""Tiny helpers for reading a PE image by virtual address (no dependencies).

    from pe_read import PE
    pe = PE(path); pe.u32(va); pe.cstr(va); pe.bytes(va, n)
"""
import struct


class PE:
    def __init__(self, path):
        self.d = open(path, 'rb').read()
        pe = struct.unpack_from('<I', self.d, 0x3c)[0]
        opt = pe + 24
        self.base = struct.unpack_from('<I', self.d, opt + 28)[0]
        nsec = struct.unpack_from('<H', self.d, pe + 6)[0]
        so = opt + struct.unpack_from('<H', self.d, pe + 20)[0]
        self.sections = []
        for k in range(nsec):
            n, vs, rva, rs, ro = struct.unpack_from('<8sIIII', self.d, so + k * 40)
            self.sections.append((n.rstrip(b'\0').decode(errors='replace'), self.base + rva, max(vs, rs), ro, rs))

    def section_of(self, va):
        for s in self.sections:
            if s[1] <= va < s[1] + s[2]:
                return s
        return None

    def off(self, va):
        s = self.section_of(va)
        if s is None or va - s[1] >= s[4]:
            return None  # unmapped or in zero-filled (uninitialised) tail
        return s[3] + va - s[1]

    def bytes(self, va, n):
        o = self.off(va)
        return None if o is None else self.d[o:o + n]

    def u32(self, va):
        b = self.bytes(va, 4)
        return None if b is None or len(b) < 4 else struct.unpack('<I', b)[0]

    def cstr(self, va, limit=256):
        b = self.bytes(va, limit)
        if b is None:
            return None
        end = b.find(b'\0')
        s = b[:end if end >= 0 else limit]
        return s.decode('latin1') if all(32 <= c < 127 for c in s) and s else None
