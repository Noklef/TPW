r"""Writes the instruction listing _re/text.asm (one `ADDRESS  INSTRUCTION` line per instruction in .text) from the Ghidra project
_re/ghidra with tools/DumpAsm.java, to _re/text.new for a check before it replaces _re/text.asm:

  python -I tools/dump_listing.py [GHIDRA_DIR]   (default: C:\Standalone Programs\_tools\Ghidra)

then compare: every line of the old listing must be in the new one (functions only added); a line that changed means the project
decodes differently there (fix it with tools/Redisassemble.java against the exe's bytes) before the new listing is used.
Functions Ghidra never reached (virtual methods only referenced from vtables) are created with tools/CreateFunctions.java first.
"""
import os
import subprocess
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
ghidra = sys.argv[1] if len(sys.argv) > 1 else r'C:\Standalone Programs\_tools\Ghidra'
cmd = [os.path.join(ghidra, 'support', 'analyzeHeadless.bat'), os.path.join(ROOT, '_re', 'ghidra'), 'TPW', '-process', 'tp.exe',
       '-noanalysis', '-readOnly', '-scriptPath', os.path.join(ROOT, 'tools'), '-postScript', 'DumpAsm.java', '401000', '6e6a7a']
out = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL).stdout
tag = 'DumpAsm.java> '
lines = [l.split(tag, 1)[1].rsplit(' (GhidraScript)', 1)[0] for l in out.splitlines() if tag in l]
open(os.path.join(ROOT, '_re', 'text.new'), 'w', newline='\n').write('\n'.join(lines) + '\n')
print('%d lines to _re/text.new' % len(lines))
