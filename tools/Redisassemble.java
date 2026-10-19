// Clears code units over [start, end) and disassembles again from start, for spots where a later script left Ghidra decoding from the
// middle of an instruction (the bytes' real decoding starts at start). Args: pairs of hex addresses, start end start end ...
// Usage (headless): -postScript Redisassemble.java 622b72 622b7b 626545 626547
// @category Repair
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class Redisassemble extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] a = getScriptArgs();
        for (int i = 0; i + 1 < a.length; i += 2) {
            Address start = toAddr(Long.parseLong(a[i], 16));
            Address end = toAddr(Long.parseLong(a[i + 1], 16) - 1);
            clearListing(start, end);
            boolean ok = disassemble(start);
            println("Redisassemble " + start + ".." + end + ": " + (ok ? "ok" : "FAILED") + " -> " + getInstructionAt(start));
        }
    }
}
