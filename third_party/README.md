# Third-party libraries

The libraries the original game links, at the exact versions its exe embeds (the version strings in tp.exe match each tree's own).
They are kept unmodified as released, so the game's copies can be benched against a build of them before the game's addresses are
pointed at the library instead of hand-ported function by function.

| Tree | Version string in tp.exe | Source | MD5 of the archive |
|---|---|---|---|
| zlib-1.1.3 | `deflate 1.1.3` / `inflate 1.1.3` | https://zlib.net/fossils/zlib-1.1.3.tar.gz | ada18615d2a66dee4d6f5ff916ecd4c6 |
| jpeg-6b | `6b  27-Mar-1998` | https://www.ijg.org/files/jpegsrc.v6b.tar.gz | dbd5f3b47ed13132f04c685d608a7547 |
| jpeg-6a | `6a  7-Feb-96` | https://www.ijg.org/files/jpegsrc.v6a.tar.gz | 5360e0aa47b415daa44623196f7e6160 |

The exe holds both libjpeg releases (two copies with the same function names); linking both needs one built with its names prefixed.

CMakeLists.txt builds them as the static libraries tpw_zlib, tpw_jpeg6b and tpw_jpeg6a. Everything of ours is in config/:
- jconfig.h: jpeg-6b's jconfig.vc as released, for both jpeg trees (6a ships no MSVC configuration). Its `boolean` is an unsigned
  char, which gives the 0x1b0-byte jpeg_decompress_struct the exe's 6b copy checks for (with `int` it would be 0x1d0).
- jpeg6a_prefix.h: renames 6a's 100 external names to jpeg6a_*; force-included when 6a compiles. tools/jpegprefix.py writes it.
zlib 1.1.3 is kept as-is, known faults included (the 1.1.4 double-free fix is not applied): the game only inflates its own data, and
newer releases can deflate to different bytes.

Licences: zlib's in zlib-1.1.3/README (the zlib licence); the IJG's in each jpeg tree's README ("LEGAL ISSUES").
