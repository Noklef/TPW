// Functions whose entry point has no instruction: created over a data unit, or decoded from the middle of an instruction (0x692020 was
// decoded from 0x692022), which tools/FlowGaps.java misses when only data points at the function. Fix with tools/Redisassemble.java.
// Usage (headless, -readOnly): -postScript FunctionEntries.java
// @category Repair
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;

public class FunctionEntries extends GhidraScript {
    @Override
    public void run() throws Exception {
        int n = 0;
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
            if (f.isExternal() || f.isThunk() && getInstructionAt(f.getEntryPoint()) == null && f.getEntryPoint().getOffset() > 0x6e6a7aL) continue;
            if (getInstructionAt(f.getEntryPoint()) == null) {
                println("EF " + f.getEntryPoint() + " " + f.getName() + " data=" + getDataContaining(f.getEntryPoint()) + " next=" + getDataAfter(f.getEntryPoint()));
                n++;
            }
        }
        println("EF total " + n);
    }
}
