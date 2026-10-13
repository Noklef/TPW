// Applies re/symbols.csv (address,name,kind,confidence,notes) to the current program:
// renames functions/labels, creates missing functions, and stores notes as plate comments.
// Usage (headless): -process tp.exe -noanalysis -postScript ApplySymbols.java <path-to-symbols.csv>
// @category Import

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

public class ApplySymbols extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        String path = args.length > 0 ? args[0] : askFile("symbols.csv", "Apply").getAbsolutePath();

        int renamed = 0, created = 0, failed = 0;
        try (BufferedReader r = new BufferedReader(new FileReader(path))) {
            String line = r.readLine(); // header
            while ((line = r.readLine()) != null) {
                if (line.isBlank()) continue;
                List<String> f = parseCsvLine(line);
                Address addr = toAddr(f.get(0));
                String name = f.get(1);
                String kind = f.get(2);
                String notes = f.size() > 4 ? f.get(4) : "";
                try {
                    if (kind.equals("function")) {
                        Function fn = getFunctionAt(addr);
                        if (fn == null) {
                            fn = createFunction(addr, name);
                            created++;
                        }
                        fn.setName(name, SourceType.USER_DEFINED);
                    } else {
                        createLabel(addr, name, true, SourceType.USER_DEFINED);
                    }
                    if (!notes.isEmpty()) {
                        currentProgram.getListing().setComment(addr, CodeUnit.PLATE_COMMENT, notes);
                    }
                    renamed++;
                } catch (Exception e) {
                    failed++;
                    printerr("Failed " + f.get(0) + " " + name + ": " + e.getMessage());
                }
            }
        }
        println("ApplySymbols: applied=" + renamed + " created=" + created + " failed=" + failed);
    }

    // Minimal CSV parsing: commas, double-quoted fields, "" escapes.
    private static List<String> parseCsvLine(String line) {
        List<String> out = new ArrayList<>();
        StringBuilder cur = new StringBuilder();
        boolean quoted = false;
        for (int i = 0; i < line.length(); i++) {
            char c = line.charAt(i);
            if (quoted) {
                if (c == '"' && i + 1 < line.length() && line.charAt(i + 1) == '"') { cur.append('"'); i++; }
                else if (c == '"') quoted = false;
                else cur.append(c);
            } else if (c == '"') {
                quoted = true;
            } else if (c == ',') {
                out.add(cur.toString()); cur.setLength(0);
            } else {
                cur.append(c);
            }
        }
        out.add(cur.toString());
        return out;
    }
}
