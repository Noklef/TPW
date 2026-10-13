// Import re/types.h into the program and apply the types listed in re/type_uses.csv.
// Idempotent: re-running replaces the types with the current header.
//
// Usage (headless): -postScript ApplyTypes.java <repo>/re
//   type_uses.csv columns: address,target,type,notes
//     target = paramN (0-based), return, or data; type = a type name with optional '*'s
// @category TPW
import java.io.File;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.app.util.cparser.C.CParser;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.pcode.HighFunctionDBUtil;
import ghidra.program.model.symbol.SourceType;

public class ApplyTypes extends GhidraScript {

    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args.length > 0 ? args[0] : askDirectory("re/ folder", "OK").getPath());
        DataTypeManager dtm = currentProgram.getDataTypeManager();

        // CParser has no preprocessor: drop directives and the layout-check block.
        StringBuilder src = new StringBuilder();
        boolean skip = false;
        for (String line : Files.readAllLines(new File(dir, "types.h").toPath())) {
            String t = line.trim();
            if (t.startsWith("#ifndef __GHIDRA__")) { skip = true; continue; }
            if (skip) { if (t.startsWith("#endif")) skip = false; continue; }
            if (t.startsWith("#")) continue;
            src.append(line).append('\n');
        }
        CParser parser = new CParser(dtm, true, null);
        parser.parse(new java.io.ByteArrayInputStream(src.toString().getBytes("UTF-8")));
        println("types.h: " + parser.getComposites().size() + " structs, " + parser.getTypes().size() + " typedefs");

        DecompInterface decomp = null;
        int applied = 0;
        List<String> lines = Files.readAllLines(new File(dir, "type_uses.csv").toPath());
        for (String line : lines.subList(1, lines.size())) {
            if (line.isBlank()) continue;
            String[] c = line.split(",", 4);
            Address addr = toAddr(Long.decode(c[0].trim()));
            String target = c[1].trim();
            DataType dt = resolve(dtm, c[2].trim());
            if (dt == null) { printerr("unknown type " + c[2]); continue; }

            if (target.equals("data")) {
                DataUtilities.createData(currentProgram, addr, dt, -1,
                        DataUtilities.ClearDataMode.CLEAR_ALL_CONFLICT_DATA);
                applied++;
                continue;
            }
            Function f = getFunctionAt(addr);
            if (f == null) { printerr("no function at " + addr); continue; }
            if (target.equals("return")) {
                f.setReturnType(dt, SourceType.USER_DEFINED);
                applied++;
                continue;
            }
            int idx = Integer.parseInt(target.substring("param".length()));
            if (idx >= f.getParameterCount()) {
                // Parameters are still decompiler-inferred: commit them first.
                if (decomp == null) {
                    decomp = new DecompInterface();
                    decomp.openProgram(currentProgram);
                }
                DecompileResults r = decomp.decompileFunction(f, 60, monitor);
                if (r.getHighFunction() != null)
                    HighFunctionDBUtil.commitParamsToDatabase(r.getHighFunction(), true,
                            HighFunctionDBUtil.ReturnCommitOption.NO_COMMIT, SourceType.ANALYSIS);
            }
            if (idx >= f.getParameterCount()) { printerr(f.getName() + ": no param " + idx); continue; }
            Parameter p = f.getParameter(idx);
            p.setDataType(dt, SourceType.USER_DEFINED);
            if (p.getName().startsWith("param_") && dt instanceof Pointer
                    && ((Pointer) dt).getDataType().getName().equals("RsseContext"))
                p.setName("ctx", SourceType.USER_DEFINED);
            applied++;
        }
        if (decomp != null) decomp.dispose();
        println("applied " + applied + " type uses");
    }

    private DataType resolve(DataTypeManager dtm, String spec) {
        int stars = 0;
        while (spec.endsWith("*")) { stars++; spec = spec.substring(0, spec.length() - 1).trim(); }
        List<DataType> found = new ArrayList<>();
        dtm.findDataTypes(spec, found);
        DataType dt = found.isEmpty() ? null : found.get(0);
        if (dt == null) return null;
        for (int i = 0; i < stars; i++) dt = new PointerDataType(dt, dtm);
        return dt;
    }
}
