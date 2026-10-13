// Exports a function index and the decompiled C for every function in the program.
// Usage (headless): -postScript ExportDecomp.java <output_dir>
// @category Export

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

import java.io.File;
import java.io.PrintWriter;

public class ExportDecomp extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File outDir = new File(args.length > 0 ? args[0] : "decomp_out");
        outDir.mkdirs();

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);

        int total = currentProgram.getFunctionManager().getFunctionCount();
        int done = 0, failed = 0, chunk = 0;
        PrintWriter index = new PrintWriter(new File(outDir, "functions.tsv"), "UTF-8");
        index.println("address\tname\tsize\tcallers\tcallees\tchunk");
        PrintWriter out = null;

        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        while (it.hasNext() && !monitor.isCancelled()) {
            Function f = it.next();
            // 500 functions per file keeps each chunk readable in an editor.
            if (done % 500 == 0) {
                if (out != null) out.close();
                chunk = done / 500;
                out = new PrintWriter(new File(outDir, String.format("decomp_%03d.c", chunk)), "UTF-8");
            }
            index.printf("%s\t%s\t%d\t%d\t%d\t%03d%n", f.getEntryPoint(), f.getName(),
                    f.getBody().getNumAddresses(), f.getCallingFunctions(monitor).size(),
                    f.getCalledFunctions(monitor).size(), chunk);

            out.printf("%n// ==== %s @ %s ====%n", f.getName(), f.getEntryPoint());
            DecompileResults res = decomp.decompileFunction(f, 60, monitor);
            if (res != null && res.decompileCompleted()) {
                out.println(res.getDecompiledFunction().getC());
            } else {
                failed++;
                out.println("// decompilation failed: " + (res == null ? "null" : res.getErrorMessage()));
            }
            done++;
            if (done % 1000 == 0) println("decompiled " + done + "/" + total);
        }
        if (out != null) out.close();
        index.close();
        decomp.dispose();
        println("DONE functions=" + done + " failed=" + failed + " out=" + outDir.getAbsolutePath());
    }
}
