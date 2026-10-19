// Lists the places where the listing's control flow leads to an address with no instruction: an instruction's fall-through, or a
// jump / call target. Such a hole is a mis-decode (Ghidra decoding from inside an instruction, as 0x616774 IRETD inside SUB ECX,EDI at
// 0x616773) or code it never disassembled. Prints `FROM -> TO kind` per hole, then a count. Args: start end (hex), default all of .text.
// Usage (headless, -readOnly): -postScript FlowGaps.java 401000 6e6a7a
// @category Repair
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;

public class FlowGaps extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] a = getScriptArgs();
        Address start = toAddr(Long.parseLong(a.length > 0 ? a[0] : "401000", 16));
        Address end = toAddr(Long.parseLong(a.length > 1 ? a[1] : "6e6a7a", 16));
        int holes = 0;
        Instruction ins = getInstructionAt(start);
        if (ins == null) ins = getInstructionAfter(start);
        while (ins != null && ins.getAddress().compareTo(end) < 0) {
            Address ft = ins.getFallThrough();
            if (ft != null && ft.compareTo(end) < 0 && getInstructionAt(ft) == null) {
                println("GAP " + ins.getAddress() + " -> " + ft + " fallthrough");
                holes++;
            }
            for (Address t : ins.getFlows()) {
                if (t.compareTo(start) >= 0 && t.compareTo(end) < 0 && getInstructionAt(t) == null) {
                    println("GAP " + ins.getAddress() + " -> " + t + " " + ins.getFlowType());
                    holes++;
                }
            }
            ins = ins.getNext();
        }
        println("FlowGaps: " + holes + " holes");
    }
}
