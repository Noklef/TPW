// Registers a jump table Ghidra failed to recover, so the switch decompiles and the
// case bodies become part of the containing function.
// Usage (headless):
//   -postScript FixJumpTable.java <jmp_instr_addr> <table_addr> <count> [labels.csv <label_prefix>]
// labels.csv: CSV whose 2nd column names entry i (e.g. re/rsse_opcodes.csv -> RSSE_op_NOP ...).
// @category Repair

import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.pcode.JumpTable;
import ghidra.program.model.symbol.RefType;
import ghidra.program.model.symbol.SourceType;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class FixJumpTable extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] a = getScriptArgs();
        Address jmp = toAddr(a[0]);
        Address table = toAddr(a[1]);
        int count = Integer.decode(a[2]);
        List<String> labels = new ArrayList<>();
        String prefix = "";
        if (a.length >= 5) {
            List<String> lines = Files.readAllLines(Paths.get(a[3]));
            for (int i = 1; i < lines.size(); i++) labels.add(lines.get(i).split(",")[1]);
            prefix = a[4];
        }

        Function fn = getFunctionContaining(jmp);
        if (fn == null) throw new IllegalStateException("no function contains " + jmp);
        Instruction instr = getInstructionAt(jmp);
        if (instr == null) throw new IllegalStateException("no instruction at " + jmp);

        ArrayList<Address> targets = new ArrayList<>(); // JumpTable requires ArrayList
        for (int i = 0; i < count; i++) {
            Address t = toAddr(getInt(table.add(4L * i)) & 0xffffffffL);
            targets.add(t);
            if (getInstructionAt(t) == null) disassemble(t);
            instr.addOperandReference(0, t, RefType.COMPUTED_JUMP, SourceType.USER_DEFINED);
            if (i < labels.size()) createLabel(t, prefix + labels.get(i), true, SourceType.USER_DEFINED);
        }

        new JumpTable(jmp, targets, true, 0).writeOverride(fn); // same args as Ghidra's SwitchOverride.java
        CreateFunctionCmd.fixupFunctionBody(currentProgram, fn, monitor);
        println("FixJumpTable: " + count + " targets wired into " + fn.getName()
                + ", body now " + fn.getBody().getNumAddresses() + " bytes");
    }
}
