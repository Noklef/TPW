// Print the disassembly of [start, end) given as hex script args.
// @category TPW
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;

public class DumpAsm extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        Address start = toAddr(Long.parseLong(a[0], 16));
        Address end = toAddr(Long.parseLong(a[1], 16));
        Instruction ins = getInstructionAt(start);
        if (ins == null) ins = getInstructionAfter(start);
        while (ins != null && ins.getAddress().compareTo(end) < 0) {
            println(ins.getAddress() + "  " + ins);
            ins = ins.getNext();
        }
    }
}
