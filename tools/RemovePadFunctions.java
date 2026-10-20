// Removes functions created at addresses that are not function starts (alignment padding a data reference happened to point at) and
// clears the instructions decoded there, up to the next instruction that belongs to another function or to the next non-padding byte.
// Args: hex addresses. Prints what it removed.
// Usage (headless, not -readOnly): -postScript RemovePadFunctions.java 41786f 444c49
// @category Repair
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;

public class RemovePadFunctions extends GhidraScript {
    @Override
    protected void run() throws Exception {
        for (String arg : getScriptArgs()) {
            Address a = toAddr(Long.parseLong(arg, 16));
            Function f = getFunctionAt(a);
            if (f != null) removeFunction(f);
            Address end = a;
            for (Instruction ins = getInstructionAt(a); ins != null && getFunctionContaining(ins.getAddress()) == null
                    && isPad(ins.getAddress()); ins = ins.getNext())
                end = ins.getMaxAddress();
            if (getInstructionAt(a) != null && end.compareTo(a) >= 0 && isPad(a)) clearListing(a, end);
            println("RemovePadFunctions " + a + ": " + (f != null ? "function removed" : "no function") + ", cleared to " + end);
        }
    }

    private boolean isPad(Address a) throws Exception {   // NOP or INT3: the compiler's alignment fill
        int b = getByte(a) & 0xff;
        return b == 0x90 || b == 0xcc;
    }
}
