# Static safety audit of the no-CD exe

The no-CD `tp.exe` comes from a third party, so before running it outside a VM we checked it **without executing it**. Summary: **no sign of malicious modification**. The one remaining risk is described at the end.

## What the file is
| Check | Result |
|---|---|
| Layout vs the original `TP.icd` | Same 11 sections at the same addresses and sizes, same entry point (`0x67EE10`), no added sections or code caves |
| What differs where it can be compared | `.rdata`: rebuilt import names only. `.rsrc`: 2 padding bytes. PE header: timestamp, lower-case section names |
| `.text`/`.data` | Can't be diffed: they're SafeDisc-encrypted in `TP.icd` (entropy 7.99 vs 6.68 decrypted) |
| Entry point | Textbook MSVC 6 `WinMainCRTStartup` (GetVersion, heap init, command line, `WinMain` at `0x45A960`). No stub in front of it |

So the file is a **decrypted SafeDisc dump with a rebuilt import table**, not a wrapper or loader.

## What the code can do
| Check | Result |
|---|---|
| Imports | Only what a 1999 game needs: file I/O, threads, window/GDI, DirectX, WinMM, QMixer, Winsock, the EA Online `wea*.dll`s. **No** process creation, `WriteProcessMemory`/`CreateRemoteThread`, `ShellExecute`, `URLDownloadToFile`/WinINet, or registry *writes* (`RegCreateKeyExA` + reads only) |
| Dynamic loading | `LoadLibraryA` only for `user32.dll` (CRT message box) and `SFMAN32.DLL` (Creative SoundFont manager). `GetProcAddress` only for `MessageBoxA`, `GetActiveWindow`, `GetLastActivePopup`, `IsProcessorFeaturePresent`, `SFManager` |
| Hidden API resolution | No PEB walking (`fs:[0x30]`) anywhere. The 7 byte patterns resembling `int 2e`/`syscall` are operand bytes; none decompiles as an instruction |
| Network/URL strings | Only EA's own (`gatekeeper.ea.com`, `britain.bullfrog.ea.com`, `ftp.ea-europe.com`, complaint e-mail addresses), all for the long-dead online features |
| Microsoft Defender | No threats (signatures of 2026-10-13) |

## Residual risk
Without a decrypted original we can't prove `.text` is byte-for-byte EA's code. A hand-patched routine inside an existing function wouldn't show up in any check above. It would still be limited to the imported APIs, which can't download or launch anything. **Verdict: low risk.** Running it directly on Windows is reasonable. A VM or Windows Sandbox remains the cautious option for the first run.
