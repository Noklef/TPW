// Creates functions at addresses listed in a text file (one hex address per line, '#' comments).
// Used for code Ghidra's auto-analysis never reached, e.g. virtual methods only referenced
// from vtables. Usage (headless): -postScript CreateFunctions.java <addresses.txt>
// @category Repair

import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

import java.nio.file.Files;
import java.nio.file.Paths;

public class CreateFunctions extends GhidraScript {
    @Override
    protected void run() throws Exception {
        int created = 0, existing = 0, failed = 0, padding = 0;
        for (String raw : Files.readAllLines(Paths.get(getScriptArgs()[0]))) {
            String line = raw.split("#")[0].trim();
            if (line.isEmpty()) continue;
            Address a = toAddr(line);
            if (getFunctionAt(a) != null) { existing++; continue; }
            int first = getByte(a) & 0xff;
            if (first == 0x90 || first == 0xcc) {   // (NOP / INT3: the compiler's alignment padding, never a function start)
                padding++;
                printerr("Padding at " + a + ": not a function start");
                continue;
            }
            if (getInstructionAt(a) == null) disassemble(a);
            CreateFunctionCmd cmd = new CreateFunctionCmd(a);
            if (cmd.applyTo(currentProgram, monitor)) created++;
            else { failed++; printerr("Failed at " + a + ": " + cmd.getStatusMsg()); }
        }
        println("CreateFunctions: created=" + created + " existing=" + existing + " padding=" + padding + " failed=" + failed);
    }
}
